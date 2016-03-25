
################# Simple tagset class to return short tag  ###########

package Tagset;

sub new {
  my $class = shift;
  my $file = shift;

  my $self = {};
  bless $self, $class;

  # load file
  open(my $fh, "<", $file);
  while (<$fh>) {
      if ($_ =~ /^</) {
          chomp;
          $section = $_; 
          next;
      }

      if ($section eq "<DirectTranslations>") {
          ($tag,$sht) = split;
          $self{Direct}{$tag} = $sht;
      }
      elsif ($section eq "<DecompositionRules>") {
          ($tag,$digs) = split;
          $self{Digits}{$tag} = $digs;
      }

  }

  return $self;
};


sub ShortTag {
    my $self = shift;
    my $tag = shift;

    if ($tag eq "OUT_OF_BOUNDS") { return $tag; }

    elsif ($self{Direct}{$tag}) { return $self{Direct}{$tag}; }
 
    elsif (! defined $self{Digits}{substr($tag,0,1)}) {
        print STDERR "Unknown tag $tag\n";
        return $tag;
    }

    else {
        my @x = split (",",$self{Digits}{substr($tag,0,1)});
        if (@x == 1) {
            if ($x[0] == 0) { return $tag; }
            else { return substr($tag,0,$x[0]); }
        }
        else {
            my $t = ""; 
            for ($i=0; $i<@x; $i++)  {   
                if ($x[$i] < length($tag)) {
                    $t = $t.substr($tag,$x[$i],1);
                }
                else {
                    print STDERR "Tag $tag too short for required digits\n"; 
                    return $tag;
                }
            }            
            return $t;
        }

        return $tag
    }

};

1;

