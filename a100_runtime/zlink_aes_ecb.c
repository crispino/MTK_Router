/*
 * AES128-ECB-PKCS5Padding 
 * 2017-10-24 liyue3@zowee.com.cn
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "zlink_aes_ecb.h"

void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen)  
{  
    short i;  
    unsigned char highByte, lowByte;  
  
    for (i = 0; i < sourceLen; i++)  
    {  
        highByte = source[i] >> 4;  
        lowByte = source[i] & 0x0f ;  
  
        highByte += 0x30;  
  
        if (highByte > 0x39)  
                dest[i * 2] = highByte + 0x07;  
        else  
                dest[i * 2] = highByte;  
  
        lowByte += 0x30;  
        if (lowByte > 0x39)  
            dest[i * 2 + 1] = lowByte + 0x07;  
        else  
            dest[i * 2 + 1] = lowByte;  
    }  
    return ;  
}

void HexStrToByte(const char* source, unsigned char* dest, int sourceLen)  
{  
    short i;  
    unsigned char highByte, lowByte;  
      
    for (i = 0; i < sourceLen; i += 2)  
    {  
        highByte = toupper(source[i]);  
        lowByte  = toupper(source[i + 1]);  
  
        if (highByte > 0x39)  
            highByte -= 0x37;  
        else  
            highByte -= 0x30;  
  
        if (lowByte > 0x39)  
            lowByte -= 0x37;  
        else  
            lowByte -= 0x30;  
  
        dest[i / 2] = (highByte << 4) | lowByte;  
    }  
    return ;  
}

static void pkcs5_padding(unsigned char *output, int output_len, int data_len )
{
	size_t padding_len = output_len - data_len;
	unsigned char i;

	for( i = 0; i < padding_len; i++ )
		output[data_len + i] = (unsigned char)padding_len;
	output[data_len + i] = (unsigned char)'\0';
}

/*
 * 根据项目要求，加密方法具体如下:
 * 加密算法: AES-128
 * 加密模式: ECB
 * 填充方式: PKCS5Padding
 * 加密密钥: 约定key通过MD5计算得到。
 * 输出格式: base64
 * 字符集: utf-8
 *
 */
void aes_ecb_encrypt(unsigned char *key, unsigned char *input, unsigned char *output)
{
	AES_KEY realkey;
	AES_set_encrypt_key(key, AES_BLOCK_SIZE*8, &realkey);

	/* 该模式下pkcs5Padding 与 pkcs7Padding无异 */
	pkcs5_padding(input, ((strlen(input)/AES_BLOCK_SIZE)+1)*AES_BLOCK_SIZE, strlen(input));

	int len;
	for (len = 0; len < strlen(input); len+=AES_BLOCK_SIZE)
	{
		AES_encrypt(input+len, output+len, &realkey);
	}

	return ;
}

void aes_ecb_decrypt(unsigned char *key, unsigned char *input, unsigned char *output)
{
	AES_KEY realkey;
	AES_set_decrypt_key(key, AES_BLOCK_SIZE*8, &realkey);

	int len;
	for (len = 0; len < strlen(input); len+=AES_BLOCK_SIZE)
	{
		AES_decrypt(input+len, output+len, &realkey);
	}

	unsigned char padding_len = 0;
	padding_len = output[strlen(output) - 1];

	printf("padding_len: 0x%02x\n", padding_len);
	output[strlen(output) - padding_len] = '\0';

	return ;
}

void aes128_ecb_pkcs5_encrypt(unsigned char *key, unsigned char *input, unsigned char *output)
{
	unsigned char digest[16] = {0};
	MD5_CTX md5c;
    unsigned char aes_data[512] = {0};

	MD5Init(&md5c);
	MD5Update(&md5c, key, strlen(key));
	MD5Final(digest, &md5c);

	aes_ecb_encrypt(digest, input, aes_data);
	ByteToHexStr(aes_data, output, strlen(aes_data));

	return ;
}

/* aes_data MAXlength is 512. Not good */
void aes128_ecb_pkcs5_decrypt(unsigned char *key, unsigned char *input, unsigned char *output)
{
	unsigned char digest[16] = {0};
	MD5_CTX md5c;
    unsigned char aes_data[512] = {0};

	MD5Init(&md5c);
	MD5Update(&md5c, key, strlen(key));
	MD5Final(digest, &md5c);

	HexStrToByte(input, aes_data, strlen(input));
	aes_ecb_decrypt(digest, aes_data, output);

	return ;
}


