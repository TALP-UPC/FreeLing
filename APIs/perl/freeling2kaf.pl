#! /usr/bin/perl

use FindBin qw($Bin);
use lib $Bin;
use freeling;
use strict;
use XML::LibXML;
use File::Basename;

#binmode STDOUT, ":utf8";

# TODO: use configuration file for all this settings
#       IDEA: use 'analyzer.cfg' from analyze
# TODO: have to know where freeling.so is!

## Modify this line to be your FreeLing installation directory
my $FREELINGDIR = "/usr/local";

my $DATA = $FREELINGDIR."/share/freeling/";
my $LANG = "es";

## local directory for storing config files (overrides system-wide config files)
my $LOCAL_CFGDIR = "freeling_cfg/";

## global variables (analyzers)

my $iden;     # Lang identifier
my $tk;       # tokenizer
my $sp;       # splitter
my $morfo;    # maco
my $neclass;  # nec
my $tagger;   # POS tagger
my $parser;   # chart parser
my $dep;      # dependency parser

## global variable (libxml)

my $ODOC = XML::LibXML::Document->new('1.0',"UTF-8");

&main();


sub loc_cfg_file {

  my $str = shift;

  my @F = ("${LOCAL_CFGDIR}${LANG}", "${LOCAL_CFGDIR}",
	   "${DATA}${LANG}", "${DATA}");

  foreach my $dir (@F) {
    return $dir."/$str" if -e $dir."/$str";
  }
  die "No $str found in the following directories:\n".join(" ", @F)."\n";
}


sub Freeling_CreateAnalyzers {

  my %loc;

  $loc{"locucions"} =  &loc_cfg_file("locucions.dat");
  $loc{"quantities"} = &loc_cfg_file("quantities.dat");
  $loc{"afixos"} = &loc_cfg_file("afixos.dat");
  $loc{"probabilitats"} = &loc_cfg_file("probabilitats.dat");
  $loc{"dicc"} = &loc_cfg_file("dicc.src");
  $loc{"np"} = &loc_cfg_file("np.dat");
  $loc{"punct"} = &loc_cfg_file("common/punct.dat");
  $loc{"tokenizer"} = &loc_cfg_file("tokenizer.dat");
  $loc{"splitter"} = &loc_cfg_file("splitter.dat");
  $loc{"tagger"} = &loc_cfg_file("tagger.dat");
  $loc{"grammar-chunk"} = &loc_cfg_file("chunker/grammar-chunk.dat");
  $loc{"constr_gram"} = &loc_cfg_file("constr_gram-B.dat");
  $loc{"dependences"} = &loc_cfg_file("dep_txala/dependences.dat");

  # uncomment to trace FreeLing (if it was compiled with --enable-traces)
  #$freeling::traces::TraceLevel=4;
  #$freeling::traces::TraceModule=0x00000040;

  freeling::util::init_locale("default");

  # create options set for maco analyzer. Default values are Ok, except for data files.
  my $op=new freeling::maco_options("es");

  $op->set_data_files("",                                        # UserMapFile
		      $loc{"punct"},				 # PunctuationFile
		      $loc{"dicc"},				 # DictionaryFile
		      $loc{"afixos"},				 # AffixFile
                      "",
		      $loc{"locucions"},                         # LocutionsFile
		      $loc{"np"},				 # NPDataFile
		      $loc{"quantities"},			 # QuantitiesFile
		      $loc{"probabilitats"}			 # ProbabilityFile
                     );

  # create analyzers
  $tk = new freeling::tokenizer($loc{"tokenizer"});
  $sp = new freeling::splitter($loc{"splitter"});
  $morfo = new freeling::maco($op);
  $morfo->set_active_options(0, 1, 1, 1,   # select which among created 
                             1, 1, 0, 1,   # submodules are to be used. 
                             1, 1, 1, 1);  # default: all created submodules are used


  ## exchange comments in two following lines to change the tagger type used
  $tagger = new freeling::hmm_tagger($loc{"tagger"},1,2);
  #my $tagger=new freeling::relax_tagger($loc{"constr_gram"}, 500,670.0,0.001, 1,2);

  # create chunker
  $parser = new freeling::chart_parser($loc{"grammar-chunk"});
  # create dependency parser
  $dep = new freeling::dep_txala($loc{"dependences"}, $parser->get_start_symbol());

  # could be a
  # return ($tk, $sp, $morfo, $tagger, $parser, $dep);
}


