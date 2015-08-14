#!/bin/bash
set -x

# raw -> test input file
# cwh -> compressed with header
# cwo -> compressed without header (requires code book)
# bak -> should match the raw file
# cbk -> saved code book 

#V=valgrind
H=../huff
if [ ! -x $H ]; then echo "run make in parent directory first"; fi

# normal encode/decode
$V $H -e -i raw -o cwh
$V $H -d -i cwh -o bak
diff raw bak

# encode (saving the code book externally), decode using it
$V $H -e -i raw -o cwo -C cbk
$V $H -d -i cwo -o bak -c cbk
diff raw bak

# encode (using the code book from above), decode using it
$V $H -e -i raw -o cwo -c cbk
$V $H -d -i cwo -o bak -c cbk
diff raw bak
