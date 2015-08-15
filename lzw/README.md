LZW encoding
------------

Back up [parent page](https://github.com/troydhanson/info-theory).

Lempel-Ziv's LZ77/LZ78 compression methods are the basis of LZW (1984).
Terry Welch described LZW in an article IEEE Computer magazine [(pdf)]
(welch_1984_lzw.pdf).

LZ78 encodes and decodes using a dictionary of already-seen sequences.
During encoding, novel sequences continue to be added.  In this way the
encoding identifies sequences by a dictionary index of the longest
previously-seen sequence.  The decoder is running the same dictionary-
update algorithm so it can interpret the index numbers.  LZW seeds the
dictionary with the 256 single-byte symbols. 

Encoding the English dictionary using LZW achieves quite a bit more
compression that Huffman encoding. Huffman coding at order 1 (byte
frequency forming the basis of bit code assignments) is surpassed
considerably by LZW which uses the higher order (digram, trigram,
etc) repetition patterns that are characteristic of text.

This implementation uses variable-width bit codes. 

Example
-------

Run 'make' to build the 'lzw' program.

    $ ./lzw -e -i /usr/share/dict/words -o words.lzw
    $ ./lzw -d -i words.lzw -o words
    $ diff words /usr/share/dict/words

