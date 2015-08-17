
#ifndef _LZCODE_H_
#define _LZCODE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "uthash.h"

#define adim(x) (sizeof(x)/sizeof(*(x)))

struct seq {
  unsigned char *s;  /* sequence bytes */
  size_t l;          /* sequence length */
  unsigned hits;
  UT_hash_handle hh;
};

typedef struct {
  struct seq *seq_all;
  struct seq *dict;
  size_t seq_used;
  size_t max_dict_entries;
} lzw;

/* standard bit vector macros */
#define BIT_TEST(c,i)  ((c[(i)/8] &  (1 << ((i) % 8))) ? 1 : 0)
#define BIT_SET(c,i)   (c[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(c,i) (c[(i)/8] &= ~(1 << ((i) % 8)))

#define MODE_ENCODE          (1U << 0)
#define MODE_DECODE          (1U << 1)

int lzw_init(lzw *s);
void lzw_release(lzw *s);
size_t lzw_compute_olen(int mode, unsigned char *ib, size_t ilen, lzw *s);
int lzw_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, 
               size_t *olen, lzw *s);

#endif /* _LZCODE_H_ */
