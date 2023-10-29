#ifndef SERVER_SOCKET_SSL_H
#define SERVER_SOCKET_SSL_H

/************************************/
/******** Include statements ********/
/************************************/

#include <openssl/ssl.h>    // OpenSSL.

/************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_SETUP_SSL_NULL_CTX    -1
#define SERVER_SOCKET_SETUP_SSL_SUCCESS     1

/************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

int ServerSocketSSLSetup(SSL_CTX* ctx, SSL* ssl, char* cert_path, char* priv_key_path);

/*************************************/

#endif