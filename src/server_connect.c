
#include "server_connect.h"
#include "client.h"
#include "server_ip.h"

void send_connect_ok(struct server_ctx *ctx, client_t *cli);

void server_on_connect(struct server_ctx *ctx)
{
    // Parse the connect packet, get guid
    int guid_len = ctx->sock_buffer[TUNNEL_PAK_GUID_LEN_IDX];
    char guid[60];
    memcpy(guid, ctx->sock_buffer + sizeof(tunnel_connect_header), guid_len);
    guid[guid_len] = '\0';
    LOGV(1, "on connect, guid : [%s]", guid);

    // Find if client has connected, check by guid

    client_t *cli = find_client_by_guid(guid);
    if(cli == NULL){
        // Create a new client, add add to hash maps
        cli = new_client();
        memcpy(cli->guid, guid, guid_len+1);
        LOGV(1, "set client guid : [%s]", cli->guid);

        server_allocate_ip(&cli->ip, &cli->sid); // Allocate IP for client

        if(!add_client(cli)){
            safe_free(cli);
            LOGE("fail to add client");
            return;
        }

        cli->src_addr = ctx->src_addr;
        cli->src_addr_len = ctx->src_addr_len;
        cli->version = ctx->sock_buffer[TUNNEL_PAK_VER_IDX];

        LOGV(1, "set client ip : [%d] sid : [%d]", cli->ip, cli->sid);
    }
    else{
        LOGV(1,"client reconnect. guid=%s",cli->guid);
    }

    // Send connect ok packet to client
    send_connect_ok(ctx, cli);
}

void send_connect_ok(struct server_ctx *ctx, client_t *cli)
{
    int buf_len = sizeof(tunnel_connect_ok_done);
    char connect_ok[256];
    char* tmp = connect_ok;
    connect_ok[0] = TUNNEL_VERSION;
    connect_ok[1] = TUNNEL_CMD_CONNECT_OK;
    tmp+=2;
    int* tmpi = (int*)tmp;
    *tmpi = cli->ip;

    connect_ok[6] = 1;

    size_t s = sendto(ctx->sockfd, connect_ok, buf_len, 0, (const struct sockaddr *)&ctx->src_addr, ctx->src_addr_len);

    if (s == -1) {
        perror("sendto");
    }
    else{
        LOGV(3, "sendto socket len: [%lu]", s);
    }
}