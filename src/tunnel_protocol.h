/*
 *  Tunnel protocols
 *
 *  connect package:
 * +----+-----+----------+----------+
 * |VER | CMD | GUID_LEN |  GUID    |
 * +----+-----+----------+----------+
 * | 1  |  1  |  1       | Variable |
 * +----+-----+----------+----------+
 *
 *  connect ok/done package:
 * +----+-----+----+-----+
 * |VER | CMD | IP | SID |
 * +----+-----+----+-----+
 * | 1  |  1  |  4 | 1   |
 * +----+-----+----+-----+
 *
 *  trans out package:
 * +----+-----+-----+---------+
 * |VER | CMD | SID | IP Pack |
 * +----+-----+-----+---------+
 * | 1  |  1  |  1  | Variable|
 * +----+-----+-----+---------+
 *
 *  trans in package:
 * +----+-----+---------+
 * |VER | CMD | IP Pack |
 * +----+-----+---------+
 * | 1  |  1  | Variable|
 * +----+-----+---------+
 */

#ifndef _TUNNEL_PROTOCOL_H
#define _TUNNEL_PROTOCOL_H

#include <stdlib.h>
#include <stdint.h>

#define TUNNEL_VERSION 0x01
#define TUNNEL_CMD_CONNECT 0x01
#define TUNNEL_CMD_CONNECT_OK 0x02
#define TUNNEL_CMD_CONNECT_DONE 0x03
#define TUNNEL_CMD_TRANS_OUT 0x04
#define TUNNEL_CMD_TRANS_IN 0x05

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint8_t guid_len;
}tunnel_connect_header;

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint32_t ip;
    uint8_t sid;
}tunnel_connect_ok_done;

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint8_t sid;
}tunnel_trans_out_header;

typedef struct {
    uint8_t ver;
    uint8_t cmd;
}tunnel_trans_in_header;

#pragma pack(pop)

#endif // _TUNNEL_PROTOCOL_H