#! /bin/bash

#
# Pack a model and associated files to be shiped with FreeLing
#
#  Usage:   ./pack.sh language feature-set train-gaz lex-filter n.weakrules
#
lang=$1
feat=$2
gz=$3
fl=$4
nwr=$5

TMPDIR=tmp-$lang-$feat-$fl-$nwr
mkdir $TMPDIR

# get model components
cp ../$lang/nerc/nec/nec-$feat.rgf $TMPDIR/
cp trained-$lang/nec-$feat-$gz.train-$fl.lex $TMPDIR/nec-$feat-$gz-$fl.lex
cat trained-$lang/adaboost/nec-$feat-$gz-$fl.abm | gawk -v nwr=$nwr '{if ($0=="---") n++; if (n<=nwr) print; else exit}' > $TMPDIR/nec-$feat-$gz-$fl.abm
cat trained-$lang/adaboost/nec-$feat-$gz-$fl.dat | sed "s#nec.rgf#nec-$feat.rgf#" | sed "s#../nec-$feat-$gz.train-$fl.lex#nec-$feat-$gz-$fl.lex#" > $TMPDIR/nec-ab-$gz.dat

# get gazetters
mkdir $TMPDIR/data
cp ../$lang/nerc/data/tw*.dat $TMPDIR/data/
cp ../$lang/nerc/data/function.dat $TMPDIR/data/
for x in ../$lang/nerc/data/gaz*.dat.rich.test; do
 cp $x $TMPDIR/data/`basename $x .rich.test`
done

# wrap it up
cd $TMPDIR
tar -czvf ../ner-ab-$lang-$feat-$gz-$fl-$nwr.tgz *
cd ..

# clean
rm -rf $TMPDIR
