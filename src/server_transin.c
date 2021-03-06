
#include <netinet/ip.h>
#include "server_transin.h"
#include "client.h"

void server_on_transin(struct server_ctx *ctx)
{
    // Parse the ip packet, get client ip
    char *ippack = ctx->tun_buffer.head;
    struct iphdr *ipheader = (struct iphdr *)ippack;
    uint8_t version = ipheader->version;
    uint32_t cli_ip;

    switch (version) {
        case 4: {
            cli_ip = ipheader->daddr;
            break;
        }
        case 6: {
            // Our virtual network is ipv4, so should not go here
            break;
        }
    }

    // Find client form ip
    client_t *cli = find_client(cli_ip);
    if(cli == NULL){
        // Client not found, has to drop the pack
        LOGV(1, "client not found when receive trans-in ip pack. ip=%d",cli_ip);
        return;
    }

    // Construct a trans-in cmd pack, and send to client
    int buf_size = ctx->tun_buffer.size + sizeof(tunnel_trans_in_header);
    ctx->tun_buffer.data[0] = TUNNEL_VERSION;
    ctx->tun_buffer.data[1] = TUNNEL_CMD_TRANS_IN;

    size_t s = sendto(ctx->sockfd, ctx->tun_buffer.data, buf_size, 0, (const struct sockaddr *)&cli->src_addr, cli->src_addr_len);

    if (s == -1) {
        perror("sendto");
    }
    else{
        LOGV(5, "sendto socket len: [%lu], cli ip=%d", s, cli->ip);
    }
}