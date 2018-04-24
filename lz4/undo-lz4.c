#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#include <lz4.h>

/*
 * undo-lz4
 *
 * example of using lz4's regular api
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
  char *file, *buf=NULL, *obuf=NULL;
  int sc, rc = -1, olen=0;
  size_t len;
 
  if ((argc < 3) || (sscanf(argv[2], "%u", &olen) != 1)) {
    fprintf(stderr, "usage: %s <file> <decompressed-size>\n", argv[0]);
    exit(-1);
  }

  file = argv[1];
  buf = map(file, &len);
  if (buf == NULL) goto done;

  fprintf(stderr, "mapped %s: %zu bytes\n", file, len);

  obuf = malloc( olen );
  if (obuf == NULL) {
    fprintf(stderr, "out of memory\n");
    goto done;
  }

  sc = LZ4_decompress_safe(buf, obuf, len, olen);
  if (sc <= 0) {
    fprintf(stderr, "LZ4_decompress_safe: %d\n", sc);
    goto done;
  }

  fprintf(stderr, "decompressed: %u bytes\n", sc);
  write(STDOUT_FILENO, obuf, sc);
  rc = 0;

 done:
  if (buf) munmap(buf, len);
  if (obuf) free(obuf);
  return rc;
}

