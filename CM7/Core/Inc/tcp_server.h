#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

//#include "mysys.h"
#include "err.h"
#include "lwip/tcp.h"

struct tcp_server {
	struct tcp_pcb *tcp_server_pcb;
	u16_t server_port;

	err_t (*accept)(void *arg, struct tcp_pcb *new_pcb, err_t err);
	err_t (*receive)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
	void (*error)(void *arg, err_t err);
};

void tcp_server_init(void);
void processCmd();

#endif
