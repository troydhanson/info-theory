Hamming codes
-------------

Back up [parent page](https://github.com/troydhanson/info-theory).

This example of an error correcting code is from Claude Shannon's
1948 paper, A Mathematical Theory of Communication, section 17.

The encoding processes takes every 4 bits and makes 7 bits from them.
The decoding process takes every 7 bits and produces 4 bits from them.
The decoding can correct for 1 erroneous bit in each group of 7.

    % echo "hello, world!" > original
    % ./ecc -i original -o encoded
    % ./ecc -n -i encoded -o noisy
    % ./ecc -d -i noisy -o decoded
    % diff original decoded
    %     

Any such noise function (such as a random 1/7 toggle) is correctable.
Our 'noise adding' mode (ecc -n) perturbs one bit in each group of 7.
It toggles bit 0 in the first group, bit 1 in the second group, etc.
Because of its determinism it is reversible by repeating the command.

Extended Hamming codes

The encoding above can correct one error in seven bits.  We can add a 
parity bit. Then we can correct one error among the 8 bits and detect 
the presence of a second, uncorrectable error.

    % echo "hello, world!" > original
    % ./ecc -x -i original -o encoded
    % ./ecc -n -x -i encoded -o noisy
    % ./ecc -d -x -i noisy -o decoded
    % diff original decoded
    %

We can add too much noise by toggling two bits per byte, using ecc -nn.
The use of extended coding (-x) allow us to detect the (uncorrectable) 
error. This is the advantage of extended codes over the standard ones.

    % ./ecc -nn -x -i encoded -o noisy
    % ./ecc -d -x -i noisy -o decoded
    ecc_recode error

