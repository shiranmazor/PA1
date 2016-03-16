#ifndef _SOCKETS_H_
#define _SOCKETS_H_
#include <Windows.h>
#include <winsock2.h>
#include "Shared.h"

#define MAX_CLIENTS 2


// Defines a pointer to a function used to search for free slots on the server
// to accept new clients

typedef enum { SUCCES, FAILED, NOT_CONNECTED } Result;
typedef struct
{
	SOCKET clientSocket;
	SOCKADDR_IN clientInfo;
} ClientSocketArgs;


Result Receive(SOCKET sd, char* OutBuff, int BytedLeft );
Result Send(SOCKET sd, const char* Buffer, int bytesLen);

Ip getIpAddress(const char* ipStr);

bool InitWinSock();
int InitClientSocket(SOCKET* socket_c, Ip remoteIp, Port remotePort);
int InitServerSocket(SOCKET* mainSocket, Ip listeningAddress, Port listeningPort);
//int InitServerSocketSimple(SocketServer* server, Ip listeningAddress, Port listeningPort);
bool CleanupServerSocket(SOCKET socketToClose);


#endif