/************************************/
/******** Include statements ********/
/************************************/

#include <stdlib.h>         // malloc, calloc, realloc, free.
#include <unistd.h>         // Write, sleep.
#include <string.h>         // strlen
#include <signal.h>         // Shutdown signal.
#include "ServerSocketUse.h"
#include "ServerSocketManageThreads.h"
#include "ServerSocketSSL.h"
#include "ServerSocketDefaultInteract.h"
#include "ServerSocket_api.h"
#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Define statements ********/
/***********************************/

#define SERVER_SOCKET_SET_SIGINT_ERR            "Error while trying to set up SIGINT handler."
#define SERVER_SOCKET_MSG_SIGINT_RECEIVED       "Received Ctrl+C (SIGINT). Cleaning up and exiting."
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

/***********************************/
/******** Private variables ********/
/***********************************/

static volatile int ctrlCPressed        = 0;

/***********************************/

/*************************************/
/**** Private function prototypes ****/
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
static int SocketStateSetupSSL(const char* cert_path, const char* priv_key_path);
static int SocketStateBind( int socket_desc             ,
                            int server_port             ,
                            sa_family_t address_family  ,
                            in_addr_t allowed_IPs       );
static int SocketStateListen(int socket_desc, int max_conn_num);
static int SocketStateAccept(int socket_desc, bool non_blocking);
static int SocketStateManageThreads(int client_socket);
static int SocketStateRefuse(int client_socket);

