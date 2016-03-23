import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

import edu.upc.freeling.*;

public class Analyzer {
  // Modify this line to be your FreeLing installation directory
  private static final String FREELINGDIR = "/usr/local";
  private static final String DATA = FREELINGDIR + "/share/freeling/";
  private static final String LANG = "es";

  public static void main( String argv[] ) throws IOException {
    System.loadLibrary( "freeling_javaAPI" );

    Util.initLocale( "default" );

    // Create options set for maco analyzer.
    // Default values are Ok, except for data files.
    MacoOptions op = new MacoOptions( LANG );

    op.setDataFiles( "", 
                     DATA + "common/punct.dat",
                     DATA + LANG + "/dicc.src",
                     DATA + LANG + "/afixos.dat",
                     "",
                     DATA + LANG + "/locucions.dat", 
                     DATA + LANG + "/np.dat",
                     DATA + LANG + "/quantities.dat",
                     DATA + LANG + "/probabilitats.dat");

    // Create analyzers.

    // language detector. Used just to show it. Results are printed  but ignored. 
    // See below.
    LangIdent lgid = new LangIdent(DATA + "/common/lang_ident/ident.dat");

    Tokenizer tk = new Tokenizer( DATA + LANG + "/tokenizer.dat" );
    Splitter sp = new Splitter( DATA + LANG + "/splitter.dat" );
    SWIGTYPE_p_splitter_status sid = sp.openSession();

    Maco mf = new Maco( op );
    mf.setActiveOptions(false, true, true, true,  // select which among created 
                        true, true, false, true,  // submodules are to be used. 
                        true, true, true, true);  // default: all created submodules 
                                                  // are used

    HmmTagger tg = new HmmTagger( DATA + LANG + "/tagger.dat", true, 2 );
    ChartParser parser = new ChartParser(
      DATA + LANG + "/chunker/grammar-chunk.dat" );
    DepTxala dep = new DepTxala( DATA + LANG + "/dep_txala/dependences.dat",
      parser.getStartSymbol() );
    Nec neclass = new Nec( DATA + LANG + "/nerc/nec/nec-ab-poor1.dat" );

    Senses sen = new Senses(DATA + LANG + "/senses.dat" ); // sense dictionary
    Ukb dis = new Ukb( DATA + LANG + "/ukb.dat" ); // sense disambiguator

    // Make sure the encoding matches your input text (utf-8, iso-8859-15, ...)
    BufferedReader input = new BufferedReader(
      new InputStreamReader( System.in, "utf-8" ) );
    String line = input.readLine();

    // Identify language of the text.  
    // Note that this will identify the language, but will NOT adapt
    // the analyzers to the detected language.  All the processing 
    // in the loop below is done by modules for LANG (set to "es" at
    // the beggining of this class) created above.
    String lg = lgid.identifyLanguage(line);
    System.out.println( "-------- LANG_IDENT results -----------" );
    System.out.println("Language detected (from first line in text): " + lg);

    while( line != null ) {
      // Extract the tokens from the line of text.
      ListWord l = tk.tokenize( line );

      // Split the tokens into distinct sentences.
      ListSentence ls = sp.split( sid, l, false );

      // Perform morphological analysis
      mf.analyze( ls );

      // Perform part-of-speech tagging.
      tg.analyze( ls );

      // Perform named entity (NE) classificiation.
      neclass.analyze( ls );

      sen.analyze( ls );
      dis.analyze( ls );
      printResults( ls, "tagged" );

      // Chunk parser
      parser.analyze( ls );
      printResults( ls, "parsed" );

      // Dependency parser
      dep.analyze( ls );
      printResults( ls, "dep" );

      line = input.readLine();
    }

    sp.closeSession(sid);
  }

  private static void printSenses( Word w ) {
    String ss = w.getSensesString();

    // The senses for a FreeLing word are a list of
    // pair<string,double> (sense and page rank). From java, we
    // have to get them as a string with format
    // sense:rank/sense:rank/sense:rank
    // which will have to be splitted to obtain the info.
    //
    // Here, we just output it:
    System.out.print( " " + ss );
  }

