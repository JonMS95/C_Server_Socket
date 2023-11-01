/************************************/
/******** Include statements ********/
/************************************/

#include "ServerSocket_api.h"
#include "GetOptions_api.h"
#include "SeverityLog_api.h"

/************************************/

/***************************************/
/********** Private constants **********/
/***************************************/

/************ Port settings ************/

#define PORT_OPT_CHAR       'p'
#define PORT_OPT_LONG       "Port"
#define PORT_OPT_DETAIL     "Range of acceptable port numbers."
#define PORT_MIN_VALUE      49152
#define PORT_MAX_VALUE      65535
#define PORT_DEFAULT_VALUE  50000

/********* Connection settings *********/

#define CLIENTS_OPT_CHAR        'm'
#define CLIENTS_OPT_LONG        "Clients"
#define CLIENTS_OPT_DETAIL      "Maximum number of clients."
#define CLIENTS_MIN_VALUE       1
#define CLIENTS_MAX_VALUE       3
#define CLIENTS_DEFAULT_VALUE   1

/********* Enable concurrency *********/

#define SIMULTANEOUS_CONNS_CHAR             'c'
#define SIMULTANEOUS_CONNS_LONG             "Concurrent"
#define SIMULTANEOUS_CONNS_DETAIL           "Enable concurrency."
#define SIMULTANEOUS_CONNS_DEFAULT_VALUE    false

/********* Secure connection *********/

#define SECURE_CONN_CHAR            's'
#define SECURE_CONN_LONG            "Secure"
#define SECURE_CONN_DETAIL          "Secure connection."
#define SECURE_CONN_DEFAULT_VALUE   false

/***************************************/

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    SetSeverityLogMask(SVRTY_LOG_MASK_ALL);
    SetSeverityLogPrintTimeStatus(true);

    int server_port         ;
    int max_clients_num     ;
    bool concurrency_enabled;
    bool secure_connection;

    SetOptionDefinitionBool(    SIMULTANEOUS_CONNS_CHAR             ,
                                SIMULTANEOUS_CONNS_LONG             ,
                                SIMULTANEOUS_CONNS_DETAIL           ,
                                SIMULTANEOUS_CONNS_DEFAULT_VALUE    ,
                                &concurrency_enabled                );

    SetOptionDefinitionInt( PORT_OPT_CHAR       ,
                            PORT_OPT_LONG       ,
                            PORT_OPT_DETAIL     ,
                            PORT_MIN_VALUE      ,
                            PORT_MAX_VALUE      ,
                            PORT_DEFAULT_VALUE  ,
                            &server_port        );

    SetOptionDefinitionInt( CLIENTS_OPT_CHAR        ,
                            CLIENTS_OPT_LONG        ,
                            CLIENTS_OPT_DETAIL      ,
                            CLIENTS_MIN_VALUE       ,
                            CLIENTS_MAX_VALUE       ,
                            CLIENTS_DEFAULT_VALUE   ,
                            &max_clients_num        );

    SetOptionDefinitionBool(    SECURE_CONN_CHAR            ,
                                SECURE_CONN_LONG            ,
                                SECURE_CONN_DETAIL          ,
                                SECURE_CONN_DEFAULT_VALUE   ,
                                &secure_connection          );

    int parse_arguments = ParseOptions(argc, argv);
    if(parse_arguments < 0)
    {
        LOG_ERR("Arguments parsing failed!");
        return parse_arguments;
    }

    LOG_INF("Arguments successfully parsed!");

    ServerSocketRun(server_port, max_clients_num, concurrency_enabled, secure_connection);

    return 0;
}
