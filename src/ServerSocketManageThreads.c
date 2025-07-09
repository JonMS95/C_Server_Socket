/************************************/
/******** Include statements ********/
/************************************/

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include "ServerSocketSSL.h"
#include "ServerSocketUse.h"
#include "ServerSocketManageThreads.h"
#include "SeverityLog_api.h"
#include "MutexGuard_api.h"

/************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_MANAGE_THREADS_SUCCESS            0
#define SERVER_SOCKET_THREADS_ALLOCATION_FAILURE        -1
#define SERVER_SOCKET_MANAGE_THREAD_CREATION_FAILURE    -2
#define SERVER_SOCKET_MANAGE_THREAD_NO_FREE_SPOTS       -3
#define SERVER_SOCKET_DATA_CLEAN_FAILURE                -4
#define SERVER_SOCKET_MTX_LOCK_FAILURE                  -5

#define SERVER_SOCKET_MSG_ERR_CREATE_THREAD         "An error ocurred while creating thread: <%s>."
#define SERVER_SOCKET_MSG_ERR_NO_FREE_SPOTS         "Maximum number of connections has already been reached: <%d>."
#define SERVER_SOCKET_MSG_THREAD_CREATION_SUCCESS   "Successfully created thread for client socket <%d> (TID: <%lu>)."
#define SERVER_SOCKET_MSG_CANCELLING_THREAD         "Cancelling thread with ID: <%lu>."
#define SERVER_SOCKET_MSG_ERR_THREAD_CANCELLATION   "An error happened while cancelling thread with ID: <%lu>."
#define SERVER_SOCKET_MSG_JOINING_THREAD            "Joining thread with ID: <%lu>."
#define SERVER_SOCKET_MSG_ERR_THREAD_JOIN           "An error happened while joining thread with ID: <%lu>."
#define SERVER_SOCKET_MSG_ERR_MTX_LOCK              "Could not lock mutex: <%x>."

#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK "SSL handshake failed."
#define SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK  "SSL handshake succeeded."

#define SERVER_SOCKET_MSG_CLOSE_NOK "An error happened while closing socket <%d> (TID: <%lu>)."
#define SERVER_SOCKET_MSG_CLOSE_OK  "Socket <%d> successfully closed (TID: <%lu>)."

/************************************/

/**********************************/
/******** Type definitions ********/
/**********************************/

/// @brief Possible connection states.
typedef enum
{
    SSL_HANDSHAKE = 0   ,
    INTERACT            ,
    CLOSE_CLIENT        ,

} SERVER_SOCKET_CONN_HANDLE_FSM;

/// @brief Common socket instance arguments.
typedef struct
{
    bool secure;
    bool non_blocking;
    int (*interact_fn)(int client_socket);
} SERVER_SOCKET_THREAD_COMMON_ARGS;

/// @brief Arguments to be received by each socket instance. 
typedef struct
{
    int client_socket;
    SERVER_SOCKET_THREAD_COMMON_ARGS* thread_common_args;
} SERVER_SOCKET_THREAD_ARGS;

/// @brief Server socket thread data managing structure definition.
typedef struct
{
    pthread_t thread;
    bool active;
    SSL* p_ssl;
    SERVER_SOCKET_THREAD_ARGS thread_args;
} SERVER_SOCKET_THREAD_DATA;


/**********************************/

/***********************************/
/******** Private variables ********/
/***********************************/

/// @brief Server instances counter.
static int server_instances_num = 0;
/// @brief Server instances data storing array.
static SERVER_SOCKET_THREAD_DATA* server_instances_data = NULL;
/// @brief Server socket common arguments storing variable pointer.
static SERVER_SOCKET_THREAD_COMMON_ARGS* server_instances_common_args = NULL;
/// @brief Mutex Guard type variable to handle concurrent read/write operations involving server_instances_data.
static MTX_GRD_CREATE(mtx_thread_array);

/***********************************/

/*************************************/
/**** Private function prototypes ****/
/*************************************/

