/************************************/
/******** Include statements ********/
/************************************/

#include "ServerSocketSSL.h"
#include <openssl/err.h>    // OpenSSL errors.

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
int ServerSocketSSLSetup(SSL_CTX* ctx, SSL* ssl, char* cert_path, char* priv_key_path)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(SSLv23_server_method());

    if(ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        return SERVER_SOCKET_SETUP_SSL_NULL_CTX;
    }

    // Load server certificate
    int ssl_load_cert = SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM);
    if (ssl_load_cert <= 0) {
        ERR_print_errors_fp(stderr);
        return ssl_load_cert;
    }

    // Load server private key
    int ssl_load_priv_key = SSL_CTX_use_PrivateKey_file(ctx, priv_key_path, SSL_FILETYPE_PEM);
    if (ssl_load_priv_key <= 0) {
        ERR_print_errors_fp(stderr);

        return ssl_load_priv_key;
    }

    return SERVER_SOCKET_SETUP_SSL_SUCCESS;
}

/*************************************/
