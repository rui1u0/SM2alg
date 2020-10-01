#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "miracl.h"
#include "sm3.h"

//填充函数，返回值为分组数
int fill(char *pliant, unsigned int *m)
{
	int i, j, k;
	
	long int len;
	char temp[20];
	len = 4 * strlen(pliant);  //消息bit长度，一个16进制4位

	i = 0;
	do
	{
		k = 512 * i + 448 - len - 8;
		i++;
	} while (k < 0);

	strcat(pliant, "80");
	for (j = 0; j < (k / 8); j++)   strcat(pliant, "00");  //补0
	sprintf(temp, "%016x", len);   //把格式化的数据len写入temp字符串缓冲区。
	temp[16] = '\0';
	strcat(pliant, temp);

	i = 0;
//	printf("填充后的消息\n");

	while (*(pliant + i * 8) != '\0') {
		for (j = 0; j < 8; j++) {
			sscanf(pliant + i * 8, "%08x", m + i);
//			printf("%08x ", m[i++]);
			i++;
		}
//		puts("");
	}
	return i;
}

//将填充后的消息按512bit分组
void m_to_B(unsigned int B[3][16], unsigned int *m, int n) {  //按512比特分组，
	int i, j, k = 0;
	for (i = 0; i < n; i++)
		for (j = 0; j < 16; j++)
			B[i][j] = m[k++];   //m中以word为单位存储
}
//消息扩展
void extend(unsigned int B[3][16], unsigned int W[3][68], unsigned int W1[3][64], int n) {
	int i, j, k;
	for (i = 0; i < n; i++)     //将B(i)划分为W0~W15
		for (j = 0; j < 16; j++)
			W[i][j] = B[i][j];
	for (i = 0; i < n; i++)
		for (j = 16; j < 68; j++)//W16...W67
			W[i][j] = p1(W[i][j - 16] ^ W[i][j - 9] ^ shift_to_left(W[i][j - 3], 15)) ^ shift_to_left(W[i][j - 13], 7) ^ W[i][j - 6];
	for (i = 0; i < n; i++)
		for (j = 0; j < 64; j++)//W0'W1'...W63'
			W1[i][j] = W[i][j] ^ W[i][j + 4];
	for (i = 0; i < n; i++) {
		k = 0;
//		printf("\n扩展后的消息\nW0W1...W67", i + 1);
		for (j = 0; j < 68; j++) {
//			if (k % 8 == 0) puts("");
//			printf("%08x ", W[i][k]);
			k++;
		}

//		printf("\nW0'W1'...W63'");

		k = 0;
		for (j = 0; j < 64; j++) {
//			if (k % 8 == 0) puts("");
//			printf("%08x ", W1[i][k]);
			k++;
		}
	}
}
//压缩函数
void CF(unsigned int V[8], unsigned int W[68], unsigned int W1[64]) {
	unsigned int A, B, C, D, E, F, G, H;
	unsigned int SS1, SS2, TT1, TT2;
	int j = 0;

	A = V[0];
	B = V[1];
	C = V[2];
	D = V[3];
	E = V[4];
	F = V[5];
	G = V[6];
	H = V[7];

	for (j = 0; j < 64; j++) {
		SS1 = shift_to_left((shift_to_left(A, 12) + E + shift_to_left((j > 15 ? T2 : T1), j)), 7);
		SS2 = SS1 ^ shift_to_left(A, 12);
		TT1 = FFj(A, B, C, j) + D + SS2 + W1[j];
		TT2 = GGj(E, F, G, j) + H + SS1 + W[j];
		D = C;
		C = shift_to_left(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = shift_to_left(F, 19);
		F = E;
		E = p0(TT2);

		/*
		printf("%02d %08x ", j, A);
		printf("%08x ", B);
		printf("%08x ", C);
		printf("%08x ", D);
		printf("%08x ", E);
		printf("%08x ", F);
		printf("%08x ", G);
		printf("%08x \n", H);
		*/
	}
	V[0] = A ^ V[0];
	V[1] = B ^ V[1];
	V[2] = C ^ V[2];
	V[3] = D ^ V[3];
	V[4] = E ^ V[4];
	V[5] = F ^ V[5];
	V[6] = G ^ V[6];
	V[7] = H ^ V[7];
}

//布尔函数
unsigned int FFj(unsigned int x, unsigned int y, unsigned int z, int j) {
	if (j > 15)
		return ((x&y) | (x&z) | (y&z));
	else
		return (x^y^z);
}

unsigned int GGj(unsigned int x, unsigned int y, unsigned int z, int j) {
	if (j > 15)
		return ((x&y) | ((~x)&z));
	else
		return (x^y^z);
}

void sm3_number(big bufn, unsigned char *result) {
	big m1;
	int groupnum, j;
	char m[2048];
	char path[1024];
//	FILE *fp;
	unsigned int m_group[4096]; //32位，字
	unsigned int B[3][16], W[3][68], W1[3][64], V[8];
	miracl *mip = mirsys(5000, 16);
	mip->IOBASE = 16;
	m1 = mirvar(0);

	//	scanf("%1024s", path);
	//	fp = fopen(path, "r");
	//	cinnum(m1, fp);  
	//从文件以大数形式读出赋给m1
	copy(bufn, m1);
	printf("\n");
	printf("需要算哈希值的大数:\n");
	cotnum(m1, stdout);
	cotstr(m1, m);
	//将大数转化成字符串
	//	fclose(fp);

	V[0] = 0x7380166f;   //初始值IV
	V[1] = 0x4914b2b9;
	V[2] = 0x172442d7;
	V[3] = 0xda8a0600;
	V[4] = 0xa96f30bc;
	V[5] = 0x163138aa;
	V[6] = 0xe38dee4d;
	V[7] = 0xb0fb0e4e;
	groupnum = fill(m, m_group) * 32 / 512;  //填充并计算按512bit分组组数
	m_to_B(B, m_group, groupnum);    //填充后消息按512bit分组（以word为单位）
	extend(B, W, W1, groupnum);  //扩展
//	printf("\n迭代压缩中间值\n");
	for (j = 0; j < groupnum; j++)   //迭代
		CF(V, &W[j][0], &W1[j][0]);

	printf("\n");
	printf("杂凑值\n");
	for (j = 0; j < 8; j++)
		printf("%08x ", V[j]);
	puts("");

	
	unsigned char *p;
	p = result;
	for (j = 0; j < 8; j++)
	{
		unit2c(V[j], p + j * 4);
		
	}
}