#include <stdio.h>
#include "mysm2.h"
#include "miracl.h"

int main()
{
	int klen, clen, i;
	unsigned char ciphertext[BUFFSIZE];

	klen = sm2_encrypt(ciphertext);//���ü��ܺ������������ĳ���
	if (klen == 0)
	{
		printf("7.1 klen error!");
		return 0;
	}
	clen = klen + 64 + 32;//���ĳ���
	printf("Cipher:\n");
	for (i = 0; i < clen; i++)
	{
		printf("%02X", ciphertext[i]);
	}
	printf("\n");

	sm2_decrypt(klen, ciphertext);//���ý��ܺ���

	return 0;
}