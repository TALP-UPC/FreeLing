#!  /bin/bash

## usage:   learn-curve.sh lang feat train-gaz
##
##    e.g.   learn-curve.sh es f42 rich

lg=$1
f=$2
gz=$3

RES=results-$1

for feat in $2; do

  cd $RES/adaboost

  echo "set key right bottom" > resum-$feat.gp
  echo "set terminal png" >> resum-$feat.gp
  echo "set yrange [84:94]" >> resum-$feat.gp
  f=`ls -1 *-$feat-$gz-*.stats`
  echo "set output \"results"-$feat".png\"" >> resum-$feat.gp
  echo $f | gawk '{printf "plot \""$1"\" with linespoints"; for (i=2;i<=NF;i++) printf ", \""$i"\" with linespoints";} END {print ""}' >> resum-$feat.gp
  gnuplot resum-$feat.gp

  cd ..
end

