#! /usr/bin/gawk -f

{print $2,substr($1,1,1);}
