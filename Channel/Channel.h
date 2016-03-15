#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "..\Shared\Sockets.h"
#include <math.h>


int channelMain(Port senderListenPort, Port receiverPort, double bit_error_p, unsigned int randSeed);
int handleSender(void* pArgs);
int handleReceiver(void* pArgs);