#!/bin/bash
#set -x

# raw -> test input file
# cwh -> compressed with header
# cwo -> compressed without header (requires code book)
# bak -> should match the raw file
# cbk -> saved code book 

#V=valgrind
H=../lzw
if [ ! -x $H ]; then echo "run make in parent directory first"; exit -1; fi

echo -n "generating random data... "; 
dd if=/dev/urandom of=/tmp/rand bs=1 count=1000000 >/dev/null 2>&1
echo "done."; echo

for RAW in raw /usr/share/dict/words /tmp/rand
do
  echo -en "original file: "; ls -ltH $RAW

# encode/decode with various max #dictionary entries
  for D in 512 4096 8192 16384 100000 999999
  do
    $V $H -e -i $RAW -o cwh -D $D
    $V $H -d -i  cwh -o bak -D $D
    diff $RAW bak
    echo -en "dictionary size $D:\t "; ls -lt cwh
  done
  echo  

done
