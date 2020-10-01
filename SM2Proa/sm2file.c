#include <stdio.h>
#include "mysm2.h"
#include "miracl.h"

int main()
{
	int klen, clen, i;
	unsigned char ciphertext[BUFFSIZE];

	klen = sm2_encrypt(ciphertext);//调用加密函数，返回明文长度
	if (klen == 0)
	{
		printf("7.1 klen error!");
		return 0;
	}
	clen = klen + 64 + 32;//密文长度
	printf("Cipher:\n");
	for (i = 0; i < clen; i++)
	{
		printf("%02X", ciphertext[i]);
	}
	printf("\n");

	sm2_decrypt(klen, ciphertext);//调用解密函数

	return 0;
}