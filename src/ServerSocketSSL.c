/************************************/
/******** Include statements ********/
/************************************/

#include <openssl/ssl.h>        // OpenSSL.
#include <openssl/err.h>        // OpenSSL errors.
#include "ServerSocketSSL.h"
#include "ServerSocketUse.h"    // Set/Unset O_NONBLOCK flag.
#include "ServerSocketSSL.h"
#include "ServerSocketManageThreads.h"  // Get current thread's SSL object (if any).
#include "SeverityLog_api.h"

/************************************/

/************************************/
/********* Define statements ********/
/************************************/

#define SERVER_SOCKET_MSG_PATH_TO_PRIV_KEY  "Path to certificate: <%s>."
#define SERVER_SOCKET_MSG_PATH_TO_CERT      "Path to private key: <%s>."

#define SERVER_SOCKET_MSG_CLEANING_UP_SSL_CTX   "Cleaning up server SSL context."
#define SERVER_SOCKET_MSG_CLEANING_UP_SSL       "Cleaning up server SSL data."

#define SERVER_SOCKET_MSG_SSL_EQUAL_PATHS   "Path for certificate is the same as for private key."

#define SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS 0
#define SERVER_SOCKET_SSL_HANDSHAKE_ERR     -1
#define SERVER_SOCKET_SSL_ACCEPT_SUCCESS    1

#define SERVER_SOCKET_SETUP_SSL_SUCCESS             0
#define SERVER_SOCKET_SETUP_SSL_NULL_SSL            -1
#define SERVER_SOCKET_SETUP_SSL_NULL_CTX            -2
#define SERVER_SOCKET_SETUP_SSL_ERR_LOAD_CERT       -3
#define SERVER_SOCKET_SETUP_SSL_ERR_LOAD_PRIV_KEY   -4
#define SERVER_SOCKET_SETUP_SSL_EQUAL_PATHS         -5

/************************************/

/***********************************/
/******** Private variables ********/
/***********************************/

static SSL_CTX* ctx         = NULL;
static bool     is_secure   = false;

/***********************************/

/*************************************/
/**** Private function prototypes ****/
/*************************************/

static SSL_CTX** ServerSocketGetPointerToSSLContext(void);
static void ServerSocketSetSecure(bool secure);

/*************************************/

/*************************************/
/******* Function definitions ********/
/*************************************/

/// @brief Retrieves pointer to SSL_CTX pointer.
/// @return Double pointer to ssl context variable.
static SSL_CTX** ServerSocketGetPointerToSSLContext(void)
{
    return &ctx;
}


/// @brief Sets security variable.
/// @param secure Target security variable value.
static void ServerSocketSetSecure(const bool secure)
{
    is_secure = secure;
}

/// @brief Tells whether socket is secure or not.
/// @return True if security is enabled, false otherwise.
bool ServerSocketIsSecure(void)
{
    return is_secure;
}

/// @brief Setup SSL data and context.
/// @param cert_path Path to certificate.
/// @param priv_key_path Path to private key.
/// @return 1 if succeeded, < 1 if any error ocurred.
int ServerSocketSSLSetup(const char* restrict cert_path, const char* restrict priv_key_path)
{
    SVRTY_LOG_DBG(SERVER_SOCKET_MSG_PATH_TO_CERT, cert_path);
    SVRTY_LOG_DBG(SERVER_SOCKET_MSG_PATH_TO_PRIV_KEY, priv_key_path);

    if(cert_path == priv_key_path)
    {
        SVRTY_LOG_ERR(SERVER_SOCKET_MSG_SSL_EQUAL_PATHS);
        return SERVER_SOCKET_SETUP_SSL_EQUAL_PATHS;
    }

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
    if (ssl_load_cert <= 0)
    {
        ERR_print_errors_fp(stderr);
        return SERVER_SOCKET_SETUP_SSL_ERR_LOAD_CERT;
    }

    // Load server private key
    int ssl_load_priv_key = SSL_CTX_use_PrivateKey_file(*(ServerSocketGetPointerToSSLContext()), priv_key_path, SSL_FILETYPE_PEM);
    if (ssl_load_priv_key <= 0)
    {
        ERR_print_errors_fp(stderr);
        return SERVER_SOCKET_SETUP_SSL_ERR_LOAD_PRIV_KEY;
    }

    ServerSocketSetSecure(true);

    return SERVER_SOCKET_SETUP_SSL_SUCCESS;
}

