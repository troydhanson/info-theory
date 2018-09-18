#include "bf.h"

struct bf {
  unsigned n;
  unsigned hashv[NUM_HASHES];
  uint8_t filter[]; /* C99 flexible array member */
};

static unsigned hash_ber(const char *in, size_t len) {
  unsigned hashv = 0;
  while (len--)  hashv = ((hashv) * 33) + *in++;
  return hashv;
}

static unsigned hash_fnv(const char *in, size_t len) {
 unsigned hashv = 2166136261UL;
 while(len--) hashv = (hashv * 16777619) ^ *in++;
 return hashv;
}

static void get_hashv(struct bf *bf, const char *in, size_t len) {
  assert(NUM_HASHES==2);
  bf->hashv[0] = MASK(hash_ber(in,len), bf->n);
  bf->hashv[1] = MASK(hash_fnv(in,len), bf->n);
}

struct bf *bf_new(unsigned n) {
  struct bf *bf;
  size_t len;

  len = sizeof(*bf) + byte_len(n);

  bf = malloc(len);
  if (bf == NULL) {
    fprintf(stderr, "out of memory\n");
    goto done;
  }

  memset(bf, 0, len);
  bf->n = n;

 done:
  return bf;
}

void bf_free(struct bf *bf) {
  free(bf);
}

int bf_test(struct bf *bf, const char *data, size_t len) {
  unsigned i;

  get_hashv(bf, data,len);

  for(i=0;i<NUM_HASHES;i++) {
    if (BIT_TEST(bf->filter, bf->hashv[i]) == 0)
      return 0;
  }

  return 1;
}

void bf_add(struct bf *bf, const char *data, size_t len) {
  unsigned i;

  get_hashv(bf, data,len);

  for(i=0;i<NUM_HASHES;i++)
    BIT_SET(bf->filter, bf->hashv[i]);
}

void bf_info(struct bf *bf, FILE *out) {
  unsigned i, on=0;
  for(i=0; i<num_bits(bf->n); i++)
    if (BIT_TEST(bf->filter,i)) on++;

  fprintf(out, "%.2f%% saturation (%lu bits)\n",
     on*100.0/num_bits(bf->n), num_bits(bf->n));
}

