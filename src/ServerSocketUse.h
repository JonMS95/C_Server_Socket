#ifndef SERVER_SOCKET_USE_H
#define SERVER_SOCKET_USE_H

/************************************/
/******** Include statements ********/
/************************************/

#include <arpa/inet.h>
#include <stdbool.h>

/************************************/

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
int BindSocket( int socket_desc             ,
                uint16_t listen_port        ,
                sa_family_t address_family  ,
                in_addr_t allowed_IPs       );
int SocketListen(int socket_desc, int connections_number);
int SocketSetNonBlocking(int socket_fd);
int SocketUnsetNonBlocking(int socket_fd);
int SocketAccept(int socket_desc, bool non_blocking);
int CloseSocket(int client_socket);

/*************************************/

#endif