#! /usr/bin/gawk -f

## usage
##
##    nec2ner.awk -v lang=XX

BEGIN {
    c["NP00SP0"]="PER"; c["NP00G00"]="LOC"
    c["NP00O00"]="ORG"; c["NP00V00"]="MISC"

    FLcmd="/home/usuaris/tools/bin/analyze -f "lang".cfg --noloc --nodat --nonum --noprob --noquant --noner --outf morfo --inpf splitted"
}

# sentence separator
NF==0 {print ""; next}

# NPs: unassemble and look up components.
$3~/^NP/ {
     $1=fix_contr($1,lang)
     n=split($1,t,"_")
     print t[1],"B-"c[$3],$2,$3,"#",accFL(t[1])
     for (i=2;i<=n;i++) {
	 print t[i],"I-"c[$3],$2,$3,"#",accFL(t[i])
     }
     next
}

# fix wrong numeral tags 
$3~/^[DP]N/ { $3="Z"; }

# other multiwords and dates: keep as they are, add dummy prob value
$1~/_/ || $3~/^W/ { print $1,"O",$2,$3,"#",$2,$3,"-1"; next}

# normal line, just look word up in FL
{
    # if numeral, add to possible tags
    tags = accFL($1)
    if (substr($3,1,1)=="Z") {
	$3="Z"tolower(substr($3,2));
	tags=$2" "$3" -1 "tags
    }
    gsub(" $","",tags)
    print $1,"O",$2,$3,"#",tags; 
}


#-----------------------------------
function accFL(w) {
    sav=$0
    print w"\n.\n" |& FLcmd;
    FLcmd |& getline;
    FLcmd |& getline kk;
    FLcmd |& getline kk;
    x = substr($0,length($1)+2)
    $0=sav
    return x
}

#---------------------------------
function fix_contr(w,lg) {

  gsub("^_","",w); gsub("_$","",w); gsub("__","_",w);

  if (lg=="es") {
      while (p=match(w,"_[Dd][Ee][Ll]_|_[Dd][Ee][Ll]$")) w=substr(w,1,p+2)"_"substr(w,p+2)
      while (p=match(w,"^[Dd][Ee][Ll]_")) w=substr(w,1,p+1)"_"substr(w,p+1)
      
      while (p=match(w,"_[Aa][Ll]_|_[Aa][Ll]$")) w=substr(w,1,p+1)"_e"substr(w,p+2)
      while (p=match(w,"^[Aa][Ll]_")) w=substr(w,1,p)"_e"substr(w,p+1)
      while (p=match(w,"^[Aa][Ll]$")) w=substr(w,1,p)"_e"substr(w,p+1)
  }

  else if (lg=="ca") {
      while (p=match(w,"_[Dd][Ee][Ll][Ss]?_|_[Dd][Ee][Ll][Ss]?$")) w=substr(w,1,p+2)"_"substr(w,p+2)
      while (p=match(w,"^[Dd][Ee][Ll][Ss]?_")) w=substr(w,1,p+1)"_"substr(w,p+1)
      
      while (p=match(w,"_[Aa][Ll][Ss]?_|_[Aa][Ll][Ss]?$")) w=substr(w,1,p+1)"_e"substr(w,p+2)
      while (p=match(w,"^[Aa][Ll][Ss]?_")) w=substr(w,1,p)"_e"substr(w,p+1)
      while (p=match(w,"^[Aa][Ll][Ss]?$")) w=substr(w,1,p)"_e"substr(w,p+1)

      while (p=match(w,"_[Pp][Ee][Ll][Ss]?_|_[Pp][Ee][Ll][Ss]?$")) w=substr(w,1,p+1)"er_"substr(w,p+2)
      while (p=match(w,"^[Pp][Ee][Ll][Ss]?_")) w=substr(w,1,p)"er_"substr(w,p+1)
  }

  else if (lg=="pt") {
      while (p=match(w,"_[Dd][OoAa]_|_[Dd][OoAa]$")) w=substr(w,1,p+1)"e_"substr(w,p+2)
      while (p=match(w,"^[Dd][OoAa]?_")) w=substr(w,1,p)"e_"substr(w,p+1)
      
      while (p=match(w,"_[Pp][Ee][Ll][Oo][Ss]?_|_[Pp][Ee][Ll][oO][Ss]?$")) w=substr(w,1,p+1)"or_"substr(w,p+4)
      while (p=match(w,"^[Pp][Ee][Ll][Oo][Ss]?_")) w=substr(w,1,p)"or_"substr(w,p+3)
  }

  return w
}