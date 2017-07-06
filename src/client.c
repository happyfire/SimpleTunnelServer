#include <stdio.h>
#include "client.h"
#include "utils.h"

static client_t *g_clients_ip = NULL;     //ip to client_t map
static client_t *g_clients_guid = NULL; //guid to client_t map

client_t* new_client()
{
    client_t *s = (client_t *)malloc(sizeof(client_t));
    if(s == NULL){
        LOGE("malloc failed!");
    }
    return s;
}

client_t* find_client(uint32_t ip)
{
    client_t *s = NULL;
    HASH_FIND(hh_ip, g_clients_ip, &ip, sizeof(uint32_t), s);
    return s;
}

client_t* find_client_by_guid(char guid[])
{
    client_t *s = NULL;
    HASH_FIND(hh_guid, g_clients_guid, guid, strlen(guid), s);
    return s;
}

bool add_client(client_t* client)
{
    if(client == NULL){
        return false;
    }

    client_t *s = find_client_by_guid(client->guid);

    if(s == NULL){
        //add to g_clients_guid
        HASH_ADD(hh_guid, g_clients_guid, guid, strlen(client->guid), client);

        //add to g_clients_ip
        HASH_ADD(hh_ip, g_clients_ip, ip, sizeof(uint32_t), client);

        return true;
    }
    else{
        LOGE("client already in hashmap!");
        return false;
    }
}

void delete_client_by_ip(uint32_t ip)
{
    client_t *s = NULL;
    HASH_FIND(hh_ip, g_clients_ip, &ip, sizeof(uint32_t), s);
    if(s != NULL){
        HASH_DELETE(hh_ip, g_clients_ip, s);
        HASH_DELETE(hh_guid, g_clients_guid, s);
        free(s);
    }
}

void delete_client(client_t *s)
{
    if(s != NULL){
        HASH_DELETE(hh_ip, g_clients_ip, s);
        HASH_DELETE(hh_guid, g_clients_guid, s);
        free(s);
    }
}

void delete_all_clients()
{
    HASH_CLEAR(hh_guid, g_clients_guid);

    client_t *cur = NULL, *tmp = NULL;

    HASH_ITER(hh_ip, g_clients_ip, cur, tmp){
        HASH_DELETE(hh_ip, g_clients_ip, cur);
        free(cur);
    }
}

int get_clients_count()
{
    unsigned int count = HASH_CNT(hh_ip, g_clients_ip);
    return count;
}