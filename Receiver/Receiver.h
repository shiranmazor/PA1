#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "..\Shared\Sockets.h"


void printUsage();
int receiver_main(listenIp, listenPort, filename);
bool sendSummaryMessage(SOCKET socket);
void printResult(const ResultMessage* Res);
int HandleData(void* pArgs, const ResultMessage* Res);

