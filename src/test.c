#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "server_ip.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    g_verbose = 5;

    server_ip_init();

    uint32_t ip;
    uint8_t sid;
    for(int i=0; i<5; i++) {
        server_allocate_ip(&ip, &sid);
    }

    server_ip_debug();

    struct in_addr sin_addr;
    inet_pton(AF_INET, "10.0.0.3", &sin_addr);
    server_reclaim_ip(sin_addr.s_addr);
    server_ip_debug();

    server_allocate_ip(&ip, &sid);
    server_allocate_ip(&ip, &sid);
    server_ip_debug();

    server_allocate_ip(&ip, &sid);
    server_ip_debug();

    inet_pton(AF_INET, "10.0.0.6", &sin_addr);
    server_reclaim_ip(sin_addr.s_addr);
    server_ip_debug();

    server_allocate_ip(&ip, &sid);
    server_ip_debug();

    server_allocate_ip(&ip, &sid);
    server_ip_debug();



    return 0;
}