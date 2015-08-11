#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
 
/*
 * Calculate second-order (digram) entropy on a stream. 
 * 
 * The input data is considered a sample from a message system.
 *
 * usage example:
 *
 * ./eo2 /usr/share/dict/words
 * 8.03 bits per digram
 * 4.02 bits per byte
 *
 */

void usage(char *prog) {
  fprintf(stderr, "usage: %s [-v] [file]\n", prog);
  exit(-1);
}
 
unsigned counts[256][256];
unsigned total;

int main(int argc, char * argv[]) {
  int opt,verbose=0,i,j,a,b;
  FILE *ifilef=stdin;
  char *ifile=NULL,line[100];
  double p, lp, sum=0;
 
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
  while (1) {
    a = fgetc(ifilef); if (a == EOF) break;
    b = fgetc(ifilef); if (b == EOF) break;
    counts[a][b]++;
    total++;
  }

  /* compute the final entropy. this is -SUM[0,255](p*l(p))
   * where p is the probability of byte value [0..255]
   * and l(p) is the base-2 log of p. Unit is bits per byte.
   */
  for(i=0; i < 256; i++) {
    for(j=0; j < 256; j++) {
      if (counts[i][j] == 0) continue;
      p = 1.0*counts[i][j]/total;
      lp = log(p)/log(2);
      sum -= p*lp;
    }
  }
  printf("%.2f bits per digram\n", sum);
  printf("%.2f bits per byte\n", sum/2);
}
