#ifndef __MYSM2_HEADER__
#define __MYSM2_HEADER__

#include <stdio.h>
#include "miracl.h"
#define SEED_CONST 0xFF6666FF
#define BUFFSIZE 2048

#define unit2c(u,c) (*(c  ) = (((u)>>24)&0xff), \
					 *(c+1) = (((u)>>16)&0xff), \
					 *(c+2) = (((u)>>8 )&0xff), \
					 *(c+3) = (((u)	   )&0xff))

struct FPECC
{
	char *p;
	char *a;
	char *b;
	char *n;
	char *x;
	char *y;
};

struct FPECCNUM
{
	big a, b, p, n, x, y;
};

int kdf(unsigned char *buf, int klen, unsigned char *cipher);
void sm2_keygen();
int sm2_encrypt(unsigned char *cp);
void sm2_decrypt(int klen, unsigned char *ciphertext);

#endif // !__MYSM2_HEADER__
