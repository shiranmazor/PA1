#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include "..\Common\Sockets.h"
#include "..\Common\Common.h"


void printUsage();
int receiverMain(Ip listenIp, Port listenPort, char* filename);
bool sendResultsMessage();
void printResult();
/*
receiving buffer in chunks from channel and calculating acomulative crc and checksum
saving all data to disk
compare actual codes to receive code
prapare the result message and send it back to channel
*/
bool HandleData();

#endif
