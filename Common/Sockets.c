#pragma warning(disable: 4996)
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "sockets.h"

static bool winSockInit = FALSE;

bool InitWinSock()
{
	WSADATA wsa;
	if (winSockInit)
		return TRUE;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (iResult != NO_ERROR)
	{
		printf("Failed in WSAStartup. Error Code : %d", WSAGetLastError());
		return FALSE;
	}
	winSockInit = TRUE;
	return TRUE;

}

Ip getIpAddress(const char* ipStr)
{
	Ip address;
	if (strcmp(ipStr, "localhost") == 0)
		address.S_un.S_addr = inet_addr("127.0.0.1");
	else
		address.S_un.S_addr = inet_addr(ipStr);

	return address;
}


Result Send(SOCKET sd, char* Buffer, int bytesLen)
{
	int BytesSent, remainingBytes;
	char* buffPtr = Buffer;
	remainingBytes = bytesLen;

	while (remainingBytes > 0)
	{
		BytesSent = send(sd, buffPtr, remainingBytes, 0); //send bytes and check if all bytes sent
		if (BytesSent == SOCKET_ERROR)
			return FAILED;
		remainingBytes -= BytesSent;
		buffPtr += BytesSent;
	}

	return SUCCES;
}

Result Receive(SOCKET sd, char* OutBuffer, int BytedLeft, int* byteReceived)
{

	int BytesTransferred, remainingBytes;
	char* buffPtr = OutBuffer;
	remainingBytes = BytedLeft;
	*byteReceived = 0;

	while (remainingBytes > 0)
	{
		BytesTransferred = recv(sd, buffPtr, remainingBytes, 0);
		if (BytesTransferred == SOCKET_ERROR)
			return FAILED;
		else if (BytesTransferred == 0)
			return NOT_CONNECTED; // recv() returns zero if connection was  disconnected - finish 

		remainingBytes -= BytesTransferred;
		buffPtr += BytesTransferred;
		*byteReceived += BytesTransferred;
		
	}
	
	return SUCCES;
}

int InitClientSocket(SOCKET* socket_c, Ip remoteIp, Port remotePort)
{
	SOCKADDR_IN clientInfo;

	if (!InitWinSock())
		return FALSE;

	// Create a socket.
	*socket_c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	if (*socket_c == INVALID_SOCKET) {
		printf("Error at creating socket(): %ld\n", WSAGetLastError());
		return FALSE;
	}

	clientInfo.sin_family = AF_INET;
	clientInfo.sin_addr = remoteIp;
	clientInfo.sin_port = htons(remotePort);

	// Call the connect function
	int status = connect(*socket_c, (SOCKADDR*)&clientInfo, sizeof(clientInfo));
	if (status == SOCKET_ERROR)
	{
		printf("Failed to connect to %s:%d. Error: %ld\n", inet_ntoa(remoteIp), remotePort, WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

/*
without accept
*/
int InitServerSocket(SOCKET* mainSocket, Ip listeningAddress, Port listeningPort)
{
	SOCKADDR_IN service;
	int bindResult, ListenResult;

	if (!InitWinSock())
	{
		return FALSE;
	}

	// Create a socket.    
	*mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*mainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
	}

	service.sin_family = AF_INET;
	service.sin_addr = listeningAddress;
	service.sin_port = htons(listeningPort);

	// Call the bind function
	bindResult = bind(*mainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindResult == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		closesocket(*mainSocket);
		return FALSE;
	}

	// Listen on the Socket.
	ListenResult = listen(*mainSocket, SOMAXCONN);
	if (ListenResult == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		closesocket(*mainSocket);
		return FALSE;
	}


	return TRUE;
}


bool CleanupServerSocket(SOCKET socketToClose)
{
	if (closesocket(socketToClose) == SOCKET_ERROR)
	{
		printf("Error while closing the socket\n");
		return FALSE;
	}
	
	return TRUE;


}