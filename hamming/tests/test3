#!/bin/bash

# exit immediately if a command fails
set -e

DICT=/usr/share/dict/words
RAWFILE=/tmp/raw.$$;
ECCFILE=/tmp/ecc.$$;
DECFILE=/tmp/dec.$$;
NOSFILE=/tmp/nos.$$;
ECC=../ecc

if [ ! -x ${ECC} ]; then echo "run \"make\" first, then try again"; exit -1; fi
if [ ! -f ${DICT} ]; then echo "edit this script to redefine DICT"; exit -1; fi
head -100 ${DICT} > ${RAWFILE}

echo encoding...
${ECC} -i ${RAWFILE} -o ${ECCFILE}
echo adding noise...
${ECC} -n -i ${ECCFILE} -o ${NOSFILE}
echo decoding noisy file...
${ECC} -d -i ${NOSFILE} -o ${DECFILE}
echo resulting files:
ls -lt ${RAWFILE} ${ECCFILE} ${NOSFILE} ${DECFILE}
echo diffing...
diff -q ${RAWFILE} ${DECFILE}
if [ $? -eq 0 ]; then rm ${RAWFILE} ${ECCFILE} ${NOSFILE} ${DECFILE}; fi
echo done
