/************************************/
/******** Include statements ********/
/************************************/

#include <stdlib.h>         // malloc, calloc, realloc, free.
#include <unistd.h>         // Fork if concurrency is accepted.
#include <string.h>         // strcpy
#include <signal.h>         // Shutdown signal.
#include <openssl/err.h>
#include "ServerSocketUse.h"
#include "ServerSocketFSM.h"
#include "ServerSocketConcurrency.h"
#include "ServerSocketSSL.h"
#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Private variables ********/
/***********************************/

static volatile int ctrlCPressed        = 0;
static pid_t*       server_instances    = NULL;
static SSL_CTX*     ctx                 = NULL;
static SSL*         ssl                 = NULL;

/***********************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

// WIP
void SocketFreeResources(void)
{
    if(server_instances != NULL)
    {
        LOG_DBG(SERVER_SOCKET_MSG_CLEANING_UP_PID, getpid());
        free(server_instances);
    }

    if(ssl != NULL)
    {
        LOG_DBG(SERVER_SOCKET_MSG_CLEANING_UP_SSL);
        SSL_free(ssl);
    }

    if(ctx != NULL)
    {
        LOG_DBG(SERVER_SOCKET_MSG_CLEANING_UP_SSL_CTX);
        SSL_CTX_free(ctx);
    }

    // To be revised:
    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    // ERR_remove_thread_state(NULL);
    ERR_free_strings();
    // ERR_remove_state(0); // Deprecated.
    // ENGINE_cleanup();    // Doesn't seem to exist in openssl.
    CONF_modules_unload(1);
    CONF_modules_free();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
}

/// @brief Handle SIGINT signal (Ctrl+C).
/// @param signum Signal number (SIGINT by default).
void SocketSIGINTHandler(int signum)
{
    LOG_WNG(SERVER_SOCKET_MSG_SIGINT_RECEIVED);
    ctrlCPressed = 1; // Set the flag to indicate Ctrl+C was pressed

    SocketFreeResources();

    exit(EXIT_SUCCESS);
}

/// @brief Create socket descriptor.
/// @return < 0 if it failed to create the socket.
int SocketStateCreate(void)
{
    int socket_desc = CreateSocketDescriptor(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if(socket_desc < 0)
        LOG_ERR(SERVER_SOCKET_MSG_CREATION_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_CREATION_OK);

    return socket_desc;
}

/// @brief Setup SSL data and context.
/// @param ctx SSL context
/// @param ssl SSL data
/// @param cert_path Path to certificate.
/// @param priv_key_path Path to private key.
/// @return 0 if succeeded, < 0 otherwise.
int SocketStateSetupSSL(SSL_CTX** ctx, SSL** ssl, char* cert_path, char* priv_key_path)
{
    int server_socket_SSL_setup = ServerSocketSSLSetup(ctx, ssl, cert_path, priv_key_path);

    if(server_socket_SSL_setup != SERVER_SOCKET_SETUP_SSL_SUCCESS)
        LOG_ERR(SERVER_SOCKET_MSG_SETUP_SSL_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_SETUP_SSL_OK);

    return (server_socket_SSL_setup == SERVER_SOCKET_SETUP_SSL_SUCCESS ? 0 : -1);
}

/// @brief Set socket options.
/// @param socket_desc Socket file descriptor.
/// @return < 0 if it failed to set options.
int SocketStateOptions(int socket_desc)
{
    int socket_options = SocketOptions(socket_desc, 1, 1, 50, 50, 50);

    if(socket_options < 0)
        LOG_ERR(SERVER_SOCKET_MSG_SET_OPTIONS_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_SET_OPTIONS_OK);

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
        LOG_ERR(SERVER_SOCKET_MSG_BIND_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_BIND_OK);
    
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
        LOG_ERR(SERVER_SOCKET_MSG_LISTEN_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_LISTEN_OK);
    
    return listen;
}

/// @brief Accept an incoming connection.
/// @param socket_desc Socket file descriptor.
/// @return < 0 if it failed to accept the connection.
int SocketStateAccept(int socket_desc)
{
    int client_socket = SocketAccept(socket_desc);

    if(client_socket < 0)
        LOG_ERR(SERVER_SOCKET_MSG_ACCEPT_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_ACCEPT_OK);
    
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
        return SERVER_SOCKET_CONC_ERR_REFUSE_CONN;
    }

    int retfork = fork();

    if(retfork < 0)
    {
        LOG_ERR(SERVER_SOCKET_MSG_CANNOT_FORK);
        return SERVER_SOCKET_CONC_ERR_CANNOT_FORK;
    }

    if(retfork > 0)
    {
        int new_server_instance_index = ServerSocketNewInstanceSpotIndex(server_instance_processes, max_conn_num);

        if(new_server_instance_index < 0)
        {
            return SERVER_SOCKET_CONC_ERR_REFUSE_CONN;
        }

        server_instance_processes[new_server_instance_index] = retfork;

        LOG_DBG(SERVER_SOCKET_MSG_NEW_PROCESS, retfork);

        return SERVER_SOCKET_CONC_SUCCESS_PARENT;
    }

    return SERVER_SOCKET_CONC_SUCCESS_CHILD;
}

/// @brief Refuse incoming connection. To be called when there are no free spots for a new server instance.
/// @param client_socket Client socket descriptor.
/// @return < 0 if any error happened while trying to close the socket.
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

/// @brief Perform SSL handshake if needed.
/// @param client_socket Client socket instance.
/// @param ctx SSL context.
/// @param ssl SSL data.
/// @return 0 if handhsake was successfully performed, < 0 otherwise.
int SocketStateSSLHandshake(int client_socket, SSL_CTX** ctx, SSL** ssl)
{
    int ssl_handshake = ServerSocketSSLHandshake(client_socket, ctx, ssl);

    if(ssl_handshake != SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS)
        LOG_ERR(SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK);

    return (ssl_handshake == SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS ? 0 : -1);
}

/// @brief Read data received in the socket.
/// @param client_socket Socket file descriptor.
/// @return <= 0 if it failed to read.
int SocketStateInteract(int client_socket, bool secure, SSL** ssl)
{
    int read = SocketInteract(client_socket, secure, ssl);

    return read;
}

/// @brief Close socket.
/// @param client_socket Socket file descriptor.
/// @return < 0 if it failed to close the socket.
int SocketStateClose(int client_socket)
{
    int close = CloseSocket(client_socket);
    
    if(close < 0)
        LOG_ERR(SERVER_SOCKET_MSG_CLOSE_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_CLOSE_OK);

    return close;    
}

/// @brief Socket Finite State Machine (FSM).
/// @param server_port Port number which the socket is going to be listening to.
/// @param max_conn_num Maximum amount of allowed connections.
/// @return < 0 if it failed.
int ServerSocketRun(int server_port, int max_conn_num, bool concurrent, bool secure, char* cert_path, char* key_path)
{
    SOCKET_FSM socket_fsm = CREATE_FD;
    int socket_desc;
    int client_socket;
    server_instances = (pid_t*)calloc(max_conn_num, sizeof(pid_t));

    if(signal(SIGINT, SocketSIGINTHandler) == SIG_ERR)
    {
        LOG_ERR(SERVER_SOCKET_SET_SIGINT_ERR);
        exit(EXIT_FAILURE);
    }

    while(!ctrlCPressed)
    {
        switch (socket_fsm)
        {
            case CREATE_FD:
            {
                socket_desc = SocketStateCreate();
                if(socket_desc >= 0)
                    socket_fsm = SETUP_SSL;
            }
            break;

            case SETUP_SSL:
            {
                if(!secure)
                {
                    socket_fsm = OPTIONS;
                    continue;
                }
                
                if(SocketStateSetupSSL(&ctx, &ssl, cert_path, key_path) < 0)
                    socket_fsm = CLOSE;
                else
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
                        socket_fsm = SSL_HANDSHAKE;
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
                    case SERVER_SOCKET_CONC_ERR_REFUSE_CONN:
                    {
                        socket_fsm = REFUSE;
                    }
                    break;

                    case SERVER_SOCKET_CONC_SUCCESS_PARENT:
                    case SERVER_SOCKET_CONC_ERR_CANNOT_FORK:
                    {
                        socket_fsm = ACCEPT;
                    }
                    break;

                    case SERVER_SOCKET_CONC_SUCCESS_CHILD:
                    {
                        socket_fsm = SSL_HANDSHAKE;
                    }

                    default:
                    break;
                }
            }
            break;

            case REFUSE:
            {
                SocketStateRefuse(client_socket);

                socket_fsm = ACCEPT;
            }
            break;

            case SSL_HANDSHAKE:
            {
                if(!secure)
                {
                    socket_fsm = INTERACT;
                    continue;
                }

                if(SocketStateSSLHandshake(client_socket, &ctx, &ssl) >= 0)
                    socket_fsm = INTERACT;
                else
                    socket_fsm = ACCEPT;
            }
            break;

            case INTERACT:
            {
                if(SocketStateInteract(client_socket, secure, &ssl) <= 0)
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
