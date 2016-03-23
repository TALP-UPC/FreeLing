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
##    version 3 of the License, or (at your option) any later version.
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
#             despatx C6.212 - Campus Nord UPC
##             08034 Barcelona.  SPAIN
##
################################################################

###############
#
#  Generate probabilities.dat file used by probabilities module in FreeLing
#
#  The aim is to estimate the probability of each possible PoS tag for a word
#
#  Input (stdin):
#
#   Annotated corpus, one word per line, line format:
#
#    form (lemma pos)+
#
#   The first pair (lemma pos) is the right disambiguation for the word in that context.
#   The following pairs are the discarded possibilities
#
#   The output (stdout) is the file that can be given to FreeLing for the ProbabilitiesFile option
#
#   If your corpus has unconsistent taggings, you may get some warnings
#
#   Make sure a file named 'unk-tags.$LANG' exists, with all tags for open categories
#  (i.e. all tags an  unknown word might take)

use strict;
use File::Spec::Functions qw(rel2abs);
use File::Basename;

my $path = dirname(rel2abs($0));
require $path."/short_tag.perl";

my %freqclasses1=();
my %probclasses=();
my %problex=();

my $MAX_SUF_LEN=8;

#--------- command line arguments
# language
my $LG=$ARGV[0];
# minimum occurrences to include sufixes, forms, or classes into probabilities file
my $MIN_SUF=$ARGV[1];
my $MIN_FORM=$ARGV[2];
my $MIN_CLASS=$ARGV[3];
# tagset to use
my $tsetfile = $ARGV[4];
my $tset = new Tagset($tsetfile);


my ($forma,@tags,$tagOK,%clas,$tag,$classe,%classeforma,$x,$y,@t,%seen,@uniq,$nt,%unk,%occ,%occT,$nocc);

open TAGS,$LG."/unk-tags.".$LG or die "Error opening file '$LG/unk-tags.$LG'";

while (<TAGS>) {
    chomp;
    $unk{$_}=1;
}
close TAGS;

while (($forma,@tags)=split("[ \n]",<STDIN>)) {
 
  ## calcular classe d'ambiguitat
  $forma = lc($forma);
  %clas=();
  shift @tags;  # saltar lema
  $tag = shift @tags;  # obtenir tag
  $clas{$tset->ShortTag($tag)}=1;
  
  $tagOK=$tset->ShortTag($tag);
  while (@tags) {
     shift @tags;  # saltar lema
     my $t = shift @tags;  # obtenir tag
     $clas{$tset->ShortTag($t)}=1;
  }
  $classe = join "-", (sort keys %clas);

  if ($classe) {
     # acumular frequencies
     $freqclasses1{$tagOK}++;
     $probclasses{$classe}{$tagOK}++;
     $problex{$forma}{$tagOK}++;
     if ($classeforma{$forma} && ($classeforma{$forma} ne $classe)) {
        print STDERR "ERROR - '$forma' apppears as $classeforma{$forma} and as $classe.\n";
     }
     else {
       $classeforma{$forma}=$classe; 
     }
  }

  # comptar si el tag es obert
  if ($unk{$tag}) {
    $unk{$tag}++;
    $nocc++; 
  }

  ## occurrencies de sufixos
  my $l=length($forma);
  my $mx=($l<$MAX_SUF_LEN ? $l : $MAX_SUF_LEN);
  for (my $i=1; $i<=$mx; $i++) {
      my $suf= substr($forma,$l-$i);
      $occ{$suf}++;
      $occT{$suf."#".$tag}++;
  }
  
}

print "<TagsetFile>\n./tagset.dat\n</TagsetFile>\n";

print "<UnknownTags>\n";
my $sp=0; 
my $nt=0;
foreach my $tag (sort keys %unk) {
    print "$tag $unk{$tag}\n";
    $unk{$tag}=$unk{$tag}/$nocc;
    $sp += $unk{$tag};
    $nt++;
}
my $mp=$sp/$nt;
print "</UnknownTags>\n";

my $sp=0;
foreach my $tag (keys %unk) {
    $sp +=  ($unk{$tag}-$mp)*($unk{$tag}-$mp)
}
print "<Theeta>\n".$sp/($nt-1)."\n</Theeta>\n";

print "<Suffixes>\n";
foreach my $suf (keys %occ) {
    if ($occ{$suf} >= $MIN_SUF) {
	my $lin="$suf $occ{$suf}";
	my $b=0;
	foreach my $tag (keys %unk) {
	    if ($occT{$suf."#".$tag}) {
		$b=1;
		$lin .= " $tag ".$occT{$suf."#".$tag};
	    }
	}
	if ($b) {print "$lin\n";}
    }
}
print "</Suffixes>\n";

 
print "<SingleTagFreq>\n";
for $x (sort keys %freqclasses1) {
   print "$x $freqclasses1{$x}\n";
}
print "</SingleTagFreq>\n";

print "<ClassTagFreq>\n";

for $x (sort keys %probclasses) {
   @t = split ("-",$x);
   if (@t>1) {

      # llista de tags de la classe o apareguts
      push @t, keys %{$probclasses{$x}};
      %seen = ();
      @uniq = grep { ! $seen{$_} ++ } @t;
      @t= sort @uniq;

      ## classe sense NP
      my $x2=join("-", grep { ($_ ne "NP") and ($_ ne "NNP") } @t);

      if ($x eq $x2) {
	  my $lin = $x;
	  my $cnt = 0;
	  for $y (@t) {
	      if (! $probclasses{$x}{$y}) {
		  $probclasses{$x}{$y} = 0;
	      }
	      $lin .= " $y $probclasses{$x}{$y}";
	      $cnt += $probclasses{$x}{$y};
	  }
	  if ($cnt >= $MIN_CLASS) {print $lin."\n";}
      }
      else { # hi ha NP

	  my $lin = $x;
          my $cnt = 0;
          # tag with max occurrences in normal class (no NP)
	  my $mx=0;  
	  for $y (@t) { 
	      if ($probclasses{$x2}{$y}>$mx) { $mx=$probclasses{$x2}{$y}; } 
	  }
          
	  # all tags, including NP
	  my $prob;
	  for $y (@t) {
	      if ($y eq "NP" or $y eq "NNP") { 
		  $prob= int(0.7 * $mx);
	      }
	      elsif (! $probclasses{$x2}{$y}) {
		  $prob = 0;
	      }
	      else {
		  $prob = $probclasses{$x2}{$y};
	      }
	      
	      $lin .= " $y $prob";
	      $cnt += $prob;
	  }
	  if ($cnt >= $MIN_CLASS) {print $lin."\n";}
      }
   }
}
print "</ClassTagFreq>\n";

print "<FormTagFreq>\n";
for $x (sort keys %problex) {
   @t = split ("-",$classeforma{$x});
   if (@t>1) {

      # llista de tags de la classe o apareguts
      push @t, keys %{$problex{$x}};
      %seen = ();
      @uniq = grep { ! $seen{$_} ++ } @t;
      @t= sort @uniq;

      my $lin =  "$x $classeforma{$x}";
      my $cnt = 0;
      for $y (@t) {
        if (! $problex{$x}{$y}) {
          $problex{$x}{$y} = 0;
        }
        $lin .= " $y $problex{$x}{$y}";
	$cnt += $problex{$x}{$y};
      }
      if ($cnt >= $MIN_FORM) {print $lin."\n";}
   }
}
print "</FormTagFreq>\n";


