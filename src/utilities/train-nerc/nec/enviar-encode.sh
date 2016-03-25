#! /bin/bash

## $1: list of language codes
## $2: list of feature sets
## $3: list of gazetters (test, rich.train, poor1.train, ...)

## example usages:
##
##   enviar-encode.sh ca f35 test
##   enviar-encode.sh "ca es" "f35 f37" "rich.test poor1.test rich.train poor1.train"

# encode requested (test/train) corpus

for lang in $1; do
  for feat in $2; do
    for gz in $3; do
      qsub -l so_rel=12.04 -l h_vmem=512M -q eixam bin/encode-corpus.sh $lang $feat $gz
    done
  done
done





