Shannon-Fano encoding
---------------------

Back up [parent page](https://github.com/troydhanson/info-theory).

This example of Shannon-Fano encoding is from Claude Shannon's
1948 paper, A Mathematical Theory of Communication, section 9.

Shannon-Fano encoding is a compression method. See Caveats below.

Method

For each symbol i, the number of bits we use to encode it, m(i),
is the integer satisfying:

    log2( 1/p(i) ) <= m(i) < 1 + log2( 1/p(i) )

P(i) is the cumulative probability of the symbols more likely than i. 
Symbol i is encoded as the binary expansion of P(i) to m(i) digits.

Example 1
---------

Run 'make' to build the 'sf' program.

    % echo -n "AAAABBCD" > abcd
    % ./sf -v -i abcd -o encoded
    byte c count rank code-len bitcode
    ---- - ----- ---- -------- ----------
    0x41 A     4    0        1 0
    0x42 B     2    1        2 10
    0x43 C     1    2        3 110
    0x44 D     1    3        3 111

This replicates an example in Shannon's paper (section 11),
where there are four symbols A, B, C, D with probabilities 
(1/2, 1/4, 1/8, 1/8). For illustration the p, m and P are:

             A           B            C            D
    p(i)    1/2         1/4          1/8          1/8
    m(i)     1           2            3            3
    P(i)     0          1/2       1/2+1/4     1/2+1/4+1/8
    code     0          10           110          111

Example 2
---------
This example encodes the dictionary and then decodes it.

    $ ./sf -e -i /usr/share/dict/words -o words.sf
    $ ./sf -d -i words.sf -o words
    $ diff words /usr/share/dict/words

Caveats

The sf program naively stores the code book in the encoded 
file in a large, fixed-size header. Therefore this is not 
a practical program. Additionally Shannon-Fano encoding is
generally less effective than Huffman or LZW.

