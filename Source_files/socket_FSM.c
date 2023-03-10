#include "socket_use.h"
#include "../Dependency_files/Header_files/SeverityLog_api.h"
#include "socket_FSM.h"

int SocketStateCreate()
{
    int socket_desc = CreateSocketDescriptor(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if(socket_desc < 0)
    {
        SeverityLog(SVRTY_LVL_ERR, "Socket file descriptor creation failed.\r\n");
    }
    else
    {
        SeverityLog(SVRTY_LVL_INF, "Socket file descriptor created.\r\n");
    }

    return socket_desc;
}

int SocketStateOptions(int socket_desc)
{
    int socket_options = SocketOptions(socket_desc, 1, 1, 50, 50, 50);

    if(socket_options < 0)
    {
        SeverityLog(SVRTY_LVL_ERR, "Failed to set socket options");
    }
    else
    {
        SeverityLog(SVRTY_LVL_INF, "Successfully set socket options.\r\n");
    }

    return socket_options;
}

int SocketStateBind(int socket_desc, int server_port)
{
    struct sockaddr_in server = PrepareForBinding(AF_INET, INADDR_ANY, server_port);

    int bind_socket = BindSocket(socket_desc, server);

    if(bind_socket  < 0)
    {
        SeverityLog(SVRTY_LVL_ERR, "Socket binding failed.\r\n");
    }
    else
    {
        SeverityLog(SVRTY_LVL_INF, "Socket file descriptor binded.\r\n");
    }
    
    return bind_socket;
}

int SocketStateListen(int socket_desc, int max_conn_num)
{
    int listen = SocketListen(socket_desc, max_conn_num);

    if(listen < 0)
    {
        SeverityLog(SVRTY_LVL_ERR, "Socket listen failed.\r\n");
    }
    else
    {
        SeverityLog(SVRTY_LVL_INF, "Socket listen succeed.\r\n");
    }
    
    return listen;
}

int SocketStateAccept(int socket_desc)
{
    int new_socket = SocketAccept(socket_desc);

    if(new_socket < 0)
    {
        SeverityLog(SVRTY_LVL_ERR, "Accept failed.\r\n");
    }
    else
    {
        SeverityLog(SVRTY_LVL_INF, "Accept succeed.\r\n");
    }
    
    return new_socket;
}

int SocketStateRead(int new_socket)
{
    int read = SocketRead(new_socket);

    if(read <= 0)
    {
        SeverityLog(SVRTY_LVL_WNG, "Client disconnected.\r\n");
    }

    return read;
}

int SocketStateClose(int new_socket)
{
    int close = CloseSocket(new_socket);
    
    if(close < 0)
    {
        SeverityLog(SVRTY_LVL_ERR, "An error happened while closing the socket");
    }
    else
    {
        SeverityLog(SVRTY_LVL_INF, "Socket successfully closed.\r\n");
    }

    return close;    
}

    // CREATE_FD = 0   ,
    // OPTIONS         ,
    // BIND            ,
    // LISTEN          ,
    // ACCEPT          ,
    // READ            ,
    // CLOSE           ,

// int SocketStateCreate()
// int SocketStateOptions(int socket_desc)
// int SocketStateBind(int socket_desc, int server_port)
// int SocketStateListen(int socket_desc, int max_conn_num)
// int SocketStateAccept(int socket_desc)
// int SocketStateRead(int new_socket)
// int SocketStateClose(int new_socket)


int SocketFSM(int server_port, int max_conn_num)
{
    SOCKET_FSM socket_fsm = CREATE_FD;
    int socket_desc;
    int new_socket;

    while(1)
    {
        switch (socket_fsm)
        {
            case CREATE_FD:
            {
                socket_desc = SocketStateCreate();
                if(socket_desc < 0)
                {
                    socket_fsm = CREATE_FD;
                }
                else
                {
                    socket_fsm = OPTIONS;
                }
            }
            break;

            case OPTIONS:
            {
                if(SocketStateOptions(socket_desc) < 0)
                {
                    socket_fsm = CREATE_FD;
                }
                else
                {
                    socket_fsm = BIND;
                }
            }
            break;

            case BIND:
            {
                if(SocketStateBind(socket_desc, server_port) < 0)
                {
                    socket_fsm = CREATE_FD;
                }
                else
                {
                    socket_fsm = LISTEN;
                }
            }
            break;

            case LISTEN:
            {
                if(SocketStateListen(socket_desc, max_conn_num) < 0)
                {
                    socket_fsm = CREATE_FD;
                }
                else
                {
                    socket_fsm = ACCEPT;
                }
            }
            break;

            case ACCEPT:
            {
                if(SocketStateAccept(socket_desc) < 0)
                {
                    socket_fsm = LISTEN;
                }
                else
                {
                    socket_fsm = READ;
                }
            }
            break;

            case READ:
            {
                if(SocketStateRead(new_socket) < 0)
                {
                    socket_fsm = LISTEN;
                }
                else
                {
                    socket_fsm = CLOSE;
                }
            }
            break;

            case CLOSE:
            {
                if(SocketStateClose(new_socket) < 0)
                {
                    socket_fsm = CLOSE;
                }
                else
                {
                    // socket_fsm = LISTEN;
                    return -1;
                }
            }
            break;
            
            default:
            break;
        }
    }
}