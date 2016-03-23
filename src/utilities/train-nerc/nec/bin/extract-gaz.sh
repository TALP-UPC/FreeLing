#!/bin/bash

lang=$1

if (test "x"$2 != "x"); then 
  rgaz=$2; 
else
  rgaz="none"
fi

echo "rgaz=$rgaz"

rm -rf gaz-$lang
mkdir gaz-$lang

## -- create test gazetters 

## extract all names in train corpus (to create test gazetteer)
cat corpus/$lang.train.nec | grep ' NP0' | cut -d' ' -f2- | sort | uniq -c | gawk '{print $2,$3,$1}' | gawk '{if ($1==ant) {lin=lin" "$2" "$3;} else {print ant,lin; lin=$2" "$3;} ant=$1} END {print ant,lin}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; print s,$0}' | sort -k 1 -n -r >tmp-gz
#fix category names
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-c.dat.test; done;
cat tmp-gz | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {for (x=3;x<=NF;x+=2) print $2>"gaz-"lg"/gaz"nom[$x]"-c.dat.test"}'
#extract gazetter of name parts
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-p.dat.test; done;
cat tmp-gz | gawk -v lg=$lang -f bin/extract-gaz-p.awk | sort | gawk '{if (ant==$1" "$2) s+=$3; else {print ant,s; s=$3}; ant=$1" "$2} END {print ant,s;}' | gawk '{if (ant==$1) lin=lin" "$2" "$3; else {print lin; lin=$0}; ant=$1} END {print lin;}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; n=(NF-1)/2; x=(float)s/((float)(n+1)); for (i=2;i<=NF;i+=2) if ($(i+1)+0>=x+0) print $1,$i,$(i+1); }' | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {print $1>"gaz-"lg"/gaz"nom[$2]"-p.dat.test"}'


if (test $rgaz == "none"); then ## we need to compute the ratio to match that of test set.

  # compute which ratio we need to discard to obtain during training the same coverage we have for test.
  goal=`cat corpus/$lang.test.nec | gawk 'BEGIN {while (getline<"tmp-gz") g[$2]=1} $3~/^NP/ {if (g[$2]) n++; nt++} END {print int(100*n/nt)}' `
  if (test $goal -lt 5); then goal=5; fi
  r1=0
  r2=100
  r=50
  stop=0
  while (test $stop == 0); do
  ## compute current coverage, using ratio r
      cat corpus/$lang.train.nec | grep ' NP0' | cut -d' ' -f2- | gawk -v rat=$r '{if (100*rand()<=rat) print}' | sort | uniq -c | gawk '{print $2,$3,$1}' | gawk '{if ($1==ant) {lin=lin" "$2" "$3;} else {print ant,lin; lin=$2" "$3;} ant=$1} END {print ant,lin}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; print s,$0}' | sort -k 1 -n -r >tmp-gz
      curr=`cat corpus/$lang.train.nec | gawk 'BEGIN {while (getline<"tmp-gz") g[$2]=1} $3~/^NP/ {if (g[$2]) n++; nt++} END {print int(100*n/nt)}' `
  ## dicotomic search
      if (test $curr -gt $(($goal+1)) ); then r2=$r
      elif (test $curr -lt $(($goal-1)) ); then r1=$r
      else stop=1
      fi
      r=$(( ($r1+$r2)/2 ))
      echo "Computed ratio="$r"  goal="$goal"  achieved="$curr
  done
  echo "Computed ratio="$r"  goal="$goal"  achieved="$curr

else ## fixed ratio given as parameter

    cat corpus/$lang.train.nec | grep ' NP0' | cut -d' ' -f2- | gawk -v rat=$rgaz '{if (100*rand()<=rat) print}' | sort | uniq -c | gawk '{print $2,$3,$1}' | gawk '{if ($1==ant) {lin=lin" "$2" "$3;} else {print ant,lin; lin=$2" "$3;} ant=$1} END {print ant,lin}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; print s,$0}' | sort -k 1 -n -r >tmp-gz
  
fi

## -- create train gazetters, extracting rgaz% of names in train corpus (to create train gazetteer)


#fix category names
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-c.dat.train; done;
cat tmp-gz | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {for (x=3;x<=NF;x+=2) print $2>"gaz-"lg"/gaz"nom[$x]"-c.dat.train"}'
#extract gazetter of name parts
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-p.dat.train; done;
cat tmp-gz | gawk -v lg=$lang -f bin/extract-gaz-p.awk | sort | gawk '{if (ant==$1" "$2) s+=$3; else {print ant,s; s=$3}; ant=$1" "$2} END {print ant,s;}' | gawk '{if (ant==$1) lin=lin" "$2" "$3; else {print lin; lin=$0}; ant=$1} END {print lin;}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; n=(NF-1)/2; x=(float)s/((float)(n+1)); for (i=2;i<=NF;i+=2) if ($(i+1)+0>=x+0) print $1,$i,$(i+1); }' | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {print $1>"gaz-"lg"/gaz"nom[$2]"-p.dat.train"}'
rm tmp-gz
