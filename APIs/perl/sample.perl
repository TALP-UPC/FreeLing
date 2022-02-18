#! /usr/bin/perl

use lib '.';
use plfreeling;
use strict;

## Init library locale, to properly handle UTF8 characters.
plfreeling::util::init_locale("default");

my $FREELINGDIR;
if ($ENV{'FREELINGDIR'} eq "") {
    print "FREELINGDIR not defined, using /usr/local";
    $FREELINGDIR = "/usr/local";
}
else {
    $FREELINGDIR = $ENV{'FREELINGDIR'};
}

# uncomment to trace FreeLing (if it was compiled with -DTRACES=ON)
#$plfreeling::traces::TraceLevel=4;
#$plfreeling::traces::TraceModule=0xFFFF;

## Modify this line to be your FreeLing installation directory
my $DATA = $FREELINGDIR."/share/freeling/";
my $LANG = "es";

# create options set for maco analyzer. 
my $op = new plfreeling::analyzer_config();

# define creation options for morphological analyzer modules
$op->swig_config_opt_get()->swig_Lang_set($LANG);
$op->swig_config_opt_get()->swig_MACO_PunctuationFile_set($DATA."common/punct.dat");
$op->swig_config_opt_get()->swig_MACO_DictionaryFile_set($DATA.$LANG."/dicc.src");
$op->swig_config_opt_get()->swig_MACO_AffixFile_set($DATA.$LANG."/afixos.dat" );
$op->swig_config_opt_get()->swig_MACO_CompoundFile_set($DATA.$LANG."/compounds.dat" );
$op->swig_config_opt_get()->swig_MACO_LocutionsFile_set($DATA.$LANG."/locucions.dat");
$op->swig_config_opt_get()->swig_MACO_NPDataFile_set($DATA.$LANG."/np.dat");
$op->swig_config_opt_get()->swig_MACO_QuantitiesFile_set($DATA.$LANG."/quantities.dat");
$op->swig_config_opt_get()->swig_MACO_ProbabilityFile_set($DATA.$LANG."/probabilitats.dat");

# chose which modules among those available will be used by default
# (can be changed at each call if needed)
$op->swig_invoke_opt_get()->swig_MACO_AffixAnalysis_set(1);
$op->swig_invoke_opt_get()->swig_MACO_CompoundAnalysis_set(1);
$op->swig_invoke_opt_get()->swig_MACO_MultiwordsDetection_set(1);
$op->swig_invoke_opt_get()->swig_MACO_NumbersDetection_set(1);
$op->swig_invoke_opt_get()->swig_MACO_PunctuationDetection_set(1 );
$op->swig_invoke_opt_get()->swig_MACO_DatesDetection_set(1);
$op->swig_invoke_opt_get()->swig_MACO_QuantitiesDetection_set(1);
$op->swig_invoke_opt_get()->swig_MACO_DictionarySearch_set(1);
$op->swig_invoke_opt_get()->swig_MACO_ProbabilityAssignment_set(1);
$op->swig_invoke_opt_get()->swig_MACO_NERecognition_set(1);
$op->swig_invoke_opt_get()->swig_MACO_RetokContractions_set(1);


# create analyzers
my $tk=new plfreeling::tokenizer($DATA.$LANG."/tokenizer.dat");
my $sp=new plfreeling::splitter($DATA.$LANG."/splitter.dat");
my $sid=$sp->open_session();

my $mf=new plfreeling::maco($op);

## exchange comments in two following lines to change the tagger type used

$op->swig_config_opt_get()->swig_TAGGER_HMMFile_set($DATA.$LANG."/tagger.dat");
$op->swig_invoke_opt_get()->swig_TAGGER_Retokenize_set(1);
$op->swig_invoke_opt_get()->swig_TAGGER_ForceSelect_set(2);
my $tg=new plfreeling::hmm_tagger($op);

my $nc=new plfreeling::nec($DATA.$LANG."/nerc/nec/nec-ab-poor1.dat");

# create chunker
my $parser= new plfreeling::chart_parser($DATA.$LANG."/chunker/grammar-chunk.dat");
# create dependency parser
my $dep=new plfreeling::dep_txala($DATA.$LANG."/dep_txala/dependences.dat", $parser->get_start_symbol());

## read input text and analyze it.
while (<STDIN>) {
  chomp;
  my $l = $tk->tokenize($_);	# tokenize
  my $ls = $sp->split($sid,$l,0);  # split sentences

  $ls=$mf->analyze_sentence_list($ls);	# morphological analysis
  $ls=$tg->analyze_sentence_list($ls);	# PoS tagging
  $ls=$nc->analyze_sentence_list($ls);  # NE classification
  print "** print tagged Sentence\n";
  &printSent($ls);

  $ls = $parser->analyze_sentence_list($ls);
  print "** print shallow Tree\n";
  &printTree($ls);

  $ls = $dep->analyze_sentence_list($ls);
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
