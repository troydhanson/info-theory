LZW encoding
------------

Back up [parent page](https://github.com/troydhanson/info-theory).

Lempel-Ziv's LZ77/LZ78 compression methods are the basis of LZW
described in a 1984 IEEE Computer [article](welch_1984_lzw.pdf).

LZ78 encodes using a continually-updated dictionary of already-seen sequences.
The encoding identifies sequences as a dictionary index to their longest
previously-seen prefix sequence and a subsequent index of the following
sequence. Their concatenation forms a novel sequence. This sequence is added
to the dictionary. Next time it occurs it can be encoded as a single index.

The decoder runs the same dictionary-update algorithm - when it decodes an
index, and emits the sequence it encodes, and goes on to the next index,
the concatenation of the sequences is a novel sequence that goes into the 
dictionary. Since the encoder and decoder are running the same algorithm
they both generate the same dictionary at every point in time. The index
numbers in the encoded buffer are thus valid in the decoder dictionary.
There is no need to store the dictionary in the encoded buffer. LZW seeds the
dictionary with the 256 singleton sequences (bytes).

This implementation uses a hash table to look up sequences in the dictionary
during encoding.  It also stores dictionary indexes in the encoded buffer
as variable-width bit codes based on the current dictionary size.

Encoding the English dictionary using LZW achieves quite a bit more
compression that Huffman encoding. Huffman coding at order 1 (byte
frequency forming the basis of bit code assignments) is surpassed
considerably by LZW which leverages the higher order (digram, trigram,
etc) repetition patterns that are characteristic of text.

Example
-------

Run 'make' to build the 'lzw' program.

    $ ./lzw -e -i /usr/share/dict/words -o words.lzw
    $ ./lzw -d -i words.lzw -o words
    $ diff words /usr/share/dict/words

