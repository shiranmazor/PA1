#ifndef _SHARED_H_
#define _SHARED_H_
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdlib.h>

#define TRUE	1
#define FALSE	0
#define Ip IN_ADDR
typedef unsigned short Port;
typedef int bool;
typedef unsigned char bit;
typedef unsigned char byte;
typedef unsigned long long ulong;
#define CHUNK_SIZE 1 //reading and sending 1 byte every time

typedef enum { SUCCES, FAILED, NOT_CONNECTED } Result;
typedef struct ResultMessage
{
	int received;
	int written;
	Result crc16;
	Result crc32;
	Result checksum;

} ResultMessage;



//shared help functions:
int getBytesNum(int bitsNum);
//return 2 bytes in hexa
unsigned long calcCrc16(byte* chunkBuffer);
//return 4 bytes in hexa
unsigned long calcCRC32(byte* chunkBuffer);
//return 2 bytes in hexa
unsigned long calcChecksum(byte* chunkBuffer);

#endif

