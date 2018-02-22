#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <xz.h>

/* formed from enum xz_ret in xz.h */
char *xz_ret_str[] = {
	[XZ_OK]="XZ_OK",
	[XZ_STREAM_END]="XZ_STREAM_END",
	[XZ_UNSUPPORTED_CHECK]="XZ_UNSUPPORTED_CHECK",
	[XZ_MEM_ERROR]="XZ_MEM_ERROR",
	[XZ_MEMLIMIT_ERROR]="XZ_MEMLIMIT_ERROR",
	[XZ_FORMAT_ERROR]="XZ_FORMAT_ERROR",
	[XZ_OPTIONS_ERROR]="XZ_OPTIONS_ERROR",
	[XZ_DATA_ERROR]="XZ_DATA_ERROR",
	[XZ_BUF_ERROR]="XZ_BUF_ERROR",
};

/* mmap file, placing its size in len and returning address or NULL on error.
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

int main(int argc, char *argv[]) {
  char *buf=NULL, *out=NULL;
  size_t len, olen;
  struct xz_dec *d;
  enum xz_ret xc;
  char *file;
  int rc = -1;

  if (argc < 2) {
    printf("usage: %s <xz-file>\n", argv[0]);
    goto done;
  }

  xz_crc32_init();
  xz_crc64_init();
  d = xz_dec_init(XZ_SINGLE, 0);
  if (d == NULL) {
    printf("xz_dec_init: failed\n");
    goto done;
  }

  file = argv[1];
  buf = map(file, &len);
  if (buf == NULL) goto done;

  olen = len * 10; /* of course, this is for testing only */

  out = malloc(olen);
  if (out == NULL) {
    printf("out of memory\n");
    goto done;
  }
  memset(out, 0, olen);

  struct xz_buf b = {
    .in = (unsigned char*)buf,
    .in_pos = 0,
    .in_size = len,
    .out = (unsigned char*)out,
    .out_pos = 0,
    .out_size = olen,
  };

  xc = xz_dec_run(d, &b);
  if (xc != XZ_STREAM_END) {
    printf("xz_dec_run: %s\n", xz_ret_str[xc]);
    goto done;
  }

  printf("%.*s\n", (int)b.out_size, b.out);

  rc = 0;

done:
  if (d) xz_dec_end(d);
  if (buf) munmap(buf, len);
  if (out) free(out);
  return rc;
}
