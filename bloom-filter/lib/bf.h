#ifndef BF_H
#define BF_H

#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_HASHES 2
#define MASK(u,n) ( (u) & ((1UL << (n)) - 1))
/* standard bit vector macros */
#define BIT_TEST(c,i)  ((c[(i)/8] &   (1 << ((i) % 8))) ? 1 : 0)
#define BIT_SET(c,i)    (c[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(c,i)  (c[(i)/8] &= ~(1 << ((i) % 8)))
/* number of bytes needed to store 2^n bits */
#define byte_len(n) (((1UL << n) / 8) + (((1UL << n) % 8) ? 1 : 0))
/* number of bytes needed to store n bits */
#define bytes_nbits(n) ((n/8) + ((n % 8) ? 1 : 0))
/* number of bits in 2^n bits */
#define num_bits(n) (1UL << n)

struct bf;

struct bf *bf_new(unsigned n);

void bf_free(struct bf *bf);

int bf_test(struct bf *bf, const char *data, size_t len);

void bf_add(struct bf *bf, const char *data, size_t len);

void bf_info(struct bf *bf, FILE *);

#ifdef __cplusplus
}
#endif

#endif
