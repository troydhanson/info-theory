#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#include <lz4frame.h>

/*
 * unframe-lz4
 *
 * example of using lz4's framing api
 *
 *   on ubuntu, first install:
 *   sudo apt install liblz4-dev
 *
 */
 
/* mmap file, placing its size in len and 
 * returning address or NULL on error. 
 * caller should munmap the buffer eventually.
 */
char *map(char *file, size_t *len) {
  int fd = -1, rc = -1, sc;
  char *buf = NULL;
  struct stat s;

  fd = open(file, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr,"open: %s\n", strerror(errno));
    goto done;
  }

  sc = fstat(fd, &s);
  if (sc < 0) {
    fprintf(stderr,"fstat: %s\n", strerror(errno));
    goto done;
  }

  if (s.st_size == 0) {
    fprintf(stderr,"error: mmap zero size file\n");
    goto done;
  }

  buf = mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (buf == MAP_FAILED) {
    fprintf(stderr, "mmap: %s\n", strerror(errno));
    buf = NULL;
    goto done;
  }

  rc = 0;
  *len = s.st_size;

 done:
  if (fd != -1) close(fd);
  if (rc && buf) { munmap(buf, s.st_size); buf = NULL; }
  return buf;
}

int main(int argc, char * argv[]) {
  char *file, *buf=NULL, *cbuf=NULL, *c, out[512];
  size_t len, nr, l, o;
  int rc = -1;
 
  if (argc < 2) {
    fprintf(stderr, "usage: %s <file>\n", argv[0]);
    exit(-1);
  }

  file = argv[1];
  buf = map(file, &len);
  if (buf == NULL) goto done;

  fprintf(stderr, "mapped %s: %zu bytes\n", file, len);

  /* compressed buffer */
  c = buf;
  l = len;

  /* decompression context */
  LZ4F_decompressionContext_t context;
  nr = LZ4F_createDecompressionContext(&context, LZ4F_VERSION);
  if (LZ4F_isError(nr)) {
    fprintf(stderr, "LZ4F_createDecompressoinContext: %s\n",
      LZ4F_getErrorName(nr));
    goto done;
  }

  do {
    l = len - (c - buf); /* src bytes remaining */
    o = sizeof(out);     /* output bytes avail  */
    nr = LZ4F_decompress(context, out, &o, c, &l, NULL);

    if (LZ4F_isError(nr)) {
      fprintf(stderr, "LZ4F_decompress: %s\n",
        LZ4F_getErrorName(nr));
      goto done;
    }

    if (o) {
      fprintf(stderr, "decompressed %zu bytes\n", o);
      write(STDOUT_FILENO, out, o);
    }

    c += l; /* advance src position */

  } while( nr != 0 );

  rc = 0;

 done:
  LZ4F_freeDecompressionContext(context);
  if (buf) munmap(buf, len);
  if (cbuf) free(cbuf);
  return rc;
}

