
#include "tunnel_server.h"
#include "client.h"
#include "server_common.h"
#include "server_ip.h"
#include "server_connect.h"
#include "server_transout.h"

#if !defined(IFNAMSIZ)
#define IFNAMSIZ 256
#endif

void server_init()
{
    server_ip_init();
}

int socket_setnonblock(int fd)
{
	long flags = fcntl(fd, F_GETFL);
	if(flags<0){
		LOGE("fcntl F_GETFL");
		return -1;
	}

	flags |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flags)<0){
		LOGE("fcntl F_SETFL");
		return -1;
	}

	return 0;
}

int socket_setReusePort(int sockfd)
{
	int ret = 0;
	int reuse = 1;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (void*)&reuse, sizeof(reuse));
	if(ret<0){
		LOGE("setsockopt SO_REUSEPORT error");
		return -1;
	}

	return 0;
}

// create a udp server and bind
int socket_create_and_bind(const char *port)
{
	int s, sfd;
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_protocol = IPPROTO_UDP;

	s = getaddrinfo(NULL, port, &hints, &result);
	if(s!=0){
		LOGE("getaddrinfo: %s\n", gai_strerror(s));
		if(result!=NULL){
			freeaddrinfo(result);
		}
		return -1;
	}

	for(rp=result; rp!=NULL; rp=rp->ai_next){
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sfd==-1){
			continue;
		}

		s = socket_setReusePort(sfd);
		if(s==-1){
			return -1;
		}

		s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if(s==0){
			break;
		}
		close(sfd);
	}

	if(rp==NULL){
		LOGE("Could not bind");
		return -1;
	}

	freeaddrinfo(result);
	return sfd;
}


void socket_read(struct ev_loop *main_loop, struct ev_io *client_w, int events)
{
	struct server_ctx *ctx = NULL;
	if(EV_ERROR & events){
		LOGE("error event");
		return;
	}

    ctx = (struct server_ctx *)client_w->data;
    
    struct sockaddr_storage src_addr;
    memset(&src_addr, 0, sizeof(struct sockaddr_storage));
    socklen_t src_addr_len = sizeof(struct sockaddr_storage);

    memset(ctx->sock_buffer, 0, sizeof(ctx->sock_buffer));
    
	ssize_t nread = recvfrom(ctx->sockfd, ctx->sock_buffer, TUNNEL_BUF_SIZE, 0,
                            (struct sockaddr *)&src_addr, &src_addr_len);
	if(nread <0 ){
        LOGE("recvfrom error");
        ctx->sock_buf_size = 0;
		return;
	}

    ctx->sock_buf_size = nread;
    ctx->src_addr = src_addr;
    ctx->src_addr_len = src_addr_len;

    LOGV(3, "receive udp data len: [%lu]", nread);

    uint8_t cmd = ctx->sock_buffer[TUNNEL_PAK_CMD_IDX];

    switch (cmd){
        case TUNNEL_CMD_CONNECT:
            server_on_connect(ctx);
            break;
        case TUNNEL_CMD_CONNECT_DONE:
            server_on_connect_done(ctx);
            break;
        case TUNNEL_CMD_TRANS_OUT:
            server_on_transout(ctx);
            break;
    }

}

void tun_read(struct ev_loop *main_loop, struct ev_io *client_w, int events)
{
    struct server_ctx *ctx = NULL;
    if(EV_ERROR & events){
        LOGE("error event");
        return;
    }
    
    ctx = (struct server_ctx *)client_w->data;
    
    size_t nread;
    
    if((nread = read(ctx->tunfd, (void*)ctx->tun_buffer, TUNNEL_BUF_SIZE)) < 0){
        perror("Reading data from tun");
        return;
    }
    
    ctx->tun_buf_size = nread;

    LOGV(3, "read tun len: [%lu]", nread);

    server_on_transin(ctx);
}

int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;
    char *clonedev = "/dev/net/tun";
    
    if((fd = open(clonedev, O_RDWR)) < 0){
        perror("open clonedev");
        return fd;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    
    if (dev!=NULL && strlen(dev)>0) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }
    
    if((err = ioctl(fd, TUNSETIFF, (void*)&ifr)) < 0){
        perror("ioctl TUNSETIFF");
        close(fd);
        return err;
    }
    
    strcpy(dev, ifr.ifr_name);
    
    LOG("open tun dev:%s", dev);
    
    return fd;
}

int tun_setup(const char *dev, const char *tunip, const char *netmask)
{
    struct ifreq ifr;
    struct sockaddr_in addr;
    int sockfd, err = -1;
    
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, tunip, &addr.sin_addr);
    
    bzero(&ifr, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);
    bcopy(&addr, &ifr.ifr_addr, sizeof(addr));
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    // ifconfig tap0 10.0.1.5 #set ip address
    if ((err = ioctl(sockfd, SIOCSIFADDR, (void *)&ifr)) < 0) {
        perror("ioctl SIOSIFADDR");
        goto done;
    }

    if ((err = ioctl(sockfd, SIOCGIFFLAGS, (void *)&ifr)) < 0) {
        perror("ioctl SIOCGIFADDR");
        goto done;
    }

    ifr.ifr_flags |= IFF_UP;
    // ifup tap0 #set up
    if ((err = ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr)) < 0) {
        perror("ioctl SIOCSIFFLAGS");
        goto done;
    }
    
    inet_pton(AF_INET, netmask, &addr.sin_addr);
    bcopy(&addr, &ifr.ifr_netmask, sizeof(addr));
    // ifconfig tap0 10.0.1.5/24 #set sub netmask
    if ((err = ioctl(sockfd, SIOCSIFNETMASK, (void *) &ifr)) < 0) {
        perror("ioctl SIOCSIFNETMASK");
        goto done;
    }
    
    LOG("setup tun dev:%s [%s]", tunip, netmask);
    
done:
    close(sockfd);
    return err;
}

void tunnel_server_start(const char *port)
{
    int sfd = 0, s = 0;
    int tunfd = 0;

    char tun_dev[IFNAMSIZ] = "xtun";
    tunfd = tun_alloc(tun_dev);
    tun_setup(tun_dev, "10.0.0.1", "255.255.0.0");

    s = socket_setnonblock(tunfd);
    if(s==-1){
        exit(1);
        return;
    }

    sfd = socket_create_and_bind(port);
    if(sfd==-1){
        exit(1);
        return;
    }

    s = socket_setnonblock(sfd);
    if(s==-1){
        exit(1);
        return;
    }

    server_init();

    struct ev_loop *main_loop = ev_default_loop(0);

    struct server_ctx ctx;
    memset(&ctx, 0, sizeof(struct server_ctx));

    ctx.loop = main_loop;
    ctx.sockfd = sfd;
    ctx.tunfd = tunfd;
    ctx.read_w.data = (void*)&ctx;
    ctx.tun_read_w.data = (void*)&ctx;
    ctx.sock_buf_size = 0;
    ctx.tun_buf_size = 0;
    ctx.tun_read_start = 0;

    ev_io_init(&ctx.read_w, socket_read, sfd, EV_READ);
    ev_io_init(&ctx.tun_read_w, tun_read, tunfd, EV_READ);
    ev_io_start(main_loop, &ctx.read_w);
    //ev_io_start(main_loop, &ctx.tun_read_w);

    ev_run(main_loop, 0);
}











