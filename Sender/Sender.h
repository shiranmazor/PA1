#ifndef _SENDER_H_
#define _SENDER_H_

#include "..\Common\Sockets.h"
#include "..\Common\Common.h"

void printUsage();

/*
conect to channel socket
read data from file
get result message from channel and print it out
*/
int senderMain(Ip remoteIp, Port remotePort, const char* filename);
/*
receive ResultMessage struct from channel
*/
bool getResultMessage(ResultMessage* msg);
/*
print results in the example format
*/
void printResults(const ResultMessage* msg);
/*
send buffer in chunks to channel and calculating acomulative crc and checksum
at the and send the last 8 bytes as the Crc,checksum output to the channel
*/
bool sendingFileData();



#endif
