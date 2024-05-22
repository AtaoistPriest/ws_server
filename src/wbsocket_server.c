#include "./wbsocket_server.h"

static void *client_handler(void *args);
static int send_msg(int dst_fd, unsigned char *msg, long msg_len);
static int msg_switch(ws_session_t *ws_session);
static int ws_handshark(ws_session_t *ws_session);
static int ws_transmission(ws_session_t *ws_session);
static void ws_umask(unsigned char *client_data, long length, unsigned char *mask_key);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int server_order = ORDER_RUNNING;

static int send_msg(int dst_fd, unsigned char *msg, long msg_len)
{
	unsigned char msg_buff[MSG_LEN_MAX];
	int ws_head_len = 2;

	msg_buff[0] = 0x81;
	if ( msg_len < 126 )
	{
		msg_buff[1] = msg_len;	
		ws_head_len = 2;
	}
	else if ( msg_len < 0xffff )
	{
		msg_buff[1] = 126;
		msg_buff[2] = msg_len >> 8;
		msg_buff[3] = msg_len & 0xff;
		ws_head_len = 4;
	}
	else
	{
		msg_buff[1] = 127;
		for (int i = 0; i < 8; i++)
		{
			msg_buff[2 + i] = (msg_len >> ((7 - i) * 8)) & 0xff;
		}
		ws_head_len = 10;
	}

	memcpy(msg_buff + ws_head_len, msg, msg_len);

	return send(dst_fd, msg_buff, msg_len + ws_head_len, 0);
}

static void ws_umask(unsigned char *client_data, long length, unsigned char *mask_key)
{
	for ( int i = 0; i < length; i++ )
	{
		client_data[i] ^= mask_key[i%4];
	}
}

static int ws_handshark(ws_session_t *ws_session)
{
	char line[MSG_LEN_MAX] = "";
	char sec_data[128] = "", sec_accept[32] = "";
	char msg_res[MSG_LEN_MAX] = "";
	int search_start_idx = 0;

	log_info("========= handshark request =========");
	log_info("%s", ws_session->msg_buff);
	log_info("========= handshark request =========");

	do
	{
		bzero(line, MSG_LEN_MAX);
		int line_len = nextline((char *)ws_session->msg_buff, line, search_start_idx);
		if ( line_len == ERROR )
		{
			log_error("Failed to split next line with %s", ws_session->msg_buff);
			break;
		}

		if ( strstr(line, "Sec-WebSocket-Key") )
		{
			strcat(line, GUID);			

			ws_sha1((unsigned char *)line + 19, strlen(line + 19), (unsigned char *)sec_data);
			ws_base64_encode(sec_data, strlen(sec_data), sec_accept);			

			bzero(msg_res, MSG_LEN_MAX);
			
			int msg_len = sprintf(msg_res, "HTTP/1.1 101 Switching Protocols\r\n"
					"Upgrade: websocket\r\n"
					"Connection: Upgrade\r\n"
					"Sec-WebSocket-Accept: %s\r\n\r\n", sec_accept);

			int send_len = send(ws_session->client_fd, (unsigned char *)msg_res, msg_len, 0);
			if ( send_len < 0 )
			{
				log_error("Failed to send ws handshark reponse");
				return ERROR;
			}

			ws_session->session_stage = WS_HANDSHARK;
			break;
		}
		search_start_idx += line_len; 
		// "\r\n"
		search_start_idx += 2; 

	}while ( search_start_idx < ( ws_session->msg_len - 1 ) );

	if ( search_start_idx >= (ws_session->msg_len) )
	{
		return ERROR;
	}

	return OK;
}

static int ws_transmission(ws_session_t *ws_session)
{
	if ( ws_session->msg_buff[0] == 0x88 )
	{
		log_warn("Receive Close Connection Request");
		ws_session->session_stage = WS_OVER;
		return OK;
	}
	// catch web socket head
	ws_head_t *ws_head = (ws_head_t *)ws_session->msg_buff;
	long pl_len = ws_head->pl_len;
	unsigned char msg_res[MSG_LEN_MAX];
	unsigned char *payload = NULL;
	int mask_key_start_idx = -1;
	if ( pl_len < 126 )
	{
		// ws_head_t(2) + mask_key(4)
		payload = ws_session->msg_buff + 6;
		mask_key_start_idx = 2;
	}
	else if ( pl_len == 126 )
	{
		pl_len = (ws_session->msg_buff[2] << 8) + ws_session->msg_buff[3];
		// ws_head_t(2) + pl_len(2) + mask_key(4)
		payload = ws_session->msg_buff + (2 + 2 + 4);
		mask_key_start_idx = 4;
	}
	else if ( pl_len == 127 )
	{
		for ( int i = 0; i < 8; i++ )
		{
			pl_len += (ws_session->msg_buff[i + 2] << ((7 - i) * 8));
		}
		// ws_head_t(2) + pl_len(8) + mask_key(4)
		payload = ws_session->msg_buff + (2 + 8 + 4);
		mask_key_start_idx = 10;
	}
	else
	{
		log_error("Invalid payload len with %d", pl_len);
		return ERROR;
	}
	ws_umask(payload, pl_len, ws_session->msg_buff + mask_key_start_idx);
	long res_len = ws_session->msg_handler(payload, pl_len, msg_res, MSG_LEN_MAX);
	if ( res_len != ERROR )
	{
		send_msg(ws_session->client_fd, msg_res, res_len);
	}
	ws_session->session_stage = WS_TRANSMISSION;
	// if client closes ws connection, close tcp connection together.
	return OK;
}

