#define _CRT_SECURE_NO_WARNINGS
#include "Sender.h"


static SOCKET socket_server;
static FILE* inputFile;
static unsigned int crc32Res = 0xFFFFFFFF;
static unsigned int c32 = 0xFFFFFFFF;
static unsigned short int crc16Res = 0;
static unsigned short int c16 = 0;
static unsigned short int checksumRes;

void printResults(const ResultMessage* msg)
{
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
	byte temp[3] = { '1', '1', '\0' };

	numBytesRead = fread(&fileBuff, sizeof(byte), CHUNK_SIZE, inputFile);
	while (numBytesRead > 0)
	{
		//calc crc and checksum codes on fileBuff:
		//crc16Res = calcCRC16(&fileBuff, crc16Res);
		//crc32Res = calcCRC32(&fileBuff, crc32Res);
		crc16Res = calcCRC(&fileBuff, crc16Res, 16);
		crc32Res = calcCRC(&fileBuff, crc32Res, 32);
		printf("16: %hu 32: %u\n", crc16Res, crc32Res);
		//c16 = calcCRC(&fileBuff, c16, 16);
		//c32 = calcCRC(&fileBuff, c32, 32);
		checkSumWIP += calcChecksum(&fileBuff, numBytesRead);
		temp[0] = fileBuff[0];
		temp[1] = fileBuff[1];
		//sending file buffer:
		if (Send(socket_server, (char*)fileBuff, CHUNK_SIZE) == FAILED)
		{
			printf("Failed to send data to channel\n");
			fclose(inputFile);
			return FALSE;
		}
		numBytesRead = fread(&fileBuff, sizeof(byte), CHUNK_SIZE, inputFile);
	}
	printf("temp: %s\n0x%.2x 0x%.2x\n\n", temp, temp[0], temp[1]);
	//printf("16: %hu %d 0x%.4x\n16: %hu %d 0x%.4x\n32: %u %d 0x%.8x\n32: %u %d 0x%.8x\n", crc16Res, crc16Res, crc16Res, c16, c16, c16, crc32Res, crc32Res, crc32Res, c32, c32, c32);
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
		fclose(inputFile);
		return FALSE;
	}

	fclose(inputFile);

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