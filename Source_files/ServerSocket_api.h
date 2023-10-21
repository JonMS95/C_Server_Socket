#ifndef SERVER_SOCKET_API_H
#define SERVER_SOCKET_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************/
/******** Function prototypes ********/
/*************************************/

int ServerSocketRun(int server_port, int max_conn_num);

/*************************************/

#ifdef __cplusplus
}
#endif

#endif