#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include "..\Shared\Sockets.h"


void printUsage();
int receiverMain(listenIp, listenPort, filename);
bool sendResultsMessage();
void printResult();
/*
receiving buffer in chunks from channel and calculating acomulative crc and checksum
saving all data to disk
compare actual codes to receive code
prapare the result message and send it back to channel
*/
bool HandleData(FILE* fileHandle);

#endif
