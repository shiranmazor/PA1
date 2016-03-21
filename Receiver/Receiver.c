#define _CRT_SECURE_NO_WARNINGS
#include "Receiver.h"

static ResultMessage ResultData;
static FILE* outputFile;
static SOCKET socketServer;
static unsigned int actualCrc32Res;
static unsigned short int actualCrc16Res;
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
	

	while ((res = Receive(socketServer, (char*)&buff, CHUNK_SIZE)) == SUCCES)
	{
		int bytesWritten;
		ResultData.received += CHUNK_SIZE;

		if ((numBytes = pushToBuff(&checkBuff, buff, writebuff, CHUNK_SIZE)) != 0)
		{
			actualCrc16Res ^= calcCRC(&writebuff, actualCrc16Res, CRC16POLY);
			actualCrc32Res ^= calcCRC(&writebuff, actualCrc32Res, CRC32POLY);
			checkSumWIP += calcChecksum(&writebuff, numBytes);
			
			//save data to disk:
			bytesWritten = fwrite((char*)&writebuff, sizeof(byte), numBytes, outputFile);
			ResultData.written += bytesWritten;
		}
	}
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

	// get crc32, crc16 and checkSum value from the buffer and verify
	reOrderBuff(&checkBuff);
	recvCrc32Res = (checkBuff.buff[0] << 24) + (checkBuff.buff[1] << 16) + (checkBuff.buff[2] << 8) + checkBuff.buff[3];
	recvCrc16Res = (checkBuff.buff[4] << 8) + checkBuff.buff[5];
	recvChecksumRes = (checkBuff.buff[6] << 8) + checkBuff.buff[7];

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