#ifndef SERVER_SOCKET_API_H
#define SERVER_SOCKET_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************/
/******** Include statements *********/
/*************************************/

#include <stdbool.h>
#include <openssl/ssl.h>

/*************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

/// @brief Socket Finite State Machine (FSM).
/// @param server_port Port number which the socket is going to be listening to.
/// @param max_conn_num Maximum amount of allowed connections.
/// @return < 0 if it failed.
int ServerSocketRun(int server_port, int max_conn_num, bool concurrent, bool secure, char* cert_path, char* key_path, int (*CustomSocketStateInteract)(int client_socket, bool secure, SSL** ssl));

/*************************************/

#ifdef __cplusplus
}
#endif

#endif