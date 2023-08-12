#ifndef _HMAC_SHA1_H
#define _HMAC_SHA1_H

#include <stdint.h>
#include <stdio.h>
#include "sha1.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    SHA1_CTX state[2];
} HMAC_SHA1_CTX;

#define SHA1_DIGEST_SIZE 20

void HMAC_SHA1_Init(HMAC_SHA1_CTX *context, const uint8_t *key, size_t key_len);
void HMAC_SHA1_Update(HMAC_SHA1_CTX *context, const uint8_t *data, const size_t len);
void HMAC_SHA1_Final(HMAC_SHA1_CTX *context, uint8_t digest[SHA1_DIGEST_SIZE]);

void HMAC_SHA1(const uint8_t *key, size_t key_len, const uint8_t *data, const size_t len, uint8_t digest[SHA1_DIGEST_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // _HMAC_SHA1_H
