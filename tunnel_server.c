#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_SIZE 10240

void socket_accept(struct ev_loop *main_loop, ev_io *sock_w, int events);
void socket_read(struct ev_loop *main_loop, struct ev_io *client_w, int events);
void socket_write(struct ev_loop *main_loop, struct ev_io *client_w, int events);

struct socket_conn_t {
	struct ev_loop *loop;
	ev_io read_w;
	ev_io write_w;
	char buffer[BUFFER_SIZE];
    size_t bufSize;
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

    memset(conn->buffer, 0, sizeof(conn->buffer));
    
	ssize_t read = recvfrom(client_w->fd, conn->buffer, BUFFER_SIZE, 0,
                            (struct sockaddr *)&src_addr, &src_addr_len);
	if(read <0 ){
		fprintf(stderr, "%s read error\n", __func__);
        conn->bufSize = 0;
		return;
	}
    
    conn->bufSize = read;

	fprintf(stdout, "%s recevie udp data len: [%lu]\n", __func__, read);

    

	
	//ev_io_start(conn->loop, &conn->write_w);

}

void socket_write(struct ev_loop *main_loop, struct ev_io *client_w, int events)
{
//	struct socket_conn_t *conn = NULL;
//
//	if(EV_ERROR & events){
//		fprintf(stderr, "%s error event\n", __func__);
//		return;
//	}
//
//	conn = (struct socket_conn_t *)client_w->data;
//
//	// write/clean data buffer
//	if(conn->buffer == NULL){
//		ev_io_stop(conn->loop, &conn->write_w);
//		return;
//	}
//
//	send(client_w->fd, conn->buffer, strlen(conn->buffer), 0);
//	free(conn->buffer);
//	conn->buffer = NULL;
//
//	ev_io_stop(conn->loop, &conn->write_w);
//	return;
}

int main(int argc, char *argv[])
{
	int sfd = 0, s = 0;

	if(argc!=2){
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
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
    
    struct socket_conn_t conn;
    memset(&conn, 0, sizeof(struct socket_conn_t));
    
    conn.loop = main_loop;
    conn.read_w.data = (void*)&conn;
    conn.write_w.data = (void*)&conn;
    conn.bufSize = 0;
    
    ev_io_init(&conn.read_w, socket_read, sfd, EV_READ);
    ev_io_init(&conn.write_w, socket_write, sfd, EV_WRITE);
    ev_io_start(main_loop, &conn.read_w);

	ev_run(main_loop, 0);

	return 0;
}











