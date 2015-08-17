mini-lzw
--------

This code takes a precomputed LZW dictionary and uses it to encode or decode.

Why?

This is useful for transmitting LZW-compressed buffers between endpoints that
you want to always start from the same state- a good, non-empty dictionary.
LZW normally takes a lot of input (probably several K at least) to build up
good dictionary content. If you are transmitting a lot of small buffers
it would not make sense to LZW-encode them individually. On the other hand,
if you LZW encode the stream of buffers, and sometime later need to restart
the sender or receiver, you'd need to restart both of them (or have the one
quiescently digest the stream from the beginning) to re-sync their dictionaries.

Or, if you have a third system that needs to decode some content from the
message stream between the first two, it would be necessary to know the 
dictionary state (as well as a proper boundary for the bitcodes in the 
encoded stream). Both of these requirements can be met by using a fixed LZW
dictionary and a fixed bitcode length for the encoded indexes.

First, generate a good LZW dictionary on a suitably-large data sample. Here
we generate a dictionary with up to one million sequences.

    % ./mlzw -e -i census-names.in -o census-names.out -C dict -D 1000000

Now the dictionary file can be used to encode and decode:

    % ./mlzw -e -i names.in  -o names.out -c dict 
    % ./mlzw -d -i names.out -i names.org -c dict 

The mlzw command is demonstrating the underlying C API which is the real
purpose of mini-lzw.

    mlzw_load(lzw, "names.lzw");
    rc = mlzw_recode(mode, lzw, input, input_len, output, &output_len);
    if (rc < 0) ...



