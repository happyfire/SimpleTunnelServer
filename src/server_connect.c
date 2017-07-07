
#include "server_connect.h"
#include "client.h"
#include "server_ip.h"

void send_connect_ok(struct server_ctx *ctx, client_t *cli);
void timeout_cb(struct ev_loop *main_loop, ev_timer *w, int events);

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
        LOGV(1, "new client with guid : [%s]", cli->guid);

        server_allocate_ip(&cli->ip, &cli->sid); // Allocate IP for client

        if(!add_client(cli)){
            safe_free(cli);
            LOGE("fail to add client");
            return;
        }
        LOGV(1, "set client ip : [%d] sid : [%d]", cli->ip, cli->sid);
    }
    else{
        LOGV(1,"client reconnect. guid=%s",cli->guid);
    }

    cli->state = CLIENT_STATE_CONNECT;
    cli->src_addr = ctx->src_addr;
    cli->src_addr_len = ctx->src_addr_len;
    cli->ctx = ctx;
    cli->version = ctx->sock_buffer[TUNNEL_PAK_VER_IDX];
    cli->resend_counter = 0;

    // Send connect ok packet to client
    send_connect_ok(ctx, cli);
}

void timeout_cb(struct ev_loop *main_loop, ev_timer *w, int events)
{
    if(EV_ERROR & events){
        LOGE("error event");
        return;
    }

    LOG("timeout");

    client_t *cli = (client_t *)w->data;
    if(cli->state == CLIENT_STATE_CONNECT){
        if(cli->resend_counter++ < SERVER_MAX_RESEND_COUNT) {
            LOGV(1, "resend connect ok count = %d",cli->resend_counter);
            send_connect_ok(cli->ctx, cli);
        }
        else{
            LOGV(1, "resend connect ok count max, delete client");
            delete_client(cli);
        }
    }
}

void send_connect_ok(struct server_ctx *ctx, client_t *cli)
{
    tunnel_connect_ok_done packet;
    packet.ver = TUNNEL_VERSION;
    packet.cmd = TUNNEL_CMD_CONNECT_OK;
    packet.ip = cli->ip;
    packet.sid = cli->sid;

    size_t s = sendto(ctx->sockfd, &packet, sizeof(tunnel_connect_ok_done), 0, (const struct sockaddr *)&cli->src_addr, cli->src_addr_len);

    if (s == -1) {
        perror("sendto");
    }
    else{
        LOGV(3, "sendto socket len: [%lu]", s);
    }

    // Start a timer to resend conenct ok, if not connect done received
    cli->timeout_watcher.data = (void*)cli;
    ev_timer_init(&cli->timeout_watcher, timeout_cb, 0.5, 0.);
    ev_timer_start(ctx->loop, &cli->timeout_watcher);
}

void server_on_connect_done(struct server_ctx *ctx)
{
    // Parse the connect done packet, get ip
    int ip = *((int*)&ctx->sock_buffer[2]);
    LOGV(1, "on connect done ip : [%d]", ip);
    client_t *cli = find_client(ip);
    if(cli){
        cli->state = CLIENT_STATE_CONNECT_DONE;
        client_debug(cli);
    }
}