#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

void usage(char *exe) {
  fprintf(stderr,"usage: %s <file>\n", exe);
  exit(-1);
}

char *slurp(char *file, size_t *flen) {
  struct stat stats;
  char *out;
  int fd;

  if (stat(file, &stats) == -1) {
    fprintf(stderr, "can't stat %s: %s\n", file, strerror(errno));
    exit(-1);
  }
  *flen  = stats.st_size;
  if (flen == 0) {
    fprintf(stderr, "file %s is zero length\n", file);
    exit(-1);
  }
  if ( (out = malloc(stats.st_size)) == NULL) {
    fprintf(stderr, "can't malloc space for %s\n", file);
    exit(-1);
  }
  if ( (fd = open(file,O_RDONLY)) == -1) {
    fprintf(stderr, "can't open %s: %s\n", file, strerror(errno));
    exit(-1);
  }
  if ( read(fd, out, stats.st_size) != stats.st_size) {
    fprintf(stderr, "short read on %s\n", file);
    exit(-1);
  }
  close(fd);
  return out;
}

int main( int argc, char *argv[]) {
  if (argc < 2) usage(argv[0]);
  int rc;
  size_t flen;
  char *file = argv[1];
  char *data = slurp(file, &flen);

  /* minimal required initialization of z_stream prior to deflateInit2 */
  z_stream zs = {.next_in = data, .zalloc=Z_NULL, .zfree=Z_NULL, .opaque=NULL};
#define want_gzip 16
#define def_windowbits (15 + want_gzip)
#define def_memlevel 8
  rc = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, def_windowbits,
               def_memlevel, Z_DEFAULT_STRATEGY);
  if (rc != Z_OK) {
    fprintf(stderr, "deflateInit failed: %s\n", zs.msg);
    exit(-1);
  }

  /* calculate the max space needed to deflate this buffer in a single pass */
  size_t gzmax = deflateBound(&zs, flen);
  char *out = malloc(gzmax);
  if (out == NULL) {
    fprintf(stderr, "could not allocate %u bytes for compressed file\n",
      (unsigned)gzmax);
    exit(-1);
  }

  /* initialize the remaining parts of z_stream prior to actual deflate */
  zs.avail_in = flen;
  zs.next_out = out;
  zs.avail_out = gzmax;

  /* deflate it in one fell swoop */
  rc = deflate(&zs, Z_FINISH);
  if (rc != Z_STREAM_END) {
    fprintf(stderr,"single-pass deflate failed: ");
    if (rc == Z_OK) fprintf(stderr,"additional passes needed\n");
    else if (rc == Z_STREAM_ERROR) fprintf(stderr,"stream error\n");
    else if (rc == Z_BUF_ERROR) fprintf(stderr,"buffer unavailable\n");
    else fprintf(stderr,"unknown error\n");
    exit(-1);
  }
  rc = deflateEnd(&zs);
  if (rc != Z_OK) fprintf(stderr,"deflateEnd error: %s\n", zs.msg);
  fprintf(stderr,"Original size: %u\n", (unsigned)flen);
  fprintf(stderr,"Deflated size: %u\n", (unsigned)zs.total_out);
  if (write(STDOUT_FILENO, out, zs.total_out) != zs.total_out) {
    fprintf(stderr,"error: partial write\n");
    exit(-1);
  }
  return 0;
}
