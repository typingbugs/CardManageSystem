#include "VCD.h"

#define POLYNOMIAL		0x8408		// x^16 + x^12 + x^5 + 1
#define PRESET_VALUE	0xFFFF
#define CHECK_VALUE		0xF0B8
#define CALC_CRC		1
#define CHECK_CRC		0

void CVCD::CRC16( int buflen, uchar_t buf[], uchar_t *pucCRCL, uchar_t *pucCRCH )
{
	unsigned int current_crc_value;
	unsigned char *array_of_databytes;
	int number_of_databytes;
	int i, j;

	number_of_databytes = buflen;
	array_of_databytes = buf;
	current_crc_value = PRESET_VALUE;
	for (i = 0; i < number_of_databytes; i++)
	{
		current_crc_value = current_crc_value ^ ((unsigned int)array_of_databytes[i]);
		for (j = 0; j < 8; j++)
		{
			if (current_crc_value & 0x0001)
			{
				current_crc_value = (current_crc_value >> 1) ^ POLYNOMIAL;
			}
			else
			{
				current_crc_value = (current_crc_value >> 1);
			}
		}
	}
	current_crc_value = ~current_crc_value;
	*pucCRCL = current_crc_value & 0xFF;
	*pucCRCH = (current_crc_value >> 8) & 0xFF;
}

void CVCD::ISO15693_getCRC16( int buflen, uchar_t buf[], uchar_t *pucCRCL, uchar_t *pucCRCH )
{
	CRC16( buflen, buf, pucCRCL, pucCRCH );
}

bool CVCD::ISO15693_checkCRC16( int buflen, uchar_t buf[] )
{
	uchar_t ucCRCL, ucCRCH;
	CRC16( buflen-2, buf, &ucCRCL, &ucCRCH );
	if( ucCRCL == buf[buflen-2] && ucCRCH == buf[buflen-1] )
	{
		return true;
	}
	else
	{
		return false;
	}
}
