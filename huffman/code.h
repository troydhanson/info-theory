
#ifndef _ECCODE_H_
#define _ECCODE_H_

#include <stddef.h>

struct sym {
  int leaf;
  union {
    unsigned char leaf_value;
    struct {
      struct sym *a;
      struct sym *b;
    } cn;
  } u;
  unsigned char code_length[256];/* #bits to encode  */
  unsigned int  code[256];       /* code in lower bits */
};

typedef struct {
  size_t count[256];             /* =count of byte [n] */
  size_t nbytes;                 /* count of all bytes */
  struct sym symbols[256];       /* leaf symbols */
  /* TODO vector of the dynamic compound symbols */

  /* rank and irank are 1-1 mappings of bytes to ranks. for example suppose 
   * byte 0x41 ('A') is the most popular byte (rank 0) in the input.  then 
   *   rank['A'] = 0
   *   irank[0] = 'A'  */
  unsigned char rank[256]; /* =rank of byte [n] by frequency (0=highest) */
  unsigned char irank[256];/* =byte whose rank is [n] (inverse of rank) */
} symbol_stats;

/* standard bit vector macros */
#define BIT_TEST(c,i)  ((c[(i)/8] &  (1 << ((i) % 8))) ? 1 : 0)
#define BIT_SET(c,i)   (c[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(c,i) (c[(i)/8] &= ~(1 << ((i) % 8)))

/* while the first four are mutually exclusive we use
 * bit flags to support OR'ing additional options */
#define MODE_ENCODE     (1U << 0)
#define MODE_DECODE     (1U << 1)

int huf_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, symbol_stats *s);
size_t huf_compute_olen(int mode, size_t ilen, size_t *ibits, size_t *obits, symbol_stats *s);

#endif /* _ECCODE_H_ */
