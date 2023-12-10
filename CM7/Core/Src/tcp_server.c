/**
 * @file tcp_server.c
 * @brief creat tcp server through RAW API
 * @author Li Ming <Lee.dhucst@gmail.com>
 * @url http://blog.csdn.net/lim680
 * @version V1.0
 * @date 2014-04-19
 * @Copyright (c) DHU, CST.  All Rights Reserved
 */

#include "tcp_server.h"
#include "string.h"

#define DEBUG_LWIP   1

#define lwip_log(...)  \
	do{\
		if(DEBUG_LWIP) {printf(__VA_ARGS__);}\
		else {}\
	}while(0)


static struct tcp_server server;
static const char *resp = "connect stm32 server successfully \r\n";

static err_t tcp_server_accept(void *arg, struct tcp_pcb *new_pcb, err_t err);
static err_t tcp_server_receive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_server_error(void *arg, err_t err);

//https://github.com/freereaper/stm32_lwip/blob/master/USER/tcp_server.c
//https://github.com/ganioc/ruffnet-fx/blob/master/Src/mytcp.c

void tcp_server_init(void)
{
	err_t status = ERR_OK;

	server.tcp_server_pcb = tcp_new();
	server.server_port = 5000;
	server.accept = tcp_server_accept;
	server.receive = tcp_server_receive;
	server.error  = tcp_server_error;
	if(server.tcp_server_pcb != NULL) {
		status = tcp_bind(server.tcp_server_pcb, IP_ADDR_ANY, server.server_port);
		if(status == ERR_OK) {
			server.tcp_server_pcb = tcp_listen(server.tcp_server_pcb);
			tcp_accept(server.tcp_server_pcb, server.accept);
			lwip_log("tcp server init ok \r\n");
		}
	}
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *new_pcb, err_t err)
{
	lwip_log("server accept \r\n");
	tcp_write(new_pcb, resp, strlen(resp), 1);
	tcp_err(new_pcb, server.error);
	tcp_recv(new_pcb, server.receive);

	return ERR_OK;
}

u8_t tcp_rcv[1024];
int tcp_rcv_idx = 0;
static err_t tcp_server_receive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
//	struct tcp_server_struct *es;
//    err_t ret_err;
//	es = (struct tcp_server_struct *)arg;
//	es->p;

	if(p != NULL) {

		memcpy(tcp_rcv, p->payload, p->tot_len);
		tcp_rcv[p->tot_len] = 0;
		lwip_log("get: I:%d N:%d \r\n", tcp_rcv_idx++, p->tot_len);
		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p);

		tcp_write(tpcb, "rsp:", 4, 1);
		tcp_write(tpcb, tcp_rcv, p->tot_len, 1);
	}
	else if(err == ERR_OK) {
		printf("tcp client closed\r\n");
		tcp_recved(tpcb, p->tot_len);
		return tcp_close(tpcb);
	}
	else{
		lwip_log("get: fail \r\n");
	}

	return ERR_OK;
}

static void tcp_server_error(void *arg, err_t err)
{


}