sub Freeling_AnalyzeSentence {

  my ($str) = @_;

## read input text and analyze it.
  my $l = $tk->tokenize($str);	# tokenize
  my $ls = $sp->split($l,0);	# split sentences
  $ls = $morfo->analyze($ls);	# morphological analysis
  $ls = $tagger->analyze($ls);	# PoS tagging
  $ls = $parser->analyze($ls);
  $ls = $dep->analyze($ls);

  return $ls;
}

######################################

sub word_key {

  my ($w, $ctrl) = @_;

  my $fw = $w;
  my $lw = $w;

  if($w->is_multiword()) {
    my @aux = @{ $w->get_words_mw() };
    $fw = shift(@aux);
    $lw = pop(@aux);
  }
  $lw = $fw unless defined $lw;

  my $key = sprintf("%s.%d.%d.%s.%s",
		    $fw->get_form(),
		    $ctrl->{"para_n"},
		    $ctrl->{"sent_n"},
		    $fw->get_span_start(),
		    $lw->get_span_finish());
  return $key;
}

sub add_word_element {

  my ($w, $ctrl) = @_;

  my $wf_array = $ctrl->{'wf_array'};
  my $wf_elems = $ctrl->{'wf_elems'};
  my $w2wid = $ctrl->{'w2wid'};

  my @ids;

  my $W = $w->is_multiword() ? $w->get_words_mw() : [$w];

  foreach my $wcomp (@{ $W }) {

    my $wid = &incr_id($ctrl, "wid");
    push @ids, $wid;
    my $span_start = $wcomp->get_span_start();
    my $span_end = $wcomp->get_span_finish();
    my $wcomp_key = &word_key($wcomp, $ctrl);
    $w2wid->{$wcomp_key} = $wid;

    my $wf_elem = $ODOC->createElement("wf");
    $wf_elem->setAttribute("wid", $wid);
    $wf_elem->setAttribute("sent", $ctrl->{"sent_n"});
    $wf_elem->setAttribute("offset", $span_start);
    $wf_elem->setAttribute("lenght", $span_end - $span_start);
    $wf_elem->appendText($wcomp->get_form());
    $wf_elems->{$wid} = @{ $wf_array };
    push @{ $wf_array }, $wf_elem;
  }
  return @ids;
}

sub add_term_element {

  my ($w, $wids, $ctrl) = @_;

  my $term_array = $ctrl->{'term_array'};
  my $term_elems = $ctrl->{'term_elems'};
  my $w2tid = $ctrl->{'w2tid'};

  my $pos = $w->get_tag();
  next if $pos =~ /^F/; # no punctuation marks

  my $term_elem = $ODOC->createElement("term");
  my $tid = &incr_id($ctrl, "tid");
  $term_elem->setAttribute("tid", $tid);
  #$term_elem->setAttribute("type", $sent_n);
  $term_elem->setAttribute("lemma", $w->get_lemma());
  $term_elem->setAttribute("pos", $pos);
  #$term_elem->setAttribute("netype", $sent_n);
  #$term_elem->setAttribute("head", $sent_n);
  my $span_elem = $ODOC->createElement("span");

  foreach my $wid (@{ $wids }) {
    my $tgt_elem =  $ODOC->createElement("target");
    $tgt_elem->setAttribute("id", $wid);
    $span_elem->addChild($tgt_elem);
  }
  $term_elem->addChild($span_elem);

  my $term_key = &word_key($w, $ctrl);
  $w2tid->{$term_key} = $tid;
  $term_elems->{$tid} = @{ $term_array };
  push @{ $term_array }, $term_elem;
}

sub add_deps {
  my ($n, $parent_tid, $ctrl) = @_;

  my $info = $n->get_info();
  my $dep_type = $info->get_label();
  my $child_key = &word_key($info->get_word(), $ctrl);
  my $child_tid = $ctrl->{'w2tid'}->{$child_key};
  return unless $child_tid;

  if ($parent_tid) {
    my $dep_elem = $ODOC->createElement("dep");
    $dep_elem->setAttribute("from", $child_tid);
    $dep_elem->setAttribute("to", $parent_tid);
    $dep_elem->setAttribute("rfunc", $dep_type);
    push @{ $ctrl->{'dep_array'} }, $dep_elem;
  }

  my %ch;
  for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); $d++) {
    my $dinfo = $d->get_info();
    unless ($dinfo->is_chunk()) {
      add_deps($d, $child_tid, $ctrl);
      next;
    }
    my $i = $dinfo->get_chunk_ord();
    $ch{ $i } = $d;
  }
  foreach my $i (sort { $a <=> $b } keys %ch) {
    add_deps($ch{$i}, $child_tid, $ctrl);
  }
}


sub chunk_label_ok {

  my $label = shift;

  return 1 if $label =~ /^s/;
  return 1 if $label =~ /^group/;
  return 0;
}

