#ifndef SERVER_SOCKET_CONCURRENCY_H
#define SERVER_SOCKET_CONCURRENCY_H

/************************************/
/******** Include statements ********/
/************************************/

#include <sys/types.h>      // Find child processes.
#include <sys/wait.h>       // Find child processes.

/************************************/

/************************************/
/******* Function prototypes ********/
/************************************/

int ServerSocketPossibleNewInstance(pid_t* server_instance_processes, int max_conn_num);
int ServerSocketNewInstanceSpotIndex(pid_t* server_instance_processes, int max_conn_num);

/************************************/

#endif