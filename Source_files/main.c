#include <stdio.h>
#include <stdlib.h>
#include "socket_use.h"

#define ARGV_PORT_IDX           1
#define ARGV_MAX_CONN_NUM_IDX   2
#define MIN_CONN_NUM            1
#define MAX_CONN_NUM            3
#define MIN_PORT_VALUE          49152
#define MAX_PORT_VALUE          65535

#define MAX_ERROR_MSG_LEN       100

#define ERR_NO_PORT_PROVIDED        -1
#define ERR_PORT_OUT_OF_RANGE       -2
#define ERR_NO_MAX_CONN_PROVIDED    -3
#define ERR_MAX_CONN_OUT_OF_RANGE   -4

/// @brief Parse server's port.
/// @param argument_list list of argument that is passed through the command line.
/// @return < 0 if any error happened, port number otherwise.
int ParsePort(char** argument_list)
{
    if(argument_list[ARGV_PORT_IDX] == NULL)
    {
        printf("\033[0;31m");
        printf("[ERROR] No port number was provided\r\n");
        printf("\033[0m");
        return ERR_NO_PORT_PROVIDED;
    }
    
    int server_port = atoi(argument_list[ARGV_PORT_IDX]);

    if(server_port < MIN_PORT_VALUE || server_port > MAX_PORT_VALUE)
    {
        printf("\033[0;31m");
        printf("[ERROR] Address out of acceptable range (%d-%d).\r\n", MIN_PORT_VALUE, MAX_PORT_VALUE);
        printf("\033[0m");
        return ERR_PORT_OUT_OF_RANGE;
    }

    return server_port;
}

/// @brief Parse the maximum number of connections that the server can accept.
/// @param argument_list list of argument that is passed through the command line.
/// @return < 0 if any error happened, maximum number of connections admitted by the server otherwise.
int ParseConnNum(char** argument_list)
{
    if(argument_list[ARGV_MAX_CONN_NUM_IDX] == NULL)
    {
        printf("\033[0;31m");
        printf("[ERROR] No maximum number of conections argument provided.\r\n");
        printf("\033[0m");
        return ERR_NO_MAX_CONN_PROVIDED;
    }

    int max_conn_num = atoi(argument_list[ARGV_MAX_CONN_NUM_IDX]);

    if(max_conn_num < 1 || max_conn_num > 3)
    {
        printf("\033[0;31m");
        printf("[ERROR] Maximum number of connections out of acceptable range (%d-%d)\r\n", MIN_CONN_NUM, MAX_CONN_NUM);
        printf("\033[0m");
        return ERR_MAX_CONN_OUT_OF_RANGE;
    }

    return max_conn_num;
}

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    int server_port = ParsePort(argv);
    if(server_port < 0)
    {
        exit(EXIT_FAILURE);
    }

    int max_conn_num = ParseConnNum(argv);
    if(max_conn_num < 0)
    {
        exit(EXIT_FAILURE);
    }

    int socket_desc = CreateSocketDescriptor(AF_INET, SOCK_STREAM, IPPROTO_IP);

    int socket_options = SocketOptions(socket_desc, 1, 1, 1, 5, 5);

    struct sockaddr_in server = PrepareForBinding(AF_INET, INADDR_ANY, server_port);

    int bind_socket = BindSocket(socket_desc, server);

    int listen = SocketListen(socket_desc, max_conn_num);

    int new_socket = SocketAccept(socket_desc);

    int read = SocketRead(new_socket);

    int close = CloseSocket(new_socket);

    return 0;
}
