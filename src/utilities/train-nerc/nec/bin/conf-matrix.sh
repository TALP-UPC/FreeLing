#! /bin/bash

## usage:  conf-matrix.sh lang model
##
##       eg: conf-matrix.sh es f01-poor1.rich-1abs

lg=$1
x=$2

cat results-$lg/adaboost/nec-$x.out | cut -d' ' -f20 | paste corpus/$lg.nec.gold - | gawk '{ s[$2]++; t[$2"-"$3]++ } END  {for (x in s) { printf "%s %4d : ",x,s[x]; for (y in s) {printf " %s-%4d(%2d)",y,t[x"-"y],100*t[x"-"y]/s[x];} printf "\n" } }'
