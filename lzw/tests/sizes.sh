#!/bin/bash
#set -x

# raw -> test input file
# cwh -> compressed with header
# cwo -> compressed without header (requires code book)
# bak -> should match the raw file
# cbk -> saved code book 

#V=valgrind
H=../lzw
if [ ! -x $H ]; then echo "run make in parent directory first"; fi

dd if=/dev/urandom of=/tmp/rand bs=1 count=100000 >/dev/null 2>&1

echo "testing compressing with 4 dictionary sizes on various files"

for RAW in raw /usr/share/dict/words /tmp/rand
do
  echo -en "original file: "; ls -ltH $RAW

# encode/decode with various max #dictionary entries
  for D in 1000 10000 100000 999999
  do
    $V $H -e -i $RAW -o cwh -D $D
    $V $H -d -i  cwh -o bak -D $D
    diff $RAW bak
    echo -en "dictionary size $D:\t "; ls -lt cwh
  done
  echo  

done
