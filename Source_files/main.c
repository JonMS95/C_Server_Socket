#include <stdio.h>
#include <sys/socket.h> // socket, bind, listen accept functions.
#include <stdlib.h>     // EXIT_FAILURE
#include <arpa/inet.h>  // sockaddr_in, inet_addr
#include <unistd.h>     // Write socket.
#include <string.h>     // strcpy
#include <netinet/tcp.h>

#define TEST_PORT       8080    // TCP testing port.
#define CONN_NUM        3       // Maximum number of concurrent connections that the socket may attend to.
#define IP_ADDR_SIZE    15      // IP address string size.
#define GREETING_SIZE   100
#define RX_BUFFER_SIZE  256     // RX buffer size.

/*
@brief Main function. Program's entry point.
*/
int main(int argc, char** argv)
{
    printf("Start.\r\n");
    // Create socket descriptor.
    // Use AF_INET if the socket is meant to be serving to another computer in the same net.
    // Use AF_LOCAL the purpose is to communicate different processes within the same host.
    // Use SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
    // Use 0 to use the internet protocol (IP).
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0)
    {
        perror("Socket file descriptor creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket file descriptor created.\r\n");

    // Set socket options.
    int reuse_address   = 1;
    int reuse_port      = 1;
    int keep_idle       = 1;
    int keep_counter    = 5;
    int keep_interval   = 5;
    int socket_options;

    socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR , &reuse_address, sizeof(reuse_address    ));
    socket_options = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEPORT , &reuse_port   , sizeof(reuse_port       ));
    socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPIDLE , &keep_idle    , sizeof(keep_idle        ));
    socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPCNT  , &keep_counter , sizeof(keep_counter     ));
    socket_options = setsockopt(socket_desc, SOL_TCP   , TCP_KEEPINTVL, &keep_interval, sizeof(keep_interval    ));

    if(socket_options < 0)
    {
        perror("Failed to set socket options");
    }
    printf("Successfully set socket options.\r\n");

    // Prepare the sockaddr_in structure for the binding process.
    struct sockaddr_in server;
    server.sin_family = AF_INET;            // IPv4.
    server.sin_addr.s_addr = INADDR_ANY;    // Any address is allowed to connect to the socket.
    server.sin_port = htons(TEST_PORT);     // The htons() function makes sure that numbers are stored in memory
                                            // in network byte order, which is with the most significant byte first.

    // Bind: attach the socket descriptor to a particular IP and port.
    socklen_t file_desc_len = (socklen_t)sizeof(struct sockaddr_in);
    int bind_socket_desc = bind(socket_desc, (struct sockaddr*)&server, file_desc_len);
    if(bind_socket_desc  < 0)
    {
        perror("Socket binding failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket file descriptor binded.\r\n");

    // Listen for incoming connections.
    listen(socket_desc, CONN_NUM);

    // Wait for incoming connections.
    struct sockaddr_in client;
    int new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&file_desc_len);

    if(new_socket < 0)
    {
        perror("Accept failed");
    }
    printf("Connection accepted.\r\n");

    int keep_alive = 1;
    socket_options = setsockopt(new_socket, SOL_SOCKET, SO_KEEPALIVE , &keep_alive, sizeof(keep_alive));

    // Send a message to the client as soon as it is accepted.
    char client_ip_addr[IP_ADDR_SIZE + 1];
    memset(client_ip_addr, 0, sizeof(client_ip_addr));
    strcpy(client_ip_addr, inet_ntoa(client.sin_addr));

    char greeting[GREETING_SIZE + 1];
    memset(greeting, 0, sizeof(greeting));
    sprintf(greeting, "Hello client!\r\nYour IP address is: %s\r\n", inet_ntoa(client.sin_addr));
    
    write(new_socket, greeting, sizeof(greeting));

    char rx_buffer[RX_BUFFER_SIZE];
    memset(rx_buffer, 0, sizeof(rx_buffer));

    ssize_t read_from_socket = 0;
    while(read_from_socket >= 0)
    {
        read_from_socket = read(new_socket, rx_buffer, sizeof(rx_buffer));
        // Read data from buffer.
        if (read_from_socket > 0)
        {
            printf("Data read from RX buffer: <%s>\r\n", rx_buffer);
            memset(rx_buffer, 0, read_from_socket);
        }
        else if(read_from_socket <= 0)
        {
            printf("An error happened while reading.\r\n");
            break;
        }
    }

    // Close the socket.
    close(new_socket);

    return 0;
}
