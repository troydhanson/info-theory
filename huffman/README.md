Huffman encoding
----------------

Back up [parent page](https://github.com/troydhanson/info-theory).

This compression method originated around 1952. It uses the
symbol probabilities to assign longer codes to rare symbols.
See "Coding and Information Theory", by Richard W. Hamming, 
or Robert W. Lucky, "Silicon Dreams" for good explanations.

This implementation works at order-1. In other words, on bytes.
You could implement it at order-2 (digrams), etc, with possible
better compression if the input has such higher order structure.
For example, English text does; 'th' is common, 'qq' is not.

Our output file consists of the code book and encoded buffer.
Since it is order-1, the code book maps bytes to bit patterns. 
After the code book, the encoded buffer consists of bit patterns.
During decoding we take each shortest bit pattern that is a valid
code word from the encoded buffer and replace it with its byte.

Example
-------

Run 'make' to build the 'huff' program.

    $ ./huff -e -i /usr/share/dict/words -o words.huff
    $ ./huff -d -i words.huff -o words
    $ diff words /usr/share/dict/words

