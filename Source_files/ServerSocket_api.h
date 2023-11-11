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

/// @brief Runs server socket.
/// @param server_port Port server is meant to be listening to.
/// @param max_conn_num Maximum number of connections.
/// @param concurrent Enable concurrent server instances.
/// @param secure Enable secure communication (TLS).
/// @param cert_path Path to server ceritificate.
/// @param key_path Path to server private key.
/// @param CustomSocketStateInteract Custom function to interact with client once connection is established.
/// @return 0 always, exit sending failure signal if SIGINT signal handler could not be properly set.
int ServerSocketRun(int server_port, int max_conn_num, bool concurrent, bool secure, char* cert_path, char* key_path, int (*CustomSocketStateInteract)(int client_socket, bool secure, SSL** ssl));

/*************************************/

#ifdef __cplusplus
}
#endif

#endif