static int SocketStateSSLHandshake(const int client_socket, const bool non_blocking);
static int SocketStateClose(const int client_socket);
static int SocketThreadDataClean(const int client_socket);
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
static int SocketStateSSLHandshake(const int client_socket, const bool non_blocking)
{
    int ssl_handshake = ServerSocketSSLHandshake(client_socket, non_blocking);

    if(ssl_handshake < 0)
        SVRTY_LOG_ERR(SERVER_SOCKET_MSG_SSL_HANDSHAKE_NOK);
    else
        SVRTY_LOG_INF(SERVER_SOCKET_MSG_SSL_HANDSHAKE_OK);

    return ssl_handshake;
}

/// @brief Close socket.
/// @param client_socket Socket file descriptor.
/// @return < 0 if it failed to close the socket.
static int SocketStateClose(const int client_socket)
{
    int close = CloseSocket(client_socket);
    
    if(close < 0)
        SVRTY_LOG_ERR(SERVER_SOCKET_MSG_CLOSE_NOK, client_socket, pthread_self());
    else
        SVRTY_LOG_INF(SERVER_SOCKET_MSG_CLOSE_OK, client_socket, pthread_self());

    return close;    
}

/// @brief Clean data associated to every active thread.
/// @param client_socket Target socket instance.
/// @return 0 if succeeded, < 0 otherwise.
static int SocketThreadDataClean(const int client_socket)
{ 
    // Iterate through SERVER_SOCKET_THREAD_DATA looking for a thread which client socket value matches the current one,
    // erase all related data afterwards.

    MTX_GRD_LOCK_SC(&mtx_thread_array, p_mtx_thread_array);

    if(!p_mtx_thread_array)
    {
        SVRTY_LOG_ERR(SERVER_SOCKET_MSG_ERR_MTX_LOCK, pthread_self());
        return SERVER_SOCKET_MTX_LOCK_FAILURE;
    }

    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
    {
        if(server_instances_data[thread_idx].thread_args.client_socket == client_socket)
        {
            ServerSocketFreeSSL(server_instances_data[thread_idx].p_ssl);
            server_instances_data[thread_idx] = (SERVER_SOCKET_THREAD_DATA){0};
            return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
        }
    }

    return SERVER_SOCKET_DATA_CLEAN_FAILURE;
}

/// @brief Common thread routine to be executed by every instantiated thread.
/// @param args Input arguments (SERVER_SOCKET_THREAD_ARGS).
/// @return 0 if succeeded, < 0 otherwise.
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

/// @brief Frees data associated to every threads managing submodule.
static void SocketFreeThreadsData(void)
{
    MTX_GRD_LOCK_SC(&mtx_thread_array, p_mtx_thread_array);

    if(!p_mtx_thread_array)
    {
        SVRTY_LOG_ERR(SERVER_SOCKET_MSG_ERR_MTX_LOCK, pthread_self());
        return;
    }

    if(server_instances_common_args)
    {
        free(server_instances_common_args);
        server_instances_common_args = NULL;
    }

    if(server_instances_data)
    {
        free(server_instances_data);
        server_instances_data = NULL;
    }
}

/// @brief Kills every active thread.
/// @return 0 if succeeded, < 0 otherwise.
static int SocketKillAllThreads(void)
{
    int thread_cancel_status = 0;
    int thread_join_status = 0;

    if(server_instances_data == NULL)
        return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;

    MTX_GRD_LOCK_SC(&mtx_thread_array, p_mtx_thread_array);
    
    if(!p_mtx_thread_array)
    {
        SVRTY_LOG_ERR(SERVER_SOCKET_MSG_ERR_MTX_LOCK, pthread_self());
        return SERVER_SOCKET_MTX_LOCK_FAILURE;
    }

    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
        if(server_instances_data[thread_idx].active && server_instances_data[thread_idx].thread)
        {
            SVRTY_LOG_DBG(SERVER_SOCKET_MSG_CANCELLING_THREAD, server_instances_data[thread_idx].thread);
            
            thread_cancel_status = pthread_cancel(server_instances_data[thread_idx].thread);
            
            if(thread_cancel_status < 0)
            {
                SVRTY_LOG_ERR(SERVER_SOCKET_MSG_ERR_THREAD_CANCELLATION, server_instances_data[thread_idx].thread);
                return thread_cancel_status;
            }

            SVRTY_LOG_DBG(SERVER_SOCKET_MSG_JOINING_THREAD, server_instances_data[thread_idx].thread);

            thread_join_status = pthread_join(server_instances_data[thread_idx].thread, NULL);

            if(thread_join_status < 0)
            {
                SVRTY_LOG_ERR(SERVER_SOCKET_MSG_ERR_THREAD_JOIN, server_instances_data[thread_idx].thread);
                return thread_join_status;
            }
        }

    return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
}

