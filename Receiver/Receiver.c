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
	Result result = Send(socketServer,(const char*)&ResultData, sizeof(ResultMessage));
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

void printResult()
{
	char* crc16Msg = 'FAIL';
	char* crc32Msg = 'FAIL';
	char* checksumMsg = 'FAIL';

	if (ResultData.crc16 == SUCCES)
		crc16Msg = 'PASS';
	if (ResultData.crc32 == SUCCES)
		crc32Msg = 'PASS';
	if (ResultData.checksum == SUCCES)
		checksumMsg = 'PASS';

	fprintf(stderr, "received: %d bytes written: %d bytes\n", ResultData.received, ResultData.written);
	fprintf(stderr, "CRC-32:%s. Computed %ld, received %ld\n", crc32Msg, actualCrc32Res, recvCrc32Res);
	fprintf(stderr, "CRC-16:%s. Computed %ld, received %ld\n", crc16Msg, actualCrc16Res, recvCrc16Res);
	fprintf(stderr, "Inet-cksum:%s. Computed %ld, received %ld\n", checksumMsg, actualChecksumRes, recvChecksumRes);
}

bool HandleData(FILE* fileHandle)
{
	byte buff;
	Result res;
	//init values:
	ResultData.checksum = SUCCES;
	ResultData.crc16 = SUCCES;
	ResultData.crc32 = SUCCES;

	while ((res = Receive(socketServer, (char*)&buff, CHUNK_SIZE)) == SUCCES)
	{
		int bytesWritten;
		//rececied new byte
		ResultData.received += 1;
		//Todo:calc acomulative crc...and save  it

		//save data to disk:
		bytesWritten = fwrite((char*)&buff, sizeof(byte), CHUNK_SIZE, fileHandle);
		ResultData.written += bytesWritten;
	}
	if (res == FAILED)
		return FALSE;

	fclose(fileHandle);
	outputFile = NULL;

	//close reading direction
	if (shutdown(*socket, SD_RECEIVE) != 0)
	{
		printf("cant shutdown RECEIVE channel\n");
	}

	printResult();
	if (sendResultsMessage() == FALSE)
	{
		printf("failed to send resut to sender\n");
		return FALSE;
	}
	
	return TRUE;
	
}

int receiverMain(listenIp, listenPort, filename)
{
	//creating output file
	outputFile = fopen(filename, "wb");

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
	

	//get file data and check errors and print sammary:
	if (HandleData(outputFile) == FALSE)
	{
		printf("Error receiving data from channel");
		fclose(outputFile);
		return -1;
	}
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

	return receiverMain(listenIp, listenPort, filename);
}