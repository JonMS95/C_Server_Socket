/************************************/
/******** Include statements ********/
/************************************/

#include <openssl/err.h>        // OpenSSL errors.
#include "ServerSocketSSL.h"
#include "ServerSocketUse.h"    // Set/Unset O_NONBLOCK flag.
#include "SeverityLog_api.h"

/************************************/

/***********************************/
/******** Private variables ********/
/***********************************/

static SSL_CTX*     ctx                 = NULL;
static SSL*         ssl                 = NULL;
static bool         is_secure           = false;

/***********************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

void ServerSocketSetSecure(bool secure)
{
    is_secure = secure;
}

bool ServerSocketIsSecure(void)
{
    return is_secure;
}

SSL** ServerSocketGetPointerToSSLData(void)
{
    return &ssl;
}

SSL_CTX** ServerSocketGetPointerToSSLContext(void)
{
    return &ctx;
}

/// @brief Setup SSL data and context.
/// @param cert_path Path to certificate.
/// @param priv_key_path Path to private key.
/// @return 1 if succeeded, < 1 if any error ocurred.
int ServerSocketSSLSetup(char* cert_path, char* priv_key_path)
{
    LOG_DBG(SERVER_SOCKET_MSG_PATH_TO_CERT, cert_path);
    LOG_DBG(SERVER_SOCKET_MSG_PATH_TO_PRIV_KEY, priv_key_path);

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    *(ServerSocketGetPointerToSSLContext()) = SSL_CTX_new(TLS_server_method());

    if(*(ServerSocketGetPointerToSSLContext()) == NULL)
    {
        ERR_print_errors_fp(stderr);
        return SERVER_SOCKET_SETUP_SSL_NULL_CTX;
    }

    // Load server certificate
    int ssl_load_cert = SSL_CTX_use_certificate_file(*(ServerSocketGetPointerToSSLContext()), cert_path, SSL_FILETYPE_PEM);
    if (ssl_load_cert <= 0) {
        ERR_print_errors_fp(stderr);
        return ssl_load_cert;
    }

    // Load server private key
    int ssl_load_priv_key = SSL_CTX_use_PrivateKey_file(*(ServerSocketGetPointerToSSLContext()), priv_key_path, SSL_FILETYPE_PEM);
    if (ssl_load_priv_key <= 0) {
        ERR_print_errors_fp(stderr);

        return ssl_load_priv_key;
    }

    ServerSocketSetSecure(true);

    return SERVER_SOCKET_SETUP_SSL_SUCCESS;
}

/// @brief Perform SSL handshake if needed.
/// @param client_socket Client socket instance.
/// @param bool non_blocking Tells whether or not is the socket non-blocking.
/// @return 0 if handhsake was successfully performed, < 0 otherwise.
int ServerSocketSSLHandshake(int client_socket, bool non_blocking)
{
    *(ServerSocketGetPointerToSSLData()) = SSL_new(*(ServerSocketGetPointerToSSLContext()));
    SSL_set_fd(*(ServerSocketGetPointerToSSLData()), client_socket);

    // Unset non-blocking feature by default, activate it again later if required.
    int unset_non_blocking = SocketUnsetNonBlocking(client_socket);
    if(unset_non_blocking < 0)
        return unset_non_blocking;

    int ssl_accept = SSL_accept(*(ServerSocketGetPointerToSSLData()));

    if(ssl_accept <= 0)
    {
        ERR_print_errors_fp(stderr);
    }

    // Check if socket is non-blocking. If that is the case, then set it as non-blocking again.
    int set_non_blocking = 0;
    if(non_blocking)
        set_non_blocking = SocketSetNonBlocking(client_socket);
    
    if(set_non_blocking < 0)
        return set_non_blocking;

    return ssl_accept;
}

/*************************************/
