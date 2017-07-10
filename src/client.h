#ifndef _TUNNEL_CLIENT_H
#define _TUNNEL_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <ev.h>
#include "uthash.h"

#define CLIENT_STATE_CONNECT 1
#define CLIENT_STATE_CONNECT_DONE 2

#define CLIENT_TIMEOUT_TO_DELETE (3600*24*2)
#define CLIENT_EXPAND_TIMEOUT_TIME (3600)

struct server_ctx;

typedef struct client
{
    char guid[60];
    uint32_t ip;
    uint8_t sid;
    struct sockaddr_storage src_addr;
    socklen_t src_addr_len;

    ev_timer timeout_watcher;
    uint8_t  resend_counter;
    uint8_t version;
    uint8_t state;
    ev_tstamp ts;

    struct server_ctx *ctx;
    UT_hash_handle hh_ip, hh_guid;

}client_t;

client_t* new_client();

client_t* find_client(uint32_t ip);

client_t* find_client_by_guid(char guid[]);

bool add_client(client_t* client);

void delete_client(client_t *s);

void delete_client_by_ip(uint32_t ip, bool reclaim_ip);

void delete_all_clients();

int get_clients_count();

void timeout_cb_delete_client(struct ev_loop *main_loop, ev_timer *w, int events);

void client_debug(client_t *cli);

#endif //_TUNNEL_CLIENT_H