#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sha1.h"
#include "hmac_sha1.h"

void HMAC_SHA1_Init(HMAC_SHA1_CTX *context, const uint8_t *key, size_t key_len) {
    // upper limit on the key length
    if (key_len > 0x40) key_len = 0x40;
    // prepare the sha1 contexts
    SHA1_Init(&context->state[0]);
    SHA1_Init(&context->state[1]);
    // prepare the key buffers to initialise the sha1 contexts
    uint8_t key_buf_1[0x40] = {0};
    uint8_t key_buf_2[0x40] = {0};
    memcpy(key_buf_1, key, key_len);
    memcpy(key_buf_2, key, key_len);
    for (int i = 0; i < 0x40; i++) {
        key_buf_1[i] ^= 0x36;
        key_buf_2[i] ^= 0x5C;
    }
    SHA1_Update(&context->state[0], key_buf_1, 0x40);
    SHA1_Update(&context->state[1], key_buf_2, 0x40);
}

void HMAC_SHA1_Update(HMAC_SHA1_CTX *context, const uint8_t *data, const size_t len) {
    SHA1_Update(&context->state[0], data, len);
}

void HMAC_SHA1_Final(HMAC_SHA1_CTX *context, uint8_t digest[SHA1_DIGEST_SIZE]) {
    uint8_t hash[0x14];
    SHA1_Final(&context->state[0], hash);
    // feed the final hash of the first state into the second state
    SHA1_Update(&context->state[1], hash, sizeof(hash));
    SHA1_Final(&context->state[1], digest);
}

void HMAC_SHA1(const uint8_t *key, size_t key_len, const uint8_t *data, const size_t len, uint8_t digest[SHA1_DIGEST_SIZE]) {
    HMAC_SHA1_CTX ctx = {0};
    HMAC_SHA1_Init(&ctx, key, key_len);
    HMAC_SHA1_Update(&ctx, data, len);
    HMAC_SHA1_Final(&ctx, digest);
}
