Bloom filters
-------------

Back up [parent page](https://github.com/troydhanson/info-theory).

Bloom filters (1970) are used to test set membership. When an item is tested,
they indicate whether the item MAY be in the set or is definitely not in the set.

If you have a large data set (say, a dictionary of all city names in a country), 
and you want an efficient way to match on those words, and you are willing to
put up with some false matches, then a Bloom filter can help. It represents the
true data set as a set of bits in a bitvector. The bits are based on a hash of
the true data members. You can make the bit vector small, to save memory, and
get a lower-quality filter, or give it lots of memory and get a higher quality
filter. To test an item for membership you hash it and see if the bits are set.

'bf' program
------------

The program bf.c implements a Bloom filter.  To build it, type 'make'. It is used like,

    % ./bf -n 16 members candidates

 * 'members' is file listing the known set members, one per line. 
 * 'candidates' is a file with lines to be tested for membership.
 * '-vv' prints the passing candidates (those that are possibly members) 
 * '-n' is an integer saying how much memory the filter can use (2^n bits) 
 * '-n' values that are higher make the filter more accurate- try 4, 8, 16, 24
 * the 'bf' program only works on text because it uses line-delimited files
 * a Bloom filter in general can work on any kind of data that can be hashed.
   The hash functions have to suit the data; hash collisions weaken the filter.

Examples
--------

This directory has a file called 'cities' listing the largest 100 cities in the USA. 
We make a Bloom filter from them. Then we test English dictionary words to see how
many of them appear to be city names.

    # this is our dictionary 
    % WORDS=/usr/share/dict/words

    % ./bf -n 8 cities $WORDS
    31.54% hit rate (31279/99171) n=8

    % ./bf -n 16 cities $WORDS
    0.07% hit rate (70/99171) n=16

    % ./bf -n 24 cities $WORDS
    0.07% hit rate (70/99171) n=16

Of course we can see the results if we want to (using -vv).

    % ./bf -vv -n 24 cities $WORDS
    
    hit on Akron
    hit on Albuquerque
    hit on Anaheim
    hit on Anchorage
    ...

Notice how increasing n at first (from 8 to 16) gave us a more accurate filter. 
When we increased it a second time (to 24), there was no gain.  The Bloom
filter at n=16 was sufficienly large to discriminate the cities in this
dictionary. You can see it clearly if you run bf using '-vv' to see the
"cities" that match the filter; n=14 has lots of spurious words; n=16 is perfect.

The remarkable thing from an information viewpoint is that the dictionary
which on my computer is about 1 MB, is being represented (at n=16) in 8K.
Of course the hash functions used by bf are critical; they are good on this
kind of data but on other sources (say, phone numbers) they may be bad. 

If you use a single '-v' flag, you can see the 'saturation' (that is, how many
bits are set in the Bloom filter as a percentage). If you test candidates on a
saturated filter, then "everything" would seem to be in the set. A real application
should measure the saturation; if it's too high, make the filter larger to reduce
the saturation. Here is the saturation of our cities data with n=8, n=16 and n=24.

    $ ./bf -v -n 8 cities $WORDS
    56.25% saturation (256 bits)

    $ ./bf -v -n 16 cities $WORDS
    0.30% saturation (65536 bits)

    $ ./bf -v -n24 cities $WORDS
    0.00% saturation (16777216 bits)
    
How a Bloom filter works
------------------------

A Bloom filter is a simple probabilistic device for testing set membership.
When an item is tested for membership, the Bloom filter gives one of two outcomes:

  * either the member is definitely NOT in the set, or
  * the member MAY be in the set

The true set members that were added to the set will always give the latter 
result (so the Bloom filter *never* misses its true members) but it does falsely
suggest set membership for non-members a certain percentage of the time. That
error rate decreases as the Bloom filter is made larger and larger.

Filter size

The filter itself is just a bit array.  In the test program here, the size of
the bit array is specified as a log2 (so for example these sizes correspond to
different values of n:) 

    n=4 means 2^4 bits
    n=5 means 2^5 bits
    n=16 means 2^16 bits (65536 bits occuping 8k)
    n=24 means 2^24 bits (16777216 bits occupying 2M) 

The accuracy of the filter improves with greater n.

Add a member

To add a member to the set, compute 'n' different hash functions on it and
turn those bits 'on' in the bit array.

Test membership

To test whether an item is a possible set member, compute 'n' different
hash functions on it. Then test whether *all* of those bits are 'on'
in the bit vector. If any are not on, this item cannot be a set member.
If all of those bits are on, this item may be a set member.

