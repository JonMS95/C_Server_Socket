#ifndef SERVER_SOCKET_FSM_H
#define SERVER_SOCKET_FSM_H

/************************************/
/******** Include statements ********/
/************************************/

#include <sys/types.h>      // pid_t type.
#include "ServerSocket_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SERVER_SOCKET_SET_SIGINT_ERR            "Error while trying to set up SIGINT handler."
#define SERVER_SOCKET_MSG_SIGINT_RECEIVED       "Received Ctrl+C (SIGINT). Cleaning up and exiting."
#define SERVER_SOCKET_MSG_CLEANING_UP_PID       "Cleaning up server instances array in server with PID <%d>."
#define SERVER_SOCKET_MSG_CLEANING_UP_SSL_CTX   "Cleaning up server SSL context."
#define SERVER_SOCKET_MSG_CLEANING_UP_SSL       "Cleaning up server SSL data."
#define SERVER_SOCKET_MSG_CREATION_NOK          "Socket file descriptor creation failed."
#define SERVER_SOCKET_MSG_CREATION_OK           "Socket file descriptor created."
#define SERVER_SOCKET_MSG_SETUP_SSL_NOK         "SSL setup failed."
#define SERVER_SOCKET_MSG_SETUP_SSL_OK          "SSL setup succeeded."
#define SERVER_SOCKET_MSG_SET_OPTIONS_NOK       "Failed to set socket options."
#define SERVER_SOCKET_MSG_SET_OPTIONS_OK        "Successfully set socket options."
#define SERVER_SOCKET_MSG_BIND_NOK              "Socket binding failed."
#define SERVER_SOCKET_MSG_BIND_OK               "Socket file descriptor binded."
#define SERVER_SOCKET_MSG_LISTEN_NOK            "Socket listen failed."
#define SERVER_SOCKET_MSG_LISTEN_OK             "Socket listen succeeded."
#define SERVER_SOCKET_MSG_ACCEPT_NOK            "Accept failed."
#define SERVER_SOCKET_MSG_ACCEPT_OK             "Accept succeeded."
#define SERVER_SOCKET_MSG_MAX_CONNS_REACHED     "Cannot create a new server instance as maximum number of client connections have already been reached."
#define SERVER_SOCKET_MSG_CANNOT_FORK           "Cannot fork the current server socket server process."
#define SERVER_SOCKET_MSG_NEW_PROCESS           "Created new server socket instance in process with PID <%d>."
#define SERVER_SOCKET_MSG_REFUSE                "Connection refused by the server. Closing socket in %d seconds."
#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK     "SSL handshake failed."
#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK      "SSL handshake succeeded."
#define SERVER_SOCKET_MSG_CLOSE_NOK             "An error happened while closing socket <%d>."
#define SERVER_SOCKET_MSG_CLOSE_OK              "Socket <%d> successfully closed."

#define SERVER_SOCKET_CONC_ERR_CANNOT_FORK       -2
#define SERVER_SOCKET_CONC_ERR_REFUSE_CONN       -1
#define SERVER_SOCKET_CONC_SUCCESS_PARENT        0
#define SERVER_SOCKET_CONC_SUCCESS_CHILD         1

#define SERVER_SOCKET_LEN_MSG_REFUSE            100

#define SERVER_SOCKET_SECONDS_AFTER_REFUSAL     0

/************************************/

/**********************************/
/******** Type definitions ********/
/**********************************/

typedef enum
{
    CREATE_FD = 0       ,
    SETUP_SSL           ,
    OPTIONS             ,
    BIND                ,
    LISTEN              ,
    ACCEPT              ,
    MANAGE_CONCURRENCY  ,
    REFUSE              ,
    SSL_HANDSHAKE       ,
    INTERACT            ,
    CLOSE_CLIENT        ,
    CLOSE               ,

} SOCKET_FSM;

/**********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

static void SocketFreeResources(void);
static void SocketSIGINTHandler(int signum);
static int SocketStateCreate(void);
static int SocketStateOptions(int socket_desc, bool reuse_address, bool reuse_port, unsigned long rx_timeout_usecs);
static int SocketStateBind(int socket_desc, int server_port);
static int SocketStateListen(int socket_desc, int max_conn_num);
static int SocketStateAccept(int socket_desc, bool non_blocking);
static int SocketStateManageConcurrency(int client_socket, pid_t* server_instance_processes, int max_conn_num);
static int SocketStateRefuse(int client_socket);
static int SocketStateClose(int client_socket);

/*************************************/

#endif