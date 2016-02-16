
#ifndef _LZCODE_H_
#define _LZCODE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#define adim(x) (sizeof(x)/sizeof(*(x)))

struct seq {
  unsigned char *s;  /* sequence bytes */
  size_t l;          /* sequence length */
  struct seq *n[256];/* sequence of concat(s,c) for any c */
  unsigned long x;   /* index of precursor seq in seq_all */
};

typedef struct {
  struct seq *seq_all;
  size_t seq_used;
  size_t max_dict_entries;
  struct { /* mmap'd dictionary. sequences point into the mapped region. */
    unsigned char *addr;
    off_t len;
    int fd;
  } map;
} lzw;

/* standard bit vector macros */
#define BIT_TEST(c,i)  ((c[(i)/8] &  (1 << ((i) % 8))) ? 1 : 0)
#define BIT_SET(c,i)   (c[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(c,i) (c[(i)/8] &= ~(1 << ((i) % 8)))

#define MODE_ENCODE          (1U << 0)
#define MODE_DECODE          (1U << 1)
#define MODE_MAKE_CODES      (1U << 2)
#define MODE_SHOW_CODES      (1U << 3)

int mlzw_init(lzw *s);
int mlzw_load(lzw *s, char *file );
int mlzw_save_codebook(lzw *s, char *file);
int mlzw_show_codebook(lzw *s);
void mlzw_release(lzw *s);
int mlzw_recode(int mode, lzw *s, unsigned char *ib, size_t ilen, 
                unsigned char *ob, size_t *olen);

#endif /* _LZCODE_H_ */
