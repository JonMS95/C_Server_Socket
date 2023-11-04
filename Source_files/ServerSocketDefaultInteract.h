#ifndef SERVER_SOCKET_DEFAULT_INTERACT_H
#define SERVER_SOCKET_DEFAULT_INTERACT_H

/************************************/
/******** Include statements ********/
/************************************/

#include <stdbool.h>
#include <openssl/ssl.h>

/*************************************/

/// @brief Reads from client. No perror statement exists within this function's definition, as read function can return something <= 0 if client gets disconnected.
/// @param client_socket Socket instance, based on the previously defined socket descriptor. 
/// @return <= 0 if read failed. The state where something > 0 is returned should never be reached by now.
int SocketDefaultInteractFn(int client_socket, bool secure, SSL** ssl);

#endif