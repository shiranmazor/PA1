#define _CRT_SECURE_NO_WARNINGS
#include "Receiver.h"

static ResultMessage ResultData;
static FILE* outputFile;
static SOCKET socketServer;
static unsigned long actualCrc32Res;
static unsigned long actualCrc16Res;
static unsigned long actualChecksumRes;
static unsigned long recvCrc32Res;
static unsigned long recvCrc16Res;
static unsigned long recvChecksumRes;

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
	strcpy(crc32Msg, "FAIL");

	if (ResultData.crc16 == SUCCES)
		strcpy(crc16Msg, "PASS");
	if (ResultData.crc32 == SUCCES)
		strcpy(crc32Msg, "PASS");
	if (ResultData.checksum == SUCCES)
		strcpy(checksumMsg, "PASS");

	fprintf(stderr, "received: %d bytes written: %d bytes\n", ResultData.received, ResultData.written);
	fprintf(stderr, "CRC-32:%s. Computed %ld, received %ld\n", crc32Msg, actualCrc32Res, recvCrc32Res);
	fprintf(stderr, "CRC-16:%s. Computed %ld, received %ld\n", crc16Msg, actualCrc16Res, recvCrc16Res);
	fprintf(stderr, "Inet-cksum:%s. Computed %ld, received %ld\n", checksumMsg, actualChecksumRes, recvChecksumRes);
	
	free(crc16Msg);
	free(crc32Msg);
	free(checksumMsg);
}

bool HandleData()
{
	byte buff[CHUNK_SIZE], writebuff[CHUNK_SIZE];
	DWordBuffer checkBuff;
	Result res;
	unsigned int crc32 = 0, crc32Sender;
	unsigned short int crc16 = 0, crc16Sender;
	short int checkSum = 0, checkSumSender, numBytes;
	//init values:
	ResultData.checksum = SUCCES;
	ResultData.crc16 = SUCCES;
	ResultData.crc32 = SUCCES;
	initBuff(checkBuff);

	while ((res = Receive(socketServer, (char*)&buff, CHUNK_SIZE)) == SUCCES)
	{
		int bytesWritten;
		ResultData.received += CHUNK_SIZE;

		if ((numBytes = pushToBuff(checkBuff, buff, writebuff, CHUNK_SIZE)) != 0)
		{
			crc16 ^= calcCRC(&writebuff, crc16, CRC16POLY);
			crc32 ^= calcCRC(&writebuff, crc32, CRC32POLY);
			checkSum += calcChecksum(&writebuff, numBytes);
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

	// get crc32, crc16 and checkSum value from the buffer and verify
	reOrderBuff(checkBuff);
	crc32Sender = checkBuff.buff[0] << 24 + checkBuff.buff[1] << 16 + checkBuff.buff[2] << 8 + checkBuff.buff[3];
	crc16Sender = checkBuff.buff[4] << 8 + checkBuff.buff[5];
	checkSumSender = checkBuff.buff[6] << 8 + checkBuff.buff[7];
	if (crc32 != crc32Sender) ResultData.crc32 = FAILED;
	if (crc16 != crc16Sender) ResultData.crc16 = FAILED;
	if (checkSum != checkSumSender) ResultData.checksum = FAILED;

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
	// end of reciver task - cleanup reciever:

	closesocket(socketServer);

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
	}

	listenIp = getIpAddress(argv[1]);
	listenPort = atoi(argv[2]);
	filename = argv[3];

	return receiverMain(listenIp, listenPort, filename);
}