/************************************/
/******** Include statements ********/
/************************************/

#include <sys/socket.h>     // socket, bind, listen accept functions.
#include <arpa/inet.h>      // sockaddr_in, inet_addr
#include <unistd.h>         // Write socket.
#include <string.h>         // strcpy
#include <netinet/tcp.h>    // SO_KEEPALIVE
#include "ServerSocketUse.h"
#include "SeverityLog_api.h" // Severity Log.

/*************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/// @brief Create socket descriptor.
/// @param domain Use AF_INET if the socket is meant to be serving to another computer in the same net,
/// AF_LOCAL if the purpose is to communicate different processes within the same host.
/// @param type Use SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
/// @param protocol Use IPPROTO_IP (0) to use the internet protocol (IP).
/// @return  a file descriptor > 0 number if the socket descriptor was succesfully created, -1 if any error happened. 
int CreateSocketDescriptor(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

/// @brief Set socket options.
/// @param socket_desc Previously created socket descriptor.
/// @param reuse_address 1 if the address is meant to be forcefully used again, 0 otherwise.
/// @param reuse_port 1 if the port is meant to be forcefully used again, 0 otherwise.
/// @param keep_idle defines heartbeat frequency when it's receiving ACK packets from the other side (server is continuously sending empty packets).
/// @param keep_counter dictates how many unanswered heartbeats will indicate a broken connection.
/// @param keep_interval defines heartbeat frequency when there is no answer from the client's side.
/// @return < 0 if any error happened.
int SocketOptions(int socket_desc, int reuse_address, int reuse_port, int keep_idle, int keep_counter, int keep_interval)
{
    int socket_options;

    socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR , &reuse_address, sizeof(reuse_address    ));
    socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEPORT , &reuse_port   , sizeof(reuse_port       ));
    // socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPIDLE , &keep_idle    , sizeof(keep_idle        ));
    // socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPCNT  , &keep_counter , sizeof(keep_counter     ));
    // socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPINTVL, &keep_interval, sizeof(keep_interval    ));

    return socket_options;
}

/// @brief Prepare the sockaddr_in struct for the binding process to be completed properly.
/// @param address_family Address family the server is going to work with.
/// @param allowed_IPs Use INADDR_ANY to allow any IP, specify it otherwise.
/// @param listen_port The port the server is going to be later listening to.
/// @return A struct which contains all the parameters related to a socket descriptor. 
struct sockaddr_in PrepareForBinding(sa_family_t address_family, in_addr_t allowed_IPs, uint16_t listen_port)
{
    // Prepare the sockaddr_in structure for the binding process.
    struct sockaddr_in server;
    server.sin_family = address_family;     // IPv4.
    server.sin_addr.s_addr = allowed_IPs;   // Any address is allowed to connect to the socket.
    server.sin_port = htons(listen_port);   // The htons() function makes sure that numbers are stored in memory
                                            // in network byte order, which is with the most significant byte first.

    return server;
}

/// @brief Binds socket to previously specified address and port.
/// @param socket_desc Socket descriptor.
/// @param server Previously defined server parameters struct.
/// @return < 0 if the binding failed.
int BindSocket(int socket_desc, struct sockaddr_in server)
{
    // Bind: attach the socket descriptor to a particular IP and port.
    socklen_t file_desc_len = (socklen_t)sizeof(struct sockaddr_in);
    int bind_socket = bind(socket_desc, (struct sockaddr*)&server, file_desc_len);

    return bind_socket;
}

/// @brief Marks current socket as listener (server).
/// @param socket_desc Socket descriptor.
/// @param connections_number Maximum number of allowed connections.
/// @return < 0 if listening failed.
int SocketListen(int socket_desc, int connections_number)
{
    int socket_listen = listen(socket_desc, connections_number);

    return socket_listen;
}

/// @brief Accept an incoming connection.
/// @param socket_desc Previously created socket descriptor.
/// @return New socket instance, based on the socket descriptor, oriented to the client.
int SocketAccept(int socket_desc)
{
    // Wait for incoming connections.
    struct sockaddr_in client;
    socklen_t file_desc_len = (socklen_t)sizeof(struct sockaddr_in);

    LOG_INF(SERVER_SOCKET_MSG_WATING_INCOMING_CONN);

    int client_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&file_desc_len);

    LOG_INF(SERVER_SOCKET_MSG_CLIENT_ACCEPTED, inet_ntoa(client.sin_addr));

    int keep_alive = 5;
    int socket_options = setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE , &keep_alive, sizeof(keep_alive));

    return client_socket;
}

/// @brief Closes the socket.
/// @param client_socket ID of the socket that is meant to be closed.
/// @return < 0 if close failed.
int CloseSocket(int client_socket)
{
    // Close the socket.
    int close_socket = close(client_socket);

    return close_socket;
}

/*************************************/
