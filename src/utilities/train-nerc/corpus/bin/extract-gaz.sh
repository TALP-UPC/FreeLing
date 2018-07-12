#!/bin/bash

lang=$1
rgaz=$2; 

# directory where this script is located
BINDIR=`readlink -f $(dirname $0)`
# parent directory, where the corpus should be
CORPUSDIR=`dirname $BINDIR`
# main train-nerc directory
NERCDIR=`dirname $CORPUSDIR`

rm -rf gaz-$lang
mkdir gaz-$lang

## -- create test gazetters 

echo "Extracting rich test gazetteer"
## extract all names in train corpus (to create rich.test gazetteer)
cat $NERCDIR/nec/corpus/$lang.train.nec | grep ' NP0' | cut -d' ' -f2- | sort | uniq -c | gawk '{print $2,$3,$1}' | gawk '{if ($1==ant) {lin=lin" "$2" "$3;} else {print ant,lin; lin=$2" "$3;} ant=$1} END {print ant,lin}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; print s,$0}' | sort -k 1 -n -r >tmp-gz
#fix category names
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-c.dat.rich.test; done;
cat tmp-gz | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {for (x=3;x<=NF;x+=2) print $2>"gaz-"lg"/gaz"nom[$x]"-c.dat.rich.test"}'
#extract gazetter of name parts
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-p.dat.rich.test; done;
cat tmp-gz | gawk -v lg=$lang -f $BINDIR/extract-gaz-p.awk | sort | gawk '{if (ant==$1" "$2) s+=$3; else {print ant,s; s=$3}; ant=$1" "$2} END {print ant,s;}' | gawk '{if (ant==$1) lin=lin" "$2" "$3; else {print lin; lin=$0}; ant=$1} END {print lin;}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; n=(NF-1)/2; x=(float)s/((float)(n+1)); for (i=2;i<=NF;i+=2) if ($(i+1)+0>=x+0) print $1,$i,$(i+1); }' | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {print $1>"gaz-"lg"/gaz"nom[$2]"-p.dat.rich.test"}'


## ------- Training gazetteer.  Rich: optimal ratio, matching that of test set. ---------

echo "Computing ratio for rich train gazetteer"
# compute which ratio we need to discard to obtain during training the same coverage we have for rich.test.
goal=`cat $NERCDIR/nec/corpus/$lang.test.nec | gawk 'BEGIN {while (getline<"tmp-gz") g[$2]=1} $3~/^NP/ {if (g[$2]) n++; nt++} END {print int(100*n/nt)}' `
if (test $goal -lt 5); then goal=5; fi
r1=0
r2=100
r=50
stop=0
while (test $stop == 0); do
  ## compute current coverage, using ratio r
    cat $NERCDIR/nec/corpus/$lang.train.nec | grep ' NP0' | cut -d' ' -f2- | gawk -v rat=$r '{if (100*rand()<=rat) print}' | sort | uniq -c | gawk '{print $2,$3,$1}' | gawk '{if ($1==ant) {lin=lin" "$2" "$3;} else {print ant,lin; lin=$2" "$3;} ant=$1} END {print ant,lin}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; print s,$0}' | sort -k 1 -n -r >tmp-gz
    curr=`cat $NERCDIR/nec/corpus/$lang.train.nec | gawk 'BEGIN {while (getline<"tmp-gz") g[$2]=1} $3~/^NP/ {if (g[$2]) n++; nt++} END {print int(100*n/nt)}' `
  ## dicotomic search
    if (test $curr -gt $(($goal+1)) ); then r2=$r
    elif (test $curr -lt $(($goal-1)) ); then r1=$r
    else stop=1
    fi
    r=$(( ($r1+$r2)/2 ))
    echo "   Computed ratio="$r"  goal="$goal"  achieved="$curr
done

echo "Extracting rich train gazetteer"
#fix category names
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-c.dat.rich.train; done;
cat tmp-gz | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {for (x=3;x<=NF;x+=2) print $2>"gaz-"lg"/gaz"nom[$x]"-c.dat.rich.train"}'
#extract gazetter of name parts
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-p.dat.rich.train; done;
cat tmp-gz | gawk -v lg=$lang -f $BINDIR/extract-gaz-p.awk | sort | gawk '{if (ant==$1" "$2) s+=$3; else {print ant,s; s=$3}; ant=$1" "$2} END {print ant,s;}' | gawk '{if (ant==$1) lin=lin" "$2" "$3; else {print lin; lin=$0}; ant=$1} END {print lin;}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; n=(NF-1)/2; x=(float)s/((float)(n+1)); for (i=2;i<=NF;i+=2) if ($(i+1)+0>=x+0) print $1,$i,$(i+1); }' | gawk -v lg=$lang 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {print $1>"gaz-"lg"/gaz"nom[$2]"-p.dat.rich.train"}'


## ----- POOR Gazetteer. fixed ratio given as parameter "rgaz"
echo "Extracting poor.$rgaz train gazetteer"

