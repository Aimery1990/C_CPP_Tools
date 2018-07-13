/*
 * Socket_TCP.h
 *
 *  Created on: Feb 25, 2018
 *      Author: aimery
 */

#ifndef SOCKET_TCP_H_
#define SOCKET_TCP_H_

#define READY_FOR_RECV		"READY_FOR_RECV"

typedef enum{
	SOCKET_ERROR_ZERO,
	SOCKET_ERROR_UNIVERSE,
	SOCKET_ERROR_UNAVAILABLE,
	SOCKET_ERROR_TIMEOUT,
	SOCKET_ERROR_UNREACHABLE,
	SOCKET_ERROR_REPEATED,
	SOCKET_ERROR_RESERVED,
}SOCK_ERROR_RET;


typedef struct Sock_Client{
	unsigned id;
	int nfd;
	long timeStamp;

	struct Sock_Client * prv;
	struct Sock_Client * next;
}SOCK_CLIENT_T;

#ifdef __cplusplus__
extern  "C" {
#endif




#ifdef __cplusplus__
}
#endif



#endif /* SOCKET_TCP_H_ */
