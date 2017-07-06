#include <stdio.h>
#include "client.h"

extern int verbose;

static client_t *g_clients = NULL;     //ip to client_t map
static client_t *g_clients_guid = NULL; //guid to client_t map

client_t* new_client()
{
    client_t *s = (client_t *)malloc(sizeof(client_t));
    if(s == NULL){
        if(verbose) {
            fprintf(stderr, "%s malloc failed!\n", __func__);
        }
    }
    return s;
}

client_t* find_client(uint32_t ip)
{
    client_t *s = NULL;
    HASH_FIND_INT(g_clients, &ip, s);
    return s;
}

client_t* find_client_by_guid(char guid[])
{
    client_t *s = NULL;
    HASH_FIND_STR(g_clients_guid, guid, s);
    return s;
}

void add_client(client_t* client)
{
    if(client == NULL){
        return;
    }

    client_t *s = NULL;

    HASH_FIND_STR(g_clients_guid, client->guid, s);

    if(s == NULL){
        //add to g_clients_guid
        HASH_ADD_STR(g_clients_guid, guid, client);
        //add to g_clients
        HASH_ADD_INT(g_clients, ip, client);
    }
    else if(verbose){
        fprintf(stderr, "%s client already in hashmap!\n", __func__);
    }
}

void delete_client_by_ip(uint32_t ip)
{
    client_t *s = NULL;
    HASH_FIND_INT(g_clients, &ip, s);
    if(s != NULL){
        HASH_DEL(g_clients, s);
        HASH_DEL(g_clients_guid, s);
        free(s);
    }
}

void delete_client(client_t *s)
{
    if(s != NULL){
        HASH_DEL(g_clients, s);
        HASH_DEL(g_clients_guid, s);
        free(s);
    }
}

void delete_all_clients()
{
    HASH_CLEAR(hh,g_clients_guid);

    client_t *cur = NULL, *tmp = NULL;

    HASH_ITER(hh, g_clients, cur, tmp){
        HASH_DEL(g_clients, cur);
        free(cur);
    }
}

int get_clients_count()
{
    unsigned int count = HASH_COUNT(g_clients);
    return count;
}