#ifndef SERVER_SOCKET_FSM_H
#define SERVER_SOCKET_FSM_H

#include "ServerSocket_api.h"

typedef enum
{
    CREATE_FD = 0   ,
    OPTIONS         ,
    BIND            ,
    LISTEN          ,
    ACCEPT          ,
    READ            ,
    CLOSE           ,

} SOCKET_FSM;

/// @brief Create socket descriptor.
/// @return < 0 if it failed to create the socket.
int SocketStateCreate();

/// @brief Set socket options.
/// @param socket_desc Socket file descriptor.
/// @return < 0 if it failed to set options.
int SocketStateOptions(int socket_desc);

/// @brief Bind socket to an IP address and port.
/// @param socket_desc Socket file descriptor.
/// @param server_port The port which is going to be used to listen to incoming connections.
/// @return < 0 if it failed to bind.
int SocketStateBind(int socket_desc, int server_port);

/// @brief Listen to incoming connections.
/// @param socket_desc Socket file descriptor.
/// @param max_conn_num Maximum connections number.
/// @return < 0 if it failed to listen.
int SocketStateListen(int socket_desc, int max_conn_num);

/// @brief Accept an incoming connection.
/// @param socket_desc Socket file descriptor.
/// @return < 0 if it failed to accept the connection.
int SocketStateAccept(int socket_desc);

/// @brief Read data received in the socket.
/// @param new_socket Socket file descriptor.
/// @return <= 0 if it failed to read.
int SocketStateRead(int new_socket);

/// @brief Close socket.
/// @param new_socket Socket file descriptor.
/// @return < 0 if it failed to close the socket.
int SocketStateClose(int new_socket);

#endif