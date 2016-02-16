#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "code.h"

#if 0
void print_elapsed(struct timeval *tva, struct timeval *tvb) {
  unsigned long usec_a = tva->tv_sec * 1000000 + tva->tv_usec;
  unsigned long usec_b = tvb->tv_sec * 1000000 + tvb->tv_usec;
  unsigned diff = usec_b - usec_a;
  unsigned sec = diff/1000000;
  unsigned usec = diff % 1000000;
  fprintf(stderr,"elapsed %u sec %u usec\n", sec ,usec);
}
#endif

static int have_seq(lzw *s, unsigned char *seq, size_t len, unsigned long *x) {
  struct seq *q,*p;

  if (len == 1) {
    q = &s->seq_all[ *seq ];
  } else {
    /* extension mode: suffix lookup from known precursor seq having index x */
    assert(*x < s->seq_used);
    assert(len > 1);
    p = &s->seq_all[ *x ];
    q = p->n[ seq[len-1] ];
    //fprintf(stderr,"lookup %.*s from %.*s\n", (int)len, seq, (int)p->l, p->s);
  }

  if (q == NULL) return 0;
  assert(q >= s->seq_all);
  *x = q - s->seq_all;
  return 1;
}

/* the memory pointed to by seq must be static through the 
 * lifetime of encoding or decoding the buffer */
static void add_seq(lzw *s, unsigned char *seq, size_t len, unsigned long x) {
  struct seq *q;

  /* our dictionary has a hard limit- no recycling */
  if (s->seq_used == s->max_dict_entries) return;

  q = &s->seq_all[ s->seq_used++ ];
  q->l = len;
  q->s = seq;
  q->x = x;
  //fprintf(stderr,"add [%.*s]<len %u> @ index %lu\n", (int)len, seq, (int)len, q-s->seq_all);

  /* install a pointer in precursor p to this seq */
  if (len == 1) return;
  s->seq_all[x].n[ seq[len-1] ] = q;
}

static unsigned char bytes_all[256];
int mlzw_init(lzw *s) {
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
    add_seq(s, &bytes_all[j], 1, 0);
  }

  rc = 0;

 done:
  return rc;
}

/* the number of bits to encode something depends on the 
 * number of entries in the dictionary. when using a pre-
 * loaded (static) dictionary, this becomes a fixed number. */
static unsigned char get_num_bits(lzw *s, int bump) {
  unsigned long d = s->seq_used + bump;
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
 } while(0)

int mlzw_recode(int mode, lzw *s, unsigned char *ib, size_t ilen, 
                unsigned char *ob, size_t *olen) {
  //struct timeval tva,tvb;
  //gettimeofday(&tva,NULL);
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
      add_seq(s, i, l, x);
      i += l-1;          /* start new seq */
      l = 1;
    }

    /* indicate final compresed length */
    *olen = (o + p/8 + ((p%8) ? 1 : 0)) - ob;
  }

  /* minimal decoder for use with preloaded dictionary. */
  if ((mode & MODE_DECODE)) {
    unsigned char w;

    i += sizeof(*olen);        /* skip length */
    w = get_num_bits(s, 0);    /* fixed width when dict preloaded */

    while (o - ob < *olen) {
      x = 0;
      //assert ((i + b/8 + ((b%8) ? 1 : 0)) <= ib + ilen);
      if ((i + b/8 + ((b%8) ? 1 : 0)) > ib + ilen) goto done;
      b = w;
      while(b--) {
        if (BIT_TEST(i,p)) x |= (1U << b);
        p++;
      }

      //assert (x < s->seq_used);
      //assert (o + s->seq_all[x].l <= ob + *olen);
      //assert (s->seq_all[x].l > 0);
      if (x >= s->seq_used) goto done;
      if (o + s->seq_all[x].l > ob + *olen) goto done;
      if (s->seq_all[x].l == 0) goto done;
      memcpy(o, s->seq_all[x].s, s->seq_all[x].l);
      o += s->seq_all[x].l;
    }
  }

  rc = 0;

 done:
  //gettimeofday(&tvb,NULL);
  //print_elapsed(&tva, &tvb);
  return rc;
}

void mlzw_release(lzw *s) {
  if (s->map.addr) {
    munmap(s->map.addr, s->map.len);
    assert(s->map.fd >= 0);
    close(s->map.fd);
  }
  if (s->seq_all) free(s->seq_all);
}

#define _read(pos,dst)                                              \
 do {                                                               \
   if ((pos + sizeof(dst)) > (s->map.addr + s->map.len)) {          \
     fprintf(stderr,"error: corrupted dictionary\n");               \
     goto done;                                                     \
   }                                                                \
   memcpy(&dst, pos, sizeof(dst));                                  \
   pos += sizeof(dst);                                              \
 } while(0) 

