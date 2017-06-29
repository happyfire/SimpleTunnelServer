#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>

#if !defined(IFNAMSIZ)
#define IFNAMSIZ 256
#endif

#define BUFFER_SIZE 10240

void socket_accept(struct ev_loop *main_loop, ev_io *sock_w, int events);
void socket_read(struct ev_loop *main_loop, struct ev_io *client_w, int events);
void socket_write(struct ev_loop *main_loop, struct ev_io *client_w, int events);

struct socket_conn_t {
	struct ev_loop *loop;
    int sockfd;
    int tunfd;
	ev_io read_w;
	ev_io tun_read_w;
	char sock_buffer[BUFFER_SIZE];
    size_t sock_buf_size;
    char tun_buffer[BUFFER_SIZE];
    size_t tun_buf_size;
    int tun_read_start;
    
    
    struct sockaddr_storage src_addr;
    socklen_t src_addr_len;
};

int socket_setnonblock(int fd)
{
	long flags = fcntl(fd, F_GETFL);
	if(flags<0){
		fprintf(stderr, "fcntl F_GETFL");
		return -1;
	}

	flags |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flags)<0){
		fprintf(stderr, "fcntl F_SETFL");
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
		fprintf(stderr, "%s setsockopt SO_REUSEPORT error\n", __func__);
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
		fprintf(stderr, "%s getaddrinfo: %s\n", __func__, gai_strerror(s));
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
		fprintf(stderr, "%s Could not bind\n", __func__);
		return -1;
	}

	freeaddrinfo(result);
	return sfd;
}


void socket_read(struct ev_loop *main_loop, struct ev_io *client_w, int events)
{
	struct socket_conn_t *conn = NULL;
	if(EV_ERROR & events){
		fprintf(stderr, "%s error event\n", __func__);
		return;
	}

	conn = (struct socket_conn_t *)client_w->data;
    
    struct sockaddr_storage src_addr;
    memset(&src_addr, 0, sizeof(struct sockaddr_storage));
    socklen_t src_addr_len = sizeof(struct sockaddr_storage);

    memset(conn->sock_buffer, 0, sizeof(conn->sock_buffer));
    
	ssize_t nread = recvfrom(client_w->fd, conn->sock_buffer, BUFFER_SIZE, 0,
                            (struct sockaddr *)&src_addr, &src_addr_len);
	if(nread <0 ){
		fprintf(stderr, "%s read error\n", __func__);
        conn->sock_buf_size = 0;
		return;
	}
    
    conn->sock_buf_size = nread;
    conn->src_addr = src_addr;
    conn->src_addr_len = src_addr_len;

	fprintf(stdout, "%s recevie udp data len: [%lu]\n", __func__, nread);

    //write to tun
    
    size_t nwrite;
    
    if((nwrite=write(conn->tunfd, (void*)conn->sock_buffer, conn->sock_buf_size)) < 0){
        perror("Writing data to tun");
    }
    else{
        fprintf(stdout, "%s write to tun len: [%lu]\n", __func__, nwrite);
        if(conn->tun_read_start==0){
        	ev_io_start(main_loop, &conn->tun_read_w);
            conn->tun_read_start = 1;
        }
    }
}

void tun_read(struct ev_loop *main_loop, struct ev_io *client_w, int events)
{
    struct socket_conn_t *conn = NULL;
    if(EV_ERROR & events){
        fprintf(stderr, "%s error event\n", __func__);
        return;
    }
    
    conn = (struct socket_conn_t *)client_w->data;
    
    size_t nread;
    
    if((nread = read(conn->tunfd, (void*)conn->tun_buffer, BUFFER_SIZE)) < 0){
        perror("Reading data from tun");
        exit(1);
    }
    
    conn->tun_buf_size = nread;
    
    fprintf(stdout, "%s read tun len: [%lu]\n", __func__, nread);
    
    //send to socket
//    struct sockaddr_in sockaddr4;
//    memset(&sockaddr4, 0, sizeof(sockaddr4));
//    
//    sockaddr4.sin_family      = AF_INET;
//    sockaddr4.sin_port        = htons(443);
//    sockaddr4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    size_t s = sendto(conn->sockfd, conn->tun_buffer, conn->tun_buf_size, 0, (const struct sockaddr *)&conn->src_addr, conn->src_addr_len);
    
    if (s == -1) {
        perror("sendto");
    }
    else{
        fprintf(stdout, "%s sendto socket len: [%lu]\n", __func__, s);
    }
}

int tun_alloc(char* dev)
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
    
    fprintf(stdout, "open tun dev:%s\n", dev);
    
    return fd;
}

int tun_setup(const char* dev, const char* tunip, const char* netmask)
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
    
    // ifconfig tap0 10.0.1.5 #设定ip地址
    if ((err = ioctl(sockfd, SIOCSIFADDR, (void *)&ifr)) < 0) {
        perror("ioctl SIOSIFADDR");
        goto done;
    }
    
    /* 获得接口的标志 */
    if ((err = ioctl(sockfd, SIOCGIFFLAGS, (void *)&ifr)) < 0) {
        perror("ioctl SIOCGIFADDR");
        goto done;
    }
    
    /* 设置接口的标志 */
    ifr.ifr_flags |= IFF_UP;
    // ifup tap0 #启动设备
    if ((err = ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr)) < 0) {
        perror("ioctl SIOCSIFFLAGS");
        goto done;
    }
    
    inet_pton(AF_INET, netmask, &addr.sin_addr);
    bcopy(&addr, &ifr.ifr_netmask, sizeof(addr));
    // ifconfig tap0 10.0.1.5/24 #设定子网掩码
    if ((err = ioctl(sockfd, SIOCSIFNETMASK, (void *) &ifr)) < 0) {
        perror("ioctl SIOCSIFNETMASK");
        goto done;
    }
    
    
done:
    close(sockfd);
    return err;
}

int main(int argc, char *argv[])
{
	int sfd = 0, s = 0;
    int tunfd = 0;

	if(argc!=2){
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		return -1;
	}
    
    char tun_dev[IFNAMSIZ];
    tun_dev[0] = '\0';
    tunfd = tun_alloc(tun_dev);
    tun_setup(tun_dev, "10.0.1.1", "255.255.255.0");
    
    s = socket_setnonblock(tunfd);
    if(s==-1){
        abort();
        return -1;
    }

	sfd = socket_create_and_bind(argv[1]);
	if(sfd==-1){
		abort();
		return -1;
	}

	s = socket_setnonblock(sfd);
	if(s==-1){
		abort();
		return -1;
	}

	struct ev_loop *main_loop = ev_default_loop(0);
    
    struct socket_conn_t sock_conn;
    memset(&sock_conn, 0, sizeof(struct socket_conn_t));
    
    sock_conn.loop = main_loop;
    sock_conn.sockfd = sfd;
    sock_conn.tunfd = tunfd;
    sock_conn.read_w.data = (void*)&sock_conn;
    sock_conn.tun_read_w.data = (void*)&sock_conn;
    sock_conn.sock_buf_size = 0;
    sock_conn.tun_buf_size = 0;
    sock_conn.tun_read_start = 0;
    
    ev_io_init(&sock_conn.read_w, socket_read, sfd, EV_READ);
    ev_io_init(&sock_conn.tun_read_w, tun_read, tunfd, EV_READ);
    ev_io_start(main_loop, &sock_conn.read_w);
    //ev_io_start(main_loop, &sock_conn.tun_read_w);

	ev_run(main_loop, 0);

	return 0;
}











