#ifndef _SHARED_H_
#define _SHARED_H_

#define TRUE	1
#define FALSE	0
#define Ip IN_ADDR
typedef unsigned short Port;
typedef int bool;
typedef unsigned char bit;
typedef unsigned char byte;
typedef unsigned long long ulong;
#define CHUNK_SIZE 1024 //reading and sending 1K every time

typedef struct ResultMessage
{
	int received;
	Result crc16;
	Result crc32;
	Result checksum;
} ResultMessage;

//shared help functions:
int getBytesNum(int bitsNum);





#endif