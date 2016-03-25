#! /bin/bash

## encode all corpora for train or test.

## usage:
##   encode-corpus lang feat gaz-type
##
##  e.g.: 
##   encode-corpus en f43 rich.train
##   encode-corpus es f21 poor1.test
##

## Library path where FreeLing is found (e.g. /usr/local/lib
export LD_LIBRARY_PATH=/home/usuaris/tools/lib
export LC_ALL=en_US.UTF-8

## Directory where NER training scripts and data are found
cd /home/usuaris/padro/train-nerc/ner

LG=$1
feat=$2
gz=$3
FEAT=../$LG/nerc/ner

CORPUS=corpus
TRN=trained-$LG

# build lexicon, encode train corpus
if [[ -f $CORPUS/$LG.$feat.$gz.enc ]]; then
  echo "** Encoded corpus "$CORPUS/$LG.$feat.$gz.enc" already exists. Skipping "$gz" encoding."
else
  echo "** Encoding "$CORPUS/$LG.$feat.$gz.enc" using ("$FEAT/ner-$feat.rgf","$TRN/ner-$feat")"
  if ( echo $gz | grep -q test ); then
     lex="-nolex"
     type="test"
  else
     lex=$TRN/ner-$feat-$gz
     type="train"
  fi

  cat $FEAT/ner-$feat.rgf | sed "s/\(gaz.*-[cp].dat\)/\1.$gz/" > $FEAT/tmp.ner-$feat-$gz.rgf
  cat $CORPUS/$LG.$type.ner | bin/lexicon $FEAT/tmp.ner-$feat-$gz.rgf $lex >$CORPUS/$LG.$feat.$gz.enc
  rm -f $FEAT/tmp.ner-$feat-$gz.rgf
fi

