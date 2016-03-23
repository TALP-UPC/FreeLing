#! /usr/bin/gawk -f

$3~/^NP/ {print $1,$3;}
