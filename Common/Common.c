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
	short int sum = 0;
	bool end = FALSE;
	if (bytesNum % 2 != 0){
		chunkBuffer[(bytesNum / 2) + 1] = 0;
		bytesNum++;
		end = TRUE;
	}
	
	for (i = 0; i < bytesNum / 2; i++)
		sum += chunkBuffer[i];
	
	if (end) return ~sum;
	return sum;
}

void initBuff(DWordBuffer buff)
{
	buff.idx = 0;
	buff.size = 0;
}

void reOrderBuff(DWordBuffer buff)
{
	int i = 0;
	byte temp;
	while (buff.idx != 0)
	{
		temp = buff.buff[i];
		buff.buff[i] = buff.buff[buff.idx];
		buff.buff[buff.idx] = temp;
		buff.idx = (buff.idx + 1) % 8;
		i++;
	}
}

int pushToBuff(DWordBuffer buff, byte* source, byte* res, int length)
{
	int i = 0, j = 0;
	byte res[CHUNK_SIZE];
	if (buff.size < 8)
	{
		for (i = 0; i < length && buff.size < 8; i++)
		{
			buff.buff[buff.idx] = source[i];
			buff.idx++;
			buff.size++;
		}
	}
	if (buff.size == 8 && i < length)
	{
		buff.idx %= 8;
		for (; i < length && buff.idx < 8; i++)
		{
			res[j] = buff.buff[buff.idx];
			buff.buff[buff.idx] = source[i];
			buff.idx = (buff.idx + 1) % 8;
			j++;
		}
		return j;
	}
	else return 0;
}