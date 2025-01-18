/************************************/
/******** Include statements ********/
/************************************/

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "ServerSocketSSL.h"
#include "ServerSocketUse.h"
#include "ServerSocketManageThreads.h"
#include "SeverityLog_api.h"

/************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_MANAGE_THREADS_SUCCESS            0
#define SERVER_SOCKET_THREADS_ALLOCATION_FAILURE        -1
#define SERVER_SOCKET_MANAGE_THREAD_CREATION_FAILURE    -2
#define SERVER_SOCKET_MANAGE_THREAD_NO_FREE_SPOTS       -3
#define SERVER_SOCKET_DATA_CLEAN_FAILURE                -4

#define SERVER_SOCKET_MSG_ERR_CREATE_THREAD         "An error ocurred while creating thread: <%s>."
#define SERVER_SOCKET_MSG_ERR_NO_FREE_SPOTS         "Maximum number of connections has already been reached: <%d>."
#define SERVER_SOCKET_MSG_THREAD_CREATION_SUCCESS   "Successfully created thread for client socket <%d> (TID: <%lu>)."
#define SERVER_SOCKET_MSG_CANCELLING_THREAD         "Cancelling thread with ID: <%lu>."
#define SERVER_SOCKET_MSG_ERR_THREAD_CANCELLATION   "An error happened while cancelling thread with ID: <%lu>."
#define SERVER_SOCKET_MSG_JOINING_THREAD            "Joining thread with ID: <%lu>."
#define SERVER_SOCKET_MSG_ERR_THREAD_JOIN           "An error happened while joining thread with ID: <%lu>."

#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK "SSL handshake failed."
#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK  "SSL handshake succeeded."

#define SERVER_SOCKET_MSG_CLOSE_NOK "An error happened while closing socket <%d> (TID: <%lu>)."
#define SERVER_SOCKET_MSG_CLOSE_OK  "Socket <%d> successfully closed (TID: <%lu>)."

/************************************/

/**********************************/
/******** Type definitions ********/
/**********************************/

typedef enum
{
    SSL_HANDSHAKE = 0   ,
    INTERACT            ,
    CLOSE_CLIENT        ,

} SERVER_SOCKET_CONN_HANDLE_FSM;

typedef struct
{
    bool secure;
    bool non_blocking;
    int (*interact_fn)(int client_socket);
} SERVER_SOCKET_THREAD_COMMON_ARGS;

typedef struct
{
    int client_socket;
    SERVER_SOCKET_THREAD_COMMON_ARGS* thread_common_args;
} SERVER_SOCKET_THREAD_ARGS;

typedef struct
{
    pthread_t thread;
    bool active;
    SERVER_SOCKET_THREAD_ARGS thread_args;
} SERVER_SOCKET_THREAD_DATA;


/**********************************/

/***********************************/
/******** Private variables ********/
/***********************************/

static int server_instances_num = 0;
static SERVER_SOCKET_THREAD_DATA* server_instances_data = NULL;
static SERVER_SOCKET_THREAD_COMMON_ARGS* server_instances_common_args = NULL;

/***********************************/

/*************************************/
/**** Private function prototypes ****/
/*************************************/

static int SocketStateSSLHandshake(int client_socket, bool non_blocking);
static int SocketStateClose(int client_socket);
static int SocketThreadDataClean(int client_socket);
static void* ServerSocketThreadRoutine(void* args);
static void SocketFreeThreadsData();
static int SocketKillAllThreads();

