#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#include <lz4.h>

/*
 * do-lz4
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
  char *file, *buf=NULL, *cbuf=NULL;
  size_t len, cbound;
  int sc, rc = -1;
 
  if (argc < 2) {
    fprintf(stderr, "usage: %s <file>\n", argv[0]);
    exit(-1);
  }

  file = argv[1];
  buf = map(file, &len);
  if (buf == NULL) goto done;

  fprintf(stderr, "mapped %s: %zu bytes\n", file, len);

  cbound = LZ4_compressBound(len);
  cbuf = malloc(cbound);
  if (cbuf == NULL) {
    fprintf(stderr, "out of memory\n");
    goto done;
  }

  sc = LZ4_compress_default(buf, cbuf, len, cbound);
  assert(sc >= 0);
  if (sc == 0) {
    fprintf(stderr, "LZ4_compress_default: cannot compress\n");
    goto done;
  }

  fprintf(stderr, "compressed to %u bytes\n", sc);
  write(STDOUT_FILENO, cbuf, sc);
  rc = 0;

 done:
  if (buf) munmap(buf, len);
  if (cbuf) free(cbuf);
  return rc;
}

