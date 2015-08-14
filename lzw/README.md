LZW encoding
------------

Back up [parent page](https://github.com/troydhanson/info-theory).

LZ has innate ability to encode frequent sequences efficiently. This
is one of their advantages over Huffman codes that operate on bytes.

Lempel-Ziv's LZ77/LZ78 compression methods was adapted into LZW (1984).
LZ78 uses a dictionary of already-seen sequences that is updated
during encoding so that longer novel sequences continue to be added.
In this way the encoding can encode the longest already-seen sequence
as an index entry to the dictionary. The decoder is running the same
dictionary-update algorithm so it can interpret the index numbers.
LZW seeds the dictionary with the 256 single-byte symbols.

Example
-------

Run 'make' to build the 'lzw' program.

    $ ./lzw -e -i /usr/share/dict/words -o words.lzw
    $ ./lzw -d -i words.lzw -o words
    $ diff words /usr/share/dict/words

