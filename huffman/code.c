#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utlist.h"
#include "code.h"

static void dump_codes(symbol_stats *s) {
  unsigned char l;
  unsigned long code;
  size_t i;

  for(i=0; i < 256; i++) {
    l = s->syms[i].code_length;
    code = s->syms[i].code;

    fprintf(stderr,"0x%02x ", (int)i);
    while(l--) fprintf(stderr, "%c", ((code >> l) & 1) ? '1' : '0');
    fprintf(stderr,"\n");
  }
}

/* assign a Huffman code by recursing depth-first. a regular iterative
 * routine is preferable; at least the recursion is bounded by assert. */
void assign_recursive(symbol_stats *s, struct sym *root, 
                      unsigned long code, unsigned char code_length) {

  assert(code_length <= sizeof(code) * 8); // code length (in bits)

  root->code = code;
  root->code_length = code_length;

  if (root->is_leaf) return;

  assign_recursive(s, root->n.a, (code << 1U) & 0x0, code_length + 1);
  assign_recursive(s, root->n.b, (code << 1U) & 0x1, code_length + 1);
  free(root);
}

/* this sorts the least frequent symbols to the front of the list */
static int frequency_sort(struct sym *a, struct sym *b) {
  if (a->count < b->count) return -1;
  if (a->count > b->count) return  1;
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
  struct sym *tmp, *a, *b, *c, *first_leaf;
  size_t num_nodes;
  int rc=-1;
  first_leaf = s->syms; 

  do {
    LL_SORT(s->syms, frequency_sort);
    a = s->syms;
    b = s->syms->next;
    LL_DELETE(s->syms, a);
    LL_DELETE(s->syms, b);
    c = calloc(1, sizeof(struct sym));
    if (c == NULL) { fprintf(stderr,"oom\n"); goto done; }
    c->count = a->count + b->count;
    c->n.a = a;
    c->n.b = b;
    LL_PREPEND(s->syms, c);
    LL_COUNT(s->syms, tmp, num_nodes);
#if 0
    fprintf(stderr,"iteration %d\n", iteration++);
    LL_FOREACH(s->syms, tmp) {
      fprintf(stderr, "  is_leaf %d (%u) count: %lu\n", tmp->is_leaf, tmp->is_leaf ? tmp->leaf_value : 0, tmp->count);
    }
#endif
  } while(num_nodes > 2);

  assert(num_nodes == 2);
  a = s->syms;
  b = s->syms->next;
  s->syms = first_leaf;
  assign_recursive(s,a,1,1);
  assign_recursive(s,b,0,1);
  rc = 0;

 done:
  return rc;
}

/* header is all code lengths (256 chars) then codes (256 ints). TODO
 * we'd use a tighter header but it'd just complicate this example */
#define header_len (    sizeof(size_t) +         \
                    256*sizeof(unsigned char) +  \
                    256*sizeof(unsigned long))

/* call before encoding or decoding to determine the necessary
 * output buffer size to perform the (de-)encoding operation. */
size_t huf_compute_olen( int mode, unsigned char *ib, size_t ilen, 
     size_t *ibits, size_t *obits, symbol_stats *s, int verbose) {
  size_t i, olen = 0;

  if ((mode & MODE_ENCODE)) {
    memset(s, 0, sizeof(*s));
    *ibits = ilen * 8;
    *obits = 0;

    s->syms = calloc(256, sizeof(struct sym)); // TODO release at end of program
    if (s->syms == NULL) { fprintf(stderr,"oom\n"); goto done; }
    for(i=0; i < 256; i++) {
      s->syms[i].is_leaf = 1;
      s->syms[i].leaf_value = i;
      s->syms[i].next = (i < 255) ? &s->syms[i+1] : NULL;
    }
    for(i=0; i < ilen; i++) s->syms[ ib[i] ].count++;
    if (form_codes(s) < 0) goto done;
    if (verbose) dump_codes(s);
    for(i=0; i < ilen; i++) *obits += s->syms[ ib[i] ].code_length;
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

