
#ifndef _HCCODE_H_
#define _HCCODE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define adim(x) (sizeof(x)/sizeof(*(x)))

struct sym {
  int is_leaf;
  size_t count;
  union {
    unsigned char leaf_value;
    struct {
      struct sym *a;
      struct sym *b;
    } n;
  };
  unsigned char code_length;
  unsigned long code;
  struct sym *next;
};

typedef struct {
  struct sym *syms; /* working set of top level symbols */
  size_t sym_take;  /* index of next free sym in sym_all */
  struct sym sym_all[256*2];
  size_t header_len;
  unsigned char header[256*(1+1+sizeof(long))]; /* max */
} symbol_stats;

/* standard bit vector macros */
#define BIT_TEST(c,i)  ((c[(i)/8] &  (1 << ((i) % 8))) ? 1 : 0)
#define BIT_SET(c,i)   (c[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(c,i) (c[(i)/8] &= ~(1 << ((i) % 8)))

#define MODE_ENCODE          (1U << 0)
#define MODE_DECODE          (1U << 1)
#define MODE_SAVE_CODES      (1U << 2)
#define MODE_USE_SAVED_CODES (1U << 3)
#define MODE_DISPLAY_CODES   (1U << 4)

int huf_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, 
               symbol_stats *s);
size_t huf_compute_olen(int mode, unsigned char *ib, size_t ilen, 
               size_t *ibits, size_t *obits, symbol_stats *s);
int huf_load_codebook(char *file, symbol_stats *s);
int huf_save_codebook(char *file, symbol_stats *s);

#endif /* _HCCODE_H_ */
