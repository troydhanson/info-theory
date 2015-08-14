#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utlist.h"
#include "code.h"

void dump_codes(symbol_stats *s) {
  unsigned char l;
  unsigned long code;
  struct sym *sym;
  size_t i;

  for(i=0; i < 256; i++) {
    sym = &s->sym_all[i];
    code = sym->code;
    l = sym->code_length;
    if (l == 0) continue;

    fprintf(stderr,"0x%02x ", (int)sym->leaf_value);
    while(l--) fprintf(stderr, "%c", ((code >> l) & 1) ? '1' : '0');
    fprintf(stderr,"\n");
  }
}

/* assign a Huffman code by recursing depth-first. */
void assign_recursive(symbol_stats *s, struct sym *root, 
                      unsigned long code, unsigned char code_length) {

  assert(code_length <= sizeof(code) * 8); // code length (in bits)

  root->code = code;
  root->code_length = code_length;

  if (root->is_leaf) return;

  assign_recursive(s, root->n.a, (code << 1U) | 0x0, code_length + 1);
  assign_recursive(s, root->n.b, (code << 1U) | 0x1, code_length + 1);
}

/* sort the code table short-to-long before decoding */
static int decoder_sort(const void *_a, const void *_b) {
  struct sym *a = (struct sym*)_a, *b = (struct sym*)_b;
  if (a->code_length < b->code_length) return -1;
  if (a->code_length > b->code_length) return  1;
  return 0;
}

