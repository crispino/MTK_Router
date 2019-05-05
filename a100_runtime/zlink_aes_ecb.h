#ifndef ZLINK_AES_ECB_H
#define ZLINK_AES_ECB_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "aes.h"
#include "md5.h"


typedef struct {
	unsigned char *input;
	unsigned char *output;
	unsigned char *cbc_cipher;
	unsigned char *base64;
} cbc_base64_t;

void aes_ecb_encrypt(unsigned char *key, unsigned char *input, unsigned char *output);
void aes_ecb_decrypt(unsigned char *key, unsigned char *input, unsigned char *output);
void aes128_ecb_pkcs5_encrypt(unsigned char *key, unsigned char *input, unsigned char *output);
void aes128_ecb_pkcs5_decrypt(unsigned char *key, unsigned char *input, unsigned char *output);
void aes_cbc_encrypt(unsigned char *key, unsigned char *input, unsigned char *output);
void aes_cbc_decrypt(unsigned char *key, unsigned char *input, int input_len, unsigned char *output);

extern cbc_base64_t *aes_cbc_base64_encrpt(unsigned char *key, unsigned char *data);
extern cbc_base64_t *aes_cbc_base64_decrpt(unsigned char *key, unsigned char *data, size_t data_len);
void aes_cbc_base64_encrpt_free(cbc_base64_t *p);
void md5Data(const char *key, char *output);


#ifdef  __cplusplus
}
#endif


#endif


