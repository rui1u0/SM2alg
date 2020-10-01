#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "miracl.h"
#include "mysm2.h"
#include "sm3.h"

struct FPECC Ecc256 = {
"FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF",
"FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC",
"28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93",
"FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123",
"32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7",
"BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0",
};

int kdf(unsigned char *buf, int klen, unsigned char *cipher)
{
	unsigned int ct = 0x00000001;
	unsigned char digest[32];//最后的哈希
	unsigned char *p;
	int i, m, n;
	big bufn;
	bufn = mirvar(0);

	m = klen / 256;//取商
	n = klen % 256;//取余
	p = cipher;

	for (i = 0; i < m; i++)
	{
		unit2c(ct, buf + 64);//把无符号整型数转成字符串
		bytes_to_big(68, buf, bufn);
		sm3_number(bufn, p);
		p += 32;
		ct++;
	}

	if (n != 0)
	{
		unit2c(ct, buf + 64);
		bytes_to_big(68, buf, bufn);
		sm3_number(bufn, digest);
	}

	memcpy(p, digest, n);

	for (i = 0; i < klen / 8; i++)
	{
		if (cipher[i] != 0)
			break;
	}

	if (i < klen / 8)
		return 1;
	else
		return 0;
}

void sm2_keygen()//密钥对生成器
{
	big x2, y2, db;
	struct FPECC *cfig = &Ecc256;
	epoint *g, *pb;
	struct FPECCNUM *knum;
	miracl *mip = mirsys(5000, 16);
	mip->IOBASE = 16;
	knum = (struct FPECCNUM*)malloc(sizeof(*knum));
	knum->a = mirvar(0);
	knum->b = mirvar(0);
	knum->n = mirvar(0);
	knum->p = mirvar(0);
	knum->x = mirvar(0);
	knum->y = mirvar(0);
	x2 = mirvar(0);
	y2 = mirvar(0);
	db = mirvar(0);

	cinstr(knum->a, cfig->a);
	cinstr(knum->b, cfig->b);
	cinstr(knum->n, cfig->n);
	cinstr(knum->p, cfig->p);
	cinstr(knum->x, cfig->x);
	cinstr(knum->y, cfig->y);

	ecurve_init(knum->a, knum->b, knum->p, MR_PROJECTIVE);
	g = epoint_init();
	pb = epoint_init();
	epoint_set(knum->x, knum->y, 0, g);

	irand(time(NULL) + SEED_CONST);
	bigrand(knum->n, db);
	ecurve_mult(db, g, pb);
	epoint_get(pb, x2, y2);

	FILE *fp;
	fp = fopen("PB.txt", "w");
	cotnum(x2, fp);
	cotnum(y2, fp);
	fclose(fp);
	fp = fopen("DB.txt", "w");
	cotnum(db, fp);
	fclose(fp);
}

