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
#define SERVER_SOCKET_CLIENT_DISCONNECTED   "Client disconnected."
#define SERVER_SOCKET_MSG_CLOSE_NOK         "An error happened while closing the socket."
#define SERVER_SOCKET_MSG_CLOSE_OK          "Socket successfully closed."

/************************************/

/**********************************/
/******** Type definitions ********/
/**********************************/

typedef enum
{
    CREATE_FD = 0   ,
    OPTIONS         ,
    BIND            ,
    LISTEN          ,
    ACCEPT          ,
    READ            ,
    CLOSE           ,

} SOCKET_FSM;

/**********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

int SocketStateCreate();
int SocketStateOptions(int socket_desc);
int SocketStateBind(int socket_desc, int server_port);
int SocketStateListen(int socket_desc, int max_conn_num);
int SocketStateAccept(int socket_desc);
int SocketStateRead(int new_socket);
int SocketStateClose(int new_socket);

#endif