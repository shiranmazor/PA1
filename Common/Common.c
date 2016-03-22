#include "Common.h"


unsigned int calcCRC(byte* chunkBuffer, unsigned int crc, int bytesNum, int size)
{
	int i, j;
	unsigned int poly = size == 16 ? CRC16POLY : CRC32POLY;
	bool p = (crc == 38802);
	if (p) printf("claccrc got: 0x%.2x 0x%.2x, num: %d\n\n", chunkBuffer[0], chunkBuffer[1], bytesNum);
	for (i = 0; i < bytesNum; i++)
	{
		crc ^= chunkBuffer[i] << (size - 8);
		if (p) printf("out crc %hu %.2x\n", crc, crc);
		for (j = 0; j < 8; j++)
		{
			if (p) printf("crc %hu %.2x j: %d\n", crc, crc, j);
			if (crc & (1 << (size - 1))) crc = (crc << 1) ^ poly;
			else crc <<= 1;
			if (p) printf("crc %hu %.2x j: %d\n", crc, crc, j);
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
	byte temp[8];
	int i;
	for (i = 0; i < 8; i++){
		temp[i] = buff->buff[buff->idx];
		buff->idx = (buff->idx + 1) % 8;
	}
	for (i = 0; i < 8; i++) buff->buff[i] = temp[i];
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