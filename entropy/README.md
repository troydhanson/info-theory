Entropy
-------

Back up [parent page](https://github.com/troydhanson/info-theory).

Programs
--------

The C programs here can be built by typing 'make'. These are:

* ent: compute first-order (byte) entropy of input
* eo2: compute second-order (digram) entropy of input
* rel:  compute relative entropy and compression potential of input
* tbl:  compute entropy of input in 1000-byte chunks, using log table
* mkpb: outputs a stream of bytes with a given probabilities e.g. 90/10

Examples
--------

Compute entropy of a stream having two symbols in probabilities 1/3 and 2/3.

    % ./mkpb -c 10000 33 67 | ./ent
    0.92 bits per byte

Compute compression potential of a random stream of symbols with probability
distribution (0.1, 0.2, 0.3, 0.4). Note that the probabilities given to mkpb
are multiplied by ten and must sum to 100.

    % ./mkpb -c 10000 10 20 30 40 | ./rel
    
    E: Source entropy:         1.85 bits per byte
    M: Max entropy:            8.00 bits per byte
    R: Relative entropy (E/M): 23.07%
    
    Original:          10000 bytes
    Compression to E:   2307 bytes
    Compression ratio: 4.3 to 1

Run 'gzip' on a random stream of 1,000,000 bytes having those probabilities.
Compare that to the compression possible according to first-order entropy E.

    % ./mkpb -c 1000000 10 20 30 40 | gzip | wc -c
    286324
    % ./mkpb -c 1000000 10 20 30 40 | ./rel 
    Compression to E:  230793 bytes

