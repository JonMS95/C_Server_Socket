#ifndef SERVER_SOCKET_USE_H
#define SERVER_SOCKET_USE_H

/************************************/
/******** Include statements ********/
/************************************/

#include <sys/socket.h>
#include <arpa/inet.h>

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SERVER_SOCKET_LEN_MSG_GREETING          100
#define SERVER_SOCKET_LEN_RX_BUFFER             256     // RX buffer size.
#define SERVER_SOCKET_MSG_GREETING              "Hello client!\r\nYour IP address is: %s\r\n"
#define SERVER_SOCKET_MSG_CLIENT_ACCEPTED       "Connection accepted. Client's IP address: <%s>"
#define SERVER_SOCKET_MSG_CLIENT_DISCONNECTED   "Client with IP <%s> disconnected."
#define SERVER_SOCKET_MSG_DATA_READ_FROM_CLIENT "Data read from RX buffer: <%s>"

/***********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

int CreateSocketDescriptor(int domain, int type, int protocol);
int SocketOptions(int socket_desc, int reuse_address, int reuse_port, int keep_idle, int keep_counter, int keep_interval);
struct sockaddr_in PrepareForBinding(sa_family_t address_family, in_addr_t allowed_IPs, uint16_t listen_port);
int BindSocket(int socket_desc, struct sockaddr_in server);
int SocketListen(int socket_desc, int connections_number);
int SocketAccept(int socket_desc);
int SocketRead(int new_socket);
void SocketDisplayOnConsole(int bytes_read, char* rx_buffer);
int CloseSocket(int new_socket);

/*************************************/

#endif