/// @brief Perform SSL handshake if needed.
/// @param client_socket Client socket instance.
/// @param bool non_blocking Tells whether or not is the socket non-blocking.
/// @param p_ssl Pointer to SSL object.
/// @return 0 if handhsake was successfully performed, < 0 otherwise.
int ServerSocketSSLHandshake(const int client_socket, const bool non_blocking)
{
    SSL** restrict dp_ssl = SocketGetCurrentThreadSSLObj();
    SSL* restrict p_ssl = NULL;
    
    if(!dp_ssl || !*dp_ssl)
        return SERVER_SOCKET_SETUP_SSL_NULL_SSL;
    
    *dp_ssl = SSL_new(*(ServerSocketGetPointerToSSLContext()));

    if(!*dp_ssl)
        return SERVER_SOCKET_SETUP_SSL_NULL_SSL;

    int set_ssl_fd = SSL_set_fd(*dp_ssl, client_socket);

    // Unset non-blocking feature by default, activate it again later if required.
    int unset_non_blocking = SocketUnsetNonBlocking(client_socket);
    if(unset_non_blocking < 0)
        return unset_non_blocking;

    int ssl_accept = SSL_accept(*dp_ssl);

    if(ssl_accept <= 0)
        ERR_print_errors_fp(stderr);

    // Check if socket is non-blocking. If that is the case, then set it as non-blocking again.
    int set_non_blocking = 0;
    if(non_blocking)
        set_non_blocking = SocketSetNonBlocking(client_socket);
    
    if(set_non_blocking < 0)
        return set_non_blocking;

    return (ssl_accept == SERVER_SOCKET_SSL_ACCEPT_SUCCESS ? SERVER_SOCKET_SSL_HANDSHAKE_SUCCESS : SERVER_SOCKET_SSL_HANDSHAKE_ERR);
}

/// @brief Reads encrypted data from buffer.
/// @param rx_buffer Target buffer to read from.
/// @param rx_buffer_size Buffer size, or amount of bytes to be read from target buffer.
/// @return > 0 as read bytes if succeeded, < 0 if no data could be read, 0 if client got disconnected.
int ServerSocketSSLRead(char* restrict rx_buffer, const unsigned long rx_buffer_size)
{
    SSL** restrict dp_ssl = SocketGetCurrentThreadSSLObj();

    return SSL_read(*dp_ssl, rx_buffer, rx_buffer_size);
}

/// @brief Writes encrypted data to buffer.
/// @param tx_buffer TX buffer.
/// @param tx_buffer_size TX buffer size.
/// @return < 0 if any error happens, number of bytes sent otherwise.
int ServerSocketSSLWrite(const char* restrict tx_buffer, const unsigned long tx_buffer_size)
{
    SSL** restrict dp_ssl = SocketGetCurrentThreadSSLObj();

    return SSL_write(*dp_ssl, tx_buffer, tx_buffer_size);
}

/// @brief Frees previously allocated SSL resources.
void SocketFreeSSLResources(void)
{
    if(*(ServerSocketGetPointerToSSLContext()) != NULL)
    {
        SVRTY_LOG_DBG(SERVER_SOCKET_MSG_CLEANING_UP_SSL_CTX);
        SSL_CTX_free(*(ServerSocketGetPointerToSSLContext()));
        *(ServerSocketGetPointerToSSLContext()) = NULL;
    }

    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    CONF_modules_unload(1);
    CONF_modules_free();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
}

/// @brief Frees given SSL object.
/// @param p_ssl Pointer to SSL object to be freed.
void ServerSocketFreeSSL(SSL* p_ssl)
{
    if(!p_ssl)
        return;
    
    SSL_free(p_ssl);
    p_ssl = NULL;
}

/*************************************/
