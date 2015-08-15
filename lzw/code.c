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
    if (ib + sizeof(olen) > ib + ilen) goto done;
    memcpy(&olen, ib, sizeof(olen));
    *obits = olen*8;
  }

  olen = (*obits/8) + ((*obits % 8) ? 1 : 0);

 done:
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

int init_dict(symbol_stats *s) {
  unsigned char a;
  int rc = -1;
  size_t j;

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

  rc = 0;

 done:
  return rc;
}

/* while x is an index into s->seq_all, we cheat and encode it
 * as fewer bits. x really only indexes into s->seq_all up to
 * the current item count (d) of the dictionary hash table. 
 */
unsigned char get_num_bits(symbol_stats *s) {
  unsigned long d = HASH_COUNT(s->dict);
  assert(d >= 256); /* the single-bytes seqs are always in the dict */

  /* let b = log2(d) rounded up to a whole integer. this is
   * the number of bits needed to distinguish d items. */
  unsigned char b=0;
  while (((d-1) >> b) != 0) b++;

  return b;
}

/* this macro emits the index x encoded as b bits in e */
#define emit()                                          \
 do {                                                   \
   b = get_num_bits(s);                                 \
   fprintf(stderr,"emit index %lu (in %u bits)\n",x,b); \
   if (p + b > eop) goto done;                          \
   while (b--) {                                        \
     if ((x >> b) & 1) BIT_SET(o,p);                    \
     p++;                                               \
   }                                                    \
   s->seq_all[x].hits++;                                \
 } while(0)

int lzw_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, 
     size_t olen, symbol_stats *s) {
  unsigned char b, *i=ib, *o=ob;
  unsigned long x;
  int rc = -1;
  size_t l;
  size_t p=0; 
  size_t eop = olen*8;

  if ((mode & MODE_ENCODE)) {
    if (init_dict(s) < 0) goto done;

    /* store length of decoded buffer */
    if (o + sizeof(ilen) > ib + ilen) goto done;
    memcpy(o, &ilen, sizeof(ilen));
    o += sizeof(ilen);

    /* LZW encode input buffer */
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
    if (init_dict(s) < 0) goto done;

    /* skip length */
    i += sizeof(olen);

    unsigned char q[s->max_seq_length], *qs;
    unsigned long _x;
    int first_time=1;

    while (o - ob < olen) {
      x = 0;
      b = get_num_bits(s);
      if ((i + b/8 + ((b%8) ? 1 : 0)) > ib + ilen) goto done;
      while(b--) {
        if (BIT_TEST(i,p)) x |= (1U << b);
        p++;
      }
      fprintf(stderr,"got index %lu (in %u bits)\n",x,get_num_bits(s));
      if (x >= HASH_COUNT(s->dict)) goto done;
      if (o + s->seq_all[x].l > ob + olen) goto done;
      if (s->seq_all[x].l == 0) goto done;
      memcpy(o, s->seq_all[x].s, s->seq_all[x].l);
      o += s->seq_all[x].l;

      /* add concatenated previous seq + extension */
      if (first_time) first_time=0;
      else if (s->seq_all[_x].l + s->seq_all[x].l <= s->max_seq_length) {
        qs = q;
        memcpy(qs, s->seq_all[_x].s, s->seq_all[_x].l);
        qs += s->seq_all[_x].l;
        memcpy(qs, s->seq_all[ x].s, s->seq_all[ x].l);
        add_seq(s,q,s->seq_all[_x].l+ s->seq_all[x].l);
      }
      _x = x;
    }
  }

  rc = 0;

 done:
  return rc;
}

