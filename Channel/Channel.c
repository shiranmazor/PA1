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

	//init server and listen to sender:
	if (InitServerSocket(&listenSocketSender, listenIp, senderPort) == FALSE)
	{
		printf("Error in opening Channel  server, can't listening to sender\n");
		return -1;
	}

	//init sender and listen to reciever
	if (InitServerSocket(&listenSocketReceiver, listenIp, receiverPort) == FALSE)
	{
		printf("Error in opening Channel  server, can't listening to receiver\n");
		return -1;
	}

	printf("waiting for connections...\n");

	//now we will accept the sender  and accept the socket receiver and will flip every  bit
	handleSenderFile();

	//getting reciver message and send back to sender
	ResultMessage message = handleReceiverMessage();

	//close sockets to reciver and sender:
	if (CleanupServerSocket(listenSocketSender) == FALSE)
		return -1;
	if (CleanupServerSocket(listenSocketReceiver) == FALSE)
		return -1;

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
	int receiverSender = sizeof(receiverSocket.clientInfo);

	//accept sender socket and reciever socket:
	senderSocket.clientSocket = accept(listenSocketSender, (SOCKADDR*)&senderSocket.clientInfo, &sizeSender);
	receiverSocket.clientSocket = accept(listenSocketReceiver, (SOCKADDR*)&receiverSocket.clientInfo, &receiverSender);

	// both socket are connected, start reading data
	while (Receive(senderSocket.clientSocket, &buffer, CHUNK_SIZE) == SUCCES)
	{
		// create error in buffer - flip  chunk bytes (per bit)
		for (int i = 0; i < CHUNK_SIZE; i++)
		{
			ErrorBuffer[i] = flipBits(buffer[i]);
		}
		
		//sending all error chunk to receiver
		if (Send(receiverSocket.clientSocket, &ErrorBuffer, CHUNK_SIZE) != SUCCES)
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
	ResultMessage message;
	// now transfer the message from receiver to sender
	// now transfer the message from receiver to sender
	if (Receive(receiverSocket.clientSocket, (char*)&message, sizeof(ResultMessage)) == SUCCES)
	{
		if (Send(senderSocket.clientSocket, (const char*)&message, sizeof(ResultMessage)) != SUCCES)
		{
			// handle error
			printf("error sending data to sender\n");
		}
	}
	return message;
}

void printUsage()
{
	printf("Usage: channel.exe <sender_port> <receiver_port> <bit_error_probability> <random_seed>");
}


int main(int argc, char** argv)
{
	Port senderPort;
	Port receiverPort;
	double bit_error_p;
	unsigned int random_seed, real_seed;
	int num = power(2, 16);

	if (argc != 5)
	{
		printUsage();
		return -1;
	}

	senderPort = atoi(argv[1]);
	receiverPort = atoi(argv[2]);
	bit_error_p = atof(argv[3]);
	random_seed = atoi(argv[4]);

	real_seed = random_seed / num;
	

	if (receiverPort == senderPort)
	{
		printUsage();
		return -1;
	}

	if (channelMain(senderPort, receiverPort, bit_error_p, real_seed) != 0)
	{
		printf("error while running channel");
		return -1;
	}

	return 0;
}
