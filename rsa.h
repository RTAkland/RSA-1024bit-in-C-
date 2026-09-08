/*
 * rsa.h
 */

#ifndef RSA_H_
#define RSA_H_

void addbignum(uint64_t res[], uint64_t op1[], uint64_t op2[], uint32_t n);
int subbignum(uint64_t res[], uint64_t op1[], uint64_t op2[], uint32_t n);
void modbignum(uint64_t res[], uint64_t op1[], uint64_t op2[], uint32_t n);
void modnum(uint64_t res[], uint64_t op1[], uint64_t op2[], uint32_t n);
void modmult1024(uint64_t res[], uint64_t op1[], uint64_t op2[], uint64_t mod[]);
void rsa1024(uint64_t res[], uint64_t data[], uint64_t expo[], uint64_t key[]);
void multbignum(uint64_t res[], uint64_t op1[], uint32_t op2, uint32_t n);
uint32_t bit_length(uint64_t op[], uint32_t n);
int32_t compare(uint64_t op1[], uint64_t op2[], uint32_t n);
void slnbignum(uint64_t res[], uint64_t op[], uint32_t len, uint32_t n);
void srnbignum(uint64_t res[], uint64_t op[], uint32_t len, uint32_t n);

#endif /* RSA_H_ */
