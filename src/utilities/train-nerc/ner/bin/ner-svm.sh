#! /bin/bash

##################################################################################
## ner-svm.sh
##
## ./ner-svm.sh lang feature lexicons Cparam
## 
## Trains and tests SVM NER models for given language, using given feature rule sets, 
## and given lexicons, and with given values for C param.
##
##  E.g
##         ./ner-svm.sh es f42 1abs 10
##         ./ner-svm.sh es "f42 f43" "1abs 3abs" "1 10 100"
## 
##  See ../../README for more details


## Change this to te location where libfreeling is installed
export LD_LIBRARY_PATH=/usr/local/lib
export LC_ALL=en_US.UTF-8

cd $(dirname $0)/..

LG=$1

FEAT=../$LG/ner

CORPUS=corpus

TRN=trained-$LG
RES=results-$LG

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
           bin/train-svm.sh $TRN/ner-$feat-$fl.lex "0 B 1 I 2 O" $TRN/svm/ner-$feat-$fl $CORPUS/$LG.train.$feat.enc $C

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
           bin/test-svm $TRN/svm/ner-$feat-$fl-C$C.dat < $CORPUS/$LG.test.$feat.enc >$RES/svm/ner-$feat-$fl-C$C.out

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
     	    paste -d' ' $CORPUS/$LG.ner.gold $RES/svm/ner-$feat-$fl-C$C.out | ./bin/conlleval.pl | gawk '{if($1=="accuracy:") print $8}' > $RES/svm/ner-$feat-$fl-C$C.stats
          fi
       done

    done
done
