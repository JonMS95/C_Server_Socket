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

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @param secure True if TLS security is wanted, false otherwise.
/// @param ssl SSL data.
/// @return < 0 if any error happened, 0 otherwise.
int SocketDefaultInteractFn(int client_socket);

/*************************************/

#endif