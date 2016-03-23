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
##             despatx C6.212 - Campus Nord UPC
##             08034 Barcelona.  SPAIN
##
################################################################

################################################################
# train-relax.perl
#
# Script used to read a disambiguated corpus and create a basic bigram 
# constraint grammar for relax tagger.
#
# The corpus is expected to be in FreeLing "tagged" inpf/outf format, that is:
# one word per line, each line with format:
#     form lemma tag
#
# Sentences are separated with a blank line.
#
# The output is sent to stodut, which may be straightforwadly used as a 
# constraint grammar for the relax tagger (command line option -R, or config file
# option "TaggerRelaxFile")
#
# Two parameters are expected in command line, the language code, and a boolean
# stating whether the used tagset is Eagles (1) or not (0)
#
################################################################

use strict;

use File::Spec::Functions qw(rel2abs);
use File::Basename;

my $path = dirname(rel2abs($0));
require $path."/short_tag.perl";

my ($f1,$l1,$t1,$f2,$l2,$t2,$f3,$l3,$t3);
my (@resta,%unig,%bigr,%trig,%bigrD2,$nu,$nb,$nt,$b,$t,$imA,$imB,$imC);

# ------------ command line arguments
my $LG=$ARGV[0];
my $EAGLES=$ARGV[1];
my $tsetfile = $ARGV[2];
my $tset = new Tagset($tsetfile);

 $t1="OUT_OF_BOUNDS";
 $_=<STDIN>;
 ($f2,$l2,$t2,@resta)=split("[ \n]",$_);
 while (<STDIN>) {
     
     if ($_ =~ /^$/) { $t3="OUT_OF_BOUNDS"; }
     else { ($f3,$l3,$t3,@resta)=split("[ \n]",$_); }
     
     $unig{$tset->ShortTag($t3)}++;
     $bigr{$tset->ShortTag($t2)."#".$tset->ShortTag($t3)}++;
     $bigrD2{$tset->ShortTag($t1)."#".$tset->ShortTag($t3)}++;
     $trig{$tset->ShortTag($t1)."#".$tset->ShortTag($t2)."#".$tset->ShortTag($t3)}++;

     ($f1,$l1,$t1)=($f2,$l2,$t2);
     ($f2,$l2,$t2)=($f3,$l3,$t3);
     
     $nb++;
 }

 $unig{$tset->ShortTag($t3)}++;
 $nu=$nb+1;

 for $b (keys %trig) {
    ($t1,$t2,$t3) = split("#",$b);

    $imA=log($trig{$b}*$nb/($unig{$t1}*$bigr{$t2."#".$t3}))/log(2);
    $imB=log($trig{$b}*$nb/($unig{$t2}*$bigrD2{$t1."#".$t3}))/log(2);
    $imC=log($trig{$b}*$nb/($bigr{$t1."#".$t2}*$unig{$t3}))/log(2);

    if ($EAGLES && !($t1 =~ /^[FIWZ]/ || $t1 eq "OUT_OF_BOUNDS" )) {$t1 .= "*";}
    if ($EAGLES && !($t2 =~ /^[FIWZ]/ || $t2 eq "OUT_OF_BOUNDS" )) {$t2 .= "*";}
    if ($EAGLES && !($t3 =~ /^[FIWZ]/ || $t3 eq "OUT_OF_BOUNDS" )) {$t3 .= "*";}

    ## restriccio A seguit de BC
#    if (abs($imA)>=0.0001) 
#      if ($t1 ne "OUT_OF_BOUNDS") {printf "%f\t%s\t(1 %s) (2 %s);\n",$imA,$t1,$t2,$t3;}
    ## restriccio B entre A-C
#    if (abs($imB)>=0.0001) 
#      if ($t2 ne "OUT_OF_BOUNDS") {printf "%f\t%s\t(-1 %s) (1 %s);\n",$imB,$t2,$t1,$t3;}
    ## restriccio C precedit de AB
    if (abs($imC)>=0.0001) {
      if ($t3 ne "OUT_OF_BOUNDS") {printf "%f\t%s\t(-2 %s) (-1 %s);\n",$imC,$t3,$t1,$t2;}
    }
}
