#ifndef RSA_H_
#define RSA_H_

#include <stdint.h>
#include <stddef.h>

void addbignum(
    uint64_t res[],
    const uint64_t op1[],
    const uint64_t op2[],
    uint32_t n
);

int subbignum(
    uint64_t res[],
    uint64_t op1[],
    const uint64_t op2[],
    uint32_t n
);

void modbignum(
    uint64_t res[],
    const uint64_t op1[],
    const uint64_t op2[],
    uint32_t n
);

 * Legacy/simple modulo implementation.
 */
void modnum(
    uint64_t res[],
    const uint64_t op1[],
    const uint64_t op2[],
    uint32_t n
);


/*
 * res = (op1 * op2) % mod
 *
 * RSA-1024:
 *   op1 = 1024-bit
 *   op2 = 1024-bit
 *   mod = 1024-bit
 */
void modmult1024(
    uint64_t res[],
    const uint64_t op1[],
    const uint64_t op2[],
    const uint64_t mod[]
);


void rsa1024(
    uint64_t res[],
    const uint64_t data[],
    const uint64_t expo[],
    const uint64_t key[]
);

void multbignum(
    uint64_t res[],
    const uint64_t op1[],
    uint32_t op2,
    uint32_t n
);

uint32_t bit_length(
    const uint64_t op[],
    uint32_t n
);

int32_t compare(
    const uint64_t op1[],
    const uint64_t op2[],
    uint32_t n
);

void slnbignum(
    uint64_t res[],
    const uint64_t op[],
    uint32_t len,
    uint32_t n
);


void srnbignum(
    uint64_t res[],
    const uint64_t op[],
    uint32_t len,
    uint32_t n
);

int rsa1024_raw(
    uint8_t out[128],
    const uint8_t data[128],
    const uint8_t exponent[128],
    const uint8_t modulus[128]
);


int rsa1024_pkcs1_encrypt(
    uint8_t out[128],
    const uint8_t *plaintext,
    size_t plaintext_len,
    const uint8_t modulus[128],
    const uint8_t exponent[128]
);


#endif /* RSA_H_ */
