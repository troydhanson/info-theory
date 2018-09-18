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
 *  This program treats its input file as a bit vector and then
 *  reports on its saturation (number of set bits, over total bits).
 */

struct {
  char *prog;
  int verbose;

  char *ifile;
  unsigned char *ibuf;
  size_t ilen;

} CF;


void usage() {
  fprintf(stderr,"usage: %s [-v] <file>\n", CF.prog);
  exit(-1);
}

void report_saturation(unsigned char *ib, size_t ilen) {
  size_t set=0, n = ilen * 8;
  while (n--) set += BIT_TEST(ib,n);
  fprintf(stderr,"%.2f\n", set/(ilen*8.0));
  if (CF.verbose) fprintf(stderr,"%lu / %lu\n", set, ilen*8);
}

int mmap_input(void) {
  struct stat s;
  int fd, rc=-1;

  if ( (fd = open(CF.ifile, O_RDONLY)) == -1) {
    fprintf(stderr,"can't open %s: %s\n", CF.ifile, strerror(errno));
    goto done;
  }

  if (fstat(fd, &s) == -1) {
    fprintf(stderr,"can't stat %s: %s\n", CF.ifile, strerror(errno));
    goto done;
  }

  CF.ilen = s.st_size;
  CF.ibuf = mmap(0, CF.ilen, PROT_READ, MAP_PRIVATE, fd, 0);
  if (CF.ibuf == MAP_FAILED) {
    fprintf(stderr, "failed to mmap %s: %s\n", CF.ifile, strerror(errno));
    goto done;
  }

  rc = 0;

 done:
  if (fd != -1) close(fd);
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

  if (optind < argc) CF.ifile = argv[optind++];
  if (!CF.ifile) usage();
  if (mmap_input() < 0) goto done;

  report_saturation(CF.ibuf, CF.ilen);
  rc = 0;

 done:
  if (CF.ibuf) munmap(CF.ibuf, CF.ilen);
  return rc;
}

