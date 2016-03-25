#! /bin/bash

# categories for wich tw files will be created
categs="PER LOC ORG MISC GENT"
# source languages
src="ca en es pt"
# target languages
trg="gl"

# compute synsets relevant for each category
for cat in $categs; do
  rm -f syn$cat.dat
  for lg in $src; do
    cat ../$lg/nerc/data/tw$cat.dat | gawk '{print " "$0" "; print " "$0"$"}' | grep -f - /home/usuaris/tools/share/freeling/$lg/senses30.src | gawk '{print "^"$1}' | grep -f - /home/usuaris/tools/share/freeling/common/wn30.src | gawk -v cat=$cat 'BEGIN {f[18]="PER"; f2[18]=="GENT"; f[06]="LOC"; f[15]="LOC"; f[17]="LOC"; f[14]="ORG" } {if (f[$3]==cat || f2[$3]==cat) print "^"$1}' >> syn$cat.dat
  done

  for lg in $trg; do
    rm -f $lg.tw$cat.new
    cat syn$cat.dat | sort | uniq | grep -f - /home/usuaris/tools/share/freeling/$lg/senses30.src | gawk '{for (i=2;i<=NF;i++) print $i}' | grep -v "_" | sort | uniq >$lg.tw$cat.new
  done
done

rm -f syn*.dat

