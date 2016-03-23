#! /usr/bin/perl

##################################################################
##
##    FreeLing - Open Source Language Analyzers
##
##    Copyright (C) 2014   TALP Research Center
##                         Universitat Politecnica de Catalunya
##
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Affero General Public
##    License as published by the Free Software Foundation; either
##    version 2.1 of the License, or (at your option) any later version.
##
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Affero General Public License for more details.
##
##    You should have received a copy of the GNU Affero General Public
##    License along with this library; if not, write to the Free Software
##    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##
##    contact: Lluis Padro (padro@lsi.upc.es)
##             TALP Research Center
##             despatx C6.212 - Campus Nord UPC
##             08034 Barcelona.  SPAIN
##
################################################################

################################################################
# hmm_smoth.perl
# 
# Script used to create a probability file (prob.dat) 
# in the format needed by the hmm_tagger class in FreeLing.
#
# The format of the train corpus is one word per line, each
# line with format:
#    form lemma tag
#
#  usage: 
#    cat train_corpus | hmm_smoth.perl language tagset
#
################################################################

use File::Spec::Functions qw(rel2abs);
use File::Basename;

my $path = dirname(rel2abs($0));
require $path."/short_tag.perl";

#------ SMOOTHING PARAMETERS ----------
# lambda parameters are used in Lidstone smoothing for unknown 
# tags and words.  
$lambda_tag=0.1;
$lambda_pi=0.1; 
$lambda_w=0.01; 

# These are the number of bins used in Lidstone smoothing for unknown 
# tags and words.  
$B1=66;            # number of possible tags
$Bw=1000000;       # approximate number os possible word forms
#--------------------------------------

# language
my $LG=$ARGV[0];
# tagset to use
my $tsetfile = $ARGV[1];
my $tset = new Tagset($tsetfile);

$t1="0"; 
($word,$lema,$t2,@x)=  split(" ",<STDIN>);
$word = lc($word);

$t2=$tset->ShortTag($t2);

$nf=1;  # sentence number
$n=1; $ns=0;
$nw{$word}++;

$ftag{$t1}++;
$ftag{$t2}++;

$fbg{"$t1.$t2"}++;

$pi{"$t1.$t2"}++;

while ( ($word,$lema,$t3,@x)=  split(" ",<STDIN>) ) {
  $word = lc($word);
  $t3=$tset->ShortTag($t3);

  $ftag{$t3}++;
  $fbg{"$t2.$t3"}++;
  $ftg{"$t1.$t2.$t3"}++;  

  if ($t3 eq "Fp") {
      $t1="0";
      if (($word,$lema,$t2,@x)=split(" ",<STDIN>)) {
	  $word = lc($word);
	 
	  $t2=$tset->ShortTag($t2);

	  $ftag{$t1}++;
	  $ftag{$t2}++;

	  $fbg{"$t1.$t2"}++;

	  $pi{"$t1.$t2"}++;
	  $nf++;  
      }
  }    
  else {
      $t1=$t2; $t2=$t3;
  }

  if ($word){
      $nw{$word}++;
      $n++; #nombre total de paraules
  }
}


###### compute linear interpolation coeficients: ##########
$c1=0; $c2=0; $c3=0; $ctotal=0;

for $k (sort keys %ftg) {
    $aux=0; $max=0; $argmax="cap";
    ($t1,$t2,$t3) = split(/\./,$k);

    if (($n-1) ne 0){
	$aux=($ftag{$t3}-1)/($n-1);
	if($max < $aux) { 
	    $max=$aux; 
	    $argmax=1;
	}
    }

    if (($ftag{"$t2"}-1) ne 0){
	$aux=($fbg{"$t2.$t3"}-1)/($ftag{"$t2"}-1);
	if($max < $aux) { 
	    $max=$aux; 
	    $argmax=2;
	}
    }

    if (($fbg{"$t1.$t2"}-1) ne 0){
	$aux=($ftg{"$t1.$t2.$t3"}-1)/($fbg{"$t1.$t2"}-1);
	if($max < $aux) { 
	    $max=$aux; 
	    $argmax=3;
	}
    }

    if    ($argmax == 1) { $c1+=$ftg{$k} }
    elsif ($argmax == 2) { $c2+=$ftg{$k} }
    elsif ($argmax == 3) { $c3+=$ftg{$k} }   
}

#normalization

$ctotal=$c1+$c2+$c3;

$c1=$c1/$ctotal;
$c2=$c2/$ctotal;
$c3=$c3/$ctotal;


##############################################
# write results to STDOUT

##### tag
print "<Tag>\n";
for $k (sort keys %ftag) {
    print $k." ".($ftag{$k}+$lambda_tag)/($n+$B1*$lambda_tag)."\n";
}
print "x ".$lambda_tag/($n+$B1*$lambda_tag)."\n"; #unobserved tag
print "</Tag>\n";

###### bigram
print "<Bigram>\n";
for $k (sort keys %fbg) {
    ($t1,$t2) = split(/\./,$k);
    #print "$k  $fbg{$k}  $t1  ".$ftag{$t1}."\n";
    print $k." ".$fbg{$k}/$ftag{$t1}."\n";
}
print "</Bigram>\n";

###### trigram
print "<Trigram>\n";
for $k (sort keys %ftg) {
    ($t1,$t2,$t3) = split(/\./,$k);
    #print "$k  $ftg{$k}  $t1.$t2  ".$fbg{"$t1.$t2"}."\n";
    print $k." ".$ftg{$k}/$fbg{"$t1.$t2"}."\n";
}
print "</Trigram>\n";


###### initial
print "<Initial>\n";
for $k (sort keys %pi) {
   print $k." ".(log($pi{$k}+$lambda_pi)-log($nf+$B1*$lambda_pi))."\n";
}
print "0.x ".(log($lambda_pi)-log($nf+$B1*$lambda_pi))."\n";
print "</Initial>\n";

###### word
print "<Word>\n";
for $k (sort keys %nw) {
   print $k." ".(log($nw{$k}+$lambda_w)-log($n+$Bw*$lambda_w))."\n";
}
print "<UNOBSERVED_WORD> ".(log($lambda_w)-log($n+$Bw*$lambda_w))."\n";
print "</Word>\n";


##### coeficients
print "<Smoothing>\n";
print "c1 $c1\nc2 $c2\nc3 $c3\n"; 
print "</Smoothing>\n";

