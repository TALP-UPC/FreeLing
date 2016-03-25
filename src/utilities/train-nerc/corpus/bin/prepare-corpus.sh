#! /bin/bash

# Prepare test and train corpus for NER and NEC

# usage:   prepare-corpus.sh "ca en es pt"

cd /home/usuaris/padro/train-nerc/corpus

for lg in $1; do
  echo ""
  echo "---- Preparing corpus for $lg ------------------"

  # format train corpus for ner and nec
  cat $lg.train.full | bin/full2ner.awk > ../ner/corpus/$lg.train.ner
  cat $lg.train.full | bin/full2nec.awk > ../nec/corpus/$lg.train.nec

  # format test corpus for ner and nec. 
  cat $lg.test.full | bin/full2ner.awk | tee ../ner/corpus/$lg.test.ner | bin/ner2gold.awk > ../ner/corpus/$lg.ner.gold
  cat $lg.test.full | bin/full2nec.awk | tee ../nec/corpus/$lg.test.nec | bin/nec2gold.awk > ../nec/corpus/$lg.nec.gold

  # extract gazeteers for train and test
  bin/extract-gaz.sh $lg 1
  mv gaz-$lg/gaz* ../$lg/nerc/data/
  rmdir gaz-$lg
done

