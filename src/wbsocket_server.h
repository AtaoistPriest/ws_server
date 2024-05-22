// =====================================================================================
//  Copyright (C) 2024 by 冯飞宇. All rights reserved
//  文 件 名:  wbsocket_server.h
//  作    者:  冯飞宇
//  创建时间:  2024年04月15日
//  描    述: this is a c/c++ file 
// =====================================================================================

#ifndef WS_WBSOCKET_SERVER_H
#define WS_WBSOCKET_SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "./util/ws_common.h"
#include "./util/ws_string.h"
#include "./util/ws_ssl.h"
#include "./util/logger.h"

// 2KB
#define MSG_LEN_MAX 2048


// ---------------------------------------------
// ---------------------------------------------
// ---------------------------------------------
/*
 * websocket socket parameters
 * */

#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

typedef struct ws_session_s        ws_session_t;
typedef struct ws_head_s           ws_head_t;

enum {
	WS_NULL = 0,
	WS_HANDSHARK = 1,
	WS_TRANSMISSION,
	WS_OVER
};

/*
 * web socket protocol structure
 
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len | Extended payload length       |
|I|S|S|S| (4)   |A| (7)         | (16/64)                       |
|N|V|V|V|       |S|             | (if payload len==126/127)     |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|    Extended payload length continued, if payload len == 127   |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               | Masking-key, if MASK set to 1 |
+-------------------------------+-------------------------------+
|    Masking-key (continued)    |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
                   : Payload Data continued ... :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                   Payload Data continued ...                  |
+---------------------------------------------------------------+

 * */
struct ws_head_s{
	unsigned char opcode:4,
				  rsv3:1,
				  rsv2:1,
				  rsv1:1,
				  fin:1;
	unsigned char pl_len:7,
				  mask:1;
};

struct ws_session_s{
	// ws protocol
	ws_head_t ws_head;
	int session_stage;

	// message buffer and handler
	unsigned char msg_buff[MSG_LEN_MAX];
	int msg_len;
	long (*msg_handler)(unsigned char *, long, unsigned char *, long);

	// tcp socket fd
	int client_fd;
	char client_ip[16];
	unsigned short client_port;
};

enum{
	ORDER_RUNNING = 0,
	ORDER_STOP,
	ORDER_PAUSE,
};

void set_server_order(int order);

#define MAX_CLIENT_NUM 20
// start web socket server with ip, port and msg handler
int start_server(char *server_ip, char *server_port, long (*msg_handler_ptr)(unsigned char *, long, unsigned char *, long));

#endif
