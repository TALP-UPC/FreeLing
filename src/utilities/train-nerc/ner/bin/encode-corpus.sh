#! /bin/bash

## encode all corpora for train or test.

## usage:
##   encode-corpus lang feat gaz-type
##
##  e.g.: 
##   encode-corpus en f43 rich.train
##   encode-corpus es f21 poor1.test
##

if (test $# -lt 3); then
    echo "usage:  encode-corpus lang feat gaz-type"
    exit
fi

# directory where this script is located
BINDIR=`readlink -f $(dirname $0)`
# parent directory, where the corpus should be
NERDIR=`dirname $BINDIR`
# main train-nerc directory
NERCDIR=`dirname $NERDIR`

if [[ ! -f $BINDIR/lexicon ]]; then
    echo -e "'$BINDIR/lexicon' binary not found.\nPlease use 'make' to compile binaries needed to encode the corpus"
    exit
fi

## Library path where FreeLing is found (e.g. /usr/local/lib
FL=`ldd $BINDIR/lexicon | grep freeling | grep 'not found' | wc -l`
if [[ $FL != 0 ]]; then
   echo "libfreeling not found in LD_LIBRARY_PATH. Please set LD_LIBRARY_PATH to include libfreeling location"    
   exit
fi

export LC_ALL=en_US.UTF-8

LG=$1
feat=$2
gz=$3
FEAT=$NERCDIR/$LG/nerc/ner

CORPUSDIR=$NERDIR/corpus
TRN=$NERDIR/trained-$LG
mkdir -p $TRN

# build lexicon, encode train corpus
if [[ -f $CORPUSDIR/$LG.$feat.$gz.enc ]]; then
  echo "** Encoded corpus "$CORPUSDIR/$LG.$feat.$gz.enc" already exists. Skipping "$gz" encoding."
else
  echo "** Encoding "$CORPUSDIR/$LG.$feat.$gz.enc" using ("$FEAT/ner-$feat.rgf","$TRN/ner-$feat")"
  if ( echo $gz | grep -q test ); then
     lex="-nolex"
     type="test"
  else
     lex=$TRN/ner-$feat-$gz
     type="train"
  fi

  cat $FEAT/ner-$feat.rgf | sed "s/\(gaz.*-[cp].dat\)/\1.$gz/" > $FEAT/tmp.ner-$feat-$gz.rgf
  cat $CORPUSDIR/$LG.$type.ner | $BINDIR/lexicon $FEAT/tmp.ner-$feat-$gz.rgf $lex >$CORPUSDIR/$LG.$feat.$gz.enc
  rm -f $FEAT/tmp.ner-$feat-$gz.rgf
fi

