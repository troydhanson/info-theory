#include <math.h>   /* log,ceil */
#include <stdlib.h> /* qsort_r */
#include <stdio.h>  /* fprintf */
#include <assert.h> 
#include <string.h> /* memcpy */
#include "sfcode.h"

/* see section 9 of Claude Shannon's 1948 paper 
 * A Mathematical Theory of Communication for the
 * example of Shannon Fano encoding used here */

/* 
 * TODO while this program compresses the data bytes, it 
 *      could store its code book (mapping of codes to
 *      bytes) concisely. Currently its a 1k+ raw table.
 */

static void count_symbols(symbol_stats *s, unsigned char *ib, size_t ilen) {
  size_t i;
  for(i=0; i < ilen; i++) s->count[ ib[i] ]++;
  s->nbytes = ilen;
}

/* Determine the code length in bits, for each byte value [0-255].
 * The Shannon paper describes m[i] as the integer that satisfies:
 *   log2( 1/p(i) ) <= m(i) < 1 + log2( 1/p(i) )
 * In other words- if the left side isn't an integer, round it up.
 */
static void find_code_lengths(symbol_stats *s) {
  int i;

  double lb = log(s->nbytes);

  for(i=0; i < 256; i++) {
    if (s->count[i] == 0) continue;

    double lc = log(s->count[i]);
    double dv = lb - lc;
    double m = dv/log(2);

    if (m == 0.0) m = 1.0;            /* only one symbol */
    s->code_length[i] = ceil(m);      /* "round up" */
    assert(s->code_length[i] <= sizeof(unsigned)*8);
  }
}

// hack; since qsort_r has a different prototype
// on Mac OS X compared to Linux, I switched from qsort_r
// to regular qsort (which is uniform), and put this global in.
symbol_stats *_global_s; 

static int sort_by_count_desc(const void *_a, const void *_b) {
  unsigned char a = *(unsigned char*)_a;
  unsigned char b = *(unsigned char*)_b;
  symbol_stats *s = _global_s;
  int rc;

  if (s->count[a] < s->count[b])  rc =  1;
  if (s->count[a] > s->count[b])  rc = -1;
  if (s->count[a] == s->count[b]) rc =  0;

  return rc;
}

/* the binary code for a symbol i is constructed as 
 * the binary expansion (bits after the binary point)
 * of the cumulative probability of the symbols more 
 * probable than itself. Shannon calls this P(i) (p.402)
 * taken only to the m(i) digit.
 */
static void generate_codes(symbol_stats *s) {
  unsigned int c;
  unsigned int i,j;

  /* sort to produce byte ranking */
  for(i=0; i < 256; i++) s->irank[i] = i; /* to be ranked */
  _global_s = s;
  qsort(s->irank,256,sizeof(*s->irank),sort_by_count_desc);
  for(i=0; i < 256; i++) s->rank[ s->irank[i] ] = i;

  /* get P(i) for each byte (well, just its numerator Pcount[i]) */
  for(i=0; i < 256; i++) {
    for(j=0; j < s->rank[i]; j++) { /* for ranks higher than i's */
      s->Pcount[i] += s->count[ s->irank[j] ];
    }
  }

  /* Ready to make the binary code: expand P(i) to m(i) places. 
   * Iterate over m(i) bits (from msb to lsb) in code[i].
   * Set each bit if code[i] remains <= P(i), else clear it.
   */
  for(i=0; i < 256; i++) {
    size_t Pn = s->Pcount[i] << s->code_length[i];
    size_t Pd = s->nbytes;

    j = s->code_length[i];
    while (j--) {
      c = s->code[i] | (1U << j);
      if (c <= (Pn/Pd)) s->code[i] = c;
    }
  }
}

static void dump_symbol_stats(symbol_stats *s) {
  unsigned i,j,b;
  fprintf(stderr,"byte c count rank code-len bitcode\n");
  fprintf(stderr,"---- - ----- ---- -------- ----------\n");
  for(i=0; i < 256; i++) {
    b = s->irank[i];
    if (s->count[b] == 0) continue;
    fprintf(stderr,"0x%02x %c %5ld %4d %8d ", b,
    (b>=' ' && b <= '~') ? b : ' ',
    (long)s->count[b],
    s->rank[b],
    s->code_length[b]);

    j = s->code_length[b];
    while (j--) {
      fprintf(stderr,"%c",(s->code[b] & (1U << j)) ? '1':'0');
    }
    fprintf(stderr,"\n");
  }
}

