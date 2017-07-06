#ifndef __TUNNEL_SERVER_IP_H
#define __TUNNEL_SERVER_IP_H

#include <stdint.h>

void server_ip_init();

// allocate a ip address in network order
void server_allocate_ip(uint32_t *out_ip, uint8_t *out_sid);

// ip: 32 bit integer in network order
void server_reclaim_ip(uint32_t ip);

#endif
