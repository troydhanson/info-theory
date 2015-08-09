#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
 
/*
 * Calculaty entropy on a stream. This program is table driven.
 */

void usage(char *prog) {
  fprintf(stderr, "usage: %s [-v] [file]\n", prog);
  exit(-1);
}
 
#define MAX_SYMBOLS 1000
unsigned counts[256];
unsigned total;
double logtbl[MAX_SYMBOLS+1]; /* +1 since log(0) is undefined */

int main(int argc, char * argv[]) {
  int opt,verbose=0,i,c;
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

  /* pre-compute a table of base-2 logs for efficient use in the loop */
  for(i=1; i <= MAX_SYMBOLS; i++) logtbl[i] = log(i)/log(2);
 
  while(1) {  /* loop over MAX_SYMBOLS symbols at a time */

    /* accumulate counts of each byte value */
    while ((total < MAX_SYMBOLS) && ((c=fgetc(ifilef)) != EOF)) {
      counts[c]++;
      total++;
    }

    /* compute the chunk entropy. this is -SUM[0,255](p*l(p))
     * where p is the probability of byte value [0..255]
     * and l(p) is the base-2 log of p. Unit is bits per byte.
     */
    for(i=0; i < 256; i++) {
      if (counts[i] == 0) continue;
      p = 1.0*counts[i]/total;
      lp = logtbl[counts[i]] - logtbl[total]; /* ln(a/b)=ln(a)-ln(b) */
      sum -= p*lp;
    }
    if (total) printf("%.2f bits per byte\n", sum);
    if (c == EOF) break;

    /* reset counters for next chunk of symbols */
    memset(counts,0,sizeof(int)*256);
    total = 0;
    sum = 0;
 }
}
