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
#define SERVER_SOCKET_MANAGE_THREAD_NO_FREE_SPOTS       -3
#define SERVER_SOCKET_DATA_CLEAN_FAILURE                -4

#define SERVER_SOCKET_MSG_ERR_CREATE_THREAD         "An error ocurred while creating thread: <%s>"
#define SERVER_SOCKET_MSG_ERR_NO_FREE_SPOTS         "Maximum number of connections has already been reached: <%d>"
#define SERVER_SOCKET_MSG_THREAD_CREATION_SUCCESS   "Successfully created thread for client socket <%d> (TID: <%lu>)"

#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK     "SSL handshake failed."
#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK      "SSL handshake succeeded."

#define SERVER_SOCKET_MSG_CLOSE_NOK             "An error happened while closing socket <%d>."
#define SERVER_SOCKET_MSG_CLOSE_OK              "Socket <%d> successfully closed."

/************************************/

/************************************/
/******* Function prototypes ********/
/************************************/

int SocketSetupThreads(int max_conn_num, bool secure, bool non_blocking, int (*interact_fn)(int client_socket));
int SocketLaunchServerInstance(int client_socket);
int SocketFreeThreadsResources();

/************************************/

#endif