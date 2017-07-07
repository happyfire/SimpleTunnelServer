#include <netinet/ip.h>
#include "server_transout.h"
#include "client.h"

void send_disconnect(struct server_ctx *ctx)
{
    tunnel_disconnect packet;
    packet.ver = TUNNEL_VERSION;
    packet.cmd = TUNNEL_CMD_DISCONNECT;

    size_t s = sendto(ctx->sockfd, &packet, sizeof(tunnel_disconnect), 0, (const struct sockaddr *)&ctx->src_addr, ctx->src_addr_len);

    if (s == -1) {
        perror("sendto");
    }
    else{
        LOGV(3, "sendto socket len: [%lu]", s);
    }
}

void server_on_transout(struct server_ctx *ctx)
{
    // Parse the packet, get sid and ip (ip get from ip pack)

    uint8_t sid = ctx->sock_buffer[2];
    char *ippack = &(ctx->sock_buffer[3]);
    struct iphdr *ipheader = (struct iphdr *)ippack;
    uint8_t version = ipheader->version;
    uint32_t cli_ip;

    switch (version) {
        case 4: {
            cli_ip = ipheader->saddr;
            break;
        }
        case 6: {
            // Our virtual network is ipv4, so should not go here
            break;
        }
    }

    // Find client form ip and check sid
    client_t *cli = find_client(cli_ip);
    if(cli == NULL){
        // Client not found, it should not connect first or the server restarted, so this client should disconnect.
        LOGV(1, "client not found when receive trans-out pack. ip=%d, sid=%d",cli_ip,sid);
        send_disconnect(ctx);
        return;
    }

    if(cli->sid != sid){
        // SID not match, the ip must be reallocated to another client, so this client should disconnect.
        send_disconnect(ctx);
        return;
    }

    // Update client addr (real outband ip)  (Client addr maybe changed due to swich network)
    cli->src_addr = ctx->src_addr;
    cli->src_addr_len = ctx->src_addr_len;

    // Write ip pack to tun
    size_t nwrite;
    int ippack_size = ctx->sock_buf_size - sizeof(tunnel_trans_out_header);
    if((nwrite=write(ctx->tunfd, (void*)ippack, ippack_size)) < 0){
        perror("Writing ip pack to tun");
    }
    else{
        LOGV(3, "write to tun len: [%lu]", nwrite);

        if(ctx->tun_read_start==0){
            ev_io_start(ctx->loop, &ctx->tun_read_w);
            ctx->tun_read_start = 1;
        }
    }
}