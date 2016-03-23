
## Form generator for regular morphological patterns
##
## This file can be used to generate regular flexive forms and
## extend FreeLing dictionary.
##
## E.g., to generate regular verbs in Spanish, you can do:
##
##   gawk -f flex.awk -v frules=es/1conj.verb <myverbs
##
## "myverbs" must be a list of 1st conjugation infinitives.
## rule files for 2nd and 3rd can be found in the same directory.
##
## The output will be all forms of the verb, in a format compatible with
## FreeLing dictionary. 

BEGIN {
   # load sufix file
   getline<frules
   if ($1=="SUFFIX") inf=$2; 
   else { print "ERROR - missing SUFFIX line."; exit}
   while (getline < frules) suf[$1]=$2; 
}

{
   # read lemma
   lemma = $1
   p = match(lemma,inf"$")
   if (p==0) {print "ERROR - verb has different ending than rule set."; exit}

   root = substr(lemma,1,p-1)

   # generate forms
   for (s in suf)
     print root suf[s], lemma, s
}

  