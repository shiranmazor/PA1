#define _CRT_SECURE_NO_WARNINGS
#include "Sender.h"


static SOCKET socket_server;
static FILE* inputFile;
static unsigned int crc32Res;
static unsigned short int crc16Res;
static unsigned short int checksumRes;

void printResults(const ResultMessage* msg)
{
	//char fail[4] = 'FAIL';
	char* crc16Msg = malloc(5 * sizeof(char));
	char* crc32Msg = malloc(5 * sizeof(char));
	char* checksumMsg = malloc(5 * sizeof(char));
	strcpy(crc16Msg, "FAIL");
	strcpy(crc32Msg, "FAIL");
	strcpy(checksumMsg, "FAIL");

	if (msg->crc16 == SUCCES)
		strcpy(crc16Msg, "PASS");
	if (msg->crc32 == SUCCES)
		strcpy(crc32Msg, "PASS");
	if (msg->checksum == SUCCES)
		strcpy(checksumMsg, "PASS");
	
	fprintf(stderr, "CRC-32: %s; CRC-16: %s; Intenet cksum: %s\n", crc32Msg, crc16Msg, checksumMsg);
	free(crc16Msg);
	free(crc32Msg);
	free(checksumMsg);
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
	unsigned int checkSumWIP = 0;
	byte fileBuff[CHUNK_SIZE], tail[8];

	numBytesRead = fread(&fileBuff, sizeof(byte), CHUNK_SIZE, inputFile);
	while (numBytesRead > 0)
	{
		//calc crc and checksum codes on fileBuff:
		crc16Res ^= calcCRC(&fileBuff, crc16Res, CRC16POLY);
		crc32Res ^= calcCRC(&fileBuff, crc32Res, CRC32POLY);
		checkSumWIP += calcChecksum(&fileBuff, numBytesRead);

		//sending file buffer:
		if (Send(socket_server, (char*)fileBuff, CHUNK_SIZE) == FAILED)
		{
			printf("Failed to send data to channel\n");
			fclose(inputFile);	// TODO: proper cleanup
			return FALSE;
		}
		numBytesRead = fread(&fileBuff, sizeof(byte), CHUNK_SIZE, inputFile);
	}
	checksumRes = closeCheckSum(checkSumWIP);

	// insert crc32, crc16 and checkSum into a char buffer for sending
	tail[0] = (crc32Res >> 24) & 0xFF;
	tail[1] = (crc32Res >> 16) & 0xFF;
	tail[2] = (crc32Res >> 8) & 0xFF;
	tail[3] = crc32Res & 0xFF;
	tail[4] = (crc16Res >> 8) & 0xFF;
	tail[5] = crc16Res & 0xFF;
	tail[6] = (checksumRes >> 8) & 0xFF;
	tail[7] = checksumRes & 0xFF;

	if (Send(socket_server, (char*)tail, 8) == FAILED)
	{
		printf("Failed to send tail to channel\n");
		fclose(inputFile);	// TODO: proper cleanup
		return FALSE;
	}

	fclose(inputFile);	// TODO: proper cleanup

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
		return -1;
	}

	channelIp = getIpAddress(argv[1]);
	channelPort = atoi(argv[2]);
	filename = argv[3];

	return senderMain(channelIp, channelPort, filename);
}