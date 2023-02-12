#include "socket_use.h"

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    int socket_desc = CreateSocketDescriptor(AF_INET, SOCK_STREAM, IPPROTO_IP);

    int socket_options = SocketOptions(socket_desc, 1, 1, 1, 5, 5); 

    struct sockaddr_in server = PrepareForBinding(AF_INET, INADDR_ANY, TEST_PORT);

    int bind_socket = BindSocket(socket_desc, server);

    int listen = SocketListen(socket_desc, CONN_NUM);

    int new_socket = SocketAccept(socket_desc);

    int read = SocketRead(new_socket);

    int close = CloseSocket(new_socket);

    return 0;
}
