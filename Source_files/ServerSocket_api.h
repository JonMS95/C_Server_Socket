#ifndef SERVER_SOCKET_API_H
#define SERVER_SOCKET_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************/
/******** Include statements *********/
/*************************************/

#include <stdbool.h>

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

/*************************************/

/*************************************/
/********* Define statements *********/
/*************************************/

#define C_SERVER_SOCKET_API __attribute__((visibility("default")))

/*************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

/// @brief Get client's IP address.
/// @param client_socket Client socket.
/// @param client_IPv4 Target string to which client IPv4 address is meant to be copied.
C_SERVER_SOCKET_API void ServerSocketGetClientIPv4(int client_socket, char* client_IPv4);

/// @brief Reads from client socket.
/// @param client_socket Client socket.
/// @param rx_buffer Required RX buffer to read from socket.
/// @return > 0 equaling the amount of bytes read, 0 if client got disconnected, < 0 if any error happened.
C_SERVER_SOCKET_API int ServerSocketRead(int client_socket, char* rx_buffer, unsigned long rx_buffer_size);

#define SERVER_SOCKET_READ(client_socket, rx_buffer)                \
        ServerSocketRead(client_socket, rx_buffer, sizeof(rx_buffer))

/// @brief Writes to client socket.
/// @param client_socket Client socket.
/// @param tx_buffer Required TX buffer in which data to write is found.
/// @return > 0 equaling the amount of bytes written, 0 if client got disconnected, < 0 if any error happened.
C_SERVER_SOCKET_API int ServerSocketWrite(int client_socket, const char* tx_buffer, unsigned long tx_buffer_size);

#define SERVER_SOCKET_WRITE(client_socket, tx_buffer)                \
        ServerSocketWrite(client_socket, (const char*)tx_buffer, strlen(tx_buffer))

/// @brief Runs server socket.
/// @param server_port Port server is meant to be listening to.
/// @param max_conn_num Maximum number of connections.
/// @param concurrent Enable concurrent server instances.
/// @param non_blocking Tells whether or not is the socket meant to be non-blocking.
/// @param reuse_address Reuse address, does not hold the address after socket is closed.
/// @param reuse_port Reuse port, does not hold the port after socket is closed.
/// @param rx_timeout_usecs Receive timeout in microseconds.
/// @param secure Enable secure communication (TLS).
/// @param cert_path Path to server ceritificate.
/// @param key_path Path to server private key.
/// @param CustomSocketStateInteract Custom function to interact with client once connection is established.
/// @return 0 always, exit sending failure signal if SIGINT signal handler could not be properly set.
C_SERVER_SOCKET_API int ServerSocketRun(int             server_port                                     ,
                                        int             max_conn_num                                    ,
                                        bool            concurrent                                      ,
                                        bool            non_blocking                                    ,
                                        bool            reuse_address                                   ,
                                        bool            reuse_port                                      ,
                                        unsigned long   rx_timeout_usecs                                ,
                                        bool            secure                                          ,
                                        char*           cert_path                                       ,
                                        char*           pkey_path                                       ,
                                        int             (*CustomSocketStateInteract)(int client_socket) );

/*************************************/

#ifdef __cplusplus
}
#endif

#endif