  private static void printResults( ListSentence ls, String format ) {

    if (format == "parsed") {
      System.out.println( "-------- CHUNKER results -----------" );

      ListSentenceIterator sIt = new ListSentenceIterator(ls);
      while (sIt.hasNext()) {
	Sentence s = sIt.next();
        ParseTree tree = s.getParseTree();
        printParseTree( 0, tree );
      }
    }
    else if (format == "dep") {
      System.out.println( "-------- DEPENDENCY PARSER results -----------" );

      ListSentenceIterator sIt = new ListSentenceIterator(ls);
      while (sIt.hasNext()) {
	Sentence s = sIt.next();
        DepTree tree = s.getDepTree();
        printDepTree( 0, tree);
      }
    }
    else
    {
      System.out.println( "-------- TAGGER results -----------" );

      // get the analyzed words out of ls.
      ListSentenceIterator sIt = new ListSentenceIterator(ls);
      while (sIt.hasNext()) {
        Sentence s = sIt.next();
        ListWordIterator wIt = new ListWordIterator(s);
        while (wIt.hasNext()) {
          Word w = wIt.next();

          System.out.print(
            w.getForm() + " " + w.getLemma() + " " + w.getTag() );
          printSenses( w );
          System.out.println();
        }

        System.out.println();
      }
    }
  }

  private static void printParseTree( int depth, ParseTree tr ) {
    Word w;
    long nch;

    // Indentation
    for( int i = 0; i < depth; i++ ) {
      System.out.print( "  " );
    }

    nch = tr.numChildren();

    if( nch == 0 ) {
      // The node represents a leaf
        if( tr.begin().getInformation().isHead() ) {
        System.out.print( "+" );
      }
      w = tr.begin().getInformation().getWord();
      System.out.print("(" + w.getForm() + " " + w.getLemma() + " " + w.getTag() );
      printSenses( w );
      System.out.println( ")" );
    }
    else
    {
      // The node is non-terminal
      if ( tr.begin().getInformation().isHead() ) {
        System.out.print( "+" );
      }

      System.out.println( tr.begin().getInformation().getLabel() + "_[" );

      for( int i = 0; i < nch; i++ ) {
        ParseTree child = tr.nthChildRef( i );

        if ( ! child.empty() ) {
          printParseTree( depth + 1, child );
        }
        else {
          System.err.println( "ERROR: Unexpected NULL child." );
        }
      }

      for( int i = 0; i < depth; i++ ) {
        System.out.print( "  " );
      }

      System.out.println( "]" );
    }
  }

  private static void printDepTree( int depth, DepTree tr ) {
    DepTree child = null;
    DepTree fchild = null;
    long nch;
    int last, min;
    Boolean trob;

    for( int i = 0; i < depth; i++ ) {
      System.out.print( "  " );
    }

    System.out.print(
      tr.begin().getLink().getLabel() + "/" +
      tr.begin().getLabel() + "/" );

    Word w = tr.begin().getWord();

    System.out.print(
      "(" + w.getForm() + " " + w.getLemma() + " " + w.getTag() );
    printSenses( w );
    System.out.print( ")" );

    nch = tr.numChildren();

    if( nch > 0 ) {
      System.out.println( " [" );

      for( int i = 0; i < nch; i++ ) {
        child = tr.nthChildRef( i );

        if( child != null ) {
          if( !child.begin().isChunk() ) {
            printDepTree( depth + 1, child );
          }
        }
        else {
          System.err.println( "ERROR: Unexpected NULL child." );
        }
      }

      // Print chunks (in order)
      last = 0;
      trob = true;

      // While an unprinted chunk is found, look for the one with lower
      // chunk_ord value.
      while( trob ) {
        trob = false;
        min = 9999;

        for( int i = 0; i < nch; i++ ) {
          child = tr.nthChildRef( i );

          if( child.begin().isChunk() ) {
            if( (child.begin().getChunkOrd() > last) &&
                (child.begin().getChunkOrd() < min) ) {
              min = child.begin().getChunkOrd();
              fchild = child;
              trob = true;
            }
          }
        }
        if( trob && (child != null) ) {
          printDepTree( depth + 1, fchild );
        }

        last = min;
      }

      for( int i = 0; i < depth; i++ ) {
        System.out.print( "  " );
      }

      System.out.print( "]" );
    }

    System.out.println( "" );
  }
}

