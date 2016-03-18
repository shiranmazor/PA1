#ifndef _COMMON_H_
#define _COMMON_H_
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#define TRUE	1
#define FALSE	0
#define Ip IN_ADDR
#define CRC16POLY 0x8005
#define CRC32POLY 0x04C11DB7

typedef unsigned short Port;
typedef int bool;
typedef unsigned char bit;
typedef unsigned char byte;
typedef unsigned long long ulong;
#define CHUNK_SIZE 2 //reading and sending 2 bytes every time

typedef enum { SUCCES, FAILED, NOT_CONNECTED } Result;
typedef struct ResultMessage
{
	int received;
	int written;
	Result crc16;
	Result crc32;
	Result checksum;

} ResultMessage;

typedef struct DWordBuffer
{
	byte buff[8];
	short int idx;
	short int size;
} DWordBuffer;


//shared help functions:
int getBytesNum(int bitsNum);
unsigned int calcCRC(byte* chunkBuffer, unsigned int remainder, unsigned int polynom);
short int calcChecksum(short int* chunkBuffer, int bytesNum);
void initBuff(DWordBuffer buff);
void reOrderBuff(DWordBuffer buff);
int pushToBuff(DWordBuffer buff, byte* source, byte* res, int length);

#endif

