#!/bin/bash

##################################################################################
## nec-adaboost.sh
##
## ./nec-adaboost.sh lang feature lexicons
## 
## Trains and tests AdaBoost NEC models for given language, using given feature rule sets, 
## and given lexicons
##
##  E.g
##         ./nec-adaboost.sh es f42 1abs rich rich
## 
##  See ../../README for more details

## Change this to te location where libfreeling is installed
export LD_LIBRARY_PATH=/home/usuaris/tools/lib
export LC_ALL=en_US.UTF-8

cd /home/usuaris/padro/train-nerc/nec

LG=$1
FEAT=../$LG/nec
feat=$2
fl=$3
gz=$4
gzts=$5

CORP=corpus

TRN=trained-$LG
RES=results-$LG

STEP=20


echo "============================================"
echo "Features: "$feat" -- "`date`
echo "------- Filter "$fl" ------- Gaz "$gz" ---------"

###### train adaboost
if [[ -f $TRN/adaboost/nec-$feat-$gz-$fl.abm ]]; then
  echo "** Trained AB model " $TRN/adaboost/nec-$feat-$gz-$fl.abm" already exists. Skipping model training."
else
  echo "Training with filter $fl, gaz $gz, class NP00V00"
  bin/train-adaboost $TRN/nec-$feat-$gz.train-$fl.lex $TRN/adaboost/nec-$feat-$gz-$fl.abm "0 NP00SP0 1 NP00G00 2 NP00O00 3 NP00V00" <$CORP/$LG.$feat.$gz.train.enc >$TRN/adaboost/nec-$feat-$gz-$fl.dat

  echo "Training with filter $fl, gaz $gz, class <others>"
  bin/train-adaboost $TRN/nec-$feat-$gz.train-$fl.lex $TRN/adaboost/nec-$feat-$gz-$fl-def.abm "0 NP00SP0 1 NP00G00 2 NP00O00 <others> NP00V00" <$CORP/$LG.$feat.$gz.train.enc >$TRN/adaboost/nec-$feat-$gz-$fl-def.dat

  ## if model re-trained, make sure that tests are repeated
  rm -f $RES/adaboost/nec-$feat-$gz.$gzts-$fl.out
fi
 
###### test adaboost
if [[ -f $RES/adaboost/nec-$feat-$gz.$gzts-$fl.out ]]; then
  echo "** Results for AB model "nec-$feat-$gz-$fl" with test "$gzts" already exist. Skipping test."
else
  echo "** Testing AB model "$TRN/adaboost/nec-$feat-$gz-$fl" with test "$gzts
  bin/test-adaboost $TRN/adaboost/nec-$feat-$gz-$fl.dat $STEP < $CORP/$LG.$feat.$gzts.test.enc >$RES/adaboost/nec-$feat-$gz.$gzts-$fl.out
  bin/test-adaboost $TRN/adaboost/nec-$feat-$gz-$fl-def.dat $STEP < $CORP/$LG.$feat.$gzts.test.enc >$RES/adaboost/nec-$feat-$gz.$gzts-$fl-def.out
  ## if test re-executed, make sure that statistics are recomputed
  rm -f $RES/adaboost/nec-$feat-$gz.$gzts-$fl.stats
fi

###### stats for adaboost
if [[ -f $RES/adaboost/nec-$feat-$gz.$gzts-$fl.stats ]]; then
  echo "** Statistics for AB model "nec-$feat-$gz-$fl" with test "$gzts" already exist. Skipping statistics."
else
  echo "** Computing statistics for AB model "nec-$feat-$gz-$fl" with test "$gzts
  nrul=`grep -c '\-\-\-' $TRN/adaboost/nec-$feat-$gz-$fl.abm`
  nstp=`head -1 $RES/adaboost/nec-$feat-$gz.$gzts-$fl.out | wc -w`
  r=$STEP
  rm -f $RES/adaboost/nec-$feat-$gz.$gzts-$fl.stats
  rm -f $RES/adaboost/nec-$feat-$gz.$gzts-$fl-def.stats
  for ((st=1; st<=$nstp; st++)); do
    printf "%s " $r >> $RES/adaboost/nec-$feat-$gz.$gzts-$fl.stats
    printf "%s " $r >> $RES/adaboost/nec-$feat-$gz.$gzts-$fl-def.stats

    cut -d' ' -f$st <$RES/adaboost/nec-$feat-$gz.$gzts-$fl.out | paste -d' ' $CORP/$LG.nec.gold - | gawk '{if ($2==$3) nok++;} END {print 100*nok/NR}' >> $RES/adaboost/nec-$feat-$gz.$gzts-$fl.stats
    cut -d' ' -f$st <$RES/adaboost/nec-$feat-$gz.$gzts-$fl-def.out | paste -d' ' $CORP/$LG.nec.gold - | gawk '{if ($2==$3) nok++;} END {print 100*nok/NR}' >> $RES/adaboost/nec-$feat-$gz.$gzts-$fl-def.stats
	      
    let r=r+STEP
  done
fi

