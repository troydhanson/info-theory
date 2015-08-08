#include "eccode.h"

/* call before encoding or decoding to determine the necessary
 * output buffer size to perform the (de-)encoding operation. */
size_t ecc_compute_olen( int mode, size_t ilen, size_t *ibits, size_t *obits) {

  // 4->7. Every byte becomes 14 bits.
  if ((mode & MODE_ENCODE) && !(mode & MODE_EXTEND)) {
      *ibits = ilen * 8;
      *obits = ilen * 14;
  }

  // 7->4. Every 7 bits becomes 4 bits.
  if ((mode & MODE_DECODE) && !(mode & MODE_EXTEND)) {
      *ibits = (ilen*8) - ((ilen*8) % 7);
      *obits = (*ibits/7) * 4;
  }

  if (mode & (MODE_NOISE1 | MODE_NOISE2)) {
      *ibits = ilen * 8;
      *obits = ilen * 8;
  }

  // 4->8. Every byte becomes 16 bits.
  if ((mode & MODE_ENCODE) && (mode & MODE_EXTEND)) {
      *ibits = ilen * 8;
      *obits = ilen * 16;
  }

  // 8->4. Every byte becomes 4 bits.
  if ((mode & MODE_DECODE) && (mode & MODE_EXTEND)) {
      *ibits = ilen * 8;
      *obits = *ibits / 2;
  }

  size_t olen = (*obits/8) + ((*obits % 8) ? 1 : 0);
  return olen;
}

/* An implementation of Hamming codes (perfect 1-error correcting codes) and 
 * extended Hamming codes (2-error detecting codes). The process used below
 * is described by Claude Shannon in "A Mathematical Theory of Communication",
 * Bell System Technical Journal, July 1948. An Example of Efficient Coding.  
 *
 *"The following example, although somewhat unrealistic, is a case in which 
 * exact matching to a noisy channel is possible. There are two channel 
 * symbols 0 and 1, and the noise affects them in blocks of seven symbols. A
 * block of seven is either transmitted without error, or exactly one symbol of
 * the seven is incorrect. These eight possibilities are equally likely. We have
 *
 *           capacity C = 4/7 bits/symbol
 *
 * An efficient code, allowing complete correction of errors and transmitting
 * at the rate C, is the following (found by a method due to R. Hamming):
 *
 * Let a a block of seven symbols be x1,x2,...,x7. Of these x3, x5, x6 and x7
 * are message symbols and chosen arbitrarily by the source. The other three
 * are redundant and calculated as follows:
 *
 *      x4 is chosen to make a = x4 + x5 + x6 + x7 even 
 *      x2 is chosen to make b = x2 + x3 + x6 + x7 even 
 *      x1 is chosen to make c = x1 + x3 + x5 + x7 even 
 *
 * When a block of seven is received, a, b and c are calculated and if even
 * called zero, if odd called one. The binary number abc then gives the 
 * subscript of the Xi that is incorrect (if 0 there was no error)."
 * 
 */ 