## -- create train gazetters, extracting rgaz% of names in train corpus (to create train gazetteer)
cat $NERCDIR/nec/corpus/$lang.train.nec | grep ' NP0' | cut -d' ' -f2- | gawk -v rat=$rgaz '{if (100*rand()<=rat) print}' | sort | uniq -c | gawk '{print $2,$3,$1}' | gawk '{if ($1==ant) {lin=lin" "$2" "$3;} else {print ant,lin; lin=$2" "$3;} ant=$1} END {print ant,lin}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; print s,$0}' | sort -k 1 -n -r >tmp-gz

## inform of coverage provided by given ratio
curr=`cat $NERCDIR/nec/corpus/$lang.train.nec | gawk 'BEGIN {while (getline<"tmp-gz") g[$2]=1} $3~/^NP/ {if (g[$2]) n++; nt++} END {print int(100*n/nt)}' `
echo "Poor.$rgaz gazetteer provides a $curr% coverage on training set"

#fix category names
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-c.dat.poor$rgaz.train; done;
cat tmp-gz | gawk -v lg=$lang -v r=$rgaz 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {for (x=3;x<=NF;x+=2) print $2>"gaz-"lg"/gaz"nom[$x]"-c.dat.poor"r".train"}'
#extract gazetter of name parts
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-p.dat.poor$rgaz.train; done;
cat tmp-gz | gawk -v lg=$lang -f $BINDIR/extract-gaz-p.awk | sort | gawk '{if (ant==$1" "$2) s+=$3; else {print ant,s; s=$3}; ant=$1" "$2} END {print ant,s;}' | gawk '{if (ant==$1) lin=lin" "$2" "$3; else {print lin; lin=$0}; ant=$1} END {print lin;}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; n=(NF-1)/2; x=(float)s/((float)(n+1)); for (i=2;i<=NF;i+=2) if ($(i+1)+0>=x+0) print $1,$i,$(i+1); }' | gawk -v lg=$lang -v r=$rgaz 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {print $1>"gaz-"lg"/gaz"nom[$2]"-p.dat.poor"r".train"}'

echo "Computing ratio for poor.$rgaz test gazetteer"
goal=$curr
if (test $goal -lt 5); then goal=5; fi
r1=0
r2=100
r=50
rant=0
stop=0
while (test $stop == 0); do
  ## compute current coverage on test, using ratio r
    cat $NERCDIR/nec/corpus/$lang.train.nec | grep ' NP0' | cut -d' ' -f2- | gawk -v rat=$r '{if (100*rand()<=rat) print}' | sort | uniq -c | gawk '{print $2,$3,$1}' | gawk '{if ($1==ant) {lin=lin" "$2" "$3;} else {print ant,lin; lin=$2" "$3;} ant=$1} END {print ant,lin}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; print s,$0}' | sort -k 1 -n -r >tmp-gz
    curr=`cat $NERCDIR/nec/corpus/$lang.test.nec | gawk 'BEGIN {while (getline<"tmp-gz") g[$2]=1} $3~/^NP/ {if (g[$2]) n++; nt++} END {print int(100*n/nt)}' `
  ## dicotomic search
    if (test $curr -gt $(($goal+1)) ); then r2=$r
    elif (test $curr -lt $(($goal-1)) ); then r1=$r
    else stop=1
    fi
    r=$(( ($r1+$r2)/2 ))
    echo "   Computed ratio="$r"  goal="$goal"  achieved="$curr
    # if no changes, stop.
    if (test $r == $rant); then
	stop=1;
	echo "   Too few training data. 'poor' gazetteer will probably be useless."
    fi
    rant=$r
done

echo "Extracting poor.$rgaz test gazetteer"
#fix category names
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-c.dat.poor$rgaz.test; done;
cat tmp-gz | gawk -v lg=$lang -v r=$rgaz 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {for (x=3;x<=NF;x+=2) print $2>"gaz-"lg"/gaz"nom[$x]"-c.dat.poor"r".test"}'
#extract gazetter of name parts
for cl in PER ORG LOC MISC; do touch gaz-$lang/gaz$cl-p.dat.poor$rgaz.test; done;
cat tmp-gz | gawk -v lg=$lang -f $BINDIR/extract-gaz-p.awk | sort | gawk '{if (ant==$1" "$2) s+=$3; else {print ant,s; s=$3}; ant=$1" "$2} END {print ant,s;}' | gawk '{if (ant==$1) lin=lin" "$2" "$3; else {print lin; lin=$0}; ant=$1} END {print lin;}' | gawk '{s=0; for (i=3;i<=NF;i+=2) s=s+$i; n=(NF-1)/2; x=(float)s/((float)(n+1)); for (i=2;i<=NF;i+=2) if ($(i+1)+0>=x+0) print $1,$i,$(i+1); }' | gawk -v lg=$lang -v r=$rgaz 'BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";} {print $1>"gaz-"lg"/gaz"nom[$2]"-p.dat.poor"r".test"}'

rm tmp-gz
