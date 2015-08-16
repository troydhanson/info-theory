#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "code.h"

/* the memory pointed to by seq must be static through the 
 * lifetime of encoding or decoding the buffer */
static void add_seq(symbol_stats *s, unsigned char *seq, size_t len) {
  struct seq *q;

  /* our dictionary has a hard limit- no recycling */
  if (s->seq_used == s->max_dict_entries) return;

  q = &s->seq_all[ s->seq_used++ ];
  q->hits = 0;
  q->l = len;
  q->s = seq;
  HASH_ADD_KEYPTR(hh, s->dict, q->s, q->l, q);
  //fprintf(stderr,"add [%.*s]<len %u> @ index %lu\n", (int)len, seq, (int)len, q-s->seq_all);
}

static int have_seq(symbol_stats *s, unsigned char *seq, size_t len, unsigned long *index) {
  struct seq *q;

  HASH_FIND(hh, s->dict, seq, len, q);
  if (q == NULL) return 0;

  assert(q >= s->seq_all);
  *index = q - s->seq_all;
  return 1;
}

static unsigned char bytes_all[256];
int mlzw_init(symbol_stats *s) {
  int rc = -1;
  size_t j;

  /* allocate the dictionary as one contiguous buffer */
  s->seq_all = calloc(s->max_dict_entries, sizeof(struct seq));
  if (s->seq_all == NULL) {
    fprintf(stderr,"out of memory\n");
    goto done;
  }

  /* seed the single-byte sequences */
  for(j=0; j < 256; j++) {
    bytes_all[j] = j;
    add_seq(s, &bytes_all[j], 1);
  }

  rc = 0;

 done:
  return rc;
}

/* used instead of mlzw_init to read a saved dictionary */
int mlzw_load(symbol_stats *s, char *file) {
  int rc = -1;

  rc = 0;

 done:
  return rc;
}

/* while x is an index into s->seq_all, we cheat and encode it
 * as fewer bits. x really only indexes into s->seq_all up to
 * the current item count (d) of the dictionary hash table. 
 * in doing so this implementation uses variable-width indexes
 * into the dictionary. the encoder and decoder sync permits it.
 */
static unsigned char get_num_bits(symbol_stats *s, int post) {
  unsigned long d = HASH_COUNT(s->dict) + post;
  assert(d >= 256); /* one-byte seqs always in the dict */

  /* let b = log2(d) rounded up to a whole integer. this is
   * the number of bits needed to distinguish d items. */
  unsigned char b=0;
  while (((d-1) >> b) != 0) b++;

  return b;
}

/* this macro emits the index x encoded as b bits in e */
#define emit()                                                \
 do {                                                         \
   b = get_num_bits(s,0);                                     \
   if(0) fprintf(stderr,"emit index %lu (in %u bits)\n",x,b); \
   if (p + b > eop) goto done;                                \
   while (b--) {                                              \
     if ((x >> b) & 1) BIT_SET(o,p);                          \
     p++;                                                     \
   }                                                          \
   s->seq_all[x].hits++;                                      \
 } while(0)

int mlzw_recode(int mode, symbol_stats *s, unsigned char *ib, size_t ilen, 
                unsigned char *ob, size_t *olen) {
  unsigned char b, *i=ib, *o=ob;
  unsigned long x=0;
  int rc = -1;
  size_t l;
  size_t p=0;
  size_t eop = (*olen)*8;

  /* special usage when output buffer is null; caller wants to know
   * how big to make the output buffer. on encoding this is unknown
   * until encoding finishes; we actually specify it as 2x _larger_
   * then the input buffer in case "compression" backfires, as it 
   * does on truly random data. for decoding, we know the olen. */
  if (ob == NULL) {
    if (mode & MODE_ENCODE) *olen = ilen * 2;
    if (mode & MODE_DECODE) {
      if (ilen < sizeof(olen)) *olen = 0;
      else memcpy(olen, ib, sizeof(*olen));
    }
    return 0;
  }

  if ((mode & MODE_ENCODE)) {

    /* store length of decoded buffer */
    if (o + sizeof(ilen) > ib + ilen) goto done;
    memcpy(o, &ilen, sizeof(ilen));
    o += sizeof(ilen);

    /* LZW encode input buffer */
    l = 1;
    while(1) {
      /* sequence starts at i for length l */

      /* would sequence extend over buffer end? */
      if (i+l > ib+ilen) { if (l > 1) emit(); break; }

      /* is this sequence in the dictionary? */
      if (have_seq(s, i, l, &x)) { l++; continue; }

      /* the sequence is not in the dictionary */
      emit();            /* emit previous */
      add_seq(s, i, l);
      i += l-1;          /* start new seq */
      l = 1;
    }

    /* indicate final compresed length */
    *olen = (o + p/8 + ((p%8) ? 1 : 0)) - ob;
  }

  if ((mode & MODE_DECODE)) {

    /* skip length */
    i += sizeof(*olen);

    unsigned long _x=0; /* index (x) of the previous iteration */
    int first_time=1;   /* first time decoding a symbol */
    int bump;           /* 1 when encoder dictionary is ours+1 */

    while (o - ob < *olen) {

      /* x is the index number we're gathering from the encoded buffer.
         first we need to figure out how many bits wide x is, because
         this implementation uses a variable-width encoding. then we 
         shift those 'b' bits into x. */
      x = 0;
      bump = first_time ? 0 : ((HASH_COUNT(s->dict) == s->max_dict_entries) ? 0 : 1);
      b = get_num_bits(s, bump);
      if ((i + b/8 + ((b%8) ? 1 : 0)) > ib + ilen) goto done;
      while(b--) {
        if (BIT_TEST(i,p)) x |= (1U << b);
        p++;
      }
      //fprintf(stderr,"got index %lu in %u bits\n",x,get_num_bits(s,bump));

      if (x > HASH_COUNT(s->dict)) goto done;

      /* special case KwKwK (see LZW article); code refers to just-
      *  created code that the compressor immediately used to encode.
      *  it's not in our dictionary yet, but we know what it must be.
       * re-emit previous code suffixed with its first character. */
      if ((x == HASH_COUNT(s->dict)) && !first_time) { 
        if (o + s->seq_all[_x].l + 1 > ob + *olen) goto done;
        memcpy(o, s->seq_all[_x].s, s->seq_all[_x].l);
        o[s->seq_all[_x].l] = s->seq_all[_x].s[0];
        add_seq(s, o, s->seq_all[_x].l + 1);
        o += s->seq_all[_x].l + 1;
        _x = x;
        continue;
      }

      /* normal case. output the bytes of sequence x */
      if (o + s->seq_all[x].l > ob + *olen) goto done;
      if (s->seq_all[x].l == 0) goto done;
      memcpy(o, s->seq_all[x].s, s->seq_all[x].l);
      o += s->seq_all[x].l;
      s->seq_all[x].hits++;

      /* add to dict previous seq + extension */
      if (first_time) first_time=0;
      else {
        l = s->seq_all[_x].l + s->seq_all[x].l;
        add_seq(s, o - l, s->seq_all[_x].l + 1);
      }
      _x = x;
    }
  }

  rc = 0;

 done:
  return rc;
}

void mlzw_release(symbol_stats *s) {
  HASH_CLEAR(hh, s->dict);
  if (s->seq_all) free(s->seq_all);
}

int mlzw_save_codebook(char *file, symbol_stats *s) {
  assert(0); /* TODO */
  return 0;
}
