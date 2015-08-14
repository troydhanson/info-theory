#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "code.h"

void dump_codes(symbol_stats *s) {
}

int lzw_load_codebook(char *file, symbol_stats *s) {
  int rc = -1;
  return rc;
}

int lzw_save_codebook(char *file, symbol_stats *s) {
  int rc = -1;
  return rc;
}

size_t lzw_compute_olen( int mode, unsigned char *ib, size_t ilen, 
     size_t *ibits, size_t *obits, symbol_stats *s)
{
  size_t olen = 0;

  if (mode & MODE_ENCODE) {
  }

  if (mode & MODE_DECODE) {
  }

  olen = (*obits/8) + ((*obits % 8) ? 1 : 0);

  return olen;
}

size_t add_seq(symbol_stats *s, unsigned char *seq, size_t len) {
  struct seq *q;

  /* grab a free sequence structure or recycle */
  if (s->seq_used < s->max_dict_entries) 
    q = &s->seq_all[ s->seq_used++ ];
  else {
    q = NULL; /* FIXME. the least used one i suppose. */
  }
  
  q->l = len;
  return 0;
}

/* 
 * 
 */ 
int lzw_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, 
     size_t olen, symbol_stats *s) {
  unsigned char a;
  int j, rc = -1;

  if ((mode & MODE_ENCODE)) {
    /* allocate all the sequence entries as one contiguous buffer */
    s->seq_all = calloc(s->max_dict_entries, 
                         sizeof(struct seq) + s->max_seq_length);
    if (s->seq_all == NULL) {
      fprintf(stderr,"out of memory\n");
      goto done;
    }

    /* seed the single-byte sequences */
    for(j=0; j < 256; j++) {
      a = j;
      add_seq(s, &a, 1);
    }
  }

  if ((mode & MODE_DECODE)) {
  }

  rc = 0;

 done:
  return rc;
}

