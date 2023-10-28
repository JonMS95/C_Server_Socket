/************************************/
/******** Include statements ********/
/************************************/

#include <stdlib.h>         // malloc, calloc, realloc, free.
#include <unistd.h>         // Fork if concurrency is accepted.
#include <string.h>         // strcpy
#include "ServerSocketUse.h"
#include "ServerSocketFSM.h"
#include "ServerSocketConcurrency.h"
#include "SeverityLog_api.h"

/************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/// @brief Create socket descriptor.
/// @return < 0 if it failed to create the socket.
int SocketStateCreate()
{
    int socket_desc = CreateSocketDescriptor(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if(socket_desc < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_CREATION_NOK);
    }
    else
    {
        LOG_INF(SERVER_SOCKET_MSG_CREATION_OK);
    }

    return socket_desc;
}

/// @brief Set socket options.
/// @param socket_desc Socket file descriptor.
/// @return < 0 if it failed to set options.
int SocketStateOptions(int socket_desc)
{
    int socket_options = SocketOptions(socket_desc, 1, 1, 50, 50, 50);

    if(socket_options < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_SET_OPTIONS_NOK);
    }
    else
    {
        LOG_INF(SERVER_SOCKET_MSG_SET_OPTIONS_OK);
    }

    return socket_options;
}

/// @brief Bind socket to an IP address and port.
/// @param socket_desc Socket file descriptor.
/// @param server_port The port which is going to be used to listen to incoming connections.
/// @return < 0 if it failed to bind.
int SocketStateBind(int socket_desc, int server_port)
{
    struct sockaddr_in server = PrepareForBinding(AF_INET, INADDR_ANY, server_port);

    int bind_socket = BindSocket(socket_desc, server);

    if(bind_socket < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_BIND_NOK);
    }
    else
    {
        LOG_INF(SERVER_SOCKET_MSG_BIND_OK);
    }
    
    return bind_socket;
}

/// @brief Listen to incoming connections.
/// @param socket_desc Socket file descriptor.
/// @param max_conn_num Maximum connections number.
/// @return < 0 if it failed to listen.
int SocketStateListen(int socket_desc, int max_conn_num)
{
    int listen = SocketListen(socket_desc, max_conn_num);

    if(listen < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_LISTEN_NOK);
    }
    else
    {
        LOG_INF(SERVER_SOCKET_MSG_LISTEN_OK);
    }
    
    return listen;
}

/// @brief Accept an incoming connection.
/// @param socket_desc Socket file descriptor.
/// @return < 0 if it failed to accept the connection.
int SocketStateAccept(int socket_desc)
{
    int client_socket = SocketAccept(socket_desc);

    if(client_socket < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_ACCEPT_NOK);
    }
    else
    {
        LOG_INF(SERVER_SOCKET_MSG_ACCEPT_OK);
    }
    
    return client_socket;
}

/// @brief Manage concurrent server instances.
/// @param client_socket Client socket descriptor.
/// @param server_instance_processes Array which contains PIDs of each running server socket instance.
/// @param max_conn_num Maximum amount of allowed clients (== server socket instances).
/// @return < 0 if new instance could not be created. Otherwise, 0 if current process is parent, 1 if it is child server isntance.
int SocketStateManageConcurrency(int client_socket, pid_t* server_instance_processes, int max_conn_num)
{
    // Check if any server socket instance may be created.
    int possible_new_instance = ServerSocketPossibleNewInstance(server_instance_processes, max_conn_num);

    if(possible_new_instance < 0)
    {
        LOG_WNG(SERVER_SOCKET_MSG_MAX_CONNS_REACHED);
        return -1;
    }

    int retfork = fork();

    if(retfork < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_CANNOT_FORK);
        return -2;
    }

    if(retfork > 0)
    {
        int new_server_instance_index = ServerSocketNewInstanceSpotIndex(server_instance_processes, max_conn_num);

        if(new_server_instance_index < 0)
        {
            return -1;
        }

        server_instance_processes[new_server_instance_index] = retfork;

        LOG_DBG(SERVER_SOCKET_MSG_NEW_PROCESS, retfork);

        return 0;
    }

    return 1;
}

int SocketStateRefuse(int client_socket)
{
    char refusal_msg[SERVER_SOCKET_LEN_MSG_REFUSE] = {};
    sprintf(refusal_msg, SERVER_SOCKET_MSG_REFUSE, SERVER_SOCKET_SECONDS_AFTER_REFUSAL);

    write(client_socket, refusal_msg, strlen(refusal_msg));

    shutdown(client_socket, SHUT_RDWR);

    sleep(SERVER_SOCKET_SECONDS_AFTER_REFUSAL);

    int close_socket = SocketStateClose(client_socket);

    return close_socket;
}

/// @brief Read data received in the socket.
/// @param client_socket Socket file descriptor.
/// @return <= 0 if it failed to read.
int SocketStateRead(int client_socket)
{
    int read = SocketRead(client_socket);

    return read;
}

/// @brief Close socket.
/// @param client_socket Socket file descriptor.
/// @return < 0 if it failed to close the socket.
int SocketStateClose(int client_socket)
{
    int close = CloseSocket(client_socket);
    
    if(close < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_CLOSE_NOK);
    }
    else
    {
        LOG_INF(SERVER_SOCKET_MSG_CLOSE_OK);
    }

    return close;    
}

/// @brief Socket Finite State Machine (FSM).
/// @param server_port Port number which the socket is going to be listening to.
/// @param max_conn_num Maximum amount of allowed connections.
/// @return < 0 if it failed.
int ServerSocketRun(int server_port, int max_conn_num, bool concurrent)
{
    SOCKET_FSM socket_fsm = CREATE_FD;
    int socket_desc;
    int client_socket;
    pid_t* server_instances = (pid_t*)calloc(max_conn_num, sizeof(pid_t));

    while(1)
    {
        switch (socket_fsm)
        {
            case CREATE_FD:
            {
                socket_desc = SocketStateCreate();
                if(socket_desc >= 0)
                    socket_fsm = OPTIONS;
            }
            break;

            case OPTIONS:
            {
                if(SocketStateOptions(socket_desc) >= 0)
                    socket_fsm = BIND;
            }
            break;

            case BIND:
            {
                if(SocketStateBind(socket_desc, server_port) >= 0)
                    socket_fsm = LISTEN;
            }
            break;

            case LISTEN:
            {
                if(SocketStateListen(socket_desc, max_conn_num) >= 0)
                    socket_fsm = ACCEPT;
            }
            break;

            case ACCEPT:
            {
                client_socket = SocketStateAccept(socket_desc);

                if(client_socket >= 0)
                {
                    if(!concurrent)
                    {
                        socket_fsm = READ;
                        continue;
                    }

                    socket_fsm = MANAGE_CONCURRENCY;
                }
            }
            break;

            case MANAGE_CONCURRENCY:
            {
                int manage_concurrency = SocketStateManageConcurrency(client_socket, server_instances, max_conn_num);

                switch(manage_concurrency)
                {
                    case SERVER_SOCKET_ERR_REFUSE_CONN:
                    {
                        socket_fsm = REFUSE;
                    }
                    break;

                    case SERVER_SOCKET_SUCCESS_PARENT:
                    case SERVER_SOCKET_ERR_CANNOT_FORK:
                    {
                        socket_fsm = ACCEPT;
                    }
                    break;

                    case SERVER_SOCKET_SUCCESS_CHILD:
                    {
                        socket_fsm = READ;
                    }

                    default:
                    break;
                }
            }
            break;

            case REFUSE:
            {
                int refuse = SocketStateRefuse(client_socket);

                if(refuse >= 0)
                    socket_fsm = ACCEPT;
            }
            break;

            case READ:
            {
                if(SocketStateRead(client_socket) <= 0)
                {
                    if(concurrent)
                        socket_fsm = CLOSE;
                    else
                        socket_fsm = ACCEPT;
                }
            }
            break;

            case CLOSE:
            {
                SocketStateClose(client_socket);
                if(server_instances != NULL)
                    free(server_instances);

                return 0;
            }
            break;
            
            default:
            break;
        }
    }
}

/*************************************/
