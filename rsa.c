#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rsa.h"

static rsa_random_func g_random_func = NULL;

void rsa1024_set_random(rsa_random_func func) {
    g_random_func = func;
}

void addbignum(uint64_t res[], const uint64_t op1[], const uint64_t op2[], uint32_t n) {
    uint32_t i;
    uint64_t j, k, carry = 0;
    for (i = 0; i < n; i++) {
        j = (op1[i] & 0xffffffffULL) + (op2[i] & 0xffffffffULL) + carry;
        k = ((op1[i] >> 32) & 0xffffffffULL) + ((op2[i] >> 32) & 0xffffffffULL) + ((j >> 32) & 0xffffffffULL);
        carry = (k >> 32) & 0xffffffffULL;
        res[i] = ((k & 0xffffffffULL) << 32) | (j & 0xffffffffULL);
    }
    res[n] = carry;
}

void multbignum(uint64_t res[], const uint64_t op1[], uint32_t op2, uint32_t n) {
    uint32_t i;
    uint64_t j, k, carry1 = 0, carry2 = 0;
    for (i = 0; i < n; i++) {
        j = (op1[i] & 0xffffffffULL) * (op2 & 0xffffffffULL);
        k = ((op1[i] >> 32) & 0xffffffffULL) * (op2 & 0xffffffffULL);
        carry1 = k >> 32;
        k = (k & 0xffffffffULL) + (j >> 32);
        j = (j & 0xffffffffULL) + carry2;
        k += j >> 32;
        carry2 = carry1 + (k >> 32);
        res[i] = ((k & 0xffffffffULL) << 32) | (j & 0xffffffffULL);
    }
    res[n] = carry2;
}

int subbignum(uint64_t res[], uint64_t op1[], const uint64_t op2[], uint32_t n) {
    uint32_t i;
    int carry = 0;
    for (i = 0; i < n; i++) {
        if (carry) {
            if (op1[i] != 0) carry = 0;
            op1[i]--;
        }
        if (op1[i] < op2[i]) carry = 1;
        res[i] = op1[i] - op2[i];
    }
    return carry;
}

int32_t compare(const uint64_t op1[], const uint64_t op2[], uint32_t n) {
    while (n > 0) {
        n--;
        if (op1[n] > op2[n]) return 1;
        if (op1[n] < op2[n]) return -1;
    }
    return 0;
}

uint32_t bit_length(const uint64_t op[], uint32_t n) {
    uint32_t i;
    for (; n > 0; n--) {
        uint64_t value = op[n - 1];
        if (value == 0) continue;
        for (i = 64; i > 0; i--) {
            if (value & (1ULL << (i - 1))) return (64 * (n - 1)) + i;
        }
    }
    return 0;
}

void slnbignum(uint64_t res[], const uint64_t op[], uint32_t len, uint32_t n) {
    uint32_t i, word_shift = n / 64, bit_shift = n % 64;
    uint64_t carry = 0;
    if (word_shift >= len) {
        for (i = 0; i < len; i++) res[i] = 0;
        return;
    }
    for (i = len; i > word_shift; i--) res[i - 1] = op[i - 1 - word_shift];
    for (i = 0; i < word_shift; i++) res[i] = 0;
    if (bit_shift == 0) return;
    for (i = 0; i < len; i++) {
        uint64_t value = res[i];
        uint64_t next_carry = value >> (64 - bit_shift);
        res[i] = (value << bit_shift) | carry;
        carry = next_carry;
    }
}

void srnbignum(uint64_t res[], const uint64_t op[], uint32_t len, uint32_t n) {
    uint32_t i, word_shift = n / 64, bit_shift = n % 64;
    uint64_t carry = 0;
    if (word_shift >= len) {
        for (i = 0; i < len; i++) res[i] = 0;
        return;
    }
    for (i = 0; i + word_shift < len; i++) res[i] = op[i + word_shift];
    for (; i < len; i++) res[i] = 0;
    if (bit_shift == 0) return;
    for (i = len; i > 0; i--) {
        uint64_t value = res[i - 1];
        uint64_t next_carry = value << (64 - bit_shift);
        res[i - 1] = (value >> bit_shift) | carry;
        carry = next_carry;
    }
}

void modbignum(uint64_t res[], const uint64_t op1[], const uint64_t op2[], uint32_t n) {
    uint32_t i;
    int32_t len_op1 = (int32_t)bit_length(op1, n);
    int32_t len_op2 = (int32_t)bit_length(op2, n);
    int32_t len_diff = len_op1 - len_op2;
    uint64_t divisor[RSA1024_LIMBS + 2] = {0};

    for (i = 0; i < n; i++) {
        res[i] = op1[i];
        divisor[i] = op2[i];
    }
    if (len_diff < 0) return;

    slnbignum(divisor, divisor, n, (uint32_t)len_diff);
    for (i = 0; i <= (uint32_t)len_diff; i++) {
        while (compare(res, divisor, n) >= 0) subbignum(res, res, divisor, n);
        if (i != (uint32_t)len_diff) srnbignum(divisor, divisor, n, 1);
    }
}

void modnum(uint64_t res[], const uint64_t op1[], const uint64_t op2[], uint32_t n) {
    uint32_t i;
    uint64_t divisor[RSA1024_LIMBS + 1] = {0};
    int result = 0;

    for (i = 0; i < n; i++) {
        res[i] = op1[i];
        divisor[i] = op2[i];
    }
    while (!result) result = subbignum(res, res, divisor, n);
    addbignum(res, res, divisor, n);
}

