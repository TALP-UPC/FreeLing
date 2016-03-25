#! /bin/bash

##################################################################################
## train-svm.sh
##
## ./train-svm.sh lexicon codes outmodel corpus.enc Cparam
##
  
## change this to the place where libsvm is located
SVM=/usr/local/libsvm/libsvm-3.0

LEX=$1
COD=$2
MOD=$3
CORPUS=$4
Cparam=$5

cd $(dirname $0)/..

nom=`basename $MOD`
tmpf=`mktemp tmpsvm-$nom-XXXX`

## adapt training corpus to libsvm needs
cat $CORPUS | gawk 'NF>0' | gawk -v lexicon=$LEX -v clas="$COD" 'BEGIN {n=split(clas,t," "); for (i=1;i<=n;i+=2) c[t[i+1]]=t[i]; while(getline<lexicon) lex[$2]=$1;} {printf c[$1]; for (i=2;i<=NF;i++) if (lex[$i]) printf " %s:1",lex[$i]; printf "\n";}' >$tmpf

## learn model with given parameter C
$SVM/svm-train -t 2 -c $Cparam -h 0 -b 1 $tmpf $MOD-C$Cparam.svm
rm -f $tmpf

###### produce .dat configuration files
echo "<Lexicon>" >$MOD-C$Cparam.dat
echo "../"`basename $LEX` >>$MOD-C$Cparam.dat
echo "</Lexicon>" >>$MOD-C$Cparam.dat
echo "<Classifier>" >>$MOD-C$Cparam.dat
echo "SVM" >>$MOD-C$Cparam.dat
echo "</Classifier>" >>$MOD-C$Cparam.dat
echo "<RGF>" >>$MOD-C$Cparam.dat
echo "ner.rgf" >>$MOD-C$Cparam.dat
echo "</RGF>" >>$MOD-C$Cparam.dat
echo "<ModelFile>" >>$MOD-C$Cparam.dat
echo `basename $MOD-C$Cparam.svm` >>$MOD-C$Cparam.dat
echo "</ModelFile>" >>$MOD-C$Cparam.dat
echo "<Classes>" >>$MOD-C$Cparam.dat
echo $COD >>$MOD-C$Cparam.dat
echo "</Classes>" >>$MOD-C$Cparam.dat
echo "<NE_Tag>" >>$MOD-C$Cparam.dat
echo "NP00000" >>$MOD-C$Cparam.dat
echo "</NE_Tag>" >>$MOD-C$Cparam.dat
echo "<UseSoftMax>" >>$MOD-C$Cparam.dat
echo "no" >>$MOD-C$Cparam.dat
echo "</UseSoftMax>" >>$MOD-C$Cparam.dat

bin/train-viterbi "$COD" <$CORPUS >>$MOD-C$Cparam.dat

rm -f $tmpf

