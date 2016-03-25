#! /usr/bin/gawk -f


BEGIN { inside=0 }

{
    if (substr($2,1,1)=="B") {
	if (inside) {
	    print form,lem,tag
	    form=""; lem=""; tag="";
	    inside=0;
	}
	tag=$4;
	form=$1;
	lem=tolower($1);
	inside=1
    }

    else if (substr($2,1,1)=="I") {
	form=form"_"$1;
	lem=lem"_"tolower($1);
    }

    else {
	if (inside) {
	    print form,lem,tag
	    form=""; lem=""; tag="";
	    inside=0;
	}
	if (NF==0) print "";
	else if ($2=="O") print $1,$3,$4
    }
    
}



