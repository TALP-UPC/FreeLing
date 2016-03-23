#! /bin/bash

FLDIR=/home/usuaris/tools

for corpus in en.test en.train; do

 echo $corpus
 
 # analyze corpus with FreeLing
 zcat $corpus.gz | $FLDIR/bin/analyze -f en.cfg --inpf splitted --outf morfo --noprob --noquant --noner > analysis.tmp

 # store correct BIO tags without NE classification
 #zcat $corpus.gz | gawk '{if ($0!="") {split($4, BIO, "-"); print $1, BIO[1]} else print}' > BIO.tmp
 zcat $corpus.gz | gawk '{if ($0!="") {print $1,$2} else print}' > BIO.tmp
 
 # join BIO tags with analysis.tmp. Since some words have been joined as locutions, we have
 #  to look word by word. If the word has a "_", it is a locution

 cat BIO.tmp  | gawk -f fix-en.awk > $corpus.analyzed

# rm *.tmp

done