void aes_ecb_test(void)
{
	char key[17] = "keytest";
	unsigned char plain[5][512] = {
		/* b802ab777f4744709ef9be14f65e1c81 */
		"datatest",
		/* 4b0f3ad68f2e76b25a04c1a5f4092aca */
		"HelloWorld",
		/* be53455b52d8b1f160aa021bf7955c5bb36acf9fc574c6120eeb44b73e73861f */
		"MTcwNjM1NjE3Nzk2",
		/* be53455b52d8b1f160aa021bf7955c5bbb05cb9008cd7a97baf15dee22b8ff0c */
		"MTcwNjM1NjE3Nzk2MTIzMTIz",
		/* be53455b52d8b1f160aa021bf7955c5b30f7d4ffa8ac489f0707698c10e1797db36acf9fc574c6120eeb44b73e73861f */
		"MTcwNjM1NjE3Nzk2MTIzMTIzSxSxSxSx"
	};
	unsigned char cipher[1024] = {0};
	unsigned char decrypt[1024] = {0};

	int i, j;

	printf("============AES_ECB_TEST============");
	for (i = 0; i < 5; i++)
	{
		printf("plain: %s, plain len: %d\n", plain[i], strlen(plain[i]));
		memset(cipher, 0x0, sizeof(cipher));
		aes_ecb_encrypt(key, plain[i], cipher);

		for (j = 0; j < strlen(cipher); j++)
		{
			printf("%02x ", cipher[j]);
		}
		printf("\n");

		memset(decrypt, 0x0, sizeof(decrypt));
		aes_ecb_decrypt(key, cipher, decrypt);
		printf("decrypt: %s, decrypt len: %d\n", decrypt, strlen(decrypt));
		printf("\n\n");

	}

	return ;
}

/*
 * 根据项目要求，加密方法具体如下:
 * 加密算法: AES-128
 * 加密模式: CBC
 * 填充方式: PKCS5Padding
 * 加密密钥: 网关下发的deviceToken字段
 * 偏移量: 0000000000000000
 * 输出格式: base64
 * 字符集: utf-8
 *
 */
#define IV	"0000000000000000"

void aes_cbc_encrypt(unsigned char *key, unsigned char *input, unsigned char *output)
{
	AES_KEY realkey;
	AES_set_encrypt_key(key, AES_BLOCK_SIZE*8, &realkey);

	unsigned char ivec[AES_BLOCK_SIZE+1] = {0};
	memcpy(ivec, IV, AES_BLOCK_SIZE);

	/* 该模式下pkcs5Padding 与 pkcs7Padding无异 */
	pkcs5_padding(input, ((strlen(input)/AES_BLOCK_SIZE)+1)*AES_BLOCK_SIZE, strlen(input));

	int SetDataLen = 0;

	if (0 == (strlen(input)%AES_BLOCK_SIZE))
		SetDataLen = strlen(input);
	else
		SetDataLen = ((strlen(input)/AES_BLOCK_SIZE)+1)*AES_BLOCK_SIZE;

	/* CBC接口已封装分块加密的动作，这是与ECB模式的区别 */
	AES_cbc_encrypt(input, output, SetDataLen, &realkey, ivec, AES_ENCRYPT);

	return ;
}


cbc_base64_t *aes_cbc_base64_encrpt(unsigned char *key, unsigned char *data)
{
	if (key == NULL || data == NULL) return NULL;

	size_t cipher_len = strlen(data) + AES_BLOCK_SIZE;
	size_t base64_len = cipher_len * 2;

	cbc_base64_t *p = NULL;
	p = (cbc_base64_t *)malloc(sizeof(cbc_base64_t));
	if (p) {
		memset(p, 0, sizeof(cbc_base64_t));
    }
	
	p->input = (unsigned char *)malloc(cipher_len);
	memset(p->input, 0, cipher_len);
	strncpy(p->input, data, strlen(data));
	
	p->cbc_cipher = (unsigned char *)malloc(cipher_len);
	memset(p->cbc_cipher, 0, cipher_len);
	aes_cbc_encrypt(key, p->input, p->cbc_cipher);

	p->base64 = (unsigned char *)malloc(base64_len);
	memset(p->base64, 0, base64_len);
	base64_encode(p->cbc_cipher, strlen(p->input), p->base64, base64_len - 1);
	return p;
}

cbc_base64_t *aes_cbc_base64_decrpt(unsigned char *key, unsigned char *data, size_t data_len)
{
	if (key == NULL || data == NULL) return NULL;

	size_t cipher_len = data_len * 2;

	cbc_base64_t *p = NULL;
	p = (cbc_base64_t *)malloc(sizeof(cbc_base64_t));
	if (p) {
		memset(p, 0, sizeof(cbc_base64_t));
    }

	p->input = (unsigned char *)malloc(data_len+1);
	memset(p->input, 0, data_len+1);
	strncpy(p->input, data, data_len);

	p->cbc_cipher = (unsigned char *)malloc(cipher_len);
	memset(p->cbc_cipher, 0, cipher_len);
	int length;
	length = base64_decode(p->input, p->cbc_cipher, cipher_len - 1);
	printf("base64_decode length: %d\n", length);
	p->output= (unsigned char *)malloc(data_len+1);
	memset(p->output, 0, data_len+1);
	aes_cbc_decrypt(key, p->cbc_cipher, length, p->output);

	return p;
}

