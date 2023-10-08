#include <stdlib.h>
#include <string.h>     // memset, strlen
#include "socket_use.h" // IF_ANET, SOCK_STREAM, ...
#include "socket_FSM.h"

#include "GetOptions_api.h"
#include "SeverityLog_api.h"

/********** Private constants **********/
/***************************************/

/************ Port settings ************/

#define PORT_OPT_CHAR       'p'
#define PORT_OPT_LONG       "Port"
#define PORT_OPT_DETAIL     "Range of acceptable port numbers"
#define PORT_MIN_VALUE      49152
#define PORT_MAX_VALUE      65535
#define PORT_DEFAULT_VALUE  50000

/********* Connection settings *********/

#define CONN_OPT_CHAR       'c'
#define CONN_OPT_LONG       "Connections"
#define CONN_OPT_DETAIL     "Maximum number of connections"
#define CONN_MIN_VALUE      1
#define CONN_MAX_VALUE      3
#define CONN_DEFAULT_VALUE  1

/***************************************/

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    SetSeverityLogMask(SVRTY_LOG_MASK_ALL);

    int server_port;
    int max_conn_num;

    SetOptionDefinitionInt( PORT_OPT_CHAR       ,
                            PORT_OPT_LONG       ,
                            PORT_OPT_DETAIL     ,
                            PORT_MIN_VALUE      ,
                            PORT_MAX_VALUE      ,
                            PORT_DEFAULT_VALUE  ,
                            &server_port        );

    SetOptionDefinitionInt( CONN_OPT_CHAR       ,
                            CONN_OPT_LONG       ,
                            CONN_OPT_DETAIL     ,
                            CONN_MIN_VALUE      ,
                            CONN_MAX_VALUE      ,
                            CONN_DEFAULT_VALUE  ,
                            &max_conn_num       );

    int parse_arguments = ParseOptions(argc, argv);
    if(parse_arguments < 0)
    {
        LOG_ERR("Arguments parsing failed!");
        return parse_arguments;
    }

    LOG_INF("Arguments successfully parsed!");

    SocketFSM(server_port, max_conn_num);

    return 0;
}
