#!/usr/bin/perl

#
# This script is used like this
#
# ./noise.pl <file> [1|2] > noisy
#
# The [1|2] says: flip up to 1 or 2 bits in each byte.
#
#

use strict;
use warnings;
use bytes;
undef $/;
my $f = shift or die "no file specified";
my $mode = shift or 1;
open IN, "<$f" or die "can't open $f: $!";
my $buf = <IN>; # slurp
close IN;
my @bytes = split //, $buf;
my $len = scalar(@bytes);
my $i=0;
while ($i < $len) {
  my $o = unpack "C", $bytes[$i];
  my $c = $o;
  if ($mode == "1") {
    # cause one bit to flip
    $c += 1 if (($c & 0x01) == 0); # if its even add one
  } elsif ($mode == "2") {
    # cause two bits to flip
    $c += 2 if (($c & 0x01) == 1); # if its odd add two
  }
  $c = $o if ($c > 255);
  $bytes[$i] = pack "C", $c;
  $i++;
}
my $out = join "", @bytes;
print $out;
