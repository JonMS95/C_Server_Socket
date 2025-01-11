#ifndef SERVER_SOCKET_MANAGE_THREADS_H
#define SERVER_SOCKET_MANAGE_THREADS_H

/************************************/
/******** Include statements ********/
/************************************/

#include <pthread.h>
#include <stdbool.h>

/************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_MANAGE_THREADS_SUCCESS            0
#define SERVER_SOCKET_THREADS_ALLOCATION_FAILURE        -1
#define SERVER_SOCKET_MANAGE_THREAD_CREATION_FAILURE    -2
#define SERVER_SOCKET_DATA_CLEAN_FAILURE                -3

/************************************/

/************************************/
/******* Function prototypes ********/
/************************************/

int SocketSetupThreads(int max_conn_num, bool secure, bool non_blocking, int (*interact_fn)(int client_socket));
int SocketStateManageThreads(int client_socket);
int SocketKillAllThreads();

/************************************/

#endif