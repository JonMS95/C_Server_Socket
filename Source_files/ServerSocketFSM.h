#ifndef SERVER_SOCKET_FSM_H
#define SERVER_SOCKET_FSM_H

/************************************/
/******** Include statements ********/
/************************************/

#include "ServerSocket_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SERVER_SOCKET_MSG_CREATION_NOK      "Socket file descriptor creation failed."
#define SERVER_SOCKET_MSG_CREATION_OK       "Socket file descriptor created."
#define SERVER_SOCKET_MSG_SET_OPTIONS_NOK   "Failed to set socket options."
#define SERVER_SOCKET_MSG_SET_OPTIONS_OK    "Successfully set socket options."
#define SERVER_SOCKET_MSG_BIND_NOK          "Socket binding failed."
#define SERVER_SOCKET_MSG_BIND_OK           "Socket file descriptor binded."
#define SERVER_SOCKET_MSG_LISTEN_NOK        "Socket listen failed."
#define SERVER_SOCKET_MSG_LISTEN_OK         "Socket listen succeeded."
#define SERVER_SOCKET_MSG_ACCEPT_NOK        "Accept failed."
#define SERVER_SOCKET_MSG_ACCEPT_OK         "Accept succeeded."
#define SERVER_SOCKET_MSG_MAX_CONNS_REACHED "Cannot create a new server instance as maximum number of client connections have already been reached."
#define SERVER_SOCKET_MSG_CANNOT_FORK       "Cannot fork the current server socket server process."
#define SERVER_SOCKET_MSG_NEW_PROCESS       "Created new server socket instance in process with PID <%d>."
#define SERVER_SOCKET_MSG_REFUSE            "Connection refused by the server. Closing socket in %d seconds."
#define SERVER_SOCKET_MSG_CLOSE_NOK         "An error happened while closing the socket."
#define SERVER_SOCKET_MSG_CLOSE_OK          "Socket successfully closed."

#define SERVER_SOCKET_ERR_CANNOT_FORK       -2
#define SERVER_SOCKET_ERR_REFUSE_CONN       -1
#define SERVER_SOCKET_SUCCESS_PARENT        0
#define SERVER_SOCKET_SUCCESS_CHILD         1

#define SERVER_SOCKET_LEN_MSG_REFUSE        100

#define SERVER_SOCKET_SECONDS_AFTER_REFUSAL 0

/************************************/

/**********************************/
/******** Type definitions ********/
/**********************************/

typedef enum
{
    CREATE_FD = 0       ,
    OPTIONS             ,
    BIND                ,
    LISTEN              ,
    ACCEPT              ,
    MANAGE_CONCURRENCY  ,
    REFUSE              ,
    READ                ,
    CLOSE               ,

} SOCKET_FSM;

/**********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

int SocketStateCreate(void);
int SocketStateOptions(int socket_desc);
int SocketStateBind(int socket_desc, int server_port);
int SocketStateListen(int socket_desc, int max_conn_num);
int SocketStateAccept(int socket_desc);
int SocketStateRead(int new_socket);
int SocketStateClose(int new_socket);

/*************************************/

#endif