static int msg_switch(ws_session_t *ws_session)
{
	if ( ws_session->session_stage == WS_NULL )
	{
		return ws_handshark(ws_session);
	}
	else if ( ws_session->session_stage == WS_HANDSHARK || ws_session->session_stage == WS_TRANSMISSION )
	{
		return ws_transmission(ws_session);
	}
	else if ( ws_session->session_stage == WS_OVER )
	{
		log_error("Transmission Failed, Because WS Session has bean disconnected");
		return ERROR;
	}
	return OK;
}

// client handler
static void *client_handler(void *args)
{
    ws_session_t ws_session = *(ws_session_t *)args;

	log_info("Client (%s:%hu) Handler Start Successfully", ws_session.client_ip, ws_session.client_port);

	ws_session.session_stage = WS_NULL;

    while (TRUE)
    {
		bzero(ws_session.msg_buff, MSG_LEN_MAX);

        int msg_len = recv(ws_session.client_fd, ws_session.msg_buff, MSG_LEN_MAX, 0);
        if ( msg_len == -1 )
		{
			log_error("TCP Client (%s:%hu) Failed to receive Message\n", ws_session.client_ip, ws_session.client_port);
        }
		else if ( msg_len == 0 )
		{
			log_warn("TCP Client (%s:%hu) Has DisConnected\n", ws_session.client_ip, ws_session.client_port);
			break;
		}

		ws_session.msg_len = msg_len;

		if ( msg_switch(&ws_session) != OK )
		{
			printf("[ERROR] Failed Handle Message %s\n", ws_session.msg_buff);
		}

		if ( ws_session.session_stage == WS_OVER )
		{
			printf("[INFO] WS Client Session %s:%hu DisConnected\n", ws_session.client_ip, ws_session.client_port);
			break;
		}
    }
    close(ws_session.client_fd);
	return NULL;
}

static int get_server_order()
{
	int order = 0;
	pthread_mutex_lock(&mutex);
	order = server_order;
	pthread_mutex_unlock(&mutex);
	return order;
}

void set_server_order(int order)
{
	pthread_mutex_lock(&mutex);
	if ( order >= ORDER_RUNNING && order <= ORDER_PAUSE )
	{
		server_order = order;
	}
	pthread_mutex_unlock(&mutex);
}

/*
 *	tcp server listen args[1]:args[2]
 * */
int start_server(char *server_ip, char *server_port, long (*msg_handler_ptr)(unsigned char *, long, unsigned char *, long))
{
    // tcp socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
		log_fatal("Failed to create tcp socket");
		return SYS_ERROR;
    }

	// reuse tcp port
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR, &yes, sizeof(yes));

    // bind server address
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_port = htons(atoi(server_port));
    int result = bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    if (result == -1)
    {
		log_fatal("Failed to bind server address with ip %s port %s", server_ip, server_port);
		return SYS_ERROR;
    }

	// start to listen
    listen(sockfd, MAX_CLIENT_NUM);

	int flags = fcntl(sockfd, F_GETFL, 0);  
    if ( fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0 ) 
	{  
		  log_fatal("Failed to adjust accept() no block");
    }  

	log_info("====================================");
	log_info("TCP Server is ready");
	log_info("====================================");

    while(1)
    {
		if ( get_server_order() == ORDER_STOP )
		{
			log_warn("Main Tcp Server receive exit singal");
			break;
		}
		else if ( get_server_order() == ORDER_PAUSE )
		{
			sleep(1);
			continue;
		}
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
		if ( client_fd < 0 )
		{
			if ( errno == EWOULDBLOCK || errno == EAGAIN ) {


                sleep(1);
                continue;  
            }
			else
			{
				log_error("accept executed failed");
			}
		}

		ws_session_t ws_session;
		ws_session.msg_handler = msg_handler_ptr;
		ws_session.client_fd = client_fd;
		ws_session.client_port = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ws_session.client_ip, 16);

        log_info("TCP Client %s is connected with port %hu", ws_session.client_ip, ws_session.client_port);

		// new server thread
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, (void *)&ws_session);
		// detach
        pthread_detach(tid);
        
    }
    close(sockfd);
    return SYS_OK;
}

