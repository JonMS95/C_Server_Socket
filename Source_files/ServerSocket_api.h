#ifndef SERVER_SOCKET_API_H
#define SERVER_SOCKET_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************/
/******** Include statements *********/
/*************************************/

#include <stdbool.h>

/*************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

/// @brief Socket Finite State Machine (FSM).
/// @param server_port Port number which the socket is going to be listening to.
/// @param max_conn_num Maximum amount of allowed connections.
/// @return < 0 if it failed.
int ServerSocketRun(int server_port, int max_conn_num, bool concurrent, bool secure);

/*************************************/

#ifdef __cplusplus
}
#endif

#endif