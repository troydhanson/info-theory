Notes on Bloom filters
======================

Basics
------

A Bloom filter is a simple probabilistic device for testing set membership.
When an item is tested for membership, the Bloom filter gives one of two outcomes:

  * either the member is definitely NOT in the set, or
  * the member MAY be in the set

The true set members that were added to the set will always give the latter 
result (so the Bloom filter *never* misses its true members) but it does falsely
suggest set membership for non-members a certain percentage of the time. That
error rate decreases as the Bloom filter is made larger and larger.

Filter size
-----------
The filter itself is just a bit array.  In the test program here, the size of
the bit array is specified as a log2 (so for example these sizes correspond to
different values of n:) 

    n=4 means 2^4 bits
    n=5 means 2^5 bits
    n=16 means 2^16 bits (65536 bits occuping 8k)
    n=24 means 2^24 bits (16777216 bits occupying 2M) 

The accuracy of the filter improves with greater n.

Add a member
------------
To add a member to the set, compute 'n' different hash functions on it and
turn those bits 'on' in the bit array.

Test membership
---------------
To test whether an item is a possible set member, compute 'n' different
hash functions on it. Then test whether *all* of those bits are 'on'
in the bit vector. If any are not on, this item cannot be a set member.
If all of those bits are on, this item may be a set member.

The 'bf' test program
---------------------

The program bf.c implements a Bloom filter.  To build it, type 'make'.
It can be used like,

    ./bf -n 16 members.txt test.txt

Where the lines of members.txt are the true set members and the lines
of test.txt will be tested for membership. Varying 'n' shows how the
error rate increases with smaller values of n.

