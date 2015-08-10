#include "code.h"

/* call before encoding or decoding to determine the necessary
 * output buffer size to perform the (de-)encoding operation. */
size_t huf_compute_olen( int mode, size_t ilen, size_t *ibits, size_t *obits) {

  if ((mode & MODE_ENCODE)) {
      *ibits = ilen * 8;
      *obits = ilen * 14;
  }

  if ((mode & MODE_DECODE)) {
      *ibits = (ilen*8) - ((ilen*8) % 7);
      *obits = (*ibits/7) * 4;
  }

  size_t olen = (*obits/8) + ((*obits % 8) ? 1 : 0);
  return olen;
}

/* 
 * 
 */ 
int huf_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob) {
  int rc=-1;

  if ((mode & MODE_ENCODE)) {
  }

  if ((mode & MODE_DECODE)) {
  }

  rc = 0;

  return rc;
}

