#include "Channel.h"

static SOCKET *senderSocket;
static SOCKET *receiverSocket;
static int bitsFlipped = 0;
static double pError;

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
