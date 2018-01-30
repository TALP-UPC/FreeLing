//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


//------------------------------------------------------------------//
//
//                    IMPORTANT NOTICE
//
//  This file contains a sample main program to illustrate 
//  usage of FreeLing analyzers library.
//
//  This sample main program may be used straightforwardly as 
//  a basic front-end for the analyzers (e.g. to analyze corpora)
//
//  Neverthless, if you want embed the FreeLing libraries inside
//  a larger application, or you want to deal with other 
//  input/output formats (e.g. XML,HTML,...), the efficient and elegant 
//  way to do so is consider this file as a mere example, and call 
//  the library from your your own main code.
//
//  See README file to find out how to compile and execute this sample
//
//------------------------------------------------------------------//


#include <iostream>

#include "freeling.h"
#include "freeling/morfo/traces.h"

using namespace std;
using namespace freeling;

// predeclarations
void PrintMorfo(list<sentence> &ls);
void PrintTree(list<sentence> &ls);
void PrintDepTree(list<sentence> &ls);


/////////   MAIN SAMPLE PROGRAM  -- begin

int main (int argc, char **argv) {
  wstring text;
  list<word> lw;
  list<sentence> ls;

  /// set locale to an UTF8 comaptible locale
  util::init_locale(L"default");

  wstring ipath;
  if(argc < 2) ipath=L"/usr/local";
  else ipath=util::string2wstring(argv[1]);

  wstring path=ipath+L"/share/freeling/es/";

  // if FreeLing was compiled with --enable-traces, you can activate
  // the required trace verbosity for the desired modules.
  //   traces::TraceLevel=4;
  //   traces::TraceModule=0xFFFFF;
  
  // create analyzers
  tokenizer tk(path+L"tokenizer.dat"); 
  splitter sp(path+L"splitter.dat");
  splitter::session_id sid=sp.open_session();

  // morphological analysis has a lot of options, and for simplicity they are packed up
  // in a maco_options object. First, create the maco_options object with default values.
  maco_options opt(L"es");  
  // and provide files for morphological submodules. Note that it is not necessary
  // to set opt.QuantitiesFile, since Quantities module was deactivated.
  opt.UserMapFile=L"";
  opt.LocutionsFile=path+L"locucions.dat"; opt.AffixFile=path+L"afixos.dat";
  opt.ProbabilityFile=path+L"probabilitats.dat"; opt.DictionaryFile=path+L"dicc.src";
  opt.NPdataFile=path+L"np.dat"; opt.PunctuationFile=path+L"../common/punct.dat"; 
  // alternatively, you can set the files in a single call:
  // opt.set_data_files("", path+"locucions.dat", "", path+"afixos.dat",
  //                   path+"probabilitats.dat", opt.DictionaryFile=path+"maco.db",
  //                   path+"np.dat", path+"../common/punct.dat");

  // create the analyzer with the just build set of maco_options
  maco morfo(opt); 
  // then, set required options on/off  
  morfo.set_active_options (false,// UserMap
                             true, // NumbersDetection,
                             true, //  PunctuationDetection,
                             true, //  DatesDetection,
                             true, //  DictionarySearch,
                             true, //  AffixAnalysis,
                             false, //  CompoundAnalysis,
                             true, //  RetokContractions,
                             true, //  MultiwordsDetection,
                             true, //  NERecognition,
                             false, //  QuantitiesDetection,
                             true);  //  ProbabilityAssignment

  // create a hmm tagger for spanish (with retokenization ability, and forced 
  // to choose only one tag per word)
  hmm_tagger tagger(path+L"tagger.dat", true, FORCE_TAGGER); 
  // create chunker
  chart_parser parser(path+L"chunker/grammar-chunk.dat");
  // create dependency parser 
  dep_txala dep(path+L"dep_txala/dependences.dat", parser.get_start_symbol());
  
  // get plain text input lines while not EOF.
  while (getline(wcin,text)) {

    // tokenize input line into a list of words
    lw=tk.tokenize(text);
    
    // accumulate list of words in splitter buffer, returning a list of sentences.
    // The resulting list of sentences may be empty if the splitter has still not 
    // enough evidence to decide that a complete sentence has been found. The list
    // may contain more than one sentence (since a single input line may consist 
    // of several complete sentences).
    ls=sp.split(sid, lw, false);
    
    // perform and output morphosyntactic analysis and disambiguation
    morfo.analyze(ls);
    tagger.analyze(ls);
    PrintMorfo(ls);

    // perform and output Chunking
    parser.analyze(ls);
    PrintTree(ls);

    // Dep. parsing
    dep.analyze(ls);
    PrintDepTree(ls);
    
    // clear temporary lists;
    lw.clear(); ls.clear();    
  }

  // No more lines to read. Make sure the splitter doesn't retain anything  
  sp.split(sid, lw, true, ls);   
 
  // analyze sentence(s) which might be lingering in the buffer, if any.
  morfo.analyze(ls);
  tagger.analyze(ls);
  PrintMorfo(ls);

  parser.analyze(ls);
  PrintTree(ls);

  dep.analyze(ls);
  PrintDepTree(ls);
  
  sp.close_session(sid);
}

