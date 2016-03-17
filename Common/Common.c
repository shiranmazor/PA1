#include "Common.h"

unsigned int calcCRC(byte* chunkBuffer, unsigned int crc, unsigned int polynom)
{
	int i, j;

	for (i = 0; i < CHUNK_SIZE; i++)
	{
		crc ^= chunkBuffer[i];
		for (j = 0; j < 8; j++)
		{
			if (crc & 1) crc ^= polynom;
			crc >>= 1;
		}
	}
	return crc;
}


short int calcChecksum(short int* chunkBuffer, int bytesNum)
{
	int i;
	short int sum;
	if (bytesNum % 2 != 0) chunkBuffer[(bytesNum / 2) + 1] = 0;
	
	for (i = 0; i < bytesNum / 2; i++)
		sum += chunkBuffer[i];
	
	if (bytesNum % 2 != 0) return ~sum;
	return sum;
}
