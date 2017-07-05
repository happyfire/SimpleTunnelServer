#ifndef _TUNNEL_CLIENT_H
#define _TUNNEL_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include "base/uthash.h"

typedef struct client
{
    char guid[129];
    uint32_t ip;
    uint8_t sid;
    struct sockaddr_storage src_addr;
    socklen_t src_addr_len;

    uint8_t version;

    UT_hash_handle hh;

}client_t;

client_t* new_client();

client_t* find_client(uint32_t ip);

client_t* find_client_by_guid(char guid[]);

void add_client(client_t* client);

void delete_client(uint32_t ip);

void delete_all_clients();

int get_clients_count();

#endif //_TUNNEL_CLIENT_H