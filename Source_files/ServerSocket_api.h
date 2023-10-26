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

int ServerSocketRun(int server_port, int max_conn_num, bool concurrent);

/*************************************/

#ifdef __cplusplus
}
#endif

#endif