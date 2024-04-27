#ifndef SERVER_SOCKET_HELPER_FUNCTIONS_H
#define SERVER_SOCKET_HELPER_FUNCTIONS_H

#define getName(var)  #var

#define SERVER_SOCKET_HELPER_MSG_INSUFFICIENT_RX_BUFFER_SIZE    "Could not read from buffer due to insufficient RX buffer size."
#define SERVER_SOCKET_HELPER_MSG_RX_BUFFER_NAME                 "RX buffer variable name: <%s>, declared size: <%d>."
#define SERVER_SOCKET_HELPER_MSG_INSUFFICIENT_TX_BUFFER_SIZE    "Could not write to buffer due to insufficient TX buffer size."
#define SERVER_SOCKET_HELPER_MSG_TX_BUFFER_NAME                 "TX buffer variable name: <%s>, declared size: <%d>."

#define SERVER_SOCKET_HELPER_ERR_INSUFFICIENT_RX_BUFFER_SIZE    -1
#define SERVER_SOCKET_HELPER_ERR_INSUFFICIENT_TX_BUFFER_SIZE    -2

#endif