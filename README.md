http://troydhanson.github.io/info-theory

FILES
-----

mkprob.c    -- generates a byte stream with a particular distribution e.g. 90/10
entropy1.c  -- simple program that reads stdin/file and prints out its entropy
entropy2.c  -- computes entropy in chunks of 1000 symbols. Uses table of logs.
shlimit.c   -- compute the "shrinkability limit" for the given stdin/file

Example 1
~~~~~~~~~
Calculate entropy of a stream having (approx.) 1/3 and 2/3 ratio of two symbols:

  ./mkprob -c 10000 33 67 | ./entropy1
  0.92 bits per byte

Example 2
~~~~~~~~~
Calculate the limit to which a particular stream of 10000 bytes could be compressed:

  ./mkprob -c 10000 10 20 30 40 | ./shlimit 
  1.85 bits per byte
  This data can be reduced to 23% of its original size,
  from 10000 bytes to about 2312 bytes.

