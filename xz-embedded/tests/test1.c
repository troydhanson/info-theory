#include <stdio.h>
#include <xz.h>

int main() {
  struct xz_dec *d;
  int rc = -1;

  d = xz_dec_init(XZ_SINGLE, 0);
  if (d == NULL) {
    fprintf(stderr, "xz_dec_init: failed\n");
    goto done;
  }

  xz_dec_end(d);

  rc = 0;

done:
 return rc;
}
