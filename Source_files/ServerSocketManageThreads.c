/************************************/
/******** Include statements ********/
/************************************/

#include <stdlib.h>
#include <string.h>
#include "ServerSocketSSL.h"
#include "ServerSocketUse.h"
#include "ServerSocketManageThreads.h"
#include "SeverityLog_api.h"

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
}
SERVER_SOCKET_THREAD_ARGS;

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
/******* Function definitions ********/
/*************************************/

/// @brief Perform SSL handshake if needed.
/// @param client_socket Client socket instance.
/// @param bool non_blocking Tells whether or not is the socket non-blocking.
/// @return 0 if handhsake was successfully performed, < 0 otherwise.
static int SocketStateSSLHandshake(int client_socket, bool non_blocking)
{
    int ssl_handshake = ServerSocketSSLHandshake(client_socket, non_blocking);

    if(ssl_handshake != SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS)
        LOG_ERR(SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK);
    else
        LOG_INF(SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK);

    return (ssl_handshake == SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS ? 0 : -1);
}

/// @brief Close socket.
/// @param client_socket Socket file descriptor.
/// @return < 0 if it failed to close the socket.
static int SocketStateClose(int client_socket)
{
    int close = CloseSocket(client_socket);
    
    if(close < 0)
        LOG_ERR(SERVER_SOCKET_MSG_CLOSE_NOK, client_socket);
    else
        LOG_INF(SERVER_SOCKET_MSG_CLOSE_OK, client_socket);

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
    
    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
        if(server_instances_data[thread_idx].active && server_instances_data[thread_idx].thread)
        {
            if(pthread_cancel(server_instances_data[thread_idx].thread))
                thread_cancel_status = -1;
            else
                pthread_join(server_instances_data[thread_idx].thread, NULL);
        }

    return thread_cancel_status;
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
