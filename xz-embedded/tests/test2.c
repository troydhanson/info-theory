#include <stdio.h>
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

int main() {
  struct xz_dec *d;
  enum xz_ret xc;
  int rc = -1;

  d = xz_dec_init(XZ_SINGLE, 0);
  if (d == NULL) {
    printf("xz_dec_init: failed\n");
    goto done;
  }

  struct xz_buf b = {
    .in = NULL,
    .in_pos = 0,
    .in_size = 0,
    .out = NULL,
    .out_pos = 0,
    .out_size = 0,
  };

  xc = xz_dec_run(d, &b);
  if (xc != XZ_OK) {
    printf("xz_dec_run: %s\n", xz_ret_str[xc]);
    goto done;
  }

  xz_dec_end(d);

  rc = 0;

done:
 return rc;
}