int ecc_recode(int mode, unsigned char *ib, size_t ilen, unsigned char *ob) {
  unsigned char x[8], a, b, c, e=0, p, t;
  size_t i=0, o=0, ibits;
  int rc=-1;

  if ((mode & MODE_ENCODE) && !(mode & MODE_EXTEND)) {
    ibits = ilen * 8;
    /* iterate over 4 bits at a time, producing 7. */
    while (i < ibits) {
      x[3] = BIT_TEST(ib,i+0) ? 1 : 0;
      x[5] = BIT_TEST(ib,i+1) ? 1 : 0;
      x[6] = BIT_TEST(ib,i+2) ? 1 : 0;
      x[7] = BIT_TEST(ib,i+3) ? 1 : 0;
      x[4] = (x[5] + x[6] + x[7]) % 2;
      x[2] = (x[3] + x[6] + x[7]) % 2;
      x[1] = (x[3] + x[5] + x[7]) % 2;

      if (x[1]) BIT_SET(ob,o+0);
      if (x[2]) BIT_SET(ob,o+1);
      if (x[3]) BIT_SET(ob,o+2);
      if (x[4]) BIT_SET(ob,o+3);
      if (x[5]) BIT_SET(ob,o+4);
      if (x[6]) BIT_SET(ob,o+5);
      if (x[7]) BIT_SET(ob,o+6);

      i += 4;
      o += 7;
    }
  }

  if ((mode & MODE_DECODE) && !(mode & MODE_EXTEND)) {
    ibits = (ilen*8) - ((ilen*8) % 7);
    /* iterate over 7 bits at a time, producing 4. */
    while (i < ibits) {
      x[1] = BIT_TEST(ib,i+0) ? 1 : 0;
      x[2] = BIT_TEST(ib,i+1) ? 1 : 0;
      x[3] = BIT_TEST(ib,i+2) ? 1 : 0;
      x[4] = BIT_TEST(ib,i+3) ? 1 : 0;
      x[5] = BIT_TEST(ib,i+4) ? 1 : 0;
      x[6] = BIT_TEST(ib,i+5) ? 1 : 0;
      x[7] = BIT_TEST(ib,i+6) ? 1 : 0;

      a = (x[4] + x[5] + x[6] + x[7]) % 2;
      b = (x[2] + x[3] + x[6] + x[7]) % 2;
      c = (x[1] + x[3] + x[5] + x[7]) % 2;

      e = (a << 2U) | (b << 1U) | c;
      if (e) x[e] = x[e] ? 0 : 1;

      if (x[3]) BIT_SET(ob,o+0);
      if (x[5]) BIT_SET(ob,o+1);
      if (x[6]) BIT_SET(ob,o+2);
      if (x[7]) BIT_SET(ob,o+3);

      i += 7;
      o += 4;
    }
  }

  if ((mode & (MODE_NOISE1|MODE_NOISE2)) && !(mode & MODE_EXTEND)) {
    ibits = (ilen*8) - ((ilen*8) % 7);
    /* iterate over 7 bits at a time, adding noise. */
    while (i < ibits) {
      x[1] = BIT_TEST(ib,i+0) ? 1 : 0;
      x[2] = BIT_TEST(ib,i+1) ? 1 : 0;
      x[3] = BIT_TEST(ib,i+2) ? 1 : 0;
      x[4] = BIT_TEST(ib,i+3) ? 1 : 0;
      x[5] = BIT_TEST(ib,i+4) ? 1 : 0;
      x[6] = BIT_TEST(ib,i+5) ? 1 : 0;
      x[7] = BIT_TEST(ib,i+6) ? 1 : 0;

      t = (mode & MODE_NOISE1) ? 1 : 2;
      while (t--) {
        x[e%8] = x[e%8] ? 0 : 1;
        e++;
      }

      if (x[1]) BIT_SET(ob,o+0);
      if (x[2]) BIT_SET(ob,o+1);
      if (x[3]) BIT_SET(ob,o+2);
      if (x[4]) BIT_SET(ob,o+3);
      if (x[5]) BIT_SET(ob,o+4);
      if (x[6]) BIT_SET(ob,o+5);
      if (x[7]) BIT_SET(ob,o+6);

      i += 7;
      o += 7;
    }
  }

  if ((mode & MODE_ENCODE) && (mode & MODE_EXTEND)) {
    ibits = ilen * 8;
    /* iterate over 4 bits at a time, producing 8. */
    while (i < ibits) {
      x[3] = BIT_TEST(ib,i+0) ? 1 : 0;
      x[5] = BIT_TEST(ib,i+1) ? 1 : 0;
      x[6] = BIT_TEST(ib,i+2) ? 1 : 0;
      x[7] = BIT_TEST(ib,i+3) ? 1 : 0;
      x[4] = (x[5] + x[6] + x[7]) % 2;
      x[2] = (x[3] + x[6] + x[7]) % 2;
      x[1] = (x[3] + x[5] + x[7]) % 2;
      x[0] = (x[1] + x[2] + x[3] + x[4] + x[5] + x[6] + x[7]) % 2;

      if (x[1]) BIT_SET(ob,o+0);
      if (x[2]) BIT_SET(ob,o+1);
      if (x[3]) BIT_SET(ob,o+2);
      if (x[4]) BIT_SET(ob,o+3);
      if (x[5]) BIT_SET(ob,o+4);
      if (x[6]) BIT_SET(ob,o+5);
      if (x[7]) BIT_SET(ob,o+6);
      if (x[0]) BIT_SET(ob,o+7);

      i += 4;
      o += 8;
    }
  }

  if ((mode & MODE_DECODE) && (mode & MODE_EXTEND)) {
    ibits = ilen *8;
    /* iterate over 8 bits at a time, producing 4. */
    while (i < ibits) {
      x[1] = BIT_TEST(ib,i+0) ? 1 : 0;
      x[2] = BIT_TEST(ib,i+1) ? 1 : 0;
      x[3] = BIT_TEST(ib,i+2) ? 1 : 0;
      x[4] = BIT_TEST(ib,i+3) ? 1 : 0;
      x[5] = BIT_TEST(ib,i+4) ? 1 : 0;
      x[6] = BIT_TEST(ib,i+5) ? 1 : 0;
      x[7] = BIT_TEST(ib,i+6) ? 1 : 0;
      x[0] = BIT_TEST(ib,i+7) ? 1 : 0;

      a = (x[4] + x[5] + x[6] + x[7]) % 2;
      b = (x[2] + x[3] + x[6] + x[7]) % 2;
      c = (x[1] + x[3] + x[5] + x[7]) % 2;

      e = (a << 2U) | (b << 1U) | c;
      if (e) x[e] = x[e] ? 0 : 1;
      p = (x[0] + x[1] + x[2] + x[3] + x[4] + x[5] + x[6] + x[7]) % 2;
      if (p && e) goto done;

      if (x[3]) BIT_SET(ob,o+0);
      if (x[5]) BIT_SET(ob,o+1);
      if (x[6]) BIT_SET(ob,o+2);
      if (x[7]) BIT_SET(ob,o+3);

      i += 8;
      o += 4;
    }
  }

  if ((mode & (MODE_NOISE1|MODE_NOISE2)) && (mode & MODE_EXTEND)) {
    ibits = ilen * 8;
    /* iterate over 8 bits at a time, adding noise. */
    while (i < ibits) {
      x[1] = BIT_TEST(ib,i+0) ? 1 : 0;
      x[2] = BIT_TEST(ib,i+1) ? 1 : 0;
      x[3] = BIT_TEST(ib,i+2) ? 1 : 0;
      x[4] = BIT_TEST(ib,i+3) ? 1 : 0;
      x[5] = BIT_TEST(ib,i+4) ? 1 : 0;
      x[6] = BIT_TEST(ib,i+5) ? 1 : 0;
      x[7] = BIT_TEST(ib,i+6) ? 1 : 0;
      x[0] = BIT_TEST(ib,i+7) ? 1 : 0;

      t = (mode & MODE_NOISE1) ? 1 : 2;
      while (t--) {
        x[e%8] = x[e%8] ? 0 : 1;
        e++;
      }

      if (x[1]) BIT_SET(ob,o+0);
      if (x[2]) BIT_SET(ob,o+1);
      if (x[3]) BIT_SET(ob,o+2);
      if (x[4]) BIT_SET(ob,o+3);
      if (x[5]) BIT_SET(ob,o+4);
      if (x[6]) BIT_SET(ob,o+5);
      if (x[7]) BIT_SET(ob,o+6);
      if (x[0]) BIT_SET(ob,o+7);

      i += 8;
      o += 8;
    }
  }

  rc = 0;

 done:
  return rc;
}

