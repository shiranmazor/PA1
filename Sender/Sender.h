#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "..\Shared\Sockets.h"


void printUsage();
int senderMain(Ip remoteIp, Port remotePort, const char* filename);
bool getSummaryMessage(SummaryMessage* summary);
void printSummary(const SummaryMessage* summary);
int calcCrc16(const char* filename);
int calcCRC32(const char* filename);
int calcChecksum(const char* filename);



