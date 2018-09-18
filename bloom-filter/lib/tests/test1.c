#include "bf.h"

int main() {
  struct bf *bf=NULL;

  bf = bf_new(10);
  if (bf == NULL) goto done;

  bf_info(bf, stdout);
 
 done:
  if (bf) bf_free(bf);
  return 0;
}
