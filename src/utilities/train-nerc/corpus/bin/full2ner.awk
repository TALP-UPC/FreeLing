#! /usr/bin/gawk -f


NF==0 {print ""; next}
{
   lin = substr($2,1,1)" "$1;
   for (i=3; $i!="#"; i++);
   for (i++; i<=NF; i++) lin=lin" "$i;

   print lin
}