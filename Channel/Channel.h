#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "..\Shared\Sockets.h"
#include <math.h>


int channelMain(Port senderListenPort, Port receiverPort, double bit_error_p, unsigned int randSeed);
int handleSenderFile();
ResultMessage handleReceiverMessage();
byte flipBits(byte msg);
bool shouldFlip();
void printOutputMessage(IN_ADDR  senderAddress, IN_ADDR  receiverAddress, int byteReceived, int byteFlipped);