sub create_chunk_elem {

  my ($r, $ctrl) = @_;

  # print "* New chunk\n";
  # print " a -> ";
  # foreach my $w (@{ $r->{a} }) {
  #   print $w->get_form()." ";
  # }
  # print "\n h -> ".$r->{h}."\n";

  return if @{ $r->{a} } < 2;

  my $c_elem = $ODOC->createElement("chunk");
  my $cid = &incr_id($ctrl, "cid");
  my $span_elem = $ODOC->createElement("span");
  my $i = 0;
  my $tid_head;
  my @forms;
  foreach my $w (@{ $r->{a} }) {
    my $wkey = &word_key($w, $ctrl);
    my $tid = $ctrl->{w2tid}->{$wkey};
    next unless $tid;
    push @forms, $w->get_form();
    $tid_head = $tid if $i == $r->{h};
    my $tgt_elem =  $ODOC->createElement("target");
    $tgt_elem->setAttribute("id", $tid);
    $span_elem->addChild($tgt_elem);
    $i++;
  }
  my $comment_elem = $ODOC->createComment(join(" ", @forms));
  $c_elem->addChild($comment_elem);
  $c_elem->addChild($span_elem);

  $c_elem->setAttribute("cid", $cid);
  $c_elem->setAttribute("head", $tid_head);
  #$c_elem->setAttribute("phrase", $dep_type);
  #$c_elem->setAttribute("case", $dep_type);

  push @{ $ctrl->{'chunk_array'} }, $c_elem;
}

sub add_chunksR {
  my ($n, $ctrl) = @_;

  # only consider chunks:
  #
  #  - have more than one word AND
  #  - have a label starting with 's' ('sn*', 'sp*', 'sadv*') OR
  #  - have a label starting with 'group-verb'

  my $info = $n->get_info();
  my $label = $info->get_label();
  my $is_head = $info->is_head();

  #return unless &chunk_label_ok($label);

  my $num_children = $n->num_children();

  # return structure

  # if n == 0 we have a word

  return { a => [ $info->get_word() ], h => 0 } if $num_children == 0;

  # if n == 1, go down but erase head information
  if ($num_children == 1) {
    my $r = &add_chunksR($n->sibling_begin(), $ctrl);
    return { a => $r->{a},
	     h => $is_head ? 0 : -1,
	     l => $label};
  }

  # general case

  my $ret_st = { a => [],   # word array
		 h => -1,   # head idx
		 l => ""    # label
	       };

  for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); ++$d) {
    my $r = &add_chunksR($d, $ctrl);
    &create_chunk_elem($r, $ctrl);
    if ($d->get_info()->is_head() && $r->{h} != -1) {
      die if $ret_st->{h} != -1;
      $ret_st->{h} = $r->{h} + scalar(@{ $ret_st->{a} });
    }
    push @{ $ret_st->{a} }, @{ $r->{a} };
  }
  return $ret_st;
}


sub add_chunks {

  my ($n, $ctrl) = @_;

  # 1st level (sentence) not considered

  for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); ++$d) {
    my $r = &add_chunksR($d, $ctrl);
    &create_chunk_elem($r, $ctrl);
  }

}


#####################################################################
# Functions for creating KAF doc
#####################################################################

sub get_datetime {

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
return sprintf "%4d-%02d-%02dT%02d:%02d:%02dZ", $year+1900,$mon+1,$mday,$hour,$min,$sec;

}

sub add_kafHeader {

  my $kaf_elem = shift;

  my $hdr_elem = $ODOC->createElement("kafHeader");
  $kaf_elem->addChild($hdr_elem);
  my $lingp_elem = $ODOC->createElement('linguisticProcessors');
  $lingp_elem->setAttribute('layer', 'terms');
  $hdr_elem->addChild($lingp_elem);

  my $lp_elem = $ODOC->createElement('lp');
  $lp_elem->setAttribute('name', 'Freeling');
  $lp_elem->setAttribute('version', "3.0");
  $lp_elem->setAttribute('timestamp', &get_datetime());
  $lingp_elem->addChild($lp_elem);

}

