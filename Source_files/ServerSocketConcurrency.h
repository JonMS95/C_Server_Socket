#ifndef SERVER_SOCKET_CONCURRENCY_H
#define SERVER_SOCKET_CONCURRENCY_H

/************************************/
/******** Include statements ********/
/************************************/

#include <sys/types.h>      // pid_t type.
#include <sys/wait.h>       // Find child processes.

/************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_MSG_INSTANCE_NOT_RUNNING      "Socket instance <%d> with PID: <%d> is no longer running."
#define SERVER_SOCKET_MSG_INSTANCE_STILL_RUNNING    "Socket instance <%d> with PID: <%d> is still running."

/************************************/

/************************************/
/******* Function prototypes ********/
/************************************/

int ServerSocketPossibleNewInstance(pid_t* server_instance_processes, int max_conn_num);
int ServerSocketNewInstanceSpotIndex(pid_t* server_instance_processes, int max_conn_num);

/************************************/

#endif