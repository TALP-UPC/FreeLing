#! /bin/bash

FLDIR=/home/usuaris/tools

#for corpus in es.testa es.testb es.train; do
for corpus in kk.test kk.train; do

 echo $corpus
 
 # analyze corpus with FreeLing
 zcat $corpus.gz | $FLDIR/bin/analyze -f es.cfg --inpf splitted --outf morfo --noprob --noquant --noner > analysis.tmp

 # store correct BIO tags without NE classification
 zcat $corpus.gz | gawk '{if ($0!="") {split($2, BIO, "-"); print $1, BIO[1]} else print}' > BIO.tmp

 # join BIO tags with analysis.tmp. Since some words have been joined as locutions, we have
 #  to look word by word. If the word has a "_", it is a locution

 cat BIO.tmp  | gawk '{

  if ($0!="") {
   original_word=$1; BIOtag=$2;
   #print "\n-- BIO --", $0;

   getline < "analysis.tmp"; # now $0 has the analysis
   an=$0; an_word=$1;
   #print "-- an --", $0;

   # locutions
   if (match($1,"_")){
       # if locution contains "a_el" or "de_el" join it to sincronyze with BIO tags
       gsub("^a_el_", "al_", $1);
       gsub("^a_el$", "al", $1);
       gsub("_a_el_", "_al_", $1);
       gsub("_a_el$", "_al", $1);

       gsub("^de_el_", "del_", $1);
       gsub("^de_el$", "del", $1);
       gsub("_de_el_", "_del_", $1);
       gsub("_de_el$", "_del", $1);

       #print "---- loc ----"
       split($1,parts,"_"); # see how many words FL has joined
       i=0;
       for (x in parts) i++; # i: size of the array
       #print "-----", $1, i;
       for (j=1; j<i; j++) { # for each joined word, join also the BIO_tag
         getline;
         #print "-- BIO --", $0;
         if ($2!=BIOtag)
           BIOtag=BIOtag"_"$2 # we join the tags if they are different 
       }
     print BIOtag, an;

     # if last element of the locution is "a" or "de" it is posible that a contraction 
     # has been splitted and we have to sincronize correctly the tags: BIO.tmp says 
     # "pese O / al O" and analysis.tmp says "pese_a / el
     if ((parts[i]=="a" && $1=="al") || (parts[i]=="de" && $1=="del")) { # $0 has last BIO.tmp entry
       #print "***", parts[i], $1;
       getline < "analysis.tmp"; # now $0 has the analysis
       print BIOtag, $0;
     }  
   }

   # for non locutions, if the word is "del", "al" or "desque", take one more
   # line from analysis.tmp and add one BIOtag, since the word will be splitted 
   # in two by FreeLing analyzer
   else if (match(original_word,"^[dD][eE][lL]$") || match(original_word,"^[aA][lL]$") || match(original_word,"^[dD][eE][sS][qQ][uU][eE]$")) {
     #print "---- del/al ----", original_word; 
     print BIOtag, an; # first line
     getline < "analysis.tmp"; # now $0 has the analysis
     #print "-- an --", $0;
     if (!(match($1,"^[eE][lL]$") || match($1,"^[qQ][uU][eE]$"))) print "ERROR: \"", $1, "\"encontered when expecting \"el\"." 
     print BIOtag, $0; # second line
   }

   else print BIOtag, an;
  }

 else {print; getline < "analysis.tmp";};

 }' > $corpus.analyzed

# rm *.tmp

done
