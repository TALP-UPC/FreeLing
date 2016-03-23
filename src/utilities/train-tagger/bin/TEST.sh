#! /bin/bash

## Test the performance of a PoS tagger
## See README file for details
##
##  Usage:
##    ./TEST.sh lang [tagset-file] [cfgfile]
##

# FreeLing installation. Adjust this path if needed,
# or call the script with: FREELINGDIR=/my/FL/path ./TEST.sh lang
if ( test "x$FREELINGDIR" = "x" ); then
  echo "No FREELINGDIR defined. Defaulting to /usr/local"
  export FREELINGDIR=/usr/local
fi

## language code for FreeLing (es,en,ru)...
## A directory with this name must exists and 
## contain a "train" and a "test" corpus.
LG=$1
TAGSET=$2
CFG=$3

if ( test "x$TAGSET" = "x" ); then
  TAGSET=$FREELINGDIR/share/freeling/$LG/tagset.dat
  echo "Using default tagset file $TAGSET"
fi
if ( test "x$CFG" = "x" ); then
  CFG=$FREELINGDIR/share/freeling/config/$LG.cfg
fi

export LD_LIBRARY_PATH=$FREELINGDIR/lib
export PERL_UNICODE=SDL

# script location
BINDIR=$(cd $(dirname $0) && echo $PWD)

## get locale for current language from config file
loc=`cat $CFG | grep 'Locale=' | cut -d'=' -f2`


## copy tagset to test directory (required by tagger.dat and probabiltites.dat)
cp $TAGSET $LG/tagset.dat

# test HMM tagger
echo "Testing HMM"
cat $LG/test | $BINDIR/update-probs $LG 0.001 $LG/probabilitats.dat $loc |  gawk '{if (NF==0) print ""; else {$2="";$3="";$4=""; gsub("[ ]+"," "); print} }' | $FREELINGDIR/bin/analyze -f $CFG --input freeling --inplv morfo --outlv tagged --nortk -H $LG/tagger.dat > $LG/out-hmm.tmp
cut -d' ' -f1-3 <$LG/test | paste - $LG/out-hmm.tmp | $BINDIR/count.perl $LG $TAGSET

# test relax tagger, with different models
for c in B T BT; do
  echo "Testing relax-"$c
  cat $LG/test | $BINDIR/update-probs $LG 0.001 $LG/probabilitats.dat $loc |  gawk '{if (NF==0) print ""; else {$2="";$3="";$4=""; gsub("[ ]+"," "); print} }' | $FREELINGDIR/bin/analyze -f $CFG --input freeling --inplv morfo --outlv tagged --nortk -t relax -R $LG/constr_gram-$c.dat > $LG/out-rlx$c.tmp
  cut -d' ' -f1-3 <$LG/test | paste - $LG/out-rlx$c.tmp | $BINDIR/count.perl $LG $TAGSET
done

#rm -f $LG/*.tmp $LG/tagset.dat
