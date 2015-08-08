About
-----

Back up [parent page](http://troydhanson.github.io/info-theory/).

Programs
--------

The C programs here can be built by typing 'make'. These are:

* ent:  compute entropy of input
* rel:  compute relative entropy and compression potential of input
* tbl:  compute entropy of input in 1000-byte chunks, using log table
* mkpb: outputs a stream of bytes with a given probabilities e.g. 90/10

Examples
--------

Compute entropy of a stream having two symbols in probabilities 1/3 and 2/3.

>  ./mkpb -c 10000 33 67 | ./ent
>  0.92 bits per byte

Compute compression potential of a random stream of symbols with probability
distribution (0.1, 0.2, 0.3, 0.4). Note that the probabilities given to mkpb
are multiplied by ten and must sum to 100.

  ./mkpb -c 10000 10 20 30 40 | ./rel

> E: Source entropy:         1.85 bits per byte
> M: Max entropy:            8.00 bits per byte
> R: Relative entropy (E/M): 23.07%
> 
> Original:          10000 bytes
> Compressed:         2307 bytes
> Compression ratio: 4.3 to 1

Compute entropy over 1000-byte chunks of random input.

> dd if=/dev/random bs=1 count=2000 | ./tbl
> 7.78 bits per byte
> 7.82 bits per byte

