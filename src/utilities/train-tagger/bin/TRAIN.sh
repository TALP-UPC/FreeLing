#! /bin/bash

## Train a PoS tagger and evaluate its performance.
## See README file for details
##
##  Usage:
##    ./TRAIN.sh lang [tagset-file] [--eagles|--noeagles]
##

# FreeLing installation. Adjust this path if needed,
# or call the script with: FREELINGDIR=/my/FL/path ./TRAIN.sh lang
if ( test "x$FREELINGDIR" = "x" ); then
  echo "No FREELINGDIR defined. Defaulting to /usr/local"
  export FREELINGDIR=/usr/local
fi

## language code for FreeLing (es,en,ru)...
## A directory with this name must exists and 
## contain a "train" and a "test" corpus.
LG=$1

TAGSET=$2
if ( test "x$TAGSET" = "x" ); then
  TAGSET=$FREELINGDIR/share/freeling/$LG/tagset.dat
  echo "Using default tagset file $TAGSET"
fi

if [[ "$*" =~ "--eagles" ]]; then
  EAGLES=1
elif [[ "$*" =~ "--noeagles" ]]; then
  EAGLES=0
else
  EAGLES=1
  echo "Using default EAGLES=1"
fi


export PERL_UNICODE=SDL

# script location
BINDIR=$(cd $(dirname $0) && echo $PWD)


if (test "#$LG" == "#ru"); then 
  THR_SUF=5
  THR_FORM=2
  THR_CLAS=4
else 
  THR_SUF=2
  THR_FORM=4
  THR_CLAS=8
fi

# Create lexical probabilites file
echo "Creating probabilities file"
cat $LG/train | gawk 'NF>0' | gawk '{bo=$2"#"$3; printf "%s %s %s",$1,$2,$3; for (i=5;i<=NF; i+=3) {if (bo!=$i"#"$(i+1)) printf " %s %s",$i,$(i+1);} printf "\n"}' | $BINDIR/make-probs-file.perl $LG $THR_SUF $THR_FORM $THR_CLAS $TAGSET 2>$LG/err.tmp | cat $LG/header-probs - > $LG/probabilitats.dat
# Report inconsistencies corpus-dictionary
cat $LG/err.tmp | sort | uniq -c | egrep -v '[ \-]N?NP' | sort -nr -k 1 >$LG/report.txt

# train hmm tagger
echo "Training HMM"
cat $LG/train | cut -d' ' -f1-3 | gawk 'NF>0' | $BINDIR/hmm_smooth.perl $LG $TAGSET | cat $LG/hmm-forbidden.manual - > $LG/tagger.dat

#train relax tagger
echo "Training relax"
cat $LG/train | cut -d' ' -f1-3 | $BINDIR/train-relax-B.perl $LG $EAGLES $TAGSET >$LG/constr_gram-B.tmp
cat $LG/train | cut -d' ' -f1-3 | $BINDIR/train-relax-T.perl $LG $EAGLES $TAGSET >$LG/constr_gram-T.tmp
cat $LG/constr_gram.manual $LG/constr_gram-B.tmp > $LG/constr_gram-B.dat
cat $LG/constr_gram.manual $LG/constr_gram-T.tmp > $LG/constr_gram-T.dat
cat $LG/constr_gram.manual $LG/constr_gram-B.tmp $LG/constr_gram-T.tmp > $LG/constr_gram-BT.dat

rm -f $LG/*.tmp
