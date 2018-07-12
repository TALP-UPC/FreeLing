#! /bin/bash

# Prepare test and train corpus for NER and NEC

# usage:   prepare-corpus.sh "ca en es pt"

# directory where this script is located
BINDIR=`readlink -f $(dirname $0)`
# parent directory, where the corpus should be
CORPUSDIR=`dirname $BINDIR`
# main train-nerc directory
NERCDIR=`dirname $CORPUSDIR`

# create output directories
mkdir -p $NERCDIR/ner/corpus
mkdir -p $NERCDIR/nec/corpus

for lg in $1; do
  echo ""
  echo "---- Preparing corpus for $lg ------------------"
  
  # format train corpus for ner and nec
  cat $CORPUSDIR/$lg.train.full | $BINDIR/full2ner.awk > $NERCDIR/ner/corpus/$lg.train.ner
  cat $CORPUSDIR/$lg.train.full | $BINDIR/full2nec.awk > $NERCDIR/nec/corpus/$lg.train.nec

  # format test corpus for ner and nec. 
  cat $CORPUSDIR/$lg.test.full | $BINDIR/full2ner.awk | tee $NERCDIR/ner/corpus/$lg.test.ner | $BINDIR/ner2gold.awk > $NERCDIR/ner/corpus/$lg.ner.gold
  cat $CORPUSDIR/$lg.test.full | $BINDIR/full2nec.awk | tee $NERCDIR/nec/corpus/$lg.test.nec | $BINDIR/nec2gold.awk > $NERCDIR/nec/corpus/$lg.nec.gold

  # extract gazeteers for train and test
  $BINDIR/extract-gaz.sh $lg 1
  mv gaz-$lg/gaz* $NERCDIR/$lg/nerc/data/
  rmdir gaz-$lg
done

