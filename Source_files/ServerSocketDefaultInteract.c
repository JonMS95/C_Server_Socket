/************************************/
/******** Include statements ********/
/************************************/

#include <netinet/in.h>     // INET_ADDRSTRLEN.
#include <fcntl.h>          // Set socket flags.
#include <openssl/ssl.h>
#include <arpa/inet.h>      // sockaddr_in, inet_addr
#include <unistd.h>
#include "SeverityLog_api.h"

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_LEN_RX_BUFFER             256     // RX buffer size.
#define SERVER_SOCKET_LEN_MSG_GREETING          100

#define SERVER_SOCKET_MSG_CLIENT_DISCONNECTED   "Client with IP <%s> disconnected."
#define SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT "Data read from RX buffer: <%s>"
#define SERVER_SOCKET_MSG_GREETING              "Hello client!\r\nYour IP address is: %s\r\n"

/************************************/

/// @brief Reads from client. No perror statement exists within this function's definition, as read function can return something <= 0 if client gets disconnected.
/// @param client_socket Socket instance, based on the previously defined socket descriptor. 
/// @return <= 0 if read failed. The state where something > 0 is returned should never be reached by now.
int SocketDefaultInteractFn(int client_socket, bool secure, SSL** ssl)
{
    //////////////////////////////////////////////////////////////////////
    // START CUSTOM INTERACT FUNCTION
    //////////////////////////////////////////////////////////////////////

    ////////////////////////////////
    // GET THE CLIENT IP ADDR FIRST.
    ////////////////////////////////

    // Get client IP first, then Log it's IP address.
    char client_IP_addr[INET_ADDRSTRLEN] = {};
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    getpeername(client_socket, (struct sockaddr*)&client, &client_len);
    inet_ntop(AF_INET, &client.sin_addr, client_IP_addr, INET_ADDRSTRLEN);

    ///////////////////////////////////////////////////////////////////////////
    // END CLIENT IP ADDR RETRIEVAL.
    ///////////////////////////////////////////////////////////////////////////

    // Fisrt, try to get socket's current flag set.
    int flags = fcntl(client_socket, F_GETFL, 0);
    if(flags < 0)
    {
        LOG_ERR("ERROR WHILE GETTING SOCKET FLAGS.");
        return -1;
    }

    // Then, set the O_NONBLOCK flag (which, as the name suggests, makes the socket non-blocking).
    flags |= O_NONBLOCK;
    if(fcntl(client_socket, F_SETFL, flags) < 0)
    {
        LOG_ERR("ERROR WHILE setting O_NONBLOCK flag.");
        return -1;
    }

    char rx_buffer[SERVER_SOCKET_LEN_RX_BUFFER];
    memset(rx_buffer, 0, sizeof(rx_buffer));

    ssize_t read_from_socket = -1;

    while(read_from_socket > 0)
    {
        if(!secure)
        {
            read_from_socket = read(client_socket, rx_buffer, sizeof(rx_buffer));
        }
        else
        {
            read_from_socket = SSL_read(*ssl, rx_buffer, sizeof(rx_buffer));
        }

        // Check if the client is still connected, or if no data has been received.
        if(read_from_socket == 0)
        {
            LOG_WNG(SERVER_SOCKET_MSG_CLIENT_DISCONNECTED, client_IP_addr);
            break;
        }

        if(read_from_socket < 0)
            break;

        // If any data has been received, then display it on console.
        // Remove possible ending new line and carriage return characters first
        if(strlen(rx_buffer) > 0 && rx_buffer[strlen(rx_buffer) - 1] == '\n')
            rx_buffer[strlen(rx_buffer) - 1] = 0;

        if(strlen(rx_buffer) > 0 && rx_buffer[strlen(rx_buffer) - 1] == '\r')
            rx_buffer[strlen(rx_buffer) - 1] = 0;

        if(strlen(rx_buffer) > 0)
        {
            LOG_INF(SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT, rx_buffer);
        }

        // Clean the buffer after reading.
        memset(rx_buffer, 0, read_from_socket);
    }

    // Send a message to the client as soon as it is accepted.
    char greeting[SERVER_SOCKET_LEN_MSG_GREETING + 1];
    memset(greeting, 0, sizeof(greeting));
    sprintf(greeting, SERVER_SOCKET_MSG_GREETING, client_IP_addr);
    
    if(!secure)
        write(client_socket, greeting, sizeof(greeting));
    else
        SSL_write(*ssl, greeting, sizeof(greeting));

    //////////////////////////////////////////////////////////////////////
    // END CUSTOM INTERACT FUNCTION
    //////////////////////////////////////////////////////////////////////

    return 0;
}
