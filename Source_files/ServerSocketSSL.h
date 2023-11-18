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

#define SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS 1

#define SERVER_SOCKET_HOME_OS_DIR_NAME      "HOME"
#define SERVER_SOCKET_MSG_PATH_TO_PRIV_KEY  "Path to certificate: <%s>"
#define SERVER_SOCKET_MSG_PATH_TO_CERT      "Path to private key: <%s>"

/************************************/

/*************************************/
/******** Function prototypes ********/
/*************************************/

int ServerSocketSSLSetup(SSL_CTX** ctx, SSL** ssl, char* cert_path, char* priv_key_path);
int ServerSocketSSLHandshake(int client_socket, SSL_CTX** ctx, SSL** ssl);
void ServerSocketSSLExpandPath(char** dest_expanded_path, char** src_short_path);

/*************************************/

#endif