/*************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/// @brief Frees previously heap allocated memory before exiting the program.
static void SocketFreeResources(void)
{
    SocketFreeThreadsResources();
    SocketFreeSSLResources();
}

/// @brief Handle SIGINT signal (Ctrl+C).
/// @param signum Signal number (SIGINT by default).
static void SocketSIGINTHandler(int signum)
{
    // NEW VERSION (THREADS): apart from freeing all resources, all threads must be terminated.
    // For such thing to be possible, threads should be cancellable (deferred).
    LOG_WNG(SERVER_SOCKET_MSG_SIGINT_RECEIVED);
    ctrlCPressed = 1; // Set the flag to indicate Ctrl+C was pressed

    SocketFreeResources();

    exit(EXIT_SUCCESS);
}

/// @brief Create socket descriptor.
/// @return < 0 if it failed to create the socket.
static int SocketStateCreate(void)
{
    int socket_desc = CreateSocketDescriptor(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if(socket_desc < 0)
        LOG_ERR(SERVER_SOCKET_MSG_CREATION_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_CREATION_OK);

    return socket_desc;
}

/// @brief Set socket options.
/// @param socket_desc Socket file descriptor.
/// @param reuse_address Reuse address, does not hold the address after socket is closed.
/// @param reuse_port Reuse port, does not hold the port after socket is closed.
/// @param rx_timeout_usecs Receive timeout in seconds.
/// @param rx_timeout_usecs Receive timeout in microseconds.
/// @param tx_timeout_s Send timeout in seconds.
/// @param tx_timeout_us Send timeout in microseconds.
/// @return < 0 if it failed to set options.
static int SocketStateOptions(  int             socket_desc     ,
                                bool            reuse_address   ,
                                bool            reuse_port      ,
                                unsigned long   rx_timeout_secs ,
                                unsigned long   rx_timeout_usecs,
                                unsigned long   tx_timeout_secs ,
                                unsigned long   tx_timeout_usecs)
{
    int socket_options = SocketOptions(socket_desc, reuse_address, reuse_port, rx_timeout_secs, rx_timeout_usecs, tx_timeout_secs, tx_timeout_usecs);

    if(socket_options < 0)
        LOG_ERR(SERVER_SOCKET_MSG_SET_OPTIONS_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_SET_OPTIONS_OK);

    return socket_options;
}

/// @brief Setup SSL data and context.
/// @param cert_path Path to certificate.
/// @param priv_key_path Path to private key.
/// @return 0 if succeeded, < 0 otherwise.
static int SocketStateSetupSSL(const char* cert_path, const char* priv_key_path)
{
    int server_socket_SSL_setup = ServerSocketSSLSetup(cert_path, priv_key_path);

    if(server_socket_SSL_setup < 0)
        LOG_ERR(SERVER_SOCKET_MSG_SETUP_SSL_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_SETUP_SSL_OK);

    return server_socket_SSL_setup;
}

/// @brief Bind socket to an IP address and port.
/// @param socket_desc Socket file descriptor.
/// @param server_port The port which is going to be used to listen to incoming connections.
/// @param address_family Address family the server is going to work with.
/// @param allowed_IPs Use INADDR_ANY to allow any IP, specify it otherwise.
/// @return < 0 if it failed to bind.
static int SocketStateBind(int socket_desc, int server_port, sa_family_t address_family, in_addr_t allowed_IPs)
{
    // struct sockaddr_in server = PrepareForBinding(AF_INET, INADDR_ANY, server_port);

    int bind_socket = BindSocket(socket_desc, server_port, address_family, allowed_IPs);

    if(bind_socket < 0)
        LOG_ERR(SERVER_SOCKET_MSG_BIND_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_BIND_OK);
    
    return bind_socket;
}

/// @brief Listen to incoming connections.
/// @param socket_desc Socket file descriptor.
/// @param max_conn_num Maximum connections number.
/// @return < 0 if it failed to listen.
static int SocketStateListen(int socket_desc, int max_conn_num)
{
    int listen = SocketListen(socket_desc, max_conn_num);

    if(listen < 0)
        LOG_ERR(SERVER_SOCKET_MSG_LISTEN_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_LISTEN_OK);
    
    return listen;
}

/// @brief Accept an incoming connection.
/// @param socket_desc Socket file descriptor.
/// @param non_blocking Tells whether or not is the socket meant to be non-blocking.
/// @return < 0 if it failed to accept the connection.
static int SocketStateAccept(int socket_desc, bool non_blocking)
{
    int client_socket = SocketAccept(socket_desc, non_blocking);
    
    if(client_socket >= 0)
        LOG_INF(SERVER_SOCKET_MSG_ACCEPT_OK);
    
    return client_socket;
}

static int SocketStateManageThreads(int client_socket)
{
    int new_server_instance = SocketLaunchServerInstance(client_socket);
    
    if(new_server_instance < 0)
        LOG_ERR(SERVER_SOCKET_MANAGE_THREADS_NOK);
    else
        LOG_INF(SERVER_SOCKET_MANAGE_THREADS_OK);
    
    return new_server_instance;
}

/// @brief Refuse incoming connection. To be called when there are no free spots for a new server instance.
/// @param client_socket Client socket descriptor.
/// @return < 0 if any error happened while trying to close the socket.
static int SocketStateRefuse(int client_socket)
{
    char refusal_msg[SERVER_SOCKET_LEN_MSG_REFUSE] = {};
    sprintf(refusal_msg, SERVER_SOCKET_MSG_REFUSE, SERVER_SOCKET_SECONDS_AFTER_REFUSAL);

    write(client_socket, refusal_msg, strlen(refusal_msg));

    shutdown(client_socket, SHUT_RDWR);

    sleep(SERVER_SOCKET_SECONDS_AFTER_REFUSAL);

    int close_socket = CloseSocket(client_socket);

    return close_socket;
}

/// @brief Runs server socket.
/// @param server_port Port server is meant to be listening to.
/// @param max_conn_num Maximum number of connections.
/// @param concurrent Enable concurrent server instances.
/// @param non_blocking Tells whether or not is the socket meant to be non-blocking.
/// @param reuse_address Reuse address, does not hold the address after socket is closed.
/// @param reuse_port Reuse port, does not hold the port after socket is closed.
/// @param rx_timeout_s Receive timeout in seconds.
/// @param rx_timeout_us Receive timeout in microseconds.
/// @param tx_timeout_s Send timeout in seconds.
/// @param tx_timeout_us Send timeout in microseconds.
/// @param secure Enable secure communication (TLS).
/// @param cert_path Path to server certificate.
/// @param key_path Path to server private key.
/// @param CustomSocketStateInteract Custom function to interact with client once connection is established.
/// @return 0 always, exit sending failure signal if SIGINT signal handler could not be properly set.
int ServerSocketRun(int server_port                                     ,
                    int max_conn_num                                    ,
                    bool concurrent                                     ,
                    bool non_blocking                                   ,
                    bool reuse_address                                  ,
                    bool reuse_port                                     ,
                    unsigned long rx_timeout_s                          ,
                    unsigned long rx_timeout_us                         ,
                    unsigned long tx_timeout_s                          ,
                    unsigned long tx_timeout_us                         ,
                    bool secure                                         ,
                    const char* cert_path                               ,
                    const char* pkey_path                               ,
                    int (*CustomSocketStateInteract)(int client_socket) )
{
    SOCKET_FSM socket_fsm = CREATE_FD;
    int socket_desc;
    int client_socket;
    int (*SocketStateInteract)(int client_socket)  = SocketDefaultInteractFn;

    if(CustomSocketStateInteract != NULL)
        SocketStateInteract = CustomSocketStateInteract;

    int setup_threads = SocketSetupThreads(max_conn_num, secure, non_blocking, SocketStateInteract);
    
    if(setup_threads < 0)
        return setup_threads;

    if(signal(SIGINT, SocketSIGINTHandler) == SIG_ERR)
    {
        LOG_ERR(SERVER_SOCKET_SET_SIGINT_ERR);
        exit(EXIT_FAILURE);
    }

    while(!ctrlCPressed)
    {
        switch (socket_fsm)
        {
            // Create socket file descriptor
            case CREATE_FD:
            {
                socket_desc = SocketStateCreate();
                if(socket_desc >= 0)
                    socket_fsm = OPTIONS;
            }
            break;

            // Set general socket FD options (keepalive, heartbeat, ...)
            case OPTIONS:
            {
                if(SocketStateOptions(socket_desc, reuse_address, reuse_port, rx_timeout_s, rx_timeout_us, tx_timeout_s, tx_timeout_us) >= 0)
                    socket_fsm = SETUP_SSL;
            }
            break;

            // Setup SSL / TLS
            case SETUP_SSL:
            {
                if(!secure)
                {
                    socket_fsm = BIND;
                    continue;
                }
                
                if(SocketStateSetupSSL(cert_path, pkey_path) < 0)
                    socket_fsm = CLOSE;
                else
                    socket_fsm = BIND;

            }
            break;

            // Bind socket descriptor to a specified port
            case BIND:
            {
                if(SocketStateBind(socket_desc, server_port, AF_INET, INADDR_ANY) >= 0)
                    socket_fsm = LISTEN;
            }
            break;

            // Mark the socket as listener (as it has to be for a server)
            case LISTEN:
            {
                if(!concurrent)
                    max_conn_num = 1;

                if(SocketStateListen(socket_desc, max_conn_num) >= 0)
                    socket_fsm = ACCEPT;
            }
            break;

            // Wait until an incoming connection shows up
            case ACCEPT:
            {
                client_socket = SocketStateAccept(socket_desc, non_blocking);

                if(client_socket >= 0)
                    socket_fsm = MANAGE_THREADS;
            }
            break;

            // Clone the current process if necessary
            // Should manage threads instead of processes:
            case MANAGE_THREADS:
            {
                int manage_threads = SocketStateManageThreads(client_socket);

                if(manage_threads >= 0)
                    socket_fsm = ACCEPT;
                else
                    socket_fsm = REFUSE;
            }
            break;

            // Refuse the incomming connection if it is not allowed
            case REFUSE:
            {
                SocketStateRefuse(client_socket);

                socket_fsm = ACCEPT;
            }
            break;

            case CLOSE:
            {
                SocketFreeResources();

                return 0;
            }
            break;
            
            default:
            break;
        }
    }

    return 0;
}

/*************************************/