/* this sorts the least frequent symbols to the front of the list */
static int frequency_sort(struct sym *a, struct sym *b) {
  if (a->count < b->count) return -1;
  if (a->count > b->count) return  1;
  /* for equal frequency symbols we sort the leaf symbols 
     to the front of the list. this shortens the average 
     code lengths by creating less deep trees */
  if  (a->is_leaf && !b->is_leaf) return -1;
  if (!a->is_leaf &&  b->is_leaf) return  1;
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
static void form_codes(symbol_stats *s) {
  struct sym *tmp, *a, *b, *c;
  size_t num_nodes;

  do {
    LL_SORT(s->syms, frequency_sort);
    a = s->syms;
    b = s->syms->next;
    LL_DELETE(s->syms, a);
    LL_DELETE(s->syms, b);
    assert(s->sym_take < adim(s->sym_all));
    c = &s->sym_all[ s->sym_take++ ];
    c->count = a->count + b->count;
    c->n.a = a;
    c->n.b = b;
    LL_PREPEND(s->syms, c);
    LL_COUNT(s->syms, tmp, num_nodes);
  } while(num_nodes > 2);

  a = s->syms;
  b = s->syms->next;
  assign_recursive(s,a,1,1);
  assign_recursive(s,b,0,1);
}

static void form_header(symbol_stats *s) {
  unsigned char l, *c, *nsyms_mo;
  unsigned long code, nsyms=0;
  struct sym *sym;
  size_t i,j,h;

  /* number of symbols coming next */
  nsyms_mo = &s->header[s->header_len++];

  /* for each symbol with count > 0, sym|l|code */
  for(i=0; i < 256; i++) {
    sym = &s->sym_all[i];
    code = sym->code;
    l = sym->code_length;
    if (l == 0) continue;
    nsyms++;

    h = s->header_len;
    s->header_len += 1 + 1 + l/8 + ((l%8) ? 1 : 0);
    assert(s->header_len <= sizeof(s->header));
    s->header[h++] = sym->leaf_value;
    s->header[h++] = sym->code_length;

    c = &s->header[h];
    j = 0;

    while(l--) {
      if ((code >> l) & 1) BIT_SET(c,j); 
      j++;
    }
  }
  /* we record the number of symbols nsyms as its value minus one,
   * so that nsyms=256 can still be represented in a byte (as 255).
   * nsyms=0 is an unsupported edge case implying zero length input */
  assert(nsyms > 0);
  *nsyms_mo = nsyms-1;
}

/******************************************************************************
 * externally saved codebooks can be used when the probabilities of the data
 * remain fixed (e.g. a stable transmitter and receiver system that always 
 * deal in the same kind of data). In this case we can compute the codebook
 * once, save it, and then use it permanently for the particular application.
 *****************************************************************************/
int huf_load_codebook(char *file, symbol_stats *s) {
  int j, fd, rc = -1;
  unsigned char l;
  unsigned long c;

  fd = open(file, O_RDONLY);
  if (fd == -1) { 
    fprintf(stderr,"open %s: %s\n", file, strerror(errno)); 
    goto done;
  }

  for(j=0; j < 256; j++) {
    if (read(fd, &l, sizeof(l)) != sizeof(l)) goto done;
    if (read(fd, &c, sizeof(c)) != sizeof(c)) goto done;
    s->sym_all[j].code_length = l;
    s->sym_all[j].code = c;
  }

  rc = 0;

 done:
  if (fd != -1) close(fd);
  return rc;
}

int huf_save_codebook(char *file, symbol_stats *s) {
  int j, fd, rc = -1;
  unsigned char l;
  unsigned long c;

  fd = open(file, O_WRONLY|O_TRUNC|O_CREAT, 0664);
  if (fd == -1) { 
    fprintf(stderr,"open %s: %s\n", file, strerror(errno)); 
    goto done;
  }

  for(j=0; j < 256; j++) {
    l = s->sym_all[j].code_length;
    c = s->sym_all[j].code;
    if (write(fd, &l, sizeof(l)) != sizeof(l)) goto done;
    if (write(fd, &c, sizeof(c)) != sizeof(c)) goto done;
  }

  rc = 0;

 done:
  if (fd != -1) close(fd);
  return rc;
}

/* given a buffer this computes the output length needed to store
 * the result of encoding or decoding (specified by the mode). */
size_t huf_compute_olen( int mode, unsigned char *ib, size_t ilen, 
     size_t *ibits, size_t *obits, symbol_stats *s) {
  struct sym *sym, *tmp;
  size_t j, olen = 0;

  if ((mode & MODE_ENCODE) && !(mode & MODE_USE_SAVED_CODES)) {
    memset(s, 0, sizeof(*s));
    *ibits = ilen * 8;
    *obits = 0;

    /* take the first 256 syms for the leaf nodes (bytes) */
    s->sym_take = 256;
    s->syms = s->sym_all;
    for(j=0; j < 256; j++) {
      s->sym_all[j].is_leaf = 1;
      s->sym_all[j].leaf_value = j;
      s->sym_all[j].next = (j < 255) ? &s->sym_all[j+1] : NULL;
    }

    /* count the frequency of each byte in the input */
    for(j=0; j < ilen; j++) s->sym_all[ ib[j] ].count++;

    /* remove symbols with zero counts from the code */
    LL_FOREACH_SAFE(s->syms, sym, tmp) {
      if (sym->count == 0) LL_DELETE(s->syms,sym);
    }

    form_codes(s);
    form_header(s);
    if (mode & MODE_DISPLAY_CODES) dump_codes(s);

    for(j=0; j < ilen; j++) *obits += s->sym_all[ ib[j] ].code_length;
    *obits += sizeof(olen)*8;
    if (!(mode & MODE_SAVE_CODES)) *obits += s->header_len*8;
  }

  /* in this mode sym_all[] has been pre-populated */
  if ((mode & MODE_ENCODE) && (mode & MODE_USE_SAVED_CODES)) {
    for(j=0; j < ilen; j++) *obits += s->sym_all[ ib[j] ].code_length;
    *obits += sizeof(olen)*8;
  }

  if ((mode & MODE_DECODE)) {
    /* read decoded buffer length */
    if (ilen >= sizeof(olen)) memcpy(&olen, ib, sizeof(olen));
    *obits = olen * 8;
    *ibits = ilen * 8;
  }

  olen = (*obits/8) + ((*obits % 8) ? 1 : 0);

  return olen;
}

/* given a potential code of "len" bits, check whether it is a 
 * code; if so, store the byte it decodes to in decode and return 1.
 */
int is_code(unsigned long code, size_t len, unsigned char *decode, symbol_stats *s, int *i) {
  for(; *i < 256; (*i)++) {
    if (s->sym_all[*i].code_length > len) goto done;
    if (s->sym_all[*i].code_length < len) continue;
    if (s->sym_all[*i].code != code) continue;
    *decode = s->sym_all[*i].leaf_value;
    return 1;
  }
 done:
  return 0;
}

/* 
 * the core work is done here, either encoding or decoding
 */ 
int huf_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob, symbol_stats *s) {
  unsigned char *o=ob, *i=ib, *eib = ib+ilen, nsyms_mo, b, l, *w, lbytes, d;
  size_t j,p,olen;
  unsigned long c;
  struct sym *sym;
  int a, rc=-1;

  if ((mode & MODE_ENCODE)) {

    memcpy(o, &ilen, sizeof(ilen));
    o += sizeof(ilen);

    if (!(mode & (MODE_USE_SAVED_CODES | MODE_SAVE_CODES))) {
      memcpy(o, s->header, s->header_len);
      o += s->header_len;
    }

    p = 0;
    for(j=0; j < ilen; j++) {
      sym = &s->sym_all[ ib[j] ];
      c = sym->code;
      l = sym->code_length;
      while(l--) {
        if ((c >> l) & 1) BIT_SET(o,p);
        p++;
      }
    }
  }

  if ((mode & MODE_DECODE)) {

    /* initialize the leaf values we sort later below */
    for(j=0; j < 256; j++) s->sym_all[j].leaf_value = j;

    /* get size of decoded buffer */
    if (i + sizeof(olen) > eib) goto done;
    memcpy(&olen, i, sizeof(olen)); i += sizeof(olen);

    if (!(mode & MODE_USE_SAVED_CODES)) {
      /* from header get number of symbol codes */
      if (i + sizeof(nsyms_mo) > eib) goto done;
      nsyms_mo = *i; i++;

      /* from header get symbol codes */
      for(j=0; j < nsyms_mo+1; j++) {
        if (i + 2*sizeof(char) > eib) goto done;
        b = *i; i++;  /* symbol (byte value) */
        l = *i; i++;  /* bit length of its code */
        w = i;        /* beginning of bit code */

        /* read the bits of the code msb to lsb */
        s->sym_all[b].code_length = l;
        lbytes = l/8 + ((l%8) ? 1 : 0);
        if (i + lbytes > eib) goto done;
        p = 0;
        c = 0;
        while(l--) {
          if (BIT_TEST(w,p)) c |= (1U << l);
          p++;
        }
        s->sym_all[b].code = c;
        i += lbytes;
      }
    }

    /* sort the code table short-to-long for faster lookups */
    qsort(s->sym_all, 256, sizeof(*s->sym_all), decoder_sort);

    /* read code bits (msb to lsb) until a code is recognized, write byte */
    p = 0; /* bit position in encoded data */
    l = 0; /* length of code word */
    c = 0; /* code word */
    a = 0; /* code book lookup starts here */
    while((o - ob) < olen) {
      if ((i + p/8) >= eib) goto done;
      l++;
      c |= BIT_TEST(i,p);
      if (is_code(c,l,&d,s,&a)) { *o = d; o++; c = 0; l = 0; a = 0;} /* got one */
      else if (a < 256) { c = (c << 1U); } /* add another bit to potential code */
      else goto done;                      /* invalid code word */
      p++;
    }
  }

  rc = 0;

 done:
  return rc;
}