/////////   MAIN SAMPLE PROGRAM  -- end


//----------------------------------
/// Result processing functions
//----------------------------------


//---------------------------------------------
// print morphological information
//---------------------------------------------

void PrintMorfo(list<sentence> &ls) {
  word::const_iterator a;
  sentence::const_iterator w;

  wcout<<L"----------- MORPHOLOGICAL INFORMATION -------------"<<endl;

  // print sentence XML tag
  for (list<sentence>::iterator is=ls.begin(); is!=ls.end(); is++) {

    wcout<<L"<SENT>"<<endl;
    // for each word in sentence
    for (w=is->begin(); w!=is->end(); w++) {
      
      // print word form, with PoS and lemma chosen by the tagger
      wcout<<L"  <WORD form=\""<<w->get_form();
      wcout<<L"\" lemma=\""<<w->get_lemma();
      wcout<<L"\" pos=\""<<w->get_tag();
      wcout<<L"\">"<<endl;
      
      // for each possible analysis in word, output lemma, tag and probability
      for (a=w->analysis_begin(); a!=w->analysis_end(); ++a) {
	
	// print analysis info
	wcout<<L"    <ANALYSIS lemma=\""<<a->get_lemma();
	wcout<<L"\" pos=\""<<a->get_tag();
	wcout<<L"\" prob=\""<<a->get_prob();
	wcout<<L"\"/>"<<endl;
      }
      
      // close word XML tag after list of analysis
      wcout<<L"  </WORD>"<<endl;
    }
    
    // close sentence XML tag
    wcout<<L"</SENT>"<<endl;
  }
}  


//---------------------------------------------
// print syntactic tree
//---------------------------------------------

void rec_PrintTree(parse_tree::iterator n, int depth) {
  parse_tree::sibling_iterator d;
  
  wcout<<wstring(depth*3,' '); 
  if (n.num_children()==0) { 
    const word & w=n->get_word();
    wcout<<L"<WORD form=\""<<w.get_form()<<L"\" lemma=\""<<w.get_lemma()<<L"\" tag=\""<<w.get_tag()<<"\" head=\""<<(n->is_head()? L"1" : L"0")<<"\" />"<<endl;
  }
  else { 
    wcout<<L"<CHUNK type=\""<<n->get_label()<<L"\" head=\""<<(n->is_head()? L"1" : L"0")<<"\">"<<endl;
    for (d=n.sibling_begin(); d!=n.sibling_end(); ++d)
      rec_PrintTree(d, depth+1);

    wcout<<wstring(depth*3,' ')<<L"</CHUNK>"<<endl;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - -
void PrintTree(list<sentence> &ls) {

  wcout<<L"----------- CHUNKING -------------"<<endl; 
  for (list<sentence>::iterator is=ls.begin(); is!=ls.end(); is++) {
    parse_tree &tr = is->get_parse_tree();
    rec_PrintTree(tr.begin(), 0);
  }
}

//---------------------------------------------
// print dependency tree
//---------------------------------------------

void rec_PrintDepTree(dep_tree::iterator n, int depth) {
  dep_tree::sibling_iterator d,dm;
  int last, min;
  bool trob;
  
    wcout<<wstring(depth*3,' '); 

    wcout<<"<NODE func=\""<<n->get_label()<<"\" synt=\""
         <<n->get_link()->get_label()<<L"\"";
    const word & w=n->get_word();
    wcout<<L" form=\""<<w.get_form()<<L"\" lemma=\""<<w.get_lemma()
         <<L"\" tag=\""<<w.get_tag()<<L"\"";  

    if (n.num_children()>0) { 
       wcout<<">"<<endl;

       //Print Nodes
       for (d=n.sibling_begin(); d!=n.sibling_end(); ++d)
	 if(!d->is_chunk())
	   rec_PrintDepTree(d, depth+1);

       // print CHUNKS (in order)
       last=0; trob=true;
       while (trob) { 
	 // while unprinted chunks remain, print the one with lower chunk_ord value
	 trob=false; min=9999;  
	 for (d=n.sibling_begin(); d!=n.sibling_end(); ++d) {
	   if(d->is_chunk()) {
	     if (d->get_chunk_ord()>last && d->get_chunk_ord()<min) {
	       min=d->get_chunk_ord();
	       dm=d;
	       trob=true;
	     }
	   }
	 }
	 if (trob) rec_PrintDepTree(dm, depth+1);
	 last=min;
       }

       wcout<<wstring(depth*3,' ')<<L"</NODE>"<<endl; 
    }
    else 
       wcout<<" />"<<endl;

}

// - - - - - - - - - - - - - - - - - - - - - - -
void PrintDepTree(list<sentence> &ls) {

  wcout<<L"----------- DEPENDENCIES -------------"<<endl;
  for (list<sentence>::iterator is=ls.begin(); is!=ls.end(); is++) {
    dep_tree &dep = is->get_dep_tree();
    rec_PrintDepTree(dep.begin(), 0);
  }
}