/*************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/// @brief Perform SSL handshake if needed.
/// @param client_socket Client socket instance.
/// @param bool non_blocking Tells whether or not is the socket non-blocking.
/// @return 0 if handhsake was successfully performed, < 0 otherwise.
static int SocketStateSSLHandshake(int client_socket, bool non_blocking)
{
    int ssl_handshake = ServerSocketSSLHandshake(client_socket, non_blocking);

    if(ssl_handshake < 0)
        LOG_ERR(SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK);

    return ssl_handshake;
}

/// @brief Close socket.
/// @param client_socket Socket file descriptor.
/// @return < 0 if it failed to close the socket.
static int SocketStateClose(int client_socket)
{
    int close = CloseSocket(client_socket);
    
    if(close < 0)
        LOG_ERR(SERVER_SOCKET_MSG_CLOSE_NOK, client_socket, pthread_self());
    else
        LOG_INF(SERVER_SOCKET_MSG_CLOSE_OK, client_socket, pthread_self());

    return close;    
}

static int SocketThreadDataClean(int client_socket)
{
    // Iterate through SERVER_SOCKET_THREAD_DATA looking for a thread which client socket value matches the current one,
    // erase all related data afterwards.
    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
        if(server_instances_data[thread_idx].thread_args.client_socket == client_socket)
        {
            server_instances_data[thread_idx].active = false;
            return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
        }
    
    return SERVER_SOCKET_DATA_CLEAN_FAILURE;
}

static void* ServerSocketThreadRoutine(void* args)
{
    SERVER_SOCKET_THREAD_ARGS* conn_handle_args = (SERVER_SOCKET_THREAD_ARGS*)args;
    
    int client_socket   = conn_handle_args->client_socket;
    bool secure         = conn_handle_args->thread_common_args->secure;
    bool non_blocking   = conn_handle_args->thread_common_args->non_blocking;
    
    int (*interact_fn)(int client_socket) = conn_handle_args->thread_common_args->interact_fn;

    bool keep_routine_alive = true;

    SERVER_SOCKET_CONN_HANDLE_FSM conn_handle_fsm = SSL_HANDSHAKE;
    
    // Threads should always be cancellable, so main thread can termiante them if necessary.
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while(keep_routine_alive)
    {
        switch(conn_handle_fsm)
        {
            // Perform SSL / TLS handshake to make the connection secure
            case SSL_HANDSHAKE:
            {
                if(!(secure))
                {
                    conn_handle_fsm = INTERACT;
                    continue;
                }

                if(SocketStateSSLHandshake(client_socket, non_blocking) >= 0)
                    conn_handle_fsm = INTERACT;
                else
                    conn_handle_fsm = CLOSE_CLIENT;
            }
            break;

            // Interact with client
            case INTERACT:
            {
                if(interact_fn(client_socket) > 0)
                    conn_handle_fsm = INTERACT;
                else
                    conn_handle_fsm = CLOSE_CLIENT;
            }
            break;

            // Close client socket instance if required
            case CLOSE_CLIENT:
            {
                SocketThreadDataClean(client_socket);
                SocketStateClose(client_socket);

                keep_routine_alive = false;
            }
            break;
        }
    }

    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

static void SocketFreeThreadsData()
{
    if(server_instances_common_args)
        free(server_instances_common_args);

    if(server_instances_data != NULL)
        free(server_instances_data);
}

static int SocketKillAllThreads()
{
    int thread_cancel_status = 0;
    int thread_join_status = 0;

    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
        if(server_instances_data[thread_idx].active && server_instances_data[thread_idx].thread)
        {
            LOG_DBG(SERVER_SOCKET_MSG_CANCELLING_THREAD, server_instances_data[thread_idx].thread);
            
            thread_cancel_status = pthread_cancel(server_instances_data[thread_idx].thread);
            
            if(thread_cancel_status < 0)
            {
                LOG_ERR(SERVER_SOCKET_MSG_ERR_THREAD_CANCELLATION, server_instances_data[thread_idx].thread);
                return thread_cancel_status;
            }

            LOG_DBG(SERVER_SOCKET_MSG_JOINING_THREAD, server_instances_data[thread_idx].thread);

            thread_join_status = pthread_join(server_instances_data[thread_idx].thread, NULL);

            if(thread_join_status < 0)
            {
                LOG_ERR(SERVER_SOCKET_MSG_ERR_THREAD_JOIN, server_instances_data[thread_idx].thread);
                return thread_join_status;
            }
        }

    return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
}

int SocketSetupThreads(int max_conn_num, bool secure, bool non_blocking, int (*interact_fn)(int client_socket))
{
    server_instances_num = max_conn_num;

    if(server_instances_data != NULL && server_instances_common_args != NULL)
        return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;

    server_instances_data = (SERVER_SOCKET_THREAD_DATA*)calloc(server_instances_num, sizeof(SERVER_SOCKET_THREAD_DATA));
    
    server_instances_common_args = (SERVER_SOCKET_THREAD_COMMON_ARGS*)calloc(1, sizeof(SERVER_SOCKET_THREAD_COMMON_ARGS));
    server_instances_common_args->secure = secure;
    server_instances_common_args->non_blocking = non_blocking;
    server_instances_common_args->interact_fn = interact_fn;

    if(server_instances_data == NULL || server_instances_common_args == NULL)
        return SERVER_SOCKET_THREADS_ALLOCATION_FAILURE;
    
    return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
}

int SocketLaunchServerInstance(int client_socket)
{
    // Look for a free spot. If there's not any free spot, return FAILURE. Otherwise, launch thread and mark it as "running".
    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
    {
        if(!(server_instances_data[thread_idx].active))
        {
            server_instances_data[thread_idx].active = true;

            server_instances_data[thread_idx].thread_args.client_socket = client_socket;
            server_instances_data[thread_idx].thread_args.thread_common_args = server_instances_common_args;

            int thread_creation_status = pthread_create(&server_instances_data[thread_idx].thread       ,
                                                        NULL                                            ,
                                                        ServerSocketThreadRoutine                       ,
                                                        &server_instances_data[thread_idx].thread_args  );

            if(thread_creation_status < 0)
            {
                LOG_WNG(SERVER_SOCKET_MSG_ERR_CREATE_THREAD, strerror(thread_creation_status));
                return SERVER_SOCKET_MANAGE_THREAD_CREATION_FAILURE;
            }
            
            LOG_INF(SERVER_SOCKET_MSG_THREAD_CREATION_SUCCESS, client_socket, server_instances_data[thread_idx].thread);
            return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
        }
    }
    
    LOG_WNG(SERVER_SOCKET_MSG_ERR_NO_FREE_SPOTS, server_instances_num);
    return SERVER_SOCKET_MANAGE_THREAD_NO_FREE_SPOTS;
}

int SocketFreeThreadsResources()
{
    int kill_threads = SocketKillAllThreads();
    SocketFreeThreadsData();
    return kill_threads;
}

/*************************************/
