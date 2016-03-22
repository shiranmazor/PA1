#include "Channel.h"

static SOCKET listenSocketSender;
static SOCKET listenSocketReceiver;
static ClientSocketArgs senderSocket;
static ClientSocketArgs receiverSocket;
static int bitsFlipped = 0;
static double pError;


void printOutputMessage(IN_ADDR  senderAddress, IN_ADDR  receiverAddress, int byteReceived)
{
	fprintf(stderr, "sender: %s\n", inet_ntoa(senderAddress));
	fprintf(stderr, "receiver: %s\n", inet_ntoa(receiverAddress));
	fprintf(stderr, "%d bytes flipped %d bits\n", byteReceived, bitsFlipped);
}
bool shouldFlip()
{
	// generate a random number between [0,1]
	float x = ((float)rand() / (float)(RAND_MAX));

	// convert Bernoulli(pError)
	return x < pError ? TRUE : FALSE;
}

byte flipBits(byte msg)
{
	int i;
	byte flipMask = 0;
	for (i = 0; i < 8; i++)
	{
		bool flip = shouldFlip();
		if (flip)
			bitsFlipped++;

		flipMask = (flipMask << i) | flip;
	}

	return msg ^ flipMask;
}

int channelMain(Port senderPort, Port receiverPort, double bit_error_p, unsigned int randSeed)
{
	Ip listenIp;
	listenIp.S_un.S_addr = INADDR_ANY;

	pError = bit_error_p;
	srand(randSeed);

	//init sender and listen to reciever
	if (InitServerSocket(&listenSocketReceiver, listenIp, receiverPort) == FALSE)
	{
		printf("Error in opening Channel  server, can't listening to receiver\n");
		return -1;
	}

	//init server and listen to sender:
	if (InitServerSocket(&listenSocketSender, listenIp, senderPort) == FALSE)
	{
		printf("Error in opening Channel  server, can't listening to sender\n");
		return -1;
	}

	//now we will accept the sender  and accept the socket receiver and will flip every  bit
	handleSenderFile();

	//getting reciver message and send back to sender
	ResultMessage message = handleReceiverMessage();

	//close sockets to reciver and sender:
	if (CleanupServerSocket(listenSocketSender) == FALSE)
		return -1;
	if (CleanupServerSocket(listenSocketReceiver) == FALSE)
		return -1;
	
	int res = WSACleanup();
	if (res == SOCKET_ERROR)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		return -1;
	}
	//print output message
	printOutputMessage(senderSocket.clientInfo.sin_addr, receiverSocket.clientInfo.sin_addr, message.received);
	return 0;
}

/*
accept both sockets from sender and reciver and flip bytes from sender, send them to reciever
*/
int handleSenderFile()
{
	byte buffer[CHUNK_SIZE];
	byte ErrorBuffer[CHUNK_SIZE];
	int sizeSender = sizeof(senderSocket.clientInfo);
	int receiverSender = sizeof(receiverSocket.clientInfo), i;

	//accept sender socket and reciever socket:
	receiverSocket.clientSocket = accept(listenSocketReceiver, (SOCKADDR*)&receiverSocket.clientInfo, &receiverSender);
	senderSocket.clientSocket = accept(listenSocketSender, (SOCKADDR*)&senderSocket.clientInfo, &sizeSender);
	int n = 0;
	int *bytesRec;
	bytesRec = &n;

	// both socket are connected, start reading data
	Result res = Receive(senderSocket.clientSocket, (char*)&buffer, CHUNK_SIZE, bytesRec);
	while (res == SUCCES)
	{
		printf("Rec bytes %d\n", *bytesRec);
		// create error in buffer - flip  chunk bytes (per bit)
		for (i = 0; i < *bytesRec; i++)
			ErrorBuffer[i] = flipBits(buffer[i]);
		if (*bytesRec == 1) printf("got 1 byte!!\n");
		//sending all error chunk to receiver
		if (Send(receiverSocket.clientSocket, (char*)&ErrorBuffer, *bytesRec) != SUCCES)
		{
			printf("error sending data to receiver\n");
		}
		n = 0;
		bytesRec = &n;
		res = Receive(senderSocket.clientSocket, (char*)&buffer, CHUNK_SIZE, bytesRec);
	}
	if (res == NOT_CONNECTED && *bytesRec > 0)
	{
		printf("Rec bytes %d\n", *bytesRec);
		// create error in buffer - flip  chunk bytes (per bit)
		for (i = 0; i < *bytesRec; i++)
			ErrorBuffer[i] = flipBits(buffer[i]);

		//sending all error chunk to receiver
		if (Send(receiverSocket.clientSocket, (char*)&ErrorBuffer, *bytesRec) != SUCCES)
		{
			printf("error sending data to receiver\n");
		}
	}

	// sender finished sending, signal receiver
	//after recieving all bytes shutdown the socket
	shutdown(receiverSocket.clientSocket, SD_SEND);
	return 0;
}
ResultMessage handleReceiverMessage()
{
	int n = 0;
	int *bytesRec = &n;
	ResultMessage message;
	// now transfer the message from receiver to sender
	// now transfer the message from receiver to sender
	if (Receive(receiverSocket.clientSocket, (char*)&message, sizeof(ResultMessage), bytesRec) == SUCCES)
	{
		if (Send(senderSocket.clientSocket, (char*)&message, sizeof(ResultMessage)) != SUCCES)
		{
			// handle error
			printf("error sending data to sender\n");
		}
	}
	return message;
}

void printUsage()
{
	printf("Usage: channel.exe <sender_port> <receiver_port> <bit_error_probability> <random_seed>\n");
}


int main(int argc, char** argv)
{
	Port senderPort;
	Port receiverPort;
	double bit_error_p;
	unsigned int random_seed;
	double num = pow(2, 16);

	if (argc != 5)
	{
		printUsage();
		return -1;
	}

	senderPort = atoi(argv[1]);
	receiverPort = atoi(argv[2]);
	bit_error_p = atof(argv[3]);
	random_seed = atoi(argv[4]);

	bit_error_p = bit_error_p / num;
	

	if (receiverPort == senderPort)
	{
		printUsage();
		return -1;
	}

	if (channelMain(senderPort, receiverPort, bit_error_p, random_seed) != 0)
	{
		printf("error while running channel\n");
		return -1;
	}

	return 0;
}
