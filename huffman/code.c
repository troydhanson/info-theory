#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utlist.h"
#include "code.h"

/* this sorts the least frequent symbols to the front of the list */
static int reverse_frequency_sort(struct sym *a, struct sym *b) {
  if (a->count < b->count) return  1;
  if (a->count > b->count) return -1;
  return 0;
}

/* the heart of Huffman coding is assigning the codes. this is done
 * by starting with the initial symbol counts, taking the least common
 * two and combining them into a new item (which replaces the two).
 * the replacement keeps pointers to the replaced ones in the new item.
 * then process is repeated until only two items remain in the list.
 * these are given the bits 0 and 1 arbitrarily. their pointers are
 * followed and assigned bits in the same way, recursing.  An item's 
 * code is all its ancestor bitcodes prepended to its own.
 */ 
static int form_codes(symbol_stats *s) {
  struct sym *tmp, *a, *b, *c;
  size_t num_nodes;
  int rc=-1;

  LL_COUNT(s->syms, tmp, num_nodes);
  assert(num_nodes > 2);

  do {
    LL_SORT(s->syms, reverse_frequency_sort);
    a = &s->syms[0];
    b = &s->syms[1];
    LL_DELETE(s->syms, a);
    LL_DELETE(s->syms, b);
    c = calloc(1, sizeof(struct sym));
    if (c == NULL) { fprintf(stderr,"oom\n"); goto done; }
    c->count = a->count + b->count;
    c->n.a = a;
    c->n.b = b;
    LL_PREPEND(s->syms, c);
    LL_COUNT(s->syms, tmp, num_nodes);
  } while(num_nodes > 2);

  rc = 0;

 done:
  return rc;
}

/* call before encoding or decoding to determine the necessary
 * output buffer size to perform the (de-)encoding operation. */
size_t huf_compute_olen( int mode, size_t ilen, size_t *ibits, size_t *obits, symbol_stats *s) {
  size_t i, olen = 0;

  if ((mode & MODE_ENCODE)) {
    memset(s, 0, sizeof(*s));

    /* prepare symbol counters */
    s->syms = calloc(256, sizeof(struct sym)); // TODO release at end of program
    if (s->syms == NULL) { fprintf(stderr,"oom\n"); goto done; }
    for(i=0; i < 256; i++) {
      s->syms[i].is_leaf = 1;
      s->syms[i].leaf_value = i;
      s->syms[i].next = (i < 255) ? &s->syms[i+1] : NULL;
    }
    /* count byte frequencies */
    for(i=0; i < ilen; i++) {
      s->syms[ ibits[i] ].count++;
    }

    if (form_codes(s) < 0) goto done;

    *ibits = ilen * 8;
    *obits = ilen * 14;
  }

  if ((mode & MODE_DECODE)) {
      *ibits = (ilen*8) - ((ilen*8) % 7);
      *obits = (*ibits/7) * 4;
  }

  olen = (*obits/8) + ((*obits % 8) ? 1 : 0);

 done:
  return olen;
}

/* 
 * 
 */ 
int huf_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, symbol_stats *s) {
  int rc=-1;

  if ((mode & MODE_ENCODE)) {
  }

  if ((mode & MODE_DECODE)) {
  }

  rc = 0;

  return rc;
}