void aes_cbc_base64_encrpt_free(cbc_base64_t *p)
{
	if (p == NULL) return;

	if (p->input) {
		free(p->input);
		p->input = NULL;
	}

	if (p->output) {
		free(p->output);
		p->output = NULL;
	}
	
	if (p->cbc_cipher) {
		free(p->cbc_cipher);
		p->cbc_cipher = NULL;
	}
	
	if (p->base64) {
		free(p->base64);
		p->base64 = NULL;
	}

	free(p);
	p = NULL;
	return;
}

void aes_cbc_decrypt(unsigned char *key, unsigned char *input, int input_len, unsigned char *output)
{
	AES_KEY realkey;
	AES_set_decrypt_key(key, AES_BLOCK_SIZE*8, &realkey);

	unsigned char ivec[AES_BLOCK_SIZE+1] = {0};
	memcpy(ivec, IV, AES_BLOCK_SIZE);

	int SetDataLen = 0;

	if (0 == (input_len%AES_BLOCK_SIZE))
		SetDataLen = input_len;
	else
		SetDataLen = ((input_len/AES_BLOCK_SIZE)+1)*AES_BLOCK_SIZE;

	/* CBC接口已封装分块加密的动作，这是与ECB模式的区别 */
	AES_cbc_encrypt(input, output, SetDataLen, &realkey, ivec, AES_DECRYPT);

	unsigned char padding_len = 0;
	padding_len = output[strlen(output) - 1];

	printf("padding_len: 0x%02x\n", padding_len);
	output[strlen(output) - padding_len] = '\0';

	return ;
}

void md5Data(const char *key, char *output)
{
	MD5_CTX md5ctx;
    unsigned char digest[16];
    char sTmp[10];
    int i =0;
    int nLen = 0;

    if (0 == key || 0 == output) return;

    nLen = strlen(key);

    MD5Init(&md5ctx);
    MD5Update(&md5ctx, (unsigned char*)key, nLen);
    MD5Final(digest, &md5ctx);

    *output = 0;
    for (i = 0; i < 16; ++i) {
        sprintf(sTmp, "%02x", digest[i]);
        strcat(output, sTmp);
    }
    output[32] = 0;
	return;
}

void aes_cbc_test(void)
{
	char key[17] = "keytest";
	unsigned char plain[5][512] = {
		/* 9d8a3b19c6a195bdae67c3dc64230a13 */
		"datatest",
		/* c67970d313a2abaee0e32002e0e65ac3 */
		"HelloWorld",
		/* 4bdd8a2cbeb3f38821b26b9ba67aab42f48401ef28f8f0ecedb4296583300f13 */
		"MTcwNjM1NjE3Nzk2",
		/* 4bdd8a2cbeb3f38821b26b9ba67aab429fa43cc160f55056d221940128e64c41 */
		"MTcwNjM1NjE3Nzk2MTIzMTIz",
		/* 4bdd8a2cbeb3f38821b26b9ba67aab42cab895b58c699d9a12b3359d469fce6f0d09df856a540ef1e7692f783937a6c0 */
		"MTcwNjM1NjE3Nzk2MTIzMTIzSxSxSxSx"
	};
	unsigned char cipher[1024] = {0};
	unsigned char decrypt[1024] = {0};

	int i, j;

	printf("============AES_CBC_TEST============");
	for (i = 0; i < 5; i++)
	{
		printf("plain: %s, plain len: %d\n", plain[i], strlen(plain[i]));
		memset(cipher, 0x0, sizeof(cipher));
		aes_cbc_encrypt(key, plain[i], cipher);

		for (j = 0; j < strlen(cipher); j++)
		{
			printf("%02x ", cipher[j]);
		}
		printf("\n");

		memset(decrypt, 0x0, sizeof(decrypt));
		aes_cbc_decrypt(key, cipher, strlen(cipher), decrypt);
		printf("decrypt: %s, decrypt len: %d\n", decrypt, strlen(decrypt));
		printf("\n\n");

	}

	return ;	
}

#if 0
int main(void)
{
	/* AES128-ecb-pkcs5Padding */
	aes_ecb_test();

	/* AES128-cbc-pkcs5Padding */
	aes_cbc_test();

	return 0;
}
#endif

