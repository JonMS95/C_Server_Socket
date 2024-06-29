#ifndef SERVER_SOCKET_USE_H
#define SERVER_SOCKET_USE_H

/************************************/
/******** Include statements ********/
/************************************/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SERVER_SOCKET_MSG_WATING_INCOMING_CONN  "Waiting for an incoming connection to happen."
#define SERVER_SOCKET_MSG_CLIENT_ACCEPTED       "Connection accepted. Client's IP address: <%s>"
#define SERVER_SOCKET_MSG_ERR_GET_SOCKET_FLAGS  "Error while getting socket flags."
#define SERVER_SOCKET_MSG_ERR_SET_SOCKET_FLAGS  "Error while setting O_NONBLOCK flag."

#define SERVER_SOCKET_1_SEC_IN_USECS            1000000

/***********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

int CreateSocketDescriptor(int domain, int type, int protocol);
int SocketOptions(  int             socket_desc     ,
                    bool            reuse_address   ,
                    bool            reuse_port      ,
                    unsigned long   rx_timeout_secs ,
                    unsigned long   rx_timeout_usecs,
                    unsigned long   tx_timeout_secs ,
                    unsigned long   tx_timeout_usecs);
struct sockaddr_in PrepareForBinding(sa_family_t address_family, in_addr_t allowed_IPs, uint16_t listen_port);
int BindSocket(int socket_desc, struct sockaddr_in server);
int SocketListen(int socket_desc, int connections_number);
int SocketSetNonBlocking(int socket_fd);
int SocketUnsetNonBlocking(int socket_fd);
int SocketAccept(int socket_desc, bool non_blocking);
int CloseSocket(int client_socket);

/*************************************/

#endif