void modmult1024(uint64_t res[], const uint64_t op1[], const uint64_t op2[], const uint64_t mod[]) {
    int32_t i, j;
    uint64_t mult1[33] = {0}, mult2[33] = {0}, result[33] = {0}, xmod[33] = {0};

    for (i = 0; i < RSA1024_LIMBS; i++) xmod[i] = mod[i];

    for (i = 0; i < RSA1024_LIMBS; i++) {
        for (j = 0; j < 33; j++) { mult1[j] = 0; mult2[j] = 0; }
        multbignum(mult1, op1, (uint32_t)(op2[i] & 0xffffffffULL), RSA1024_LIMBS);
        multbignum(mult2, op1, (uint32_t)(op2[i] >> 32), RSA1024_LIMBS);
        slnbignum(mult2, mult2, 33, 32);
        addbignum(mult2, mult2, mult1, 32);
        slnbignum(mult2, mult2, 33, (uint32_t)(64 * i));
        addbignum(result, result, mult2, 32);
    }

    modbignum(result, result, xmod, 33);
    for (i = 0; i < RSA1024_LIMBS; i++) res[i] = result[i];
}

void rsa1024(uint64_t res[], const uint64_t data[], const uint64_t expo[], const uint64_t key[]) {
    int32_t i, j, expo_len;
    uint64_t mod_data[RSA1024_LIMBS + 2] = {0}, result[RSA1024_LIMBS + 2] = {0}, exponent;

    modbignum(mod_data, data, key, RSA1024_LIMBS);
    result[0] = 1;
    expo_len = (int32_t)(bit_length(expo, RSA1024_LIMBS) / 64);

    for (i = 0; i <= expo_len; i++) {
        exponent = expo[i];
        for (j = 0; j < 64; j++) {
            if (exponent & 1ULL) modmult1024(result, result, mod_data, key);
            modmult1024(mod_data, mod_data, mod_data, key);
            exponent >>= 1;
        }
    }
    for (i = 0; i < RSA1024_LIMBS; i++) res[i] = result[i];
}

static uint64_t load_be64(const uint8_t *p) {
    return ((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48) | ((uint64_t)p[2] << 40) |
           ((uint64_t)p[3] << 32) | ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16) |
           ((uint64_t)p[6] << 8)  | ((uint64_t)p[7]);
}

static void store_be64(uint8_t *p, uint64_t value) {
    p[0] = (uint8_t)(value >> 56); p[1] = (uint8_t)(value >> 48);
    p[2] = (uint8_t)(value >> 40); p[3] = (uint8_t)(value >> 32);
    p[4] = (uint8_t)(value >> 24); p[5] = (uint8_t)(value >> 16);
    p[6] = (uint8_t)(value >> 8);  p[7] = (uint8_t)value;
}

void rsa1024_bytes_to_limbs(uint64_t out[RSA1024_LIMBS], const uint8_t in[RSA1024_BYTES]) {
    for (uint32_t i = 0; i < RSA1024_LIMBS; i++) {
        out[i] = load_be64(in + (RSA1024_LIMBS - 1 - i) * 8);
    }
}

void rsa1024_limbs_to_bytes(uint8_t out[RSA1024_BYTES], const uint64_t in[RSA1024_LIMBS]) {
    for (uint32_t i = 0; i < RSA1024_LIMBS; i++) {
        store_be64(out + (RSA1024_LIMBS - 1 - i) * 8, in[i]);
    }
}

int rsa1024_raw(uint8_t out[RSA1024_BYTES], const uint8_t data[RSA1024_BYTES], const uint8_t exponent[RSA1024_BYTES], const uint8_t modulus[RSA1024_BYTES]) {
    uint64_t data_limbs[RSA1024_LIMBS], exponent_limbs[RSA1024_LIMBS], modulus_limbs[RSA1024_LIMBS], result[RSA1024_LIMBS];

    if (!out || !data || !exponent || !modulus) return -1;

    rsa1024_bytes_to_limbs(modulus_limbs, modulus);
    int nonzero = 0;
    for (uint32_t i = 0; i < RSA1024_LIMBS; i++) {
        if (modulus_limbs[i] != 0) { nonzero = 1; break; }
    }
    if (!nonzero) return -2;

    rsa1024_bytes_to_limbs(data_limbs, data);
    rsa1024_bytes_to_limbs(exponent_limbs, exponent);

    rsa1024(result, data_limbs, exponent_limbs, modulus_limbs);
    rsa1024_limbs_to_bytes(out, result);
    return 0;
}

static int random_nonzero_byte(uint8_t *out) {
    uint8_t value;
    int result;
    do {
        result = g_random_func(&value, 1);
        if (result != 0) return -1;
    } while (value == 0);
    *out = value;
    return 0;
}

int rsa1024_pkcs1_encrypt(uint8_t out[RSA1024_BYTES], const uint8_t *plaintext, size_t plaintext_len, const uint8_t modulus[RSA1024_BYTES], const uint8_t exponent[RSA1024_BYTES]) {
    uint8_t encoded[RSA1024_BYTES];
    size_t padding_len, i;
    int result;

    if (!out || !modulus || !exponent || (plaintext_len > 0 && !plaintext)) return -1;
    if (plaintext_len > RSA1024_PKCS1_MAX_DATA) return -2;
    if (!g_random_func) return -3;

    padding_len = RSA1024_BYTES - plaintext_len - 3;
    encoded[0] = 0x00;
    encoded[1] = 0x02;

    for (i = 0; i < padding_len; i++) {
        result = random_nonzero_byte(&encoded[2 + i]);
        if (result != 0) return -4;
    }

    encoded[2 + padding_len] = 0x00;
    if (plaintext_len > 0) {
        memcpy(encoded + 3 + padding_len, plaintext, plaintext_len);
    }

    return rsa1024_raw(out, encoded, exponent, modulus);
}
