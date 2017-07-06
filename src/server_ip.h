#ifndef __TUNNEL_SERVER_IP_H
#define __TUNNEL_SERVER_IP_H

#include <stdint.h>

void server_ip_init();

// allocate a ip address in network order
uint32_t server_allocate_ip();

// ip: 32 bit integer in network order
void server_reclaim_ip(uint32_t ip);

#endif
