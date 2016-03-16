#ifndef _SENDER_H_
#define _SENDER_H_
#include "..\Shared\Sockets.h"


void printUsage();
int senderMain(Ip remoteIp, Port remotePort, const char* filename);
bool getResultMessage(ResultMessage* msg);
void printResults(const ResultMessage* msg);

bool sendingFileData();



#endif
