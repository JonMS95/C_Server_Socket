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
#define SERVER_SOCKET_MSG_THREADS_CLEAN         "Cleaning up server instances."
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
#define SERVER_SOCKET_MANAGE_THREADS_NOK        "Server instance creation failed."
#define SERVER_SOCKET_MANAGE_THREADS_OK         "Server instance creation succeeded."
#define SERVER_SOCKET_MSG_REFUSE                "Connection refused by the server. Closing socket in %d seconds."

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
    MANAGE_THREADS      ,
    REFUSE              ,
    CLOSE               ,

} SOCKET_FSM;

/**********************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

static void SocketFreeResources(void);
static void SocketSIGINTHandler(int signum);
static int SocketStateCreate(void);
static int SocketStateOptions(  int             socket_desc     ,
                                bool            reuse_address   ,
                                bool            reuse_port      ,
                                unsigned long   rx_timeout_secs ,
                                unsigned long   rx_timeout_usecs,
                                unsigned long   tx_timeout_secs ,
                                unsigned long   tx_timeout_usecs);
static int SocketStateBind(int socket_desc, int server_port);
static int SocketStateListen(int socket_desc, int max_conn_num);
static int SocketStateAccept(int socket_desc, bool non_blocking);
static int SocketStateRefuse(int client_socket);

/*************************************/

#endif