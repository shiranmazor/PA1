#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "..\Shared\Sockets.h"


void printUsage();
int receiver_main(listenIp, listenPort, filename);
bool sendSummaryMessage(SOCKET socket);
void printResult(const SummaryMessage* summary);
int HandleData(void* pArgs);
bool checkCrc16();
bool checkCrc32();
bool checkChecksum();