/// @brief Performs thread managing submodule's setup.
/// @param max_conn_num Maximum number of handleable connections.
/// @param secure Tells whether the socket is secure or nor (TLS/SSL).
/// @param non_blocking Tells whether the socket is non-blocking.
/// @param interact_fn Pointer to interaction function. It can be the one by default of any function provided by using ServerSocketRun.
/// @return 0 if succeeded, < 0 otherwise.
int SocketSetupThreads(const int max_conn_num, const bool secure, const bool non_blocking, int (*interact_fn)(int client_socket))
{
    server_instances_num = max_conn_num;

    if(server_instances_data != NULL && server_instances_common_args != NULL)
        return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;

    server_instances_data = (SERVER_SOCKET_THREAD_DATA*)calloc(server_instances_num, sizeof(SERVER_SOCKET_THREAD_DATA));
    
    server_instances_common_args = (SERVER_SOCKET_THREAD_COMMON_ARGS*)calloc(1, sizeof(SERVER_SOCKET_THREAD_COMMON_ARGS));
    server_instances_common_args->secure = secure;
    server_instances_common_args->non_blocking = non_blocking;
    server_instances_common_args->interact_fn = interact_fn;

    MTX_GRD_INIT(&mtx_thread_array);

    if(server_instances_data == NULL || server_instances_common_args == NULL)
        return SERVER_SOCKET_THREADS_ALLOCATION_FAILURE;
    
    return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
}

/// @brief Launches server socket instance.
/// @param client_socket Target client-oriented server socket instance.
/// @return 0 if succeeded, < 0 otherwise.
int SocketLaunchServerInstance(const int client_socket)
{
    MTX_GRD_LOCK_SC(&mtx_thread_array, p_mtx_thread_array);

    if(!p_mtx_thread_array)
    {
        SVRTY_LOG_ERR(SERVER_SOCKET_MSG_ERR_MTX_LOCK, pthread_self());
        return SERVER_SOCKET_MTX_LOCK_FAILURE;
    }

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
                SVRTY_LOG_WNG(SERVER_SOCKET_MSG_ERR_CREATE_THREAD, strerror(thread_creation_status));
                return SERVER_SOCKET_MANAGE_THREAD_CREATION_FAILURE;
            }
            
            SVRTY_LOG_INF(SERVER_SOCKET_MSG_THREAD_CREATION_SUCCESS, client_socket, server_instances_data[thread_idx].thread);
            return SERVER_SOCKET_MANAGE_THREADS_SUCCESS;
        }
    }
    
    SVRTY_LOG_WNG(SERVER_SOCKET_MSG_ERR_NO_FREE_SPOTS, server_instances_num);
    return SERVER_SOCKET_MANAGE_THREAD_NO_FREE_SPOTS;
}

/// @brief Retrieves current thread's SSL object (if any) based on thread's ID.
/// @return SSL object belonging to current thread (if any).
SSL** SocketGetCurrentThreadSSLObj(void)
{
    if(!ServerSocketIsSecure())
        return NULL;

    for(int thread_idx = 0; thread_idx < server_instances_num; thread_idx++)
        if(server_instances_data[thread_idx].active && server_instances_data[thread_idx].thread == pthread_self())
            return &server_instances_data[thread_idx].p_ssl;
    
    return NULL;
}

/// @brief Frees resources priorly allocated by threads managing submodule.
/// @return 0 if succeeded, < 0 otherwise.
int SocketFreeThreadsResources(void)
{
    int kill_threads = SocketKillAllThreads();
    SocketFreeThreadsData();
    MTX_GRD_DESTROY(&mtx_thread_array);
    return kill_threads;
}

/*************************************/
