#include "Sender.h"


static SOCKET socket_server;
static FILE* inputFile;

void printSummary(const ResultMessage* summary)
{
	fprintf(stderr, "received:\t%d bytes\n", summary->received);
	//TODO:fix it with crc
	//fprintf(stderr, "reconstructed:\t%d bytes\n", summary->wrote);
	//fprintf(stderr, "corrected:\t%d errors\n", summary->corrected);
}

bool getSummaryMessage( ResultMessage*summary)
{
	if (Receive((char*)summary, sizeof(ResultMessage), socket_server) != SUCCES)
	{
		printf("cant receive summary message\n");
		return FALSE;
	}

	return TRUE;
}

int senderMain(Ip remoteIp, Port remotePort, const char* filename)
{
	ResultMessage summary;
	if (InitializeClientSocket(&socket_server, remoteIp, remotePort) != TRUE)
	{
		printf("Failed to connect sender to channel\n");
		return -1;
	}
	inputFile = fopen(filename, "rb");
	if (inputFile == NULL)
	{
		printf("Cant open file for read: %s\n", inputFile);
		return -1;
	}
	

	//TODO: calc CRC  and checksun add it to the end of file
	// TODO:ssending data to channel

	//finish, closing and waiting for reciever
	printf("Finished sending data\n");
	if (shutdown(socket_server, SD_SEND) != 0)
	{
		printf("error while shutting down send channel\n");
		closesocket(socket_server);
		WSACleanup();
		fclose(inputFile);
		return -1;
	}

	// finished transmitting data, wait for summary message
	if (getSummaryMessage(&summary) == TRUE)
	{
		printSummary(&summary);
	}

	closesocket(socket_server);
	WSACleanup();
	fclose(inputFile);

	return 0;
}

void printUsage()
{
	printf("Usage: sender.exe <remote_ip> <remote_port> <filename>");
}

int main(int argc, char** argv)
{
	Port remotePort;
	Ip remoteIp;
	char* filename;

	if (argc != 4)
	{
		printUsage();
	}

	remoteIp = getIpAddress(argv[1]);
	remotePort = atoi(argv[2]);
	filename = argv[3];

	return senderMain(remoteIp, remotePort, filename);
}