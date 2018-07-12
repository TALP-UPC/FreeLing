
BEGIN {nom["NP00SP0"]="PER"; nom["NP00G00"]="LOC"; 
       nom["NP00O00"]="ORG"; nom["NP00V00"]="MISC";

       if (lg=="es") 
         filtra="^(en|mi|le|sin|el|la|los|las|un|uno|una|unos|unas|a|de|por|para|[0-9\\.,'\\-]+|[a-z]\\.?)$|[\\.,)(]"

       else if (lg="en")
         filtra="^(an|and|for|in|or|the|of|[0-9\\.,'\\-]+|[a-z]\\.?)$|[\\.,)(]"
}


NF>=4 && $1>1 {

   n=split($2,t,"_"); 
   if (n>1) {
     for (i=1; i<=n; i++) 
	 if (!match(t[i],filtra))
         for (v=3;v<=NF;v+=2)
           print t[i],$v,$(v+1)
   }
}