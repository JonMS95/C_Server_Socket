/************************************/
/******** Include statements ********/
/************************************/

#include <sys/socket.h>         // socket, bind, listen accept functions.
#include <arpa/inet.h>          // sockaddr_in, inet_addr
#include <unistd.h>             // Write socket.
#include <string.h>             // strcpy
#include <netinet/tcp.h>        // SO_KEEPALIVE
#include <fcntl.h>              // Set socket flags.
#include <sys/time.h>           // Set timeout
#include "ServerSocketUse.h"
#include "SeverityLog_api.h"

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
/// @param reuse_address true if the address is meant to be forcefully used again, false otherwise.
/// @param reuse_port true if the port is meant to be forcefully used again, false otherwise.
// /// @param keep_idle defines heartbeat frequency when it's receiving ACK packets from the other side (server is continuously sending empty packets).
// /// @param keep_counter dictates how many unanswered heartbeats will indicate a broken connection.
// /// @param keep_interval defines heartbeat frequency when there is no answer from the client's side.
// /// @param keep_alive Sends probes every specified time amount in order to keep the connection alive.
/// @param reuse_address Reuse address, does not hold the address after socket is closed.
/// @param reuse_port Reuse port, does not hold the port after socket is closed.
/// @param rx_timeout_usecs Receive timeout in microseconds.
/// @return < 0 if any error happened.
int SocketOptions(int socket_desc, bool reuse_address, bool reuse_port, unsigned long rx_timeout_secs, unsigned long rx_timeout_usecs)
{
    int socket_options;

    int reuse_address_int   = reuse_address == true ? 1 : 0;
    int reuse_port_int      = reuse_port    == true ? 1 : 0;

    socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR , &reuse_address_int, sizeof(reuse_address_int));
    socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEPORT , &reuse_port_int   , sizeof(reuse_port_int   ));

    // The ones below are not meant to be used by now. These options are related to keep-alive mechanism.
    // socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPIDLE , &keep_idle    , sizeof(keep_idle        ));
    // socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPCNT  , &keep_counter , sizeof(keep_counter     ));
    // socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPINTVL, &keep_interval, sizeof(keep_interval    ));
    // socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_KEEPALIVE , &keep_alive, sizeof(keep_alive));
    
    // // JMS TESTING: HTTP Server, not meant to be used along with the simple TCP server implementation.
    // // JMS TESTING: meant to be featured as new options, not only for the current function, but for ServerSocketRun and SocketStateOptions too.
    struct timeval set_timeout =
    {
        .tv_sec     = rx_timeout_secs ,
        .tv_usec    = rx_timeout_usecs,
    };
    socklen_t set_timeout_len = sizeof(set_timeout);

    socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, &set_timeout, set_timeout_len);

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

/// @brief Sets non-blocking flag to the target socket.
/// @param socket_fd Target socket.
/// @return < 0 if any error happened, 0 otherwise.
int SocketSetNonBlocking(int socket_fd)
{
    // First, try to get socket's current flag set.
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if(flags < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_ERR_GET_SOCKET_FLAGS);
        return flags;
    }

    // Then, set the O_NONBLOCK flag (which, as the name suggests, makes the socket non-blocking).
    flags |= O_NONBLOCK;
    if(fcntl(socket_fd, F_SETFL, flags) < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_ERR_SET_SOCKET_FLAGS);
        return flags;
    }

    return 0;
}

/// @brief Unsets non-blocking flag to the target socket.
/// @param socket_fd Target socket.
/// @return < 0 if any error happened, 0 otherwise.
int SocketUnsetNonBlocking(int socket_fd)
{
    // First, try to get socket's current flag set.
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if(flags < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_ERR_GET_SOCKET_FLAGS);
        return flags;
    }

    // Then, unset the O_NONBLOCK flag (which, as the name suggests, makes the socket non-blocking).
    flags &= ~O_NONBLOCK;
    if(fcntl(socket_fd, F_SETFL, flags) < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_ERR_SET_SOCKET_FLAGS);
        return flags;
    }

    return 0;
}

/// @brief Accept an incoming connection.
/// @param socket_desc Previously created socket descriptor.
/// @param non_blocking Tells whether or not is the socket meant to be non-blocking.
/// @return New socket instance, based on the socket descriptor, oriented to the client.
int SocketAccept(int socket_desc, bool non_blocking)
{
    // Wait for incoming connections.
    struct sockaddr_in client;
    socklen_t file_desc_len = (socklen_t)sizeof(struct sockaddr_in);

    // LOG_INF(SERVER_SOCKET_MSG_WATING_INCOMING_CONN);

    int client_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&file_desc_len);

    if(client_socket >= 0)
    {
        int set_non_blocking = 0;
        if(non_blocking)
            set_non_blocking = SocketSetNonBlocking(client_socket);
        
        if(set_non_blocking < 0)
            return set_non_blocking;
    }

    if(client_socket >= 0)
        LOG_INF(SERVER_SOCKET_MSG_CLIENT_ACCEPTED, inet_ntoa(client.sin_addr));

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
