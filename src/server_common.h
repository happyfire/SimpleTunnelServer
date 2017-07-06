#ifndef __TUNNEL_COMMON_H
#define __TUNNEL_COMMON_H

#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/unistd.h>
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
#include <string.h>
#include <stdbool.h>
#include "tunnel_protocol.h"
#include "utils.h"

#define TUNNEL_BUF_SIZE 10240

struct server_ctx {
    struct ev_loop *loop;
    int state;
    int sockfd;
    int tunfd;
    ev_io read_w;
    ev_io tun_read_w;
    char sock_buffer[TUNNEL_BUF_SIZE];
    size_t sock_buf_size;
    char tun_buffer[TUNNEL_BUF_SIZE];
    size_t tun_buf_size;
    int tun_read_start;

    struct sockaddr_storage src_addr;
    socklen_t src_addr_len;
};

#endif
