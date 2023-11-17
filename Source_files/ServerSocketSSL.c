/************************************/
/******** Include statements ********/
/************************************/

#include <openssl/err.h>    // OpenSSL errors.
#include "ServerSocketSSL.h"
#include "SeverityLog_api.h"

/************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/// @brief Setup SSL data and context.
/// @param ctx SSL context
/// @param ssl SSL data
/// @param cert_path Path to certificate.
/// @param priv_key_path Path to private key.
/// @return 1 if succeeded, < 1 if any error ocurred.
int ServerSocketSSLSetup(SSL_CTX** ctx, SSL** ssl, char* cert_path, char* priv_key_path)
{
    LOG_DBG(SERVER_SOCKET_MSG_PATH_TO_CERT, cert_path);
    LOG_DBG(SERVER_SOCKET_MSG_PATH_TO_PRIV_KEY, priv_key_path);

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    *ctx = SSL_CTX_new(TLS_server_method());

    if(*ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        return SERVER_SOCKET_SETUP_SSL_NULL_CTX;
    }

    // Load server certificate
    int ssl_load_cert = SSL_CTX_use_certificate_file(*ctx, cert_path, SSL_FILETYPE_PEM);
    if (ssl_load_cert <= 0) {
        ERR_print_errors_fp(stderr);
        return ssl_load_cert;
    }

    // Load server private key
    int ssl_load_priv_key = SSL_CTX_use_PrivateKey_file(*ctx, priv_key_path, SSL_FILETYPE_PEM);
    if (ssl_load_priv_key <= 0) {
        ERR_print_errors_fp(stderr);

        return ssl_load_priv_key;
    }

    return SERVER_SOCKET_SETUP_SSL_SUCCESS;
}

/// @brief Perform SSL handshake if needed.
/// @param client_socket Client socket instance.
/// @param ctx SSL context.
/// @param ssl SSL data.
/// @return 0 if handhsake was successfully performed, < 0 otherwise.
int ServerSocketSSLHandshake(int client_socket, SSL_CTX** ctx, SSL** ssl)
{
    *ssl = SSL_new(*ctx);
    SSL_set_fd(*ssl, client_socket);

    int ssl_accept = SSL_accept(*ssl);

    if(ssl_accept <= 0)
    {
        ERR_print_errors_fp(stderr);
    }

    return ssl_accept;
}


/// @brief Expand path to server certificate and / or private key.
/// @param dest_expanded_path Expanded path destination.
/// @param src_short_path 
void ServerSocketSSLExpandPath(char** dest_expanded_path, char** src_short_path)
{
    if ((*src_short_path)[0] != '~') {
        *dest_expanded_path = calloc(strlen(*src_short_path) + 1, sizeof(char));
        strcpy(*dest_expanded_path, *src_short_path);
        return;
    }

    const char* home_dir = getenv(SERVER_SOCKET_HOME_OS_DIR_NAME);

    *dest_expanded_path = calloc(strlen(home_dir) + strlen(*src_short_path) + 1, sizeof(char));

    strcpy(*dest_expanded_path, home_dir);
    strcpy(*dest_expanded_path + strlen(home_dir), *src_short_path + 1);
}

/*************************************/
