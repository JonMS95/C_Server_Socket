#ifndef SERVER_SOCKET_DEFAULT_INTERACT_H
#define SERVER_SOCKET_DEFAULT_INTERACT_H

/************************************/
/******** Include statements ********/
/************************************/

#include <stdbool.h>
#include <openssl/ssl.h>

/*************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

int SocketDefaultInteractFn(int client_socket);

/*************************************/

#endif