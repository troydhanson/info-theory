Saturation

This tiny program just reports on the "saturation" of its
input file which is considered as a bit vector. The saturation
is the number of set bits, divided by the total number of bits.

Type 'make' to build it. Here we try it on 100 random bytes. 

    $ dd if=/dev/urandom of=rand bs=1 count=100 
    $ ./saturation rand 
    0.49