/* used instead of mlzw_init to read a saved dictionary */
int mlzw_load(lzw *s, char *file) {
  //struct timeval tva,tvb;
  //gettimeofday(&tva,NULL);
  int rc = -1;
  unsigned char *p;
  struct stat stat;
  size_t i,l,m;
  unsigned long x;

  s->map.fd = open(file, O_RDONLY);
  if (s->map.fd == -1) {
    fprintf(stderr, "open %s: %s\n", file, strerror(errno));
    goto done;
  }

  if (fstat(s->map.fd, &stat) < 0) {
    fprintf(stderr, "stat %s: %s\n", file, strerror(errno));
    goto done;
  }

  s->map.len = stat.st_size;
  s->map.addr = mmap(0, s->map.len, PROT_READ, MAP_PRIVATE, s->map.fd, 0);
  if (s->map.addr == MAP_FAILED) {
    fprintf(stderr, "mmap %s: %s\n", file, strerror(errno));
    goto done;
  }

  p = s->map.addr;
  _read(p, m);
  s->max_dict_entries = m;

  /* allocate the dictionary as one contiguous buffer */
  s->seq_all = calloc(s->max_dict_entries, sizeof(struct seq));
  if (s->seq_all == NULL) {
    fprintf(stderr,"out of memory\n");
    goto done;
  }

  /* add the sequences from the save file into dictionary */
  for(i=0; i < m; i++) {
    _read(p, l);       /* read sequence length */
    _read(p, x);       /* read index of precursor seq */
    if (p + l > s->map.addr + s->map.len) goto done;
    add_seq(s,p,l,x);  /* p points to sequence bytes */
    p += l;
  }

  rc = 0;

 done:
  if (rc < 0) {
    if (s->map.fd != -1) close(s->map.fd);
    if (s->map.addr && (s->map.addr != MAP_FAILED)) {
      munmap(s->map.addr, s->map.len);
      s->map.addr = NULL;
    }
  }
  //gettimeofday(&tvb,NULL);
  //print_elapsed(&tva, &tvb);
  return rc;
}

#define _write(f,b,l)                      \
do {                                       \
  int nr;                                  \
  if ( (nr = write(f,b,l)) != l) {         \
    fprintf(stderr,"write: %s\n", (nr<0) ? \
     strerror(errno) : "incomplete");      \
     goto done;                            \
  }                                        \
} while(0)

int mlzw_save_codebook(lzw *s, char *file) {
  int fd=-1, rc = -1;
  struct seq *q;
  size_t i;

  fd = open(file, O_WRONLY|O_TRUNC|O_CREAT, 0644);
  if (fd == -1) {
    fprintf(stderr, "open %s: %s\n", file, strerror(errno));
    goto done;
  }

  _write(fd, &s->seq_used, sizeof(s->seq_used));
  for(i=0; i < s->seq_used; i++) {
    q = &s->seq_all[i];
    _write(fd, &q->l, sizeof(q->l)); /* write length */
    _write(fd, &q->x, sizeof(q->x)); /* write precursor idx */
    _write(fd,  q->s,        q->l);  /* write seq    */
  }

  rc = 0;

 done:
  if (fd != -1) close(fd);
  return rc;
}

#define is_ascii(x) ((x >= 0x20) && (x <= 0x7e))
int mlzw_show_codebook(lzw *s) {
  int rc = -1, w, b;
  size_t i, j, l=0;
  unsigned char x;
  struct seq *q;

  for(i=0; i < s->seq_used; i++) l += s->seq_all[i].l;
  w = get_num_bits(s,0);

  fprintf(stderr,"LZW dictionary:\n");
  fprintf(stderr,"  sequences:  %10lu\n", s->seq_used);
  fprintf(stderr,"  seq bytes:  %10lu\n", l);
  fprintf(stderr,"  bit width:  %10u\n",  w);
 
  /* emit bit code, sequence length, and ascii in sequence */
  for(i=0; i < s->seq_used; i++) {
    q = &s->seq_all[i];
    b=w; while(b--) fprintf(stderr, "%c", ((i >> b) & 1) ? '1' : '0');
    fprintf(stderr, " %lu ", q->l);
    for(j=0; j < q->l; j++) {
      x = q->s[j];
      if (is_ascii(x)) fprintf(stderr, "%c", x);
      else             fprintf(stderr, "\\x%02x", (int)x);
    }
    fprintf(stderr, "\n");
  }

  rc = 0;
  return rc;
}
