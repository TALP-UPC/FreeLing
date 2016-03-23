#! /usr/bin/perl

use lib '.';
use freeling;
use strict;

## Init library locale, to properly handle UTF8 characters.
freeling::util::init_locale("default");

## Modify this line to be your FreeLing installation directory
my $FREELINGDIR = "/usr/local";
my $DATA = $FREELINGDIR."/share/freeling/";
my $LANG = "es";

# create options set for maco analyzer. Default values are Ok, except for data files.
my $op=new freeling::maco_options("es");
$op->set_data_files( "", 
                   $DATA."common/punct.dat",
                   $DATA.$LANG."/dicc.src",
                   $DATA.$LANG."/afixos.dat",
                   "",
                   $DATA.$LANG."/locucions.dat", 
                   $DATA.$LANG."/np.dat",
                   $DATA.$LANG."/quantities.dat",
                   $DATA.$LANG."/probabilitats.dat");

# uncomment to trace FreeLing (if it was compiled with --enable-traces)
#$freeling::traces::TraceLevel=4;
#$freeling::traces::TraceModule=0xFFFF;

# create analyzers
my $tk=new freeling::tokenizer($DATA.$LANG."/tokenizer.dat");
my $sp=new freeling::splitter($DATA.$LANG."/splitter.dat");
my $sid=$sp->open_session();
my $mf=new freeling::maco($op);
$mf->set_active_options(0, 1, 1, 1,   # select which among created 
                        1, 1, 0, 1,   # submodules are to be used. 
                        1, 1, 1, 1);  # default: all created submodules 
                                      # are used

## exchange comments in two following lines to change the tagger type used
my $tg=new freeling::hmm_tagger($DATA.$LANG."/tagger.dat",1,2);
#my $tg=new freeling::relax_tagger($DATA."es/constr_gram.dat", 500,670.0,0.001, 1,2);

my $nc=new freeling::nec($DATA.$LANG."/nerc/nec/nec-ab-poor1.dat");

# create chunker
my $parser= new freeling::chart_parser($DATA.$LANG."/chunker/grammar-chunk.dat");
# create dependency parser
my $dep=new freeling::dep_txala($DATA.$LANG."/dep_txala/dependences.dat", $parser->get_start_symbol());

## read input text and analyze it.
while (<STDIN>) {
  chomp;
  my $l = $tk->tokenize($_);	# tokenize
  my $ls = $sp->split($sid,$l,0);  # split sentences
  $ls=$mf->analyze($ls);	# morphological analysis
  $ls=$tg->analyze($ls);	# PoS tagging
  $ls=$nc->analyze($ls);       # NE classification

  print "** print tagged Sentence\n";
  &printSent($ls);
  $ls = $parser->analyze($ls);
  print "** print shallow Tree\n";
  &printTree($ls);
  $ls = $dep->analyze($ls);
  print "** print Dependency Tree\n";
  &printDepTree($ls);
}

$sp->close_session($sid);

sub printSent {

  my $ls = shift;

  for my $s (@$ls) {
    for my $w (@{ $s->get_words() }) {
      print $w->get_form()." ".$w->get_lemma()." ".$w->get_tag()."\n";
    }
    print "\n";
  }
}

sub rec_printTree {
  my ($n, $depth) = @_;

  print ' ' x ($depth*2);
  my $info = $n->get_info();
  print "+" if ($info->is_head());
  if ($n->num_children() == 0) {
    my $w = $info->get_word();
    printf("(%s %s %s)", $w->get_form(), $w->get_lemma(), $w->get_tag());
  }
  else {
    print $info->get_label() . "_[\n";
    for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); $d++) {
      rec_printTree($d, $depth + 1);
    }
    print ' ' x ($depth*2);
    print "]";
  }
  print "\n";
}

sub printTree {

  my $ls = shift;

  foreach my $is (@{ $ls }) {
    my $tr = $is->get_parse_tree();
    &rec_printTree($tr->begin(), 0);
  }
}

sub rec_printDepTree {

  my ($n, $depth) = @_;

  print ' ' x ($depth*2);

  my $info = $n->get_info();
  my $link = $info->get_link();
  my $linfo = $link->get_info();
  printf("%s/%s/", $link->get_info()->get_label(), $info->get_label());
  my $w = $n->get_info()->get_word();
  printf("(%s %s %s)", $w->get_form(), $w->get_lemma(), $w->get_tag());

  if ($n->num_children() > 0) {
    print " [\n";
    for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); $d++) {
      rec_printDepTree($d, $depth + 1) unless $d->get_info()->is_chunk();
    }
    my %ch;
    for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); $d++) {
	my $dinfo = $d->get_info();
	next unless $dinfo->is_chunk();
	my $i = $dinfo->get_chunk_ord();
	$ch{ $i } = $d;
    }
    foreach my $i (sort { $a <=> $b } keys %ch) {
      rec_printDepTree($ch{$i}, $depth + 1);
    }

    print ' ' x ($depth*2);
    print "]";
  }
  print "\n";
}


sub printDepTree {

  my $ls = shift;

  foreach my $is (@{ $ls }) {
    my $dep = $is->get_dep_tree();
    &rec_printDepTree($dep->begin(), 0);
  }
}


sub debug {

  print ref($_[0])."\n";
  print Dumper($_[0])."\n";
  print join(" ", keys %{ $_[0] })."\n";
}
