#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <memory.h>
#include "server_ip.h"
#include "utils.h"

static uint32_t min_ip;
static uint32_t max_ip;
static uint32_t next_ip_avaliable;

static uint8_t* ip_sids = NULL;

void server_ip_init()
{
    struct in_addr sin_addr;
    inet_pton(AF_INET, "10.0.0.2", &sin_addr);
    min_ip = ntohl(sin_addr.s_addr);
    inet_pton(AF_INET, "10.0.255.254", &sin_addr);
    max_ip = ntohl(sin_addr.s_addr);

    next_ip_avaliable = min_ip;

    int ip_cnt = max_ip - min_ip + 1;
    LOG("max ip count=%d",ip_cnt);
    ip_sids = (uint8_t *)malloc(ip_cnt);
    memset(ip_sids, 0, ip_cnt);
}

void server_allocate_ip(uint32_t *out_ip, uint8_t *out_sid)
{
    int ip = next_ip_avaliable;
    *out_ip = htonl(next_ip_avaliable);
    next_ip_avaliable ++;
    if(next_ip_avaliable > max_ip){
        next_ip_avaliable = min_ip;
    }

    *out_sid = ip_sids[ip-min_ip];
    ip_sids[ip-min_ip]++;
}

void server_reclaim_ip(uint32_t ip)
{
    int ip_h = ntohl(ip);
    ip_sids[ip_h-min_ip] = 0;
}