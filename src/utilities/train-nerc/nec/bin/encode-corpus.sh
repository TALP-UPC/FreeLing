#! /bin/bash

## encode all corpora for train and test.

## usage:
##   encode-corpus lang feat gaz-type
##
##  e.g.: 
##   encode-corpus en f43 rich.train
##   encode-corpus es f21 poor1.test
##

## Library path where FreeLing and boost are found (e.g. /usr/local/lib)
export LD_LIBRARY_PATH=/home/usuaris/tools/lib
export LC_ALL=en_US.UTF-8

## Directory where NEC training scripts and data are found
cd /home/usuaris/padro/train-nerc/nec

LG=$1
feat=$2
gz=$3
FEAT=../$LG/nerc/nec

CORPUS=corpus
TRN=trained-$LG

# build lexicon, encode corpus
if [[ -f $CORPUS/$LG.$feat.$gz.enc ]]; then
  echo "** Encoded "$gz" corpus "$CORPUS/$LG.$feat.$gz.enc" already exists. Skipping "$gz" encoding."
else
  echo "** Encoding "$gz" "$CORPUS/$LG.$feat.$gz.enc" using ("$FEAT/nec-$feat.rgf","$TRN/nec-$feat")"
  if ( echo $gz | grep -q test ); then
    lex="-nolex"
    type="test"
  else
    lex=$TRN/nec-$feat-$gz
    type="train"
  fi

  cat $FEAT/nec-$feat.rgf | sed "s/\(gaz.*-[cp].dat\)/\1.$gz/" > $FEAT/tmp.nec-$feat-$gz.rgf
  cat $CORPUS/$LG.$type.nec | bin/lexicon $FEAT/tmp.nec-$feat-$gz.rgf $lex >$CORPUS/$LG.$feat.$gz.enc
  rm -f $FEAT/tmp.nec-$feat-$gz.rgf
fi

