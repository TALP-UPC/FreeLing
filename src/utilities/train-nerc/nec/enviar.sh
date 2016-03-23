#! /bin/bash

## $1: list of language codes
## $2: list of feature sets
## $3: list of train gazetters (rich, poor1, ...)
## $4: list of test gazetters (rich, poor1, ...)

## example usages:
##
##   enviar.sh ca f35 rich rich
##   enviar.sh "ca es" "f35 f37" "rich poor1" "rich poor1"

for lang in $1; do
  for feat in $2; do
    for fl in 1abs 3abs 5abs; do
       for gz in $3; do
         for gzts in $4; do
           qsub -l so_rel=12.04 -l h_rt=400:0:0 -l h_vmem=512M -q eixam bin/nec-adaboost.sh $lang $feat $fl $gz $gzts
	 done
      done
    done
  done
done

# nec-svm
#    qsub -l so_rel=12.04 -l h_rt=400:0:0 -q eixam bin/nec-svm.sh $lang $feat $fl 1500
#    qsub -l so_rel=12.04 -l h_rt=400:0:0 -q eixam bin/nec-svm.sh $lang $feat $fl 1600
#    qsub -l so_rel=12.04 -l h_rt=400:0:0 -q eixam bin/nec-svm.sh $lang $feat $fl 1700
#    qsub -l so_rel=12.04 -l h_rt=400:0:0 -q eixam bin/nec-svm.sh $lang $feat $fl 1800
#    qsub -l so_rel=12.04 -l h_rt=400:0:0 -q eixam bin/nec-svm.sh $lang $feat $fl 1900
#    qsub -l so_rel=12.04 -l h_rt=400:0:0 -q eixam bin/nec-svm.sh $lang $feat $fl 2100
#    qsub -l so_rel=12.04 -l h_rt=400:0:0 -q eixam bin/nec-svm.sh $lang $feat $fl 2300

# comencar despres de certa data
# qsub -l h_rt=400:0:0 -a 04210005 -q eixam bin/ner-adaboost.sh $lang $feat 0.001pc

# nec-adaboost
# qsub -l h_rt=400:0:0 -q eixam bin/nec-adaboost.sh $lang $feat 0.01pc
# qsub -l h_rt=400:0:0 -q eixam bin/nec-adaboost.sh $lang $feat 0.005pc
# qsub -l h_rt=400:0:0 -q eixam bin/nec-adaboost.sh $lang $feat 0.001pc

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.005pc "0.01 0.1 1.0 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.005pc "10 100 5000"

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.001pc "0.01 1.0 100"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.001pc "0.1 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.001pc "10 5000"

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0005pc "0.01 0.1 1.0 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0005pc "10 100 5000"

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0001pc "0.01 0.1 1.0 10 100 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0001pc "5000"


