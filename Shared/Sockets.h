#ifndef _SOCKETS_H_
#define _SOCKETS_H_
#include <Windows.h>
#include <winsock2.h>
#include "Shared.h"


// Defines a pointer to a function used to search for free slots on the server
// to accept new clients

typedef enum { SUCCES, FAILED, NOT_CONNECTED } Result;


Result Receive(SOCKET sd, char* OutBuff, int BytedLeft );

Result Send(SOCKET sd, const char* Buffer, int bytesLen);

Ip getIpAddress(const char* ipStr);

bool InitWinSock();
int InitClientSocket(SOCKET* socket_c, Ip remoteIp, Port remotePort);
int InitServerSocket(SOCKET* mainSocket, Ip listeningAddress, Port listeningPort,  SOCKET* socketsArray);
//int InitServerSocketSimple(SocketServer* server, Ip listeningAddress, Port listeningPort);
void CleanupServerSocket();


#endif