int sm2_encrypt(unsigned char *ciphertext)
{
	int i;
	miracl *mip = mirsys(5000, 16);
	mip->IOBASE = 16;
	struct FPECC *cfig = &Ecc256;
	struct FPECCNUM *eccnum;
	big x1, y1, h, pr, tmpa, x2, y2, z, temp;
	eccnum = (struct FPECCNUM*)malloc(sizeof(struct FPECCNUM));
	eccnum->a = mirvar(0);
	eccnum->b = mirvar(0);
	eccnum->n = mirvar(0);
	eccnum->p = mirvar(0);
	eccnum->x = mirvar(0);
	eccnum->y = mirvar(0);
	x1 = mirvar(0);
	y1 = mirvar(0);
	h = mirvar(0);
	pr = mirvar(0);
	tmpa = mirvar(2);
	x2 = mirvar(0);
	y2 = mirvar(0);
	z = mirvar(0);
	temp = mirvar(0);


	cinstr(eccnum->a, cfig->a);
	cinstr(eccnum->b, cfig->b);
	cinstr(eccnum->n, cfig->n);
	cinstr(eccnum->p, cfig->p);
	cinstr(eccnum->x, cfig->x);
	cinstr(eccnum->y, cfig->y);
	printf("p:");
	cotnum(eccnum->p, stdout);
	printf("a:");
	cotnum(eccnum->a, stdout);
	printf("b:");
	cotnum(eccnum->b, stdout);
	printf("n:");
	cotnum(eccnum->n, stdout);
	printf("x:");
	cotnum(eccnum->x, stdout);
	printf("y:");
	cotnum(eccnum->y, stdout);

	big k;
	k = mirvar(0);
	irand(time(NULL) + SEED_CONST);

A1_sm2:
	do
	{
		bigrand(eccnum->n, k);
	} while (k->len == 0);
	printf("\nk:");
	cotnum(k, stdout);

	ecurve_init(eccnum->a, eccnum->b, eccnum->p, MR_PROJECTIVE);//初始化椭圆曲线
	epoint *g;
	g = epoint_init();
	epoint_set(eccnum->x, eccnum->y, 0, g);//设置g点
	ecurve_mult(k, g, g);
	epoint_get(g, x1, y1);

	big_to_bytes(32, x1, (char *)ciphertext, TRUE);
	big_to_bytes(32, y1, (char *)ciphertext + 32, TRUE);
	printf("C1:");
	for (i = 0; i < 64; i++)
		printf("%02X", ciphertext[i]);
	printf("\n");

	FILE *fp;
	fp = fopen("PB.txt", "r");
	cinnum(x2, fp);
	cinnum(y2, fp);
	fclose(fp);

	copy(eccnum->p, pr);//计算h
	nroot(pr, 2, pr);
	multiply(pr, tmpa, pr);
	incr(pr, 1, pr);
	add(eccnum->p, pr, pr);
	divide(pr, eccnum->n, h);
	printf("h:");
	cotnum(h, stdout);
	printf("pr:");
	cotnum(pr, stdout);

	epoint *pb;
	pb = epoint_init();
	epoint_set(x2, y2, 0, pb);

	if (point_at_infinity(pb))
	{
		mirexit();
		printf("Error: 6.1A3\n");
		return 0;
	}

	ecurve_mult(k, pb, pb);
	epoint_get(pb, x2, y2);
	printf("\nx2:");
	cotnum(x2, stdout);
	printf("y2:");
	cotnum(y2, stdout);

	printf("\nMessage file name:");
	char path[1024] = { 0 };
	scanf("%1024s", path);
	printf("\n");

	unsigned char message[BUFFSIZE] = { 0 };
	if ((fp = fopen(path, "rb")) == NULL)
	{
		printf("open file error!\n");
		return 0;
	}

	while (fread(message, 1, BUFFSIZE, fp));
	fclose(fp);
	int mslen, klen;
	mslen = strlen(message);

	printf("mslen:%d\nmessage:", mslen);
	for (i = 0; i < mslen; i++)
		printf("%02X", message[i]);
	printf("\n");
	klen = 8 * mslen;
	unsigned char zl[32], zr[32], buf[70], cipher[BUFFSIZE];
	unsigned char *tmp;
	tmp = malloc(64 + mslen);
	big_to_bytes(32, x2, (char *)zl, TRUE);
	big_to_bytes(32, y2, (char*)zr, TRUE);
	memcpy(buf, zl, 32);
	memcpy(buf + 32, zr, 32);
	printf("Z:\n");
	for (i = 0; i < 64; i++)
		printf("%02X", buf[i]);
	printf("\n");
	bytes_to_big(64, buf, z);
	cotnum(z, stdout);

	if (kdf(buf, klen, cipher) == 0)
		goto A1_sm2;

	for (i = 0; i < mslen; i++)
		cipher[i] ^= message[i];

	memcpy(tmp, zl, 32);
	memcpy(tmp + 32, message, mslen);
	memcpy(tmp + 32 + mslen, zr, 32);
	bytes_to_big(64 + mslen, tmp, temp);

	printf("\nx2||M||y2\n");
	cotnum(temp, stdout);

	unsigned char C3[32];
	sm3_number(temp, C3);

	memcpy(ciphertext + 64, cipher, mslen);
	memcpy(ciphertext + 64 + mslen, C3, 32);
	//64+mslen+32

	int clen;
	clen = 64 + mslen + 32;

	printf("Cipher:\n");
	for (i = 0; i < clen; i++)
		printf("%02X", ciphertext[i]);

	printf("\n");
	fp = fopen("c.txt", "w");
	bytes_to_big(clen, ciphertext, temp);
	cotnum(temp, fp);
	fclose(fp);
	
	return mslen;
}

