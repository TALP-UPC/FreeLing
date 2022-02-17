import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.File;
import java.math.*;
    
import edu.upc.Jfreeling.*;

public class Sample {

  private static final String OS = System.getProperty("os.name").toLowerCase();
    
  public static void main( String argv[] ) throws IOException {
    // connect to FreeLing library
    System.loadLibrary( "Jfreeling" );

    // Check whether we know where to find FreeLing data files
    String FLDIR = System.getenv("FREELINGDIR");
    if (FLDIR==null) {
	if (OS.indexOf("win") >= 0) FLDIR = "C:\\Program Files";
	else  FLDIR = "/usr/local";
	System.err.println("FREELINGDIR environment variable not defined, trying "+FLDIR);
    }

    File f = new File(FLDIR+"/share/freeling");
    if (! f.exists()) {
	System.err.println("Folder "+FLDIR+"/share/freeling not found.");
        System.err.println("Please set FREELINGDIR environment variable to FreeLing installation directory");
	System.exit(1);
    }
    
    // Location of FreeLing configuration files.
    String DATA = FLDIR + "/share/freeling/";

    //Uncomment these to activate FreeLing debugging traces
    //Traces.setTraceLevel(5);
    //BigInteger bi = new BigInteger("000F",16);
    //Traces.setTraceModule(bi);
    
    // Init locales
    Util.initLocale( "default" );

    // Create options set for maco analyzer.
    String LANG = "en";

    // define creation options for morphological analyzer modules
    AnalyzerConfig op = new AnalyzerConfig();
    op.getConfig_opt().setLang(LANG);
    op.getConfig_opt().setMACO_PunctuationFile(DATA + "common/punct.dat");
    op.getConfig_opt().setMACO_DictionaryFile(DATA + LANG + "/dicc.src");
    op.getConfig_opt().setMACO_AffixFile(DATA + LANG + "/afixos.dat");
    op.getConfig_opt().setMACO_CompoundFile(DATA + LANG + "/compounds.dat");
    op.getConfig_opt().setMACO_LocutionsFile(DATA + LANG + "/locucions.dat");
    op.getConfig_opt().setMACO_NPDataFile(DATA + LANG + "/np.dat");
    op.getConfig_opt().setMACO_QuantitiesFile(DATA + LANG + "/quantities.dat");
    op.getConfig_opt().setMACO_ProbabilityFile(DATA + LANG + "/probabilitats.dat");

    // chose which modules among those available will be used by default
    // (can be changed at each call if needed)
    op.getInvoke_opt().setMACO_AffixAnalysis(true);
    op.getInvoke_opt().setMACO_CompoundAnalysis(true);
    op.getInvoke_opt().setMACO_MultiwordsDetection(true);
    op.getInvoke_opt().setMACO_NumbersDetection(true);
    op.getInvoke_opt().setMACO_PunctuationDetection(true );
    op.getInvoke_opt().setMACO_DatesDetection(true);
    op.getInvoke_opt().setMACO_QuantitiesDetection(true);
    op.getInvoke_opt().setMACO_DictionarySearch(true);
    op.getInvoke_opt().setMACO_ProbabilityAssignment(true);
    op.getInvoke_opt().setMACO_NERecognition(true);
    op.getInvoke_opt().setMACO_RetokContractions(true);

    // Create analyzers.

    // language detector. Used just to show it. Results are printed
    // but ignored (after, it is assumed language is LANG)
    LangIdent lgid = new LangIdent(DATA + "/common/lang_ident/ident-few.dat");

    Tokenizer tk = new Tokenizer( DATA + LANG + "/tokenizer.dat" );
    Splitter sp = new Splitter( DATA + LANG + "/splitter.dat" );
    SWIGTYPE_p_splitter_status sid = sp.openSession();

    Maco mf = new Maco( op );

    op.getConfig_opt().setTAGGER_HMMFile(DATA + LANG + "/tagger.dat");
    op.getInvoke_opt().setTAGGER_Retokenize(true);
    op.getInvoke_opt().setTAGGER_ForceSelect(ForceSelectStrategy.RETOK);
    HmmTagger tg = new HmmTagger(op);
    
    ChartParser parser = new ChartParser( DATA + LANG + "/chunker/grammar-chunk.dat" );
    
    // Uncomment this for rule-based dependency parsing
    // DepTxala dep = new DepTxala( DATA + LANG + "/dep_txala/dependences.dat", parser.getStartSymbol() );

    // Uncomment this for ML-based statistical dependency parsing
    //DepTreeler dep = new DepTreeler( DATA + LANG + "/treeler/dependences.dat" );

    // Uncomment this for LSTM-based statistical dependency parsing
    DepLstm dep = new DepLstm( DATA + LANG + "/dep_lstm/params-en.dat" );

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
      // word sense annotation and disambiguation
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

    // flush any sentences lingering in the splitter buffer 
    System.out.println( "-------- Flushing buffers -----------" );
    ListSentence ls = sp.split( sid, tk.tokenize(""), true );
    mf.analyze( ls );
    tg.analyze( ls );
    neclass.analyze( ls );
    sen.analyze( ls );
    dis.analyze( ls );
    printResults( ls, "tagged" );
    parser.analyze( ls );
    printResults( ls, "parsed" );
    dep.analyze( ls );
    printResults( ls, "dep" );
    
    sp.closeSession(sid);
  }


  private static void ProcessLine(String line, boolean flush) {
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

