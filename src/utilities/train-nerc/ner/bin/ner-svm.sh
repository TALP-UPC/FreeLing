#! /bin/bash

##################################################################################
## ner-svm.sh
##
## ./ner-svm.sh lang feature filters Cparam
## 
## Trains and tests SVM NER models for given language, using given feature rule sets, 
## and given lexicons, and with given values for C param.
##
##  E.g
##         ./ner-svm.sh es f42 1abs 10
##         ./ner-svm.sh es "f42 f43" "1abs 3abs" "1 10 100"
## 
##  See ../../README for more details

#### TODO TODO TODO
#### add rich/poor gazetter as parameter to allow different trainings.

if (test $# -lt 4); then
    echo "usage:  ner-svm.sh lang feature filter Cparam"
    exit
fi

# directory where this script is located
BINDIR=`readlink -f $(dirname $0)`
# parent directory, where the corpus should be
NERDIR=`dirname $BINDIR`
# main train-nerc directory
NERCDIR=`dirname $NERDIR`

if [[ ! -f $BINDIR/test-svm ]]; then
    echo -e "'$BINDIR/test-svm' binary not found.\nPlease use 'make' to compile binaries needed to encode the corpus"
    exit
fi

## Library path where FreeLing is found (e.g. /usr/local/lib
FL=`ldd $BINDIR/test-svm | grep freeling | grep 'not found' | wc -l`
if [[ $FL != 0 ]]; then
   echo "libfreeling not found in LD_LIBRARY_PATH. Please set LD_LIBRARY_PATH to include libfreeling location"    
   exit
fi

export LC_ALL=en_US.UTF-8

LG=$1

FEAT=$NERCDIR/$LG/ner
CORPUS=$NERDIR/corpus
TRN=$NERDIR/trained-$LG
RES=$NERDIR/results-$LG

mkdir -p $TRN/svm
mkdir -p $RES/svm

for feat in $2; do

    echo "============================================"
    echo "Features: "$feat" -- "`date`
    echo "============================================"

    ## train and test each model, both with AdaBoost and SVMs
    ## for fl in 0.01pc 0.005pc 0.001pc 0.0005pc 0.0001pc 5abs 3abs 1abs all; do
    for fl in $3; do

       echo "------- Filter "$fl" -------"

       ###### train SVMs 
       for C in $4; do
         x=`ls -1 $TRN/svm/ner-$feat-$fl-C$C.dat 2>/dev/null | wc -l`
         if [[ "x$x" != "x0" ]]; then
  	   echo "** Trained SVM models " $TRN/svm/ner-$feat-$fl-C$C" already exists. Skipping model training."
         else
           echo "** Training SVM model "$TRN/svm/ner-$feat-C$C" with filter "$fl
           $BINDIR/train-svm.sh $TRN/ner-$feat-rich.train-$fl.lex "0 B 1 I 2 O" $TRN/svm/ner-$feat-$fl $CORPUS/$LG.$feat.rich.train.enc $C

   	   ## if model re-trained, make sure that test is repeated
           rm -f $RES/svm/ner-$feat-$fl-C$C.out
         fi
       done
 
       ###### test SVMs
       for C in $4; do
         x=`ls -1 $RES/svm/ner-$feat-$fl-C$C.out 2>/dev/null | wc -l`
         if [[ "x$x" != "x0" ]]; then
    	   echo "** Results for SVM models "ner-$feat-$fl-C$C" already exist. Skipping test."
         else
           echo "** Testing SVM model "$TRN/svm/ner-$feat-$fl-C$C.dat
           $BINDIR/test-svm $TRN/svm/ner-$feat-$fl-C$C.dat < $CORPUS/$LG.$feat.rich.test.enc >$RES/svm/ner-$feat-$fl-C$C.out

  	   ## if test re-executed, make sure that statistics are recomputed
           rm -f $RES/svm/ner-$feat-$fl-C$C.stats
         fi
       done

       ###### stats for SVMs

       for C in $4; do
          if [[ -f $RES/svm/ner-$feat-$fl-C$C.stats ]]; then
  	    echo "** Statistics for SVM model "ner-$feat-$fl-C$C" already exist. Skipping statistics."
          else 
            echo "** Computing Statistics for SVM model "$TRN/svm/ner-$feat-$fl-C$C.dat
     	    paste -d' ' $CORPUS/$LG.ner.gold $RES/svm/ner-$feat-$fl-C$C.out | $BINDIR/conlleval.pl | gawk '{if($1=="accuracy:") print $8}' > $RES/svm/ner-$feat-$fl-C$C.stats
          fi
       done

    done
done