sub build_kaf_doc {

  my ($ctrl) = @_;

  my $kaf_elem = $ODOC->createElement("KAF");
  $ODOC->setDocumentElement($kaf_elem);
  $kaf_elem->setAttribute("xml:lang", "es");

  # Create KAF header

  &add_kafHeader($kaf_elem);

  # Add words

  my $text_elem = $ODOC->createElement("text");
  $kaf_elem->addChild($text_elem);
  foreach my $elem (@{ $ctrl->{wf_array} }) {
    $text_elem->addChild($elem);
  }

  # Add terms

  my $terms_elem = $ODOC->createElement("terms");
  $kaf_elem->addChild($terms_elem);
  foreach my $elem (@{ $ctrl->{term_array} }) {
    $terms_elem->addChild($elem);
  }

  # Add chunks

  my $chunks_elem = $ODOC->createElement("chunks");
  $kaf_elem->addChild($chunks_elem);
  foreach my $elem (@{ $ctrl->{chunk_array} }) {
    $chunks_elem->addChild($elem);
  }

  # Add deps

  my $deps_elem = $ODOC->createElement("deps");
  $kaf_elem->addChild($deps_elem);
  foreach my $elem (@{ $ctrl->{dep_array} }) {
    $deps_elem->addChild($elem);
  }
}

# input format: each paragraph is separated by a newline

sub new_ctrl {

  my %ctrl = ("sent_n" => 0,
	      "para_n" => 0,
	      "wid_n"  => 0,
	      "tid_n" => 0,
	      "chunk_n" => 0,
	      "wf_array" => [],
	      "wf_elems" => {},
	      "w2wid" => {},     # word-form.para.sent.spanBegin.spanEnd => wid
	      "w2tid" => {},     # word-form.para.sent.spanBegin.spanEnd => tid
	      "term_array" => [],
	      "term_elems" => {},
	      "dep_array" => [],
	      "chunk_array" => []
	      );

  return %ctrl;
}

sub process_doc {

  my ($doc, $ctrl) = @_;

  my $sid = $sp->open_session();

  foreach my $par (split(/\n/, $doc)) {
    $ctrl->{'para_n'}++;
    #my $ls = Freeling_AnalyzeSentence($_);

    ## read input text and analyze it.
    my $l = $tk->tokenize($par);    # tokenize
    my $ls = $sp->split($sid,$l,0);	    # split sentences
    $ls = $morfo->analyze($ls);     # morphological analysis
    $ls = $tagger->analyze($ls);    # PoS tagging

    #&printSent($ls);

    foreach my $sent (@{ $ls }) {
      $ctrl->{'sent_n'}++;
      my $iter_beg = $sent->words_begin();
      my $iter_end = $sent->words_end();
      foreach my $w (@{ $sent->get_words() }) {
      	# add words and terms element
      	my @wids = &add_word_element($w, $ctrl);
      	&add_term_element($w, \@wids, $ctrl);
      }
    }
    $ls = $parser->analyze($ls);	# Chunking
    #&printTree($ls);
    foreach my $sent (@{ $ls }) {
      &add_chunks($sent->get_parse_tree()->begin(), $ctrl);
    }

    $ls = $dep->analyze($ls); # Dependency
    #&printDepTree($ls);
    foreach my $sent (@{ $ls }) {
      &add_deps($sent->get_dep_tree()->begin(), "", $ctrl);
    }
  }

  $sp->close_session($sid);
}

sub main {

  Freeling_CreateAnalyzers();

  my $str;
  while(my $l = <>) {
    chomp($l);
    $l .= " ." unless $l =~ /\.\s*$/;
    $str .= "$l\n";
  }
  my %ctrl = &new_ctrl();
  &process_doc($str, \%ctrl);
  &build_kaf_doc(\%ctrl);

  print $ODOC->toString(1)."\n";
}

#############################################################3


sub incr_id {
  my ($ctrl, $id) = @_;

  my $new_id;
  if($id eq "wid") {
    $new_id = "w" . ++$ctrl->{"wid_n"};
  } elsif ($id eq "tid") {
    $new_id = "t" . ++$ctrl->{"tid_n"};
  } elsif ($id eq "cid") {
    $new_id = "c" . ++$ctrl->{"cid_n"};
  } elsif ($id eq "did") {
    $new_id = "d" . ++$ctrl->{"did_n"};
  } else {
    die "Invalid id!"
  }
  $new_id .= "_$ctrl->{docId}" if defined $ctrl->{docId};
  return $new_id;

}


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
    printf("(%s %s %s)\n", $w->get_form(), $w->get_lemma(), $w->get_tag());
  } else {
    print $info->get_label() . "_[\n";
    for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); ++$d) {
      rec_printTree($d, $depth + 1);
    }
    print ' ' x ($depth*2);
    print "]\n";
  }
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
  printf("(%s %s %s %s)\n", $w->get_form(), $w->get_lemma(), $w->get_tag(), $info->get_node_id());

  if ($n->num_children() > 0) {
    print " [\n";
    for (my $d = $n->sibling_begin(); $d != $n->sibling_end(); $d++) {
      rec_printDepTree($d, $depth + 1) unless $d->get_info()->is_chunk();
    }
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
