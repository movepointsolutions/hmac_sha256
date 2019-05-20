/*
    hmac_sha256.c
    Originally written by https://github.com/h5p9sl
*/

#include "hmac_sha256.h"
#include "WjCryptLib_Sha256.h"

#include <stdlib.h>
#include <string.h>

#define SIZEOFARRAY(x) sizeof(x) / sizeof(x[0])

/* LOCAL FUNCTIONS */
void sha256(const uint8_t* data, const unsigned datalen, uint8_t* out);
void appendAndHash(const uint8_t* dest, const unsigned destlen, const uint8_t* src, const unsigned srclen, uint8_t* out, const unsigned outlen
);

// Declared in hmac_sha256.h
void hmac_sha256(
    const uint8_t* key, const unsigned keylen,
    const uint8_t* data, const unsigned datalen,
    uint8_t* out, const unsigned outlen)
{
    uint8_t k[SHA256_BLOCK_SIZE]; // block-sized key derived from 'key' parameter
    uint8_t k_ipad[SHA256_BLOCK_SIZE];
    uint8_t k_opad[SHA256_BLOCK_SIZE];
    uint8_t hash0[SHA256_HASH_SIZE];
    uint8_t hash1[SHA256_HASH_SIZE];
    int i;

    // Fill 'k' with zero bytes
    memset(k, 0, SIZEOFARRAY(k));
    if (keylen > SHA256_BLOCK_SIZE) {
        // If the key is larger than the hash algorithm's block size, we must digest it first.
        sha256(key, keylen, k);
    } else {
        memcpy(k, key, keylen);
    }

    // Create outer & inner padded keys
    memset(k_ipad, 0x36, SHA256_BLOCK_SIZE);
    memset(k_opad, 0x5c, SHA256_BLOCK_SIZE);
    for (i = 0; i < SHA256_BLOCK_SIZE; i++) {
        k_ipad[i] ^= k[i];
        k_opad[i] ^= k[i];
    }

    // Perform HMAC algorithm H(K XOR opad, H(K XOR ipad, text))
    // https://tools.ietf.org/html/rfc2104
    appendAndHash(k_ipad, SIZEOFARRAY(k_ipad), data, datalen, hash0, SIZEOFARRAY(hash0));
    appendAndHash(k_opad, SIZEOFARRAY(k_opad), hash0, SIZEOFARRAY(hash0), hash1, SIZEOFARRAY(hash1));

    // Copy the resulting hash the output buffer
    // Trunacate sha256 hash if needed
    unsigned sz = (SHA256_HASH_SIZE <= outlen) ? SHA256_HASH_SIZE : outlen;
    memcpy(out, hash1, sz);
}

void appendAndHash(
    const uint8_t* dest, const unsigned destlen,
    const uint8_t* src,  const unsigned srclen,
    uint8_t*       out,  const unsigned outlen)
{
    uint8_t buf[destlen + srclen];
    uint8_t hash[SHA256_HASH_SIZE];

    memcpy(buf, dest, destlen);
    memcpy(buf + destlen, src, srclen);

    // Hash 'buf' and store into into another buffer
    sha256(buf, SIZEOFARRAY(buf), hash);

    // Copy the resulting hash to the output buffer
    // Truncate hash if needed
    unsigned sz = (SHA256_HASH_SIZE <= outlen) ? SHA256_HASH_SIZE : outlen;
    memcpy(out, hash, SHA256_HASH_SIZE);
}

void sha256(const uint8_t* data, const unsigned datalen, uint8_t* out) {
    Sha256Context ctx;
    SHA256_HASH hash;

    Sha256Initialise(&ctx);
    Sha256Update(&ctx, data, datalen);
    Sha256Finalise(&ctx, &hash);

    memcpy(out, hash.bytes, SHA256_HASH_SIZE);
}
