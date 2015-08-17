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
#include "code.h"

/* 
 * LZW encoder/decoder
 */

struct {
  char *prog;
  int verbose;
  int mode;

  char *ifile;
  unsigned char *ibuf;
  size_t ilen;

  char *ofile;
  unsigned char *obuf;
  size_t olen;

  lzw s;

} CF = {
  .s.max_dict_entries = 1048576, /* powers of two make the most of index bits */
};


void usage() {
  fprintf(stderr,"usage: %s [-vD] -e|d -i <file> -o <file>\n", CF.prog);
  fprintf(stderr,"          -e (encode)\n");
  fprintf(stderr,"          -d (decode)\n");
  fprintf(stderr,"          -i (input file)\n");
  fprintf(stderr,"          -o (output file)\n");
  fprintf(stderr,"          -v (verbose)\n");
  fprintf(stderr,"          -D [number] (max dictionary entries) [default: 1048576]\n");
  exit(-1);
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

int mmap_output(void) {
  int fd, rc=-1;

  if ( (fd = open(CF.ofile, O_RDWR|O_CREAT|O_TRUNC,0644)) == -1) {
    fprintf(stderr,"can't open %s: %s\n", CF.ofile, strerror(errno));
    goto done;
  }

  if (ftruncate(fd, CF.olen) == -1) {
    fprintf(stderr,"ftruncate: %s\n", strerror(errno));
    goto done;
  }

  CF.obuf = mmap(0, CF.olen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if (CF.obuf == MAP_FAILED) {
    fprintf(stderr, "failed to mmap %s: %s\n", CF.ofile, strerror(errno));
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

  while ( (opt = getopt(argc,argv,"vedi:o:D:h")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'e': CF.mode |= MODE_ENCODE; break;
      case 'd': CF.mode |= MODE_DECODE; break;
      case 'i': CF.ifile = strdup(optarg); break;
      case 'o': CF.ofile = strdup(optarg); break;
      case 'D': CF.s.max_dict_entries = atoi(optarg); break;
      case 'h': default: usage(); break;
    }
  }

  if ((!CF.ifile) || (!CF.ofile)) usage();
  if ((CF.mode & (MODE_ENCODE | MODE_DECODE)) == 0) usage();
  if ((CF.mode & MODE_ENCODE) && (CF.mode & MODE_DECODE)) usage();
  if (CF.s.max_dict_entries < 512) usage();

  if (mmap_input() < 0) goto done;

  if (lzw_init(&CF.s) < 0) goto done;
  CF.olen = lzw_compute_olen(CF.mode, CF.ibuf, CF.ilen, &CF.s);
  if (mmap_output() < 0) goto done;

  rc = lzw_recode(CF.mode, CF.ibuf, CF.ilen, CF.obuf, &CF.olen, &CF.s);
  if (rc) { 
    fprintf(stderr,"lzw_recode error\n"); 
    goto done; 
  }

  if (truncate(CF.ofile, CF.olen) < 0) {
    fprintf(stderr,"truncate: %s\n", strerror(errno));
    goto done;
  }

 done:
  if (CF.ibuf) munmap(CF.ibuf, CF.ilen);
  if (CF.obuf) munmap(CF.obuf, CF.olen);
  lzw_release(&CF.s);
  free(CF.ifile);
  free(CF.ofile);
  return rc;
}

