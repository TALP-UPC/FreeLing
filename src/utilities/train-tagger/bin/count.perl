#! /usr/bin/perl


use strict;

use File::Spec::Functions qw(rel2abs);
use File::Basename;

my $path = dirname(rel2abs($0));
require $path."/short_tag.perl";

# ------------ command line arguments
my $LG=$ARGV[0];
my $tsetfile = $ARGV[1];
my $tset = new Tagset($tsetfile);

my $nt;
my $nokS;
my $nokL;

while (<STDIN>) {
  chomp $_;
  my @line=split("[ \n\t]",$_);

  if ($#line>0) {
    $nt++;
    if ($tset->ShortTag($line[2]) eq $tset->ShortTag($line[5])) {$nokS++;}
    if ($line[2] eq $line[5]) {$nokL++;}
  }
}

print "Accuracy. short tag:".(100*$nokS/$nt)."  long tag:".(100*$nokL/$nt)."\n";

