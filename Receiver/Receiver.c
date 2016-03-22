#define _CRT_SECURE_NO_WARNINGS
#include "Receiver.h"

static ResultMessage ResultData;
static FILE* outputFile;
static SOCKET socketServer;
static unsigned int actualCrc32Res = 0xFFFFFFFF;
static unsigned short int actualCrc16Res = 0;
static unsigned short int actualChecksumRes;
static unsigned int recvCrc32Res;
static unsigned short int recvCrc16Res;
static unsigned short int recvChecksumRes;

bool sendResultsMessage()
{
	Result result = Send(socketServer,(char*)&ResultData, sizeof(ResultMessage));
	// send summary message
	if (result != SUCCES)
	{
		printf("cant send summary message, error code: %d", WSAGetLastError());
		return FALSE;
	}
	if (shutdown(socketServer, SD_SEND) != 0)
	{
		printf("error while shutting down SEND channel\n");
		return FALSE;
	}

	return TRUE;
}

void printResult()
{
	//char fail[4] =;
	char* crc16Msg = malloc(5*sizeof(char));
	char* crc32Msg = malloc(5 * sizeof(char));
	char* checksumMsg = malloc(5 * sizeof(char));
	strcpy(crc16Msg, "FAIL");
	strcpy(crc32Msg, "FAIL");
	strcpy(checksumMsg, "FAIL");

	if (ResultData.crc16 == SUCCES)
		strcpy(crc16Msg, "PASS");
	if (ResultData.crc32 == SUCCES)
		strcpy(crc32Msg, "PASS");
	if (ResultData.checksum == SUCCES)
		strcpy(checksumMsg, "PASS");

	fprintf(stderr, "received: %d bytes written: %d bytes\n", ResultData.received, ResultData.written);
	fprintf(stderr, "CRC-32:%s. Computed 0x%.8x, received 0x%.8x\n", crc32Msg, actualCrc32Res, recvCrc32Res);
	fprintf(stderr, "CRC-16:%s. Computed 0x%.4x, received 0x%.4x\n", crc16Msg, actualCrc16Res, recvCrc16Res);
	fprintf(stderr, "Inet-cksum:%s. Computed 0x%.4x, received 0x%.4x\n", checksumMsg, actualChecksumRes, recvChecksumRes);
	
	free(crc16Msg);
	free(crc32Msg);
	free(checksumMsg);
}

bool HandleData()
{
	byte buff[CHUNK_SIZE], writebuff[CHUNK_SIZE];
	DWordBuffer checkBuff;
	Result res;
	unsigned int checkSumWIP = 0;
	short int numBytes;
	//init values:
	ResultData.checksum = SUCCES;
	ResultData.crc16 = SUCCES;
	ResultData.crc32 = SUCCES;
	initBuff(&checkBuff);
	byte temp[3] = { '1', '1', '\0' };
	int k = 0;
	int *receivedBytes = &k;

	while ((res = Receive(socketServer, (char*)&buff, CHUNK_SIZE, receivedBytes)) == SUCCES)
	{
		int bytesWritten;
		ResultData.received += *receivedBytes;
		if (*receivedBytes == 1) printf("got 1 byte!!\n");
		if ((numBytes = pushToBuff(&checkBuff, buff, writebuff, *receivedBytes)) != 0)
		{
			actualCrc16Res = calcCRC(&writebuff, actualCrc16Res, numBytes, 16);
			actualCrc32Res = calcCRC(&writebuff, actualCrc32Res, numBytes, 32);
			checkSumWIP += calcChecksum(&writebuff, numBytes);
			printf("16: %hu 0x%.4x 32: %u 0x%.8x bytesnum: %d\n", actualCrc16Res, actualCrc16Res, actualCrc32Res, actualCrc32Res, numBytes);
			temp[0] = writebuff[0];
			temp[1] = writebuff[1];
			//save data to disk:
			bytesWritten = fwrite((char*)&writebuff, sizeof(byte), numBytes, outputFile);
			ResultData.written += bytesWritten;
		}
		if (*receivedBytes == 1) printf("got 1 byte!! bytes from buff %d\n", numBytes);
		k = 0;
		receivedBytes = &k;
	}
	printf("temp: %s\n0x%.2x 0x%.2x\n\n", temp, temp[0], temp[1]);
	if (res == FAILED)
		return FALSE;

	if (outputFile != NULL)
	{
		fclose(outputFile);
		outputFile = NULL;
	}

	//close reading direction
	if (shutdown(socketServer, SD_RECEIVE) != 0)
	{
		printf("can't shutdown RECEIVE channel\n");
	}

	actualChecksumRes = closeCheckSum(checkSumWIP);

	reOrderBuff(&checkBuff);
	recvCrc32Res = (checkBuff.buff[0] << 24) + (checkBuff.buff[1] << 16) + (checkBuff.buff[2] << 8) + checkBuff.buff[3];
	recvCrc16Res = (checkBuff.buff[4] << 8) + checkBuff.buff[5];
	recvChecksumRes = (checkBuff.buff[6] << 8) + checkBuff.buff[7];
	//comapre with received error codes
	if (actualCrc32Res != recvCrc32Res) ResultData.crc32 = FAILED;
	if (actualCrc16Res != recvCrc16Res) ResultData.crc16 = FAILED;
	if (actualChecksumRes != recvChecksumRes) ResultData.checksum = FAILED;

	printResult();
	if (sendResultsMessage() == FALSE)
	{
		printf("failed to send result to sender\n");
		return FALSE;
	}
	
	return TRUE;
	
}

int receiverMain(Ip listenIp, Port listenPort, char* filename)
{
	//creating output file
	outputFile = fopen(filename, "wb");

	if (outputFile == NULL)
	{
		printf("Cant open file for write: %s\n", outputFile);
		return -1;
	}

	//create client socket:
	if (InitClientSocket(&socketServer, listenIp, listenPort) == FALSE)
	{
		if (outputFile != NULL)
		{
			fclose(outputFile);
			outputFile = NULL;
		}
		
		return -1;
	}
	

	//get file data and check errors and print sammary:
	if (HandleData() == FALSE)
	{
		if (outputFile != NULL)
		{
			fclose(outputFile);
			outputFile = NULL;
		}
		return -1;
	}
	

	//closesocket(socketServer);

	//in case the file is still open
	if (outputFile != NULL)
	{
		fclose(outputFile);
		outputFile = NULL;
	}
	
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
		return -1;
	}

	listenIp = getIpAddress(argv[1]);
	listenPort = atoi(argv[2]);
	filename = argv[3];

	return receiverMain(listenIp, listenPort, filename);
}