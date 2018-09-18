#include "bf.h"

int main() {
  struct bf *bf=NULL;

  bf = bf_new(10);
  if (bf == NULL) goto done;

  printf("adding hello\n");
  bf_add(bf, "hello", 5);
  printf("hello: %s\n", (bf_test(bf, "hello", 5) ? "hit" : "miss"));
  printf("world: %s\n", (bf_test(bf, "world", 5) ? "hit" : "miss"));

  bf_info(bf, stdout);
 
 done:
  if (bf) bf_free(bf);
  return 0;
}
