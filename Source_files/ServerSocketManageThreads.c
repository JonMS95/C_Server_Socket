/************************************/
/******** Include statements ********/
/************************************/

#include <stdlib.h>
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

static server_instances_num = 0;
static SERVER_SOCKET_THREAD_DATA* server_instances_data = NULL;
static SERVER_SOCKET_THREAD_COMMON_ARGS* server_instances_common_args = NULL;

/***********************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

static void* ServerSocketThreadRoutine(void* args)
{
    SERVER_SOCKET_THREAD_ARGS* conn_handle_args = (SERVER_SOCKET_THREAD_ARGS*)args;
    int client_socket = conn_handle_args->client_socket;
    bool secure = conn_handle_args->thread_common_args->secure;
    bool non_blocking = conn_handle_args->thread_common_args->non_blocking;
    bool interact_fn = conn_handle_args->thread_common_args->interact_fn;

    bool keep_connected = true;

    SERVER_SOCKET_CONN_HANDLE_FSM conn_handle_fsm = SSL_HANDSHAKE;
    
    // Threads should always be cancellable, so main thread can termiante them if necessary.
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while(keep_connected)
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
                if(SocketStateInteract(client_socket) > 0)
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

                // NEW VERSION (THREADS): whether or not do multiple threads exist, thread should end its execution after closing client connection.
                // Question: does concurrency make sense any longer?

                keep_connected = false;
            }
            break;
        }
    }

    return NULL;
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

int SocketStateManageThreads(int client_socket)
{
    // Look for a free spot. If there's not any free spot, return FAILURE. Otherwise, launch thread and mark it as "running".
    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
    {
        if(!(server_instances_data[thread_idx].active))
        {
            server_instances_data[thread_idx].active = true;

            server_instances_data[thread_idx].thread_args.client_socket = client_socket;
            server_instances_data[thread_idx].thread_args.thread_common_args = &server_instances_common_args;

            if(pthread_create(&server_instances_data[thread_idx].thread, NULL, ServerSocketThreadRoutine, &server_instances_data[thread_idx].thread_args) < 0)
                return SERVER_SOCKET_MANAGE_THREAD_CREATION_FAILURE;
            
            return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
        }
    }
    
    return SERVER_SOCKET_MANAGE_THREAD_CREATION_FAILURE;
}

int SocketKillAllThreads()
{
    int thread_cancel_failure = 0;
    
    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
        if(pthread_cancel(server_instances_data[thread_idx].thread))
            thread_cancel_failure = -1;
    
    return thread_cancel_failure;
}

/*************************************/
