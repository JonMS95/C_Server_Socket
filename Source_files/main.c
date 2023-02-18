#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset, strlen
#include "socket_use.h" // IF_ANET, SOCK_STREAM, ...
#include <getopt.h> // getopt, optarg, ...

#define ARGV_PORT_IDX           1
#define ARGV_MAX_CONN_NUM_IDX   2
#define MIN_CONN_NUM            1
#define MAX_CONN_NUM            3
#define MIN_PORT_VALUE          49152
#define MAX_PORT_VALUE          65535

#define MAX_ERROR_MSG_LEN   100

#define ERR_NO_PORT_PROVIDED        -1
#define ERR_PORT_OUT_OF_RANGE       -2
#define ERR_NO_MAX_CONN_PROVIDED    -3
#define ERR_MAX_CONN_OUT_OF_RANGE   -4

#define HAS_VALUE_OFF   0
#define HAS_VALUE_ON    1
#define MAX_DETAIL_LEN  100
#define COLON_CHAR      ':'

typedef struct 
{
    char opt_char;
    char detail[MAX_DETAIL_LEN];
    int has_value;
    int min_value;
    int max_value;
    int assigned_value;

}option_description;

option_description opt_desc[2] =
{
    {
        .opt_char   = 'p',
        .detail     = "Port",
        .has_value  = HAS_VALUE_ON,
        .min_value  = MIN_PORT_VALUE,
        .max_value  = MAX_PORT_VALUE
    },
    {
        .opt_char   = 'c',
        .detail     = "Maximum number of connections",
        .has_value  = HAS_VALUE_ON,
        .min_value  = MIN_CONN_NUM,
        .max_value  = MAX_CONN_NUM
    }
};

/// @brief Generates a string including the options that are meant to be inputed properly formatted.
/// @param option_descr Array of option_description structs. Each one includes information about a parameter.
/// @param option_descr_size Size of the previous input parameter.
/// @param opt_short Short form of command line arguments, properñy formatted.
void PreParseOptions(option_description* option_descr, int option_descr_size, char* opt_short)
{
    char option_short[option_descr_size * 2 + 1];
    memset(option_short, 0, sizeof(option_short));
    
    for(int i = 0; i < option_descr_size; i++)
    {
        option_short[strlen(option_short)] = option_descr[i].opt_char;

        if(option_descr[i].has_value == HAS_VALUE_ON)
        {
            option_short[strlen(option_short)] = COLON_CHAR;
        }
    }

    strcpy(opt_short, option_short);
}

int ParseArguments(int argc, char** argv, option_description* option_descr, int option_descr_size, char* opt_short)
{
    int option;

    while((option = getopt(argc, argv, opt_short)) != -1)
    {
        for(int i = 0; i < option_descr_size; i++)
        {
            if(option == option_descr[i].opt_char)
            {
                if(option_descr[i].assigned_value != 0)
                {
                    printf("Value already assigned to the current parameter.\r\n");
                    return -1;
                }
                else
                {
                    if(option_descr[i].has_value == HAS_VALUE_ON)
                    {
                        if(optarg != NULL)
                        {
                            int option_int = atoi(optarg);
                            if(option_int < option_descr[i].min_value || option_int > option_descr[i].max_value)
                            {
                                return -1;
                            }

                            option_descr[i].assigned_value = option_int;
                        }
                        else
                        {
                            printf("Provide a value for the input parameter [%s].\r\n", option_descr[i].detail);
                            return -1;
                        }
                    }
                }
            }
        }
    }
}

/// @brief Parse server's port.
/// @param argument_list list of argument that is passed through the command line.
/// @return < 0 if any error happened, port number otherwise.
int ParsePort(char** argument_list)
{
    if(argument_list[ARGV_PORT_IDX] == NULL)
    {
        printf("\033[0;31m");
        printf("[ERROR] No port number was provided.\r\n");
        printf("\033[0m");
        return ERR_NO_PORT_PROVIDED;
    }
    
    int server_port = atoi(argument_list[ARGV_PORT_IDX]);

    if(server_port < MIN_PORT_VALUE || server_port > MAX_PORT_VALUE)
    {
        printf("\033[0;31m");
        printf("[ERROR] Address out of acceptable range (%d-%d).\r\n", MIN_PORT_VALUE, MAX_PORT_VALUE);
        printf("\033[0m");
        return ERR_PORT_OUT_OF_RANGE;
    }

    return server_port;
}

/// @brief Parse the maximum number of connections that the server can accept.
/// @param argument_list list of argument that is passed through the command line.
/// @return < 0 if any error happened, maximum number of connections admitted by the server otherwise.
int ParseConnNum(char** argument_list)
{
    if(argument_list[ARGV_MAX_CONN_NUM_IDX] == NULL)
    {
        printf("\033[0;31m");
        printf("[ERROR] No maximum number of connections argument provided.\r\n");
        printf("\033[0m");
        return ERR_NO_MAX_CONN_PROVIDED;
    }

    int max_conn_num = atoi(argument_list[ARGV_MAX_CONN_NUM_IDX]);

    if(max_conn_num < 1 || max_conn_num > 3)
    {
        printf("\033[0;31m");
        printf("[ERROR] Maximum number of connections out of acceptable range (%d-%d).\r\n", MIN_CONN_NUM, MAX_CONN_NUM);
        printf("\033[0m");
        return ERR_MAX_CONN_OUT_OF_RANGE;
    }

    return max_conn_num;
}

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    int opt_desc_size = sizeof(opt_desc) / sizeof(opt_desc[0]);
    char options_short[opt_desc_size * 2 + 1];
    PreParseOptions(opt_desc, opt_desc_size, options_short);

    int parse_arguments = ParseArguments(argc, argv, opt_desc, opt_desc_size, options_short);

    int server_port = ParsePort(argv);
    if(server_port < 0)
    {
        exit(EXIT_FAILURE);
    }

    int max_conn_num = ParseConnNum(argv);
    if(max_conn_num < 0)
    {
        exit(EXIT_FAILURE);
    }

    int socket_desc = CreateSocketDescriptor(AF_INET, SOCK_STREAM, IPPROTO_IP);

    int socket_options = SocketOptions(socket_desc, 1, 1, 1, 5, 5);

    struct sockaddr_in server = PrepareForBinding(AF_INET, INADDR_ANY, server_port);

    int bind_socket = BindSocket(socket_desc, server);

    int listen = SocketListen(socket_desc, max_conn_num);

    int new_socket = SocketAccept(socket_desc);

    int read = SocketRead(new_socket);

    int close = CloseSocket(new_socket);

    return 0;
}
