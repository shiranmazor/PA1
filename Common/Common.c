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


unsigned int calcChecksum(byte* chunkBuffer, int bytesNum)
{
	int i;
	unsigned int sum = 0;

	if (bytesNum % 2 != 0){
		chunkBuffer[bytesNum] = 0;
		bytesNum++;
	}
	
	for (i = 0; i < bytesNum / 2; i++)
		sum += (chunkBuffer[i * 2] << 8) + chunkBuffer[i * 2 + 1];
	
	return sum;
}

unsigned short int closeCheckSum(unsigned int sum)
{
	// Fold 32-bit sum to 16 bits
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return(~sum);
}

void initBuff(DWordBuffer* buff){
	buff->idx = 0;
	buff->size = 0;
}

void reOrderBuff(DWordBuffer* buff)
{
	int i = 0;
	byte temp;
	while (buff->idx != 0)
	{
		temp = buff->buff[i];
		buff->buff[i] = buff->buff[buff->idx];
		buff->buff[buff->idx] = temp;
		buff->idx = (buff->idx + 1) % 8;
		i++;
	}
}

int pushToBuff(DWordBuffer* buff, byte* inBuff, byte* outBuff, int length)
{
	int inBuffIdx = 0, outBuffIdx = 0;
	if (buff->size < 8)
	{
		for (inBuffIdx = 0; inBuffIdx < length && buff->size < 8; inBuffIdx++)
		{
			buff->buff[buff->idx] = inBuff[inBuffIdx];
			buff->idx++;
			buff->size++;
		}
	}
	if (buff->size == 8 && inBuffIdx < length)
	{
		buff->idx = buff->idx % 8;
		for (; inBuffIdx < length; inBuffIdx++)
		{
			outBuff[outBuffIdx] = buff->buff[buff->idx];
			buff->buff[buff->idx] = inBuff[inBuffIdx];
			buff->idx = (buff->idx + 1) % 8;
			outBuffIdx++;
		}
		return outBuffIdx;
	}
	else return 0;
}