void sm2_decrypt(int klen, unsigned char *ciphertext)
{
	miracl *mip = mirsys(5000, 16);
	mip->IOBASE = 16;

	big x2, y2, db;
	int i;
	struct FPECC *cfig = &Ecc256;
	epoint *g;
	struct FPECCNUM *knum;
	
	knum = (struct FPECCNUM*)malloc(sizeof(*knum));
	knum->a = mirvar(0);
	knum->b = mirvar(0);
	knum->n = mirvar(0);
	knum->p = mirvar(0);
	knum->x = mirvar(0);
	knum->y = mirvar(0);
	x2 = mirvar(0);
	y2 = mirvar(0);
	db = mirvar(0);

	FILE *fp;
	fp = fopen("DB.txt", "r");
	cinnum(db, fp);
	fclose(fp);

	cinstr(knum->a, cfig->a);
	cinstr(knum->b, cfig->b);
	cinstr(knum->n, cfig->n);
	cinstr(knum->p, cfig->p);
	cinstr(knum->x, cfig->x);
	cinstr(knum->y, cfig->y);

	ecurve_init(knum->a, knum->b, knum->p, MR_PROJECTIVE);
	g = epoint_init();

	unsigned char C1[64], buf[70];
	unsigned char C2[BUFFSIZE], message[BUFFSIZE];
	unsigned char C3[32], u[32];
	unsigned char *tmp;
	tmp = malloc(klen + 64);

	printf("Cipher:\n");
	for (i = 0; i < klen + 96; i++)
	{
		printf("%02X", ciphertext[i]);
	}
	printf("\n");

	memcpy(C1, ciphertext, 64);
	memcpy(C2, ciphertext + 64, klen);
	memcpy(C3, ciphertext + 64 + klen, 32);

	bytes_to_big(32, C1, x2);
	bytes_to_big(32, C1 + 32, y2);

	if (!epoint_set(x2, y2, 0, g))
	{
		printf("7.1 B1 Error!\n");
		mirexit();
		return NULL;
	}

	if (point_at_infinity(g))
	{
		printf("7.1 B2 Error!\n");
		mirexit();
		return NULL;
	}

	ecurve_mult(db, g, g);
	epoint_get(g, x2, y2);

	printf("\n");
	printf("x2:");
	cotnum(x2, stdout);
	printf("y2:");
	cotnum(y2, stdout);

	big_to_bytes(32, x2, buf, TRUE);
	big_to_bytes(32, y2, buf + 32, TRUE);

	if (kdf(buf, klen * 8, message) == 0)
	{
		printf("7.1 B4 Error!\n");
		mirexit();
		return NULL;
	}

	for (i = 0; i < klen; i++)
		message[i] ^= C2[i];

	memcpy(tmp, buf, 32);
	memcpy(tmp + 32, message, klen);
	memcpy(tmp + 32 + klen, buf + 32, 32);

	bytes_to_big(klen + 64, tmp, x2);

	printf("\nx2||M'||y2\n");
	cotnum(x2, stdout);

	sm3_number(x2, u);
	
	if (memcmp(u, C3, 32) != 0)
	{
		printf("7.1 B6 Error!\n");
		mirexit();
		return NULL;
	}
	
	printf("\nmessage:\n");
	for (i = 0; i < klen; i++)
		printf("%c", message[i]);
	printf("\n");

	return NULL;
}