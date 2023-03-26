#include <stdlib.h>
#include <string.h>     // memset, strlen
#include "socket_use.h" // IF_ANET, SOCK_STREAM, ...
#include "socket_FSM.h"

#include "../Dependency_files/Header_files/GetOptions_api.h"
#include "../Dependency_files/Header_files/SeverityLog_api.h"

#define PORT_OPT_CHAR       'p'
#define PORT_OPT_DETAIL     "Port"
#define PORT_MIN_VALUE      "49152"
#define PORT_MAX_VALUE      "65535"
#define PORT_DEFAULT_VALUE  "50000"

#define CONN_OPT_CHAR       'c'
#define CONN_OPT_DETAIL     "Maximum number of connections"
#define CONN_MIN_VALUE      "1"
#define CONN_MAX_VALUE      "3"
#define CONN_DEFAULT_VALUE  "1"

#define MAX_ERROR_MSG_LEN   100

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    int set_config_opt_port =   AddOption(  PORT_OPT_CHAR       ,
                                            PORT_OPT_DETAIL     ,
                                            GETOPT_VAR_TYPE_INT ,
                                            PORT_MIN_VALUE      ,
                                            PORT_MAX_VALUE      ,
                                            PORT_DEFAULT_VALUE  );

    if(set_config_opt_port < 0)
    {
        return set_config_opt_port;
    }

    int set_config_opt_conn =   AddOption(  CONN_OPT_CHAR       ,
                                            CONN_OPT_DETAIL     ,
                                            GETOPT_VAR_TYPE_INT ,
                                            CONN_MIN_VALUE      ,
                                            CONN_MAX_VALUE      ,
                                            CONN_DEFAULT_VALUE  );

    if(set_config_opt_conn < 0)
    {
        return set_config_opt_conn;
    }

    int parse_arguments = GetOptions(argc, argv);
    if(parse_arguments < 0)
    {
        SeverityLog(SVRTY_LVL_ERR, "Arguments parsing failed!\r\n");
        return parse_arguments;
    }

    ShowOptions();

    int server_port = GetIntegerOptionValue(PORT_OPT_CHAR);
    int max_conn_num = GetIntegerOptionValue(CONN_OPT_CHAR);

    SocketFSM(server_port, max_conn_num);

    return 0;
}
