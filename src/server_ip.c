#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server_ip.h"

static uint32_t min_ip;
static uint32_t max_ip;
static uint32_t next_ip_avaliable;

void server_ip_init()
{
    struct in_addr sin_addr;
    inet_pton(AF_INET, "10.0.0.2", &sin_addr);
    min_ip = ntohl(sin_addr.s_addr);
    inet_pton(AF_INET, "10.0.255.254", &sin_addr);
    max_ip = ntohl(sin_addr.s_addr);

    next_ip_avaliable = min_ip;
}

uint32_t server_allocate_ip()
{
    uint32_t ip = htonl(next_ip_avaliable);
    next_ip_avaliable ++;
    if(next_ip_avaliable > max_ip){
        next_ip_avaliable = min_ip;
    }
    return ip;
}

void server_reclaim_ip(uint32_t ip)
{

}