#ifndef __SM3_NUM_H__
#define __SM3_NUM_H__

#include "miracl.h"

#define unit2c(u,c) (*(c  ) = (unsigned char)(((u)>>24)&0xff), \
					 *(c+1) = (unsigned char)(((u)>>16)&0xff), \
					 *(c+2) = (unsigned char)(((u)>>8 )&0xff), \
					 *(c+3) = (unsigned char)(((u)	  )&0xff))

#define T1 0x79CC4519
#define T2 0x7A879D8A
#define p0(x) (x^shift_to_left(x,9)^shift_to_left(x,17))
#define p1(x) (x^shift_to_left(x,15)^shift_to_left(x,23))
#define shift_to_left(x,n) ((((unsigned int) x) << n) | (((unsigned int) x) >> (32 - n)))//Ñ­»·×óÒÆ

void sm3_number(big, unsigned char*);

#endif 