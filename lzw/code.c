#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "code.h"

void dump_codes(symbol_stats *s) {
}

void lzw_release(symbol_stats *s) {
  HASH_CLEAR(hh, s->dict);
  if (s->seq_all) free(s->seq_all);
}

int lzw_load_codebook(char *file, symbol_stats *s) {
  int rc = -1;
  return rc;
}

int lzw_save_codebook(char *file, symbol_stats *s) {
  int rc = -1;
  return rc;
}

int sequence_frequency_sort(struct seq *a, struct seq *b) {
  if (a->hits < b->hits) return -1;
  if (a->hits > b->hits) return  1;
  return 0;
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

  /* get a free sequence structure or recycle */
  if (s->seq_used < s->max_dict_entries) 
    q = &s->seq_all[ s->seq_used++ ];
  else {
    /* recycle the seq with fewest occurences. 
     * single-bytes are exempt from recycling. */
    HASH_SORT(s->dict, sequence_frequency_sort);
    q = s->dict;
    while(q->l == 1) q=q->hh.next; 
    HASH_DEL(s->dict, q);
  }

  /* reset the structure */
  memset(q,0,sizeof(*q) + s->max_seq_length);
  q->l = len;
  memcpy(q->s, seq, len);

  HASH_ADD(hh, s->dict, s, len, q);
  return 0;
}

int have_seq(symbol_stats *s, unsigned char *seq, size_t len, unsigned long *index) {
  struct seq *q;

  HASH_FIND(hh, s->dict, seq, len, q);
  if (q == NULL) return 0;

  assert(q >= s->seq_all);
  *index = q - s->seq_all;
  return 1;
}

/* while x is an index into s->seq_all, we cheat and encode it
 * as fewer bits. x really only indexes into s->seq_all up to
 * the current item count (d) of the dictionary hash table. 
 */
unsigned char get_num_bits(symbol_stats *s, unsigned long x) {
  unsigned long d = HASH_COUNT(s->dict);
  assert(d >= 256); /* the single-bytes seqs are always in the dict */
  assert(x < d);

  /* let b = log2(d) rounded up to a whole integer. this is
   * the number of bits needed to distinguish d items. */
  unsigned char b=0;
  while (((d-1) >> b) != 0) b++;

  return b;
}

/* this macro emits the index x encoded as b bits in e */
#define emit()                                          \
 do {                                                   \
   b = get_num_bits(s,x);                               \
   if (p + b > eop) goto done;                          \
   while (b--) {                                        \
     if ((x >> b) & 1) BIT_SET(o,p);                    \
     p++;                                               \
   }                                                    \
   s->seq_all[x].hits++;                                \
 } while(0)

int lzw_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, 
     size_t olen, symbol_stats *s) {
  unsigned char a, b, *i=ib, *o=ob;
  unsigned long x;
  int j, rc = -1;
  size_t l;
  size_t p=0, eop = olen*8; /* bit position in obuf; eop is first bit beyond */

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

    /*
     * LZW encode input buffer 
     */

    l = 1;

    while(1) {

      /* sequence starts at i for length l */

      /* does sequence extend over buffer end? */
      if (i+l > ib+ilen) {
        if (l > 1) emit();
        break;
      }

      /* is this sequence in the dictionary? */
      if (have_seq(s, i, l, &x)) {
        /* is it already the max seq length? */
        if (l == s->max_seq_length) {
          emit();
          i += l;
          l = 1;
        } else l++;
        continue;
      }

      /* the sequence is not in the dictionary */
      emit();
      add_seq(s, i, l);
      i += l;
      l = 1;
    }
  }

  if ((mode & MODE_DECODE)) {
  }

  rc = 0;

 done:
  return rc;
}

