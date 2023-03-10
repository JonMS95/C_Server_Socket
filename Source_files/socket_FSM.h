#ifndef SOCKET_FSM_H
#define SOCKET_FSM_H

typedef enum
{
    CREATE_FD = 0   ,
    OPTIONS         ,
    BIND            ,
    LISTEN          ,
    ACCEPT          ,
    READ            ,
    CLOSE           ,

} SOCKET_FSM;

int SocketFSM(int server_port, int max_conn_num);

#endif