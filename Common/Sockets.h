#ifndef _SOCKETS_H_
#define _SOCKETS_H_
#include <winsock2.h>
#include <Windows.h>
#include "Common.h"

#define MAX_CLIENTS 1


// Defines a pointer to a function used to search for free slots on the server
// to accept new clients


typedef struct
{
	SOCKET clientSocket;
	SOCKADDR_IN clientInfo;
} ClientSocketArgs;


Result Receive(SOCKET sd, char* OutBuffer, int BytedLeft, int* byteReceived);
Result Send(SOCKET sd, char* Buffer, int bytesLen);
//convert string address to IN_ADDR address
Ip getIpAddress(const char* ipStr);

bool InitWinSock();
int InitClientSocket(SOCKET* socket_c, Ip remoteIp, Port remotePort);
int InitServerSocket(SOCKET* mainSocket, Ip listeningAddress, Port listeningPort);
bool CleanupServerSocket(SOCKET socketToClose);


#endif

