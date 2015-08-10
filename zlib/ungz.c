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

  /* minimal required initialization of z_stream prior to inflateInit2 */
  z_stream zs = {.next_in = data, .avail_in=flen, .zalloc=Z_NULL, .zfree=Z_NULL,
                 .opaque=NULL};
#define want_gzip 16
#define def_windowbits (15 + want_gzip)
  rc = inflateInit2(&zs, def_windowbits);
  if (rc != Z_OK) {
    fprintf(stderr, "inflateInit failed: %s\n", zs.msg);
    exit(-1);
  }

  /* start with a guess of the space needed to uncompress */
  size_t gzmax = flen * 3;
  char *out = malloc(gzmax);
  if (out == NULL) {
    fprintf(stderr, "could not allocate %u bytes for uncompressed file\n",
      (unsigned)gzmax);
    exit(-1);
  }

  /* initialize the remaining parts of z_stream prior to actual deflate */
  zs.next_out = out;
  zs.avail_out = gzmax;

  /* inflate it .. cannot do this in one pass since final size unknown */
 keepgoing:
  rc = inflate(&zs, Z_NO_FLUSH);
  if ((rc == Z_OK) || (rc == Z_BUF_ERROR)) { /* need to grow buffer */
    /* fprintf(stderr,"inflate loop..\n"); */
    off_t cur = (char*)zs.next_out - out; /* save offset */
    out = realloc(out, gzmax*2);
    if (out == NULL) {
      fprintf(stderr,"realloc failed\n");
      exit(-1);
    }
    zs.next_out = out + cur;
    zs.avail_out += gzmax;
    gzmax *= 2;
    goto keepgoing;
  }
  if (rc != Z_STREAM_END) {
    if (rc == Z_DATA_ERROR) fprintf(stderr,"input data corrupted\n");
    else if (rc == Z_STREAM_ERROR) fprintf(stderr,"stream error\n");
    else if (rc == Z_MEM_ERROR) fprintf(stderr,"insufficient memory\n");
    else fprintf(stderr,"unknown error\n");
    exit(-1);
  }
  rc = inflateEnd(&zs);
  if (rc != Z_OK) fprintf(stderr,"inflateEnd error: %s\n", zs.msg);
  fprintf(stderr,"Original size: %u\n", (unsigned)flen);
  fprintf(stderr,"Inflated size: %u\n", (unsigned)zs.total_out);
  if (write(STDOUT_FILENO, out, zs.total_out) != zs.total_out) {
    fprintf(stderr,"error: partial write\n");
    exit(-1);
  }
  return 0;
}
