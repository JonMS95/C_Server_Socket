/************************************/
/******** Include statements ********/
/************************************/

#include <netinet/in.h>     // INET_ADDRSTRLEN.
#include <openssl/ssl.h>
#include <unistd.h>
#include "SeverityLog_api.h"
#include "ServerSocket_api.h"

/*************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_LEN_RX_BUFFER             256     // RX buffer size.
#define SERVER_SOCKET_LEN_TX_BUFFER             256     // TX buffer size.

#define SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT "Data read from client: <%s>."
#define SERVER_SOCKET_MSG_GREETING              "Hello client! Your IP address is: <%s>."
#define SERVER_SOCKET_MSG_CLIENT_DISCONNECTED   "Client with IP <%s> disconnected."

/************************************/

/*************************************/
/**** Private function prototypes ****/
/*************************************/

static void ServerSocketShowReadData(char* rx_buffer);

/*************************************/

/// @brief Displays read data.
/// @param rx_buffer Reception buffer.
static void ServerSocketShowReadData(char* rx_buffer)
{
    if(strlen(rx_buffer) > 0 && rx_buffer[strlen(rx_buffer) - 1] == '\n')
        rx_buffer[strlen(rx_buffer) - 1] = 0;

    if(strlen(rx_buffer) > 0 && rx_buffer[strlen(rx_buffer) - 1] == '\r')
        rx_buffer[strlen(rx_buffer) - 1] = 0;

    if(strlen(rx_buffer) > 0)
    {
        LOG_INF(SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT, rx_buffer);
    }
}

/// @brief Reads from client, then sends a response.
/// @param client_socket Client socket.
/// @return < 0 if any error happened, 0 otherwise.
int SocketDefaultInteractFn(int client_socket)
{
    // Get client IP first, then Log it's IP address.
    char client_IP_addr[INET_ADDRSTRLEN] = {};
    ServerSocketGetClientIPv4(client_socket, client_IP_addr);

    char rx_buffer[SERVER_SOCKET_LEN_RX_BUFFER];
    memset(rx_buffer, 0, sizeof(rx_buffer));

    ssize_t read_from_socket = -1;
    bool something_read = false;

    while( (read_from_socket >= 0) || (something_read == false) )
    {
        read_from_socket = SERVER_SOCKET_READ(client_socket, rx_buffer);

        // Check if the client is still connected, or if no data has been received.
        if(read_from_socket == 0)
        {
            LOG_WNG(SERVER_SOCKET_MSG_CLIENT_DISCONNECTED, client_IP_addr);
            break;
        }

        // If no data has been found in the rx buffer but it has been found previously, then exit.
        // It is assumed that all the data that was meant to be read has already been read.
        if(read_from_socket < 0)
        {
            if(something_read)
                break;
            
            continue;
        }

        // If any data has been received, then set the something_read flag and display it on console.
        // Remove possible ending new line and carriage return characters first.
        something_read = true;
        
        ServerSocketShowReadData(rx_buffer);

        // Clean the buffer after reading.
        memset(rx_buffer, 0, read_from_socket);

        // Wait for a millisecond before trying to read again.
        usleep(1000);
    }

    // Send a message to the client as soon as it is accepted.
    char tx_buffer[SERVER_SOCKET_LEN_TX_BUFFER];
    memset(tx_buffer, 0, sizeof(tx_buffer));
    sprintf(tx_buffer, SERVER_SOCKET_MSG_GREETING, client_IP_addr);
    
    SERVER_SOCKET_WRITE(client_socket, tx_buffer);

    return 0;
}
