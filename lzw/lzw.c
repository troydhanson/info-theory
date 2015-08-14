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

  char *codefile;

  char *ifile;
  unsigned char *ibuf;
  size_t ilen, ibits;

  char *ofile;
  unsigned char *obuf;
  size_t olen, obits;

  symbol_stats s;

} CF = {
  .s.max_seq_length = 24,
  .s.max_dict_entries = 1000000,
};


void usage() {
  fprintf(stderr,"usage: %s [-vlcCsD] -e|d -i <file> -o <file>\n", CF.prog);
  fprintf(stderr,"          -e (encode)\n");
  fprintf(stderr,"          -d (decode)\n");
  fprintf(stderr,"          -i (input file)\n");
  fprintf(stderr,"          -o (output file)\n");
  fprintf(stderr,"          -v (verbose)\n");
  fprintf(stderr,"          -s [number] (max sequence length) [default:24]\n");
  fprintf(stderr,"          -D [number] (max dictionary entries) [default:1M] 0=unlimited\n");
  fprintf(stderr,"          -l (display codes) [encode mode]\n");
  fprintf(stderr,"          -c [file] (load codebook) [encode or decode mode]\n");
  fprintf(stderr,"          -C [file] (save codebook) [encode mode]\n");
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
  int opt, rc=-1, hc;
  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vedi:o:c:C:ls:D:h")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'e': CF.mode |= MODE_ENCODE; break;
      case 'd': CF.mode |= MODE_DECODE; break;
      case 'i': CF.ifile = strdup(optarg); break;
      case 'o': CF.ofile = strdup(optarg); break;
      case 'c': CF.codefile = strdup(optarg); CF.mode |= MODE_USE_SAVED_CODES;  break;
      case 'C': CF.codefile = strdup(optarg); CF.mode |= MODE_SAVE_CODES; break;
      case 'l': CF.mode |= MODE_DISPLAY_CODES; break;
      case 's': CF.s.max_seq_length = atoi(optarg); break;
      case 'D': CF.s.max_dict_entries = atoi(optarg); break;
      case 'h': default: usage(); break;
    }
  }

  if ((!CF.ifile) || (!CF.ofile)) usage();
  if ((CF.mode & (MODE_ENCODE | MODE_DECODE)) == 0) usage();
  if ((CF.mode & MODE_ENCODE) && (CF.mode & MODE_DECODE)) usage();
  if (mmap_input() < 0) goto done;

  if (CF.mode & MODE_USE_SAVED_CODES) {
    hc = lzw_load_codebook(CF.codefile, &CF.s);
    if (hc < 0) goto done; 
  }

  if (CF.mode & MODE_ENCODE) CF.olen = CF.ilen; // TODO truncate to actual later
  if (CF.mode & MODE_DECODE) CF.olen = lzw_compute_olen(CF.mode, CF.ibuf, CF.ilen,
                                                     &CF.ibits, &CF.obits, &CF.s);
  if (mmap_output() < 0) goto done;

  if (CF.mode & MODE_SAVE_CODES) {
    hc = lzw_save_codebook(CF.codefile, &CF.s);
    if (hc < 0) goto done; 
  }

  rc = lzw_recode(CF.mode, CF.ibuf, CF.ilen, CF.obuf, CF.olen, &CF.s);
  if (rc) fprintf(stderr,"lzw_recode error\n");

 done:
  if (CF.ibuf) munmap(CF.ibuf, CF.ilen);
  if (CF.obuf) munmap(CF.obuf, CF.olen);
  if (CF.s.seq_all) free(CF.s.seq_all);
  free(CF.ifile);
  free(CF.ofile);
  return rc;
}

