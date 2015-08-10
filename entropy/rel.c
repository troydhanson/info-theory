#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
 
/*
 * Print the relative entropy of the input stream.
 * This is a fraction in the range 0-1 which we express as a percent.
 * It means that random data having this probability distribution can
 * be compressed by that amount. Data with this distribution may also
 * compress a lot better in some cases e.g. if local structure exists.
 *
 * examples:
 *
 * ./rel /usr/share/dict/words
 *
 * dd if=/dev/random bs=1 count=100 | ./rel
 *
 */

void usage(char *prog) {
  fprintf(stderr, "usage: %s [-v] [file]\n", prog);
  exit(-1);
}
 
unsigned counts[256];
unsigned total;

int main(int argc, char * argv[]) {
  int opt,verbose=0,i;
  FILE *ifilef=stdin;
  char *ifile=NULL,line[100];
  double p, lp, sum=0, rel;
 
  while ( (opt = getopt(argc, argv, "v+")) != -1) {
    switch (opt) {
      case 'v': verbose++; break;
      default: usage(argv[0]); break;
    }
  }
  if (optind < argc) ifile=argv[optind++];
 
  if (ifile) {
    if ( (ifilef = fopen(ifile,"r")) == NULL) {
      fprintf(stderr,"can't open %s: %s\n", ifile, strerror(errno));
      exit(-1);
    }
  }
 
  /* accumulate counts of each byte value */
  while ( (i=fgetc(ifilef)) != EOF) {
    counts[i]++;
    total++;
  }

  /* compute the final entropy. this is -SUM[0,255](p*l(p))
   * where p is the probability of byte value [0..255]
   * and l(p) is the base-2 log of p. Unit is bits per byte.
   */
  for(i=0; i < 256; i++) {
    if (counts[i] == 0) continue;
    p = 1.0*counts[i]/total;
    lp = log(p)/log(2);
    sum -= p*lp;
  }

  rel = sum/8;

  printf("E: Source entropy:         %.2f bits per byte\n", sum);
  printf("M: Max entropy:            8.00 bits per byte\n");
  printf("R: Relative entropy (E/M): %.2f%%\n", rel*100);
  printf("\n");
  printf("Original:          %5d bytes\n", total);
  printf("Compression to E:  %5d bytes\n", (int)(total*rel));
  printf("Compression ratio: %.1f to 1\n", (8/sum));
  printf("\n");
  printf("Random data having symbol probabilities that match\n"
         "this stream can be compressed approximately as shown.\n"
         "Random data is a worst-case. Data with local structure\n"
         "but having the same first-order probabilities may be\n"
         "compressed further by suitable compression schemes.\n");
  printf("\n");
}
