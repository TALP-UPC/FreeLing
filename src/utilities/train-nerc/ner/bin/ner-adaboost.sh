#! /bin/bash

##################################################################################
## ner-adaboost.sh
##
## ./ner-adaboost.sh lang feature filter train-lexicon test-lexicon
## 
## Trains and tests AdaBoost NER models for given language, using given feature rule sets, 
## and given lexicons
##
##  E.g
##         ./ner-adaboost.sh es f42 1abs rich rich
## 
## 
##  See ../../README for more details

## Change this to the location where libfreeling is installed

if (test $# -lt 5); then
    echo "usage: ./ner-adaboost.sh lang feature filter train-lexicon test-lexicon"
    exit
fi

# directory where this script is located
BINDIR=`readlink -f $(dirname $0)`
# parent directory, where the corpus should be
NERDIR=`dirname $BINDIR`
# main train-nerc directory
NERCDIR=`dirname $NERDIR`

if [[ ! -f $BINDIR/train-adaboost ]]; then
    echo -e "'$BINDIR/train-adaboost' binary not found.\nPlease use 'make' to compile binaries needed to encode the corpus"
    exit
fi

## Library path where FreeLing is found (e.g. /usr/local/lib
FL=`ldd $BINDIR/train-adaboost | grep freeling | grep 'not found' | wc -l`
if [[ $FL != 0 ]]; then
   echo "libfreeling not found in LD_LIBRARY_PATH. Please set LD_LIBRARY_PATH to include libfreeling location"    
   exit
fi

export LC_ALL=en_US.UTF-8

LG=$1
FEAT=$NERCDIR/$LG/ner
feat=$2
fl=$3
gz=$4
gzts=$5

CORPUS=$NERDIR/corpus

TRN=$NERDIR/trained-$LG
RES=$NERDIR/results-$LG

mkdir -p $TRN/adaboost
mkdir -p $RES/adaboost

STEP=20

echo "============================================"
echo "Features: $feat -- "`date`
echo "------- Filter $fl ------- Gaz $gz --------"

###### train adaboost
if [[ -f $TRN/adaboost/ner-$feat-$gz-$fl.abm ]]; then
    echo "** Trained AB model " $TRN/adaboost/ner-$feat-$gz-$fl.abm" already exists. Skipping model training."
else
    echo "** Training AB model "$TRN/adaboost/ner-$feat-$gz-$fl
    $BINDIR/train-adaboost $TRN/ner-$feat-$gz.train-$fl.lex $TRN/adaboost/ner-$feat-$gz-$fl.abm "0 B 1 I 2 O" <$CORPUS/$LG.$feat.$gz.train.enc >$TRN/adaboost/ner-$feat-$gz-$fl.dat
    $BINDIR/train-viterbi "0 B 1 I 2 O" <$CORPUS/$LG.$feat.$gz.train.enc >>$TRN/adaboost/ner-$feat-$gz-$fl.dat
    ## if model re-trained, make sure that test is repeated
    rm -f $RES/adaboost/ner-$feat-$gz.$gzts-$fl.out
fi

###### test adaboost
if [[ -f $RES/adaboost/ner-$feat-$gz.$gzts-$fl.out ]]; then
    echo "** Results for AB model "ner-$feat-$gz-$fl" with test "$gzts" already exist. Skipping test."
else
    echo "** Testing AB model "$TRN/adaboost/ner-$feat-$gz-$fl" with test "$gzts
    $BINDIR/test-adaboost $TRN/adaboost/ner-$feat-$gz-$fl.dat $STEP < $CORPUS/$LG.$feat.$gzts.test.enc >$RES/adaboost/ner-$feat-$gz.$gzts-$fl.out
    ## if test re-executed, make sure that statistics are recomputed
    rm -f $RES/adaboost/ner-$feat-$gz.gzts-$fl.stats
fi

###### stats for adaboost
if [[ -f $RES/adaboost/ner-$feat-$gz.$gzts-$fl.stats ]]; then
    echo "** Statistics for AB model "ner-$feat-$gz-$fl" with test "$gzts" already exist. Skipping statistics."
else
    echo "** Computing statistics for AB model "ner-$feat-$gz-$fl" with test "$gzts
    nrul=`grep -c '\-\-\-' $TRN/adaboost/ner-$feat-$gz-$fl.abm`
    nstp=`head -1 $RES/adaboost/ner-$feat-$gz.$gzts-$fl.out | wc -w`
    r=$STEP
    for ((st=1; st<=$nstp; st++)); do
        printf "%s " $r >> $RES/adaboost/ner-$feat-$gz.$gzts-$fl.stats
	
	cut -d' ' -f$st <$RES/adaboost/ner-$feat-$gz.$gzts-$fl.out | paste -d' ' $CORPUS/$LG.ner.gold - | gawk '{if ($0=="  ") $0=""; print;}' | $BINDIR/conlleval.pl | gawk '{if($1=="accuracy:") print $8}' >> $RES/adaboost/ner-$feat-$gz.$gzts-$fl.stats
	
        let r=r+STEP
    done
fi

