#!/bin/bash
set -x

# raw -> test input file
# cwh -> compressed with header
# cwo -> compressed without header (requires code book)
# bak -> should match the raw file
# cbk -> saved code book 

#V=valgrind
H=../lzw
if [ ! -x $H ]; then echo "run make in parent directory first"; fi

# normal encode/decode
$V $H -e -i raw -o cwh
$V $H -d -i cwh -o bak
diff raw bak

