#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
 
/*
 * Generates a stream of bytes having a specified probability distribution.
 * The bytes will be chosen from the alphabet (a,b,c,...).
 */

void usage(char *prog) {
  fprintf(stderr, "usage: %s [-c <count>] prob1 [prob2 ...]\n", prog);
  fprintf(stderr, "e.g.,  %s 20 80\n", prog);
  fprintf(stderr, "       %s 10 80 10\n", prog);
  fprintf(stderr, "Probabilities must add to 100\n");
  exit(-1);
}

char sym[100];
 
int main(int argc, char * argv[]) {
  unsigned opt,i,count=100,w,pos=0,left=100;
  char symbol = 'a';

  while( (opt=getopt(argc,argv,"c:")) != -1) {
    switch (opt) {
      case 'c': count=atoi(optarg); break;
      default: usage(argv[0]);
    }
  }
  if (optind >= argc) usage(argv[0]);

  for(i=optind; i<argc; i++) {
    if (sscanf(argv[i],"%u",&w) != 1) usage(argv[0]);
    if (w > left) usage(argv[0]);
    memset(&sym[pos],symbol,w);
    pos += w; left -= w;
    symbol++;
  }
  if (left) usage(argv[0]);
  //printf("%.*s\n",100,sym);
  while(count--) {
    i = (unsigned)(100.0*rand()/RAND_MAX); assert(i >= 0 && i < 100);
    printf("%c", sym[i]);
  }
}
