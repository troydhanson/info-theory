LZW encoding
------------

Back up [parent page](https://github.com/troydhanson/info-theory).

Lempel-Ziv's LZ77/LZ78 compression methods are the basis of LZW,
described in Welch's 1984 IEEE Computer [article](welch_1984_lzw.pdf).
LZW encodes its input as a series of indexes into a running dictionary.
LZW seeds the dictionary with the 256 singleton "sequences"- the bytes.

Encoding is a process of emitting indexes. As it works through the input,
it considers successive symbols, making the sequence as long as possible
while still in the dictionary of already-seen sequences. At some point the
growing sequence is not in the dictionary. Now the encoder emits the index
of the last sequence- (it's one symbol shorter; it was in the dictionary).
It adds a dictionary entry for the new sequence (last sequence + symbol).
Encoding resumes by growing a new sequence from the symbol position.

The decoder reads an index, and emits the sequence it encodes. When it 
reads the next index, and emits the sequence it encodes, it also stores in 
the dictionary a new entry (the first symbol appended to the last sequence).
So, the decoder generates the same dictionary as the encoder while it works. 
As each index from the encoder is encountered by the decoder, it adds the same
dictionary entry as the encoder did when it produced that index.  There is no
need to store the dictionary in the encoded buffer. 

The encoder checks if sequences are in the dictionary; the decoder converts
index numbers to sequences. To meet these needs this implementation keeps
dictionary entries simultaneously in an array and in a hash table.

Dictionary indexes appear as variable-width bit codes in the encoded buffer.
Their width varies as the log of the dictionary size during encoding.

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

