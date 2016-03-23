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
    for fl in 1abs 3abs 5abs 9abs; do
       for gz in $3; do
         for gzts in $4; do
           qsub -l so_rel=12.04 -l h_rt=400:0:0 -l h_vmem=8G -q eixam bin/ner-adaboost.sh $lang $feat $fl $gz $gzts
	 done
      done
    done
  done
done

## baixa prio
##    qsub -p -1 -l so_rel=12.04 -l h_rt=400:0:0 -l h_vmem=8G -q eixam bin/ner-adaboost.sh $lang $feat $fl $gz $gzts

### qsub -l h_rt=400:0:0 -a 04210005 -q eixam bin/ner-adaboost.sh $lang $feat 0.001pc


#    qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 90000
#    qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 50000

    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 0.1
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 1
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 10
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 100
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 1000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 10000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 6000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 7000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 8000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 9000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 11000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 12000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 13000
    #qsub -l h_rt=400:0:0  -q eixam bin/ner-svm.sh $lang $feat $fl 14000

# qsub -l h_rt=400:0:0  -q eixam bin/ner-adaboost.sh $lang $feat 0.001pc
# qsub -l h_rt=400:0:0  -q eixam bin/ner-adaboost.sh $lang $feat 0.0005pc
# qsub -l h_rt=400:0:0  -q eixam bin/ner-adaboost.sh $lang $feat 0.0001pc
# qsub -l h_rt=400:0:0  -q eixam bin/ner-adaboost.sh $lang $feat 5abs
# qsub -l h_rt=400:0:0  -q eixam bin/ner-adaboost.sh $lang $feat 3abs
# qsub -l h_rt=400:0:0  -q eixam bin/ner-adaboost.sh $lang $feat 1abs
# qsub -l h_rt=400:0:0  -q eixam bin/ner-adaboost.sh $lang $feat all

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.005pc "0.01 0.1 1.0 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.005pc "10 100 5000"

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.001pc "0.01 1.0 100"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.001pc "0.1 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.001pc "10 5000"

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0005pc "0.01 0.1 1.0 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0005pc "10 100 5000"

# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0001pc "0.01 0.1 1.0 10 100 1000"
# qsub -l h_rt=400:0:0 -q eixam ./experiments-ner.sh $feat 0.0001pc "5000"

