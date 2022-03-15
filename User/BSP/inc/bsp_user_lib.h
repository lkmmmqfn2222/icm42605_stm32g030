/*
*********************************************************************************************************
*
*	模块名称 : 字符串操作\数值转换
*	文件名称 : bsp_user_lib.h
*	版    本 : V1.2
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef __BSP_USER_LIB_H__
#define __BSP_USER_LIB_H__

#include <stdint.h>

#define MODBUS_CRC16_ENABLE   //使用Modbus的Crc16

int str_len(char *_str);
void str_cpy(char *_tar, char *_src);
int str_cmp(char * s1, char * s2);
void mem_set(char *_tar, char _data, int _len);

void int_to_str(int _iNumber, char *_pBuf, unsigned char _len);
int str_to_int(char *_pStr);

uint16_t BEBufToUint16(uint8_t *_pBuf);
uint16_t LEBufToUint16(uint8_t *_pBuf);

uint32_t BEBufToUint32(uint8_t *_pBuf);
uint32_t LEBufToUint32(uint8_t *_pBuf);

int32_t  CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);

char BcdToChar(uint8_t _bcd);
void HexToAscll(uint8_t * _pHex, char *_pAscii, uint16_t _BinBytes);
uint32_t AsciiToUint32(char *pAscii);

#if defined(MODBUS_CRC16_ENABLE)
uint16_t CRC16_Modbus(uint8_t *_pBuf, uint16_t _usLen) ;
#endif

#endif

/*****************************  (END OF FILE) *********************************/
