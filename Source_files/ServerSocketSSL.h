#ifndef SERVER_SOCKET_SSL_H
#define SERVER_SOCKET_SSL_H

/************************************/
/******** Include statements ********/
/************************************/

#include <openssl/ssl.h>    // OpenSSL.
#include <stdbool.h>

/************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_SETUP_SSL_NULL_CTX    -1
#define SERVER_SOCKET_SETUP_SSL_SUCCESS     1

#define SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS 1

#define SERVER_SOCKET_HOME_OS_DIR_NAME      "HOME"
#define SERVER_SOCKET_MSG_PATH_TO_PRIV_KEY  "Path to certificate: <%s>"
#define SERVER_SOCKET_MSG_PATH_TO_CERT      "Path to private key: <%s>"

/************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

SSL** ServerSocketGetPointerToSSLData(void);
SSL_CTX** ServerSocketGetPointerToSSLContext(void);
bool ServerSocketIsSecure(void);

int ServerSocketSSLSetup(char* cert_path, char* priv_key_path);
int ServerSocketSSLHandshake(int client_socket, bool non_blocking);

/*************************************/

#endif