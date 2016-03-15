#include "Channel.h"

void printUsage()
{
	printf("Usage: channel.exe <sender_port> <receiver_port> <bit_error_probability> <random_seed>");
}


int main(int argc, char** argv)
{
	Port senderPort;
	Port receiverPort;
	double bit_error_p;
	unsigned int random_seed;

	if (argc != 5)
	{
		printUsage();
		return -1;
	}

	senderPort = atoi(argv[1]);
	receiverPort = atoi(argv[2]);
	bit_error_p = atof(argv[3]);
	random_seed = atoi(argv[4]);
	

	if (receiverPort == senderPort)
	{
		printUsage();
		return -1;
	}

	if (startChannel(senderPort, receiverPort, bit_error_p, random_seed) != 0)
	{
		printf("error while running channel");
		return -1;
	}

	return 0;
}