/* header is all code lengths (256 chars) then codes (256 ints).
 * we'd use a tighter header but it'd just complicate this example */
#define header_len (    sizeof(size_t) +         \
                    256*sizeof(unsigned char) +  \
                    256*sizeof(unsigned int))

/* call before encoding or decoding to determine the necessary
 * output buffer size to perform the (de-)encoding operation. */
size_t ecc_compute_olen( int mode, unsigned char *ib, size_t ilen, 
     size_t *ibits, size_t *obits, symbol_stats *s, int verbose) {
  size_t i,olen;

  if (mode == MODE_ENCODE) {
    *ibits = ilen * 8;
    *obits = 0;
    count_symbols(s, ib, ilen);
    find_code_lengths(s);
    generate_codes(s);
    if (verbose) dump_symbol_stats(s);
    for(i=0; i < ilen; i++) *obits += s->code_length[ ib[i] ];
    *obits += header_len*8;
  }

  if (mode == MODE_DECODE) {
    *ibits = ilen * 8;
    *obits = 0;
    /* get olen from header */
    assert(ilen >= sizeof(olen));
    memcpy(&olen, ib, sizeof(olen));
    *obits = olen*8;
  }

  olen = (*obits/8) + ((*obits % 8) ? 1 : 0);

  if (verbose>1) {
    fprintf(stderr,"recoding %lu data bytes into %lu data bytes\n", 
      ilen-((mode==MODE_ENCODE)?0:header_len), 
      olen-((mode==MODE_ENCODE)?header_len:0));
    fprintf(stderr,"using a code book of %lu bytes\n", header_len);
  }
  return olen;
}
/* given a potential code of "len" bits, check whether it is a 
 * code; if so, store the byte it decodes to in decode and return 1.
 * this function is a linear scan, it is not efficient. 
 */
int is_code(unsigned int code, size_t len, unsigned char *decode, 
          symbol_stats *s, int verbose) {
  size_t i;
  for(i=0; i < 256; i++) {
    if (s->code_length[i] != len) continue;
    if (s->code[i] != code) continue;
    *decode = (unsigned char)i;
    if (verbose>1) fprintf(stderr,"%d is a code of length %lu\n", code, len);
    return 1;
  }
  if (verbose>1) fprintf(stderr,"%d is NOT a code of length %lu\n", code, len);
  return 0;
}

/* 
 *
 * 
 */ 

int ecc_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, 
               symbol_stats *s, int verbose) {
  unsigned char *i = ib;
  unsigned char *o = ob;
  unsigned char d, n=0;
  unsigned int c=0;
  int rc=-1;
  size_t l=0,b=0,olen;

  if (mode == MODE_ENCODE) {
    /* dump header */
    l = sizeof(size_t);            memcpy(o, &ilen, l);          o += l;
    l = sizeof(unsigned char)*256; memcpy(o, s->code_length, l); o += l;
    l = sizeof(unsigned int)*256;  memcpy(o, s->code, l);        o += l;
    assert(header_len == (o-ob));
    /* dump encoding of input into codes. write the code bits msb to lsb. */
    while (i < ib+ilen) {
      c = s->code[ *i ];
      l = s->code_length[ *i ]; /* in bits */
      while(l--) {
        if (c & (1U << l)) BIT_SET(o,b);
        b++;
      }
      i++;
    }
  }

  if (mode == MODE_DECODE) {
    /* read header */
    l = sizeof(size_t);            memcpy(&olen, i, l);          i += l;
    l = sizeof(unsigned char)*256; memcpy(s->code_length, i, l); i += l;
    l = sizeof(unsigned int)*256;  memcpy(s->code, i, l);        i += l;
    /* read code bits (msb to lsb) until a code is recognized, write byte */
    while((o - ob) < olen) {
      if ((i + b/8) >= (ib + ilen)) goto done;
      n++; // length of code c
      c |= BIT_TEST(i,b);
      if (is_code(c,n,&d,s,verbose)) { *o = d; o++; c = 0; n = 0; } /* emit byte, reset */
      else { c = (c << 1U); };
      b++;
    }
  }

  rc = 0;

 done:
  return rc;
}

