#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <memory.h>
#include "server_ip.h"
#include "utils.h"

struct server_ip_ctx {
    uint32_t ip;
    uint8_t sid;
};

static struct server_ip_ctx *ip_list = NULL;
static int ip_count = 0;

static int next_avaliable_ip_idx;
static int last_avaliable_ip_idx;

static uint16_t *ip_to_idx = NULL; //Note: ip count should smaller than 65535, so we use uint16_t as the index.
static int32_t ip_start;

void server_ip_init()
{
    uint32_t min_ip;
    uint32_t max_ip;

    struct in_addr sin_addr;
    inet_pton(AF_INET, "10.0.0.2", &sin_addr);
    min_ip = ntohl(sin_addr.s_addr);
    inet_pton(AF_INET, "10.0.255.254", &sin_addr);
    max_ip = ntohl(sin_addr.s_addr);

    ip_count = max_ip - min_ip + 1;
    LOG("ip count=%d", ip_count);

    ip_list = (struct server_ip_ctx *)malloc(ip_count * sizeof(struct server_ip_ctx));
    ip_to_idx = (uint16_t *)malloc(ip_count * sizeof(uint16_t));

    uint32_t tmp = min_ip;
    for(int i=0; i<ip_count; i++){
        ip_list[i].ip = tmp++;
        ip_list[i].sid = 0;

        ip_to_idx[i] = i;
    }

    next_avaliable_ip_idx = 0;
    last_avaliable_ip_idx = ip_count-1;
    ip_start = min_ip;
}

int ip_list_idx_advance(int idx)
{
    int next = idx + 1;
    if(next == ip_count){
        next = 0;
    }
    return next;
}

void server_allocate_ip(uint32_t *out_ip, uint8_t *out_sid)
{
    struct server_ip_ctx *ip_ctx = &(ip_list[next_avaliable_ip_idx]);
    *out_ip = htonl(ip_ctx->ip);
    *out_sid = ip_ctx->sid++;

    if(next_avaliable_ip_idx == last_avaliable_ip_idx){
        LOGE("all ip allocated!");
        last_avaliable_ip_idx = ip_list_idx_advance(last_avaliable_ip_idx); // Just advance the last ip idx, maybe we can find out the oldest ip and reclaim it.
    }

    next_avaliable_ip_idx = ip_list_idx_advance(next_avaliable_ip_idx);

    if(IS_VERBOSE(3)){
        char ipstr[INET_ADDRSTRLEN];
        struct	in_addr sin_addr = { *out_ip };
        inet_ntop(AF_INET, &sin_addr, ipstr, sizeof(ipstr));
        LOGV(3, "allocate ip:%s [%d] sid:%d, next_idx:%d, last_idx:%d", ipstr, *out_ip, *out_sid, next_avaliable_ip_idx, last_avaliable_ip_idx);
    }
}

void server_reclaim_ip(uint32_t ip)
{
    LOGV(3, "recliam ip %d (%d)",ip, ntohl(ip));

    ip = ntohl(ip);

    int32_t ip_seq = ip - ip_start;
    int16_t ip_idx = ip_to_idx[ip_seq];
    struct server_ip_ctx ip_ctx_reclaim = ip_list[ip_idx];

    int next_last_idx = ip_list_idx_advance(last_avaliable_ip_idx);
    if(next_last_idx == ip_idx){
        last_avaliable_ip_idx = next_last_idx;
        return;
    }

    struct server_ip_ctx ip_ctx_move = ip_list[next_last_idx];

    ip_list[ip_idx] = ip_ctx_move;
    ip_list[next_last_idx] = ip_ctx_reclaim;

    last_avaliable_ip_idx = next_last_idx;

    ip_to_idx[ip_seq] = next_last_idx;
    ip_to_idx[ip_ctx_move.ip - ip_start] = ip_idx;
}

void server_ip_debug()
{
    LOGV(3, "=========== ip to idx to ip_ctx ============");
    uint32_t ip = ip_start;
    for(int i=0; i<ip_count; i++){
        int idx = ip_to_idx[ip-ip_start];
        LOGV(3, "ip %d -> %d -> [ip=%d, sid=%d] %s%s", ip, idx, ip_list[idx].ip, ip_list[idx].sid, next_avaliable_ip_idx==idx?"<--N":"", last_avaliable_ip_idx==idx?"<--L":"");
        ip++;
    }

    LOGV(3, "next_avaliable_ip_idx=%d, last_avaliable_ip_idx=%d", next_avaliable_ip_idx, last_avaliable_ip_idx);
}