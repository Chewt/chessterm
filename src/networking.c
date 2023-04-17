#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "networking.h"

int g_server_fd = -1;

int SetupServer(int port)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Forcefully attaching socket to the chosen port
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
  
    // Forcefully attaching socket to the chosen port
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket
         = accept(server_fd, (struct sockaddr*)&address,
                  (socklen_t*)&addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
  
    g_server_fd = server_fd;
    return new_socket;
}

void CloseServer(int socket)
{
    // closing the connected socket
    close(socket);
    // closing the listening socket
    shutdown(g_server_fd, SHUT_RDWR);
}

int SetupClient(char* hostname, int port)
{
    // Resolve hostname to IP address
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in serv_addr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int rv;
    if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
    {
        printf("\n address not valid \n");
        return -1;
    }
    char ip[50];
    for (p = servinfo; p != NULL ; p = p->ai_next)
    {
        strcpy(ip, inet_ntoa(((struct sockaddr_in*)p->ai_addr)->sin_addr));
    }

    // Set up socket
    int status, client_fd;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

  
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    return client_fd;
}

void CloseClient(int socket)
{
    close(socket);
}

/* RecvCommand expects to see two newline characters, so be sure to send exactly
 * two. If you send more, then the next command you send will have an extra
 * newline at the beginning. In this case the user input already has a newline
 * so I just add one more before sending.
 */
void SendCommand(int fd, char* command)
{
    char buf[256];
    int index = 0;
    index = snprintf(buf + index, 256, "%s\n\n", command);
    buf[index] = '\0';
    send(fd, buf, strlen(buf), 0);
}

char* RecvCommand(int fd)
{
    char* m = malloc(256);
    int bytes = 0;
    int i;
    for (i = 0; i < 255; ++i)
    {
        int resp = read(fd, m + bytes, 1);
        if (resp <= 0)
        {
            free(m);
            return NULL;
        }
        bytes += resp;
        if (bytes > 2 && (m[bytes-1] == '\n') && (m[bytes-2] == '\n'))
            break;
    }
    m[bytes-2] = '\0';
    m[255] = '\0';
    return m;
}
