#define _CRT_SECURE_NO_WARNINGS
#include "Sender.h"


static SOCKET socket_server;
static FILE* inputFile;
static unsigned long crc32Res;
static unsigned long crc16Res;
static unsigned long checksumRes;

void printResults(const ResultMessage* msg)
{
	char* crc16Msg = 'FAIL';
	char* crc32Msg = 'FAIL';
	char* checksumMsg = 'FAIL';

	if (msg->crc16 == SUCCES)
		crc16Msg = 'PASS';
	if (msg->crc32 == SUCCES)
		crc32Msg = 'PASS';
	if (msg->checksum == SUCCES)
		checksumMsg = 'PASS';
	
	fprintf(stderr, "CRC-32: %s; CRC-16: %s; Intenet cksum: %s\n", crc32Msg, crc16Msg, checksumMsg);
}

bool getResultMessage(ResultMessage* msg)
{
	if (Receive(socket_server,(char*)msg, sizeof(ResultMessage)) != SUCCES)
	{
		printf("cant receive summary message\n");
		return FALSE;
	}

	return TRUE;
}

bool sendingFileData()
{
	int numBytesRead = 0;
	byte fileBuff;

	do
	{
		numBytesRead = fread(&fileBuff, sizeof(byte), CHUNK_SIZE, inputFile);

		//here we will create the crc codes on fileBuff:

		//sending file buffer:
		if (Send(socket_server, (const char*)fileBuff, CHUNK_SIZE) == FALSE)
		{
			printf("Failed to send data to channel\n");
			fclose(inputFile);	// TODO: proper cleanup
			return FALSE;
		}
	} while (numBytesRead > 0);

	return TRUE;
}

int senderMain(Ip remoteIp, Port remotePort, const char* filename)
{
	ResultMessage message;
	if (InitClientSocket(&socket_server, remoteIp, remotePort) != TRUE)
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
	
	// TODO:sending data to channel
	if (sendingFileData() != TRUE)
	{
		printf("Error in sending data");
		return -1;
	}

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

	// finished transmitting data, wait for result message from receiver
	if (getResultMessage(&message) == TRUE)
	{
		printResults(&message);
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
	Port channelPort;
	Ip channelIp;
	char* filename;

	if (argc != 4)
	{
		printUsage();
	}

	channelIp = getIpAddress(argv[1]);
	channelPort = atoi(argv[2]);
	filename = argv[3];

	return senderMain(channelIp, channelPort, filename);
}