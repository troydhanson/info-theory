#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

/* standard bit vector macros */
#define BIT_TEST(c,i)  ((c[(i)/8] &   (1 << ((i) % 8))) ? 1 : 0)
#define BIT_SET(c,i)    (c[(i)/8] |=  (1 << ((i) % 8)))
#define BIT_CLEAR(c,i)  (c[(i)/8] &= ~(1 << ((i) % 8)))

/* 
 *  This program takes two prevously saved bitvectors (of equal size)
 *  and reports on their similarity 
 */

struct {
  char *prog;
  int verbose;
  char *file_a;
  char *file_b;
  unsigned char *buf_a;
  unsigned char *buf_b;
  size_t len_a;
  size_t len_b;
  int fd_a;
  int fd_b;
} CF = {
  .fd_a = -1,
  .fd_b = -1,
};


void usage() {
  fprintf(stderr,"usage: %s [-v] <bloom1.dat> <bloom2.dat>\n", CF.prog);
  exit(-1);
}

void report_similarity() {
  if (CF.len_a != CF.len_b) {
    printf("binary files differ in size\n");
    return;
  }
  size_t n = CF.len_a * 8, a, b;
  size_t i=0, u=0, axb=0, bxa=0;
  while (n--) {
    a = BIT_TEST(CF.buf_a,n);
    b = BIT_TEST(CF.buf_b,n);
    i += (a && b) ? 1 : 0;
    u += (a || b) ? 1 : 0;
    axb += (a && !b);
    bxa += (!a && b);
  }
  printf("bits: %lu\n", (unsigned long)CF.len_a);
  printf("intersection (i) of set bits: %lu bits\n", (unsigned long)i);
  printf("union (u) of set bits: %lu bits\n", (unsigned long)u);
  printf("jaccard similiarty coefficient (i/u): %.2f\n", u ? (i*1.0/u) : 0.0);
  printf("a not b: %lu bits\n", (unsigned long)axb);
  printf("b not a: %lu bits\n", (unsigned long)bxa);
}

int mmap_input(char *file, unsigned char **buf, size_t *len, int *fd) {
  struct stat s;
  int rc=-1;

  if ( (*fd = open(file, O_RDONLY)) == -1) {
    fprintf(stderr,"can't open %s: %s\n", file, strerror(errno));
    goto done;
  }

  if (fstat(*fd, &s) == -1) {
    fprintf(stderr,"can't stat %s: %s\n", file, strerror(errno));
    goto done;
  }

  *len = s.st_size;
  *buf = mmap(0, *len, PROT_READ, MAP_PRIVATE, *fd, 0);
  if (*buf == MAP_FAILED) {
    fprintf(stderr, "failed to mmap %s: %s\n", file, strerror(errno));
    goto done;
  }

  rc = 0;

 done:
  return rc;
}

int main(int argc, char *argv[]) {
  int opt, rc=-1;
  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vh")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'h': default: usage(); break;
    }
  }

  if (optind < argc) CF.file_a = argv[optind++];
  if (optind < argc) CF.file_b = argv[optind++];
  if ((!CF.file_a) || (!CF.file_b)) usage();

  if (mmap_input(CF.file_a, &CF.buf_a, &CF.len_a, &CF.fd_a) < 0) goto done;
  if (mmap_input(CF.file_b, &CF.buf_b, &CF.len_b, &CF.fd_b) < 0) goto done;

  report_similarity();
  rc = 0;

 done:
  if (CF.buf_a) munmap(CF.buf_a, CF.len_a);
  if (CF.buf_b) munmap(CF.buf_b, CF.len_b);
  if (CF.fd_a != -1) close(CF.fd_a);
  if (CF.fd_b != -1) close(CF.fd_b);
  return rc;
}

