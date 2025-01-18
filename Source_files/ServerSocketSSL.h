#ifndef SERVER_SOCKET_SSL_H
#define SERVER_SOCKET_SSL_H

/************************************/
/******** Include statements ********/
/************************************/

#include <stdbool.h>

/************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

bool ServerSocketIsSecure(void);
int ServerSocketSSLSetup(const char* cert_path, const char* priv_key_path);
int ServerSocketSSLHandshake(int client_socket, bool non_blocking);
int ServerSocketSSLRead(char* rx_buffer, unsigned long rx_buffer_size);
int ServerSocketSSLWrite(const char* tx_buffer, unsigned long tx_buffer_size);
void SocketFreeSSLResources(void);

/*************************************/

#endif