#ifndef _ECCODE_H_
#define _ECCODE_H_

#include <stddef.h>

typedef struct {
  size_t count[256];             /* =count of byte [n] */
  size_t nbytes;                 /* count of all bytes */
  unsigned char code_length[256];/* =#bits to encode [n] (Shannon's m) */
  unsigned int  code[256];       /* generated binary code for [n] in lower bits */

  /* rank and irank are 1-1 mappings of bytes to ranks. for example suppose 
   * byte 0x41 ('A') is the most popular byte (rank 0) in the input.  then 
   *   rank['A'] = 0
   *   irank[0] = 'A'  */
  unsigned char rank[256]; /* =rank of byte [n] by frequency (0=highest) */
  unsigned char irank[256];/* =byte whose rank is [n] (inverse of rank) */
  size_t Pcount[256];      /* =count of bytes more probable than [n] */
} symbol_stats;

/* standard bit vector macros */
#define BIT_TEST(c,i)  ((c[(i)/8] &  (1 << ((i) % 8))) ? 1 : 0)
#define BIT_SET(c,i)   (c[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(c,i) (c[(i)/8] &= ~(1 << ((i) % 8)))

#define MODE_ENCODE     (1U << 0)
#define MODE_DECODE     (1U << 1)

int ecc_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, symbol_stats *s, int verbose);
size_t ecc_compute_olen(int mode, unsigned char *ib, size_t ilen, size_t *ibits, size_t *obits, symbol_stats *s, int verbose);

#endif /* _ECCODE_H_ */
