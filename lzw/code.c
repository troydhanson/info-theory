#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utlist.h"
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

/* 
 * 
 */ 
int lzw_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, 
     size_t olen, symbol_stats *s, size_t max_seq_length, size_t max_dict_entries) {
  int rc=-1;

  if ((mode & MODE_ENCODE)) {
    s->seqs_all = calloc(max_dict_entries, sizeof(struct seq) + max_seq_length);
    if (s->seqs_all == NULL) {
      fprintf(stderr,"out of memory\n");
      goto done;
    }
  }

  if ((mode & MODE_DECODE)) {
  }

  rc = 0;

 done:
  return rc;
}

