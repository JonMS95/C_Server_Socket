#ifndef SERVER_SOCKET_MANAGE_THREADS_H
#define SERVER_SOCKET_MANAGE_THREADS_H

/************************************/
/******** Include statements ********/
/************************************/

#include <stdbool.h>
#include <openssl/ssl.h>

/************************************/

/************************************/
/******* Function prototypes ********/
/************************************/

int SocketSetupThreads(int max_conn_num, bool secure, bool non_blocking, int (*interact_fn)(int client_socket));
int SocketLaunchServerInstance(int client_socket);
int SocketFreeThreadsResources(void);
SSL** SocketGetCurrentThreadSSLObj(void);

/************************************/

#endif