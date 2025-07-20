#ifndef SERVER_SOCKET_SSL_H
#define SERVER_SOCKET_SSL_H

/************************************/
/******** Include statements ********/
/************************************/

#include <stdbool.h>
#include <openssl/ssl.h>

/************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

bool ServerSocketIsSecure(void);
int ServerSocketSSLSetup(const char* restrict cert_path, const char* restrict priv_key_path);
int ServerSocketSSLHandshake(const int client_socket, const bool non_blocking);
int ServerSocketSSLRead(char* restrict rx_buffer, const unsigned long rx_buffer_size);
int ServerSocketSSLWrite(const char* restrict tx_buffer, const unsigned long tx_buffer_size);
void ServerSocketFreeSSL(SSL* p_ssl);
void SocketFreeSSLResources(void);

/*************************************/

#endif