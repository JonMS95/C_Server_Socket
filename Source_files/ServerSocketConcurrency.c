/************************************/
/******** Include statements ********/
/************************************/

#include "ServerSocketConcurrency.h"
#include "SeverityLog_api.h"

/************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/// @brief Find out whether or not can a new server socket instance be created.
/// @param server_instance_processes Array that contains the PID of each running server socket instance.
/// @param max_conn_num Maximum number of allowed connections (== server socket instances).
/// @return < 0 if it is not possible. Otherwise, return the amount of remaining free spots for new processes.
int ServerSocketPossibleNewInstance(pid_t* server_instance_processes, int max_conn_num)
{
    int server_instances_count = 0;

    for(int i = 0; i < max_conn_num; i++)
    {
        if(server_instance_processes[i] == 0)
            continue;

        int status;
        pid_t waitpid_ret = waitpid(server_instance_processes[i], &status, WNOHANG);

        if( (waitpid_ret == server_instance_processes[i]) || (waitpid_ret < 0) )
        {
            LOG_DBG(SERVER_SOCKET_MSG_INSTANCE_NOT_RUNNING, i, waitpid_ret);
            server_instance_processes[i] = 0;
        }
        else
        {
            LOG_DBG(SERVER_SOCKET_MSG_INSTANCE_STILL_RUNNING, i, server_instance_processes[i]);
            server_instances_count++;
        }
    }

    if(server_instances_count >= max_conn_num)
        return -1;

    return (max_conn_num - server_instances_count);
}

int ServerSocketNewInstanceSpotIndex(pid_t* server_instance_processes, int max_conn_num)
{
    for(int i = 0; i < max_conn_num; i++)
    {
        if(server_instance_processes[i] == 0)
            return i;
    }

    return -1;
}

/*************************************/
