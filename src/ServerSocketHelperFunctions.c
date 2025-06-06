/************************************/
/******** Include statements ********/
/************************************/

#include <arpa/inet.h>      // sockaddr_in, inet_addr
#include <stdlib.h>         // EXIT_FAILURE
#include <unistd.h>
#include "ServerSocketSSL.h"
#include "SeverityLog_api.h"
#include "ServerSocket_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define getName(var)  #var

#define SERVER_SOCKET_HELPER_MSG_INSUFFICIENT_RX_BUFFER_SIZE    "Could not read from buffer due to insufficient RX buffer size."
#define SERVER_SOCKET_HELPER_MSG_RX_BUFFER_NAME                 "RX buffer variable name: <%s>, declared size: <%d>."
#define SERVER_SOCKET_HELPER_MSG_INSUFFICIENT_TX_BUFFER_SIZE    "Could not write to buffer due to insufficient TX buffer size."
#define SERVER_SOCKET_HELPER_MSG_TX_BUFFER_NAME                 "TX buffer variable name: <%s>, declared size: <%d>."

#define SERVER_SOCKET_HELPER_ERR_INSUFFICIENT_RX_BUFFER_SIZE    -1
#define SERVER_SOCKET_HELPER_ERR_INSUFFICIENT_TX_BUFFER_SIZE    -2

/***********************************/

/// @brief Get client's IP address.
/// @param client_socket Client socket.
/// @param client_IPv4 Target string to which client IPv4 address is meant to be copied.
void ServerSocketGetClientIPv4(int client_socket, char* client_IPv4)
{
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    getpeername(client_socket, (struct sockaddr*)&client, &client_len);
    inet_ntop(AF_INET, &client.sin_addr, client_IPv4, INET_ADDRSTRLEN);
}

/// @brief Reads from target client socket using specified buffer and size.
/// @param client_socket Client socket.
/// @param rx_buffer RX buffer.
/// @param rx_buffer_size RX buffer size.
/// @return Amount of bytes read if read > 0, < 0 if no data could be read, 0 if client got disconnected.
int ServerSocketRead(int client_socket, char* rx_buffer, unsigned long rx_buffer_size)
{
    int read_from_socket;

    if(rx_buffer_size == 0)
    {
        SVRTY_LOG_ERR(SERVER_SOCKET_HELPER_MSG_INSUFFICIENT_RX_BUFFER_SIZE);
        SVRTY_LOG_DBG(SERVER_SOCKET_HELPER_MSG_RX_BUFFER_NAME, getName(rx_buffer), rx_buffer_size);
        exit(EXIT_FAILURE);
    }

    if(!ServerSocketIsSecure())
        read_from_socket = read(client_socket, rx_buffer, rx_buffer_size);
    else
        read_from_socket = ServerSocketSSLRead(rx_buffer, rx_buffer_size);
    
    return read_from_socket;
}

/// @brief Writes to target client socket using specified buffer and size.
/// @param client_socket Client socket to write to.
/// @param tx_buffer TX buffer.
/// @param tx_buffer_size TX buffer size.
/// @return < 0 if any error happens, number of bytes sent otherwise.
int ServerSocketWrite(int client_socket, const char* tx_buffer, unsigned long tx_buffer_size)
{
    int write_to_socket;

    if(tx_buffer_size == 0)
    {
        SVRTY_LOG_ERR(SERVER_SOCKET_HELPER_MSG_INSUFFICIENT_TX_BUFFER_SIZE);
        SVRTY_LOG_DBG(SERVER_SOCKET_HELPER_MSG_TX_BUFFER_NAME, getName(tx_buffer), tx_buffer_size);
        exit(EXIT_FAILURE);
    }

    if(!ServerSocketIsSecure())
        write_to_socket = write(client_socket, tx_buffer, tx_buffer_size);
    else
        write_to_socket = ServerSocketSSLWrite(tx_buffer, tx_buffer_size);
    
    return write_to_socket;
}
