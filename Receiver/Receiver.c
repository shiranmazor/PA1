#include "Receiver.h"

static SummaryMessage summary;
static FILE* outputFile;
static SOCKET socketServer;

bool sendSummaryMessage(SOCKET socket)
{
	Result result = Send((const char*)&summary, sizeof(SummaryMessage), socket);
	// send summary message
	if (result != SUCCES)
	{
		printf("cant send summary message, error code: %d", WSAGetLastError());
		return FALSE;
	}
	if (shutdown(socket, SD_SEND) != 0)
	{
		printf("error while shutting down SEND channel\n");
		return FALSE;
	}

	return TRUE;
}

int receiver_main(listenIp, listenPort, filename)
{
	//creating output file
	outputFile = fopen(filename, "wb");
	int res;
	if (outputFile == NULL)
	{
		printf("Cant open file for write: %s\n", outputFile);
		return -1;
	}

	//create client socket:
	if (InitializeClientSocket(&socketServer, listenIp, listenPort) != 0)
	{
		fclose(outputFile);
		return -1;
	}
	summary.received = 0;
	summary.crc16 = 0;
	summary.crc32 = 0;
	summary.checksum = 0;

	//get file data and check errors and print sammary:
	res = HandleData((void*)&socketServer);
	// end of reciver task - cleanup reciever:

	closesocket(socketServer);

	if (outputFile != NULL)
		fclose(outputFile);
	return 0;
}
	
	
	
void printUsage()
{
	printf("Usage: sender.exe <listening_ip> <listening_port> <outputFilename>");
}

int main(int argc, char** argv)
{
	Port listenPort;
	Ip listenIp;
	char* filename;

	if (argc != 4)
	{
		printUsage();
	}

	listenIp = getIpAddress(argv[1]);
	listenPort = atoi(argv[2]);
	filename = argv[3];

	startReceiver(listenIp, listenPort, filename);
}