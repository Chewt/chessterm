#ifndef NETWORKING_H
#define NETWORKING_H

int SetupServer(int port);
void CloseServer(int socket);
int SetupClient(char* ip, int port);
void CloseClient(int socket);
void SendCommand(int fd, char* command);
char* RecvCommand(int fd);

#endif 
