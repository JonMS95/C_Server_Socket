#ifndef SOCKET_USE_H
#define SOCKET_USE_H

#include <sys/socket.h>
#include <arpa/inet.h>

#define CONN_NUM        3       // Maximum number of concurrent connections that the socket may attend to.
#define IP_ADDR_SIZE    15      // IP address string size.
#define GREETING_SIZE   100
#define RX_BUFFER_SIZE  256     // RX buffer size.

int CreateSocketDescriptor(int domain, int type, int protocol);
int SocketOptions(int socket_desc, int reuse_address, int reuse_port, int keep_idle, int keep_counter, int keep_interval);
struct sockaddr_in PrepareForBinding(sa_family_t address_family, in_addr_t allowed_IPs, uint16_t listen_port);
int BindSocket(int socket_desc, struct sockaddr_in server);
int SocketListen(int socket_desc, int connections_number);
int SocketAccept(int socket_desc);
int SocketRead(int new_socket);
int CloseSocket(int new_socket);

#endif