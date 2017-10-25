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
void PrintResults(const list<sentence> &);
void PrintMorfo(const sentence &, int);
void PrintTree(const sentence &, int);
void PrintDepTree(const sentence &, int);

const int KBEST=3;

// analyzers
tokenizer *tk;
splitter *sp;
maco *morfo;
hmm_tagger *tagger;
chart_parser *parser;
dep_txala *dep;

/////////   MAIN SAMPLE PROGRAM  -- begin

int main (int argc, char **argv) {
  wstring text;
  list<word> lw;
  list<sentence> ls;

  /// set locale to an UTF8 comaptible locale
  util::init_locale(L"default");

  wstring ipath;
  if (argc < 2) ipath=L"/usr/local";
  else ipath=util::string2wstring(argv[1]);

  wstring path=ipath+L"/share/freeling/en/";

  // if FreeLing was compiled with --enable-traces, you can activate
  // the required trace verbosity for the desired modules.
  //  traces::TraceLevel=4;
  //  traces::TraceModule=TAGGER_TRACE;
  
  // create analyzers
  tk = new tokenizer(path+L"tokenizer.dat"); 
  sp = new splitter(path+L"splitter.dat");
  splitter::session_id sid=sp->open_session();
  
  // morphological analysis has a lot of options, and for simplicity they are packed up
  // in a maco_options object. First, create the maco_options object with default values.
  maco_options opt(L"es");  
  // and provide files for morphological submodules. Note that it is not necessary
  // to set opt.QuantitiesFile, since Quantities module was deactivated.
  opt.UserMapFile=L"";
  opt.LocutionsFile=path+L"locucions.dat"; opt.AffixFile=path+L"afixos.dat";
  opt.ProbabilityFile=path+L"probabilitats.dat"; opt.DictionaryFile=path+L"dicc.src";
  opt.NPdataFile=path+L"np.dat"; opt.PunctuationFile=path+L"../common/punct.dat"; 

  // create the analyzer with the just build set of maco_options
  morfo = new maco(opt); 
  // then, set required options on/off  
  morfo->set_active_options (false,// UserMap
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

  // create a hmm tagger for spanish (with retokenization ability, forced 
  // to choose only one tag per word, producing KBEST best sequences)
  tagger = new hmm_tagger(path+L"tagger.dat", true, FORCE_TAGGER, KBEST); 
  // create chunker
  parser = new chart_parser(path+L"chunker/grammar-chunk.dat");
  // create dependency parser 
  dep = new dep_txala(path+L"dep_txala/dependences.dat", parser->get_start_symbol());

  // get plain text input lines while not EOF.
  while (getline(wcin,text)) {
    
    // tokenize input line into a list of words
    lw=tk->tokenize(text);
    
    // accumulate list of words in splitter buffer, returning a list of sentences.
    // The resulting list of sentences may be empty if the splitter has still not 
    // enough evidence to decide that a complete sentence has been found. The list
    // may contain more than one sentence (since a single input line may consist 
    // of several complete sentences).
    ls=sp->split(sid, lw, false);
    
    // Analyze sentences 
    morfo->analyze(ls);   // morphological analysis
    tagger->analyze(ls);  // PoS tagging 
    parser->analyze(ls);  // shallow parser (chunker)
    dep->analyze(ls);     // dependency parsing

    PrintResults(ls);

    // clear temporary lists;
    lw.clear(); ls.clear();    
  }
  
  // No more lines to read. Make sure the splitter doesn't retain anything  
  sp->split(sid, lw, true, ls);   
 
  // analyze sentence(s) that might be lingering in the buffer, if any.
  morfo->analyze(ls);
  tagger->analyze(ls);
  parser->analyze(ls);
  dep->analyze(ls);
  PrintResults(ls);

  delete tk;
  sp->close_session(sid);
  delete sp;
  delete morfo;
  delete tagger;
}

/////////   MAIN SAMPLE PROGRAM  -- end


//----------------------------------
/// Result processing functions
//----------------------------------

void PrintResults(const list<sentence> &ls) {
  int n=0;
  for (list<sentence>::const_iterator is=ls.begin(); is!=ls.end(); is++,n++) {
    // for each of the k best sequences proposed by the tagger
    for (int k=0; k<is->num_kbest(); k++) {
      wcout<<L"<BEST_SEQUENCE k=\""<<k<<L"\" perplexity=\""<<tagger->Perplexity(*is,k)<<L"\">"<<endl;
      wcout<<L"<TAGGING>"<<endl;
      PrintMorfo(*is,k);
      wcout<<L"</TAGGING>"<<endl;
      wcout<<L"<PARSING>"<<endl;
      PrintTree(*is,k);
      wcout<<L"</PARSING>"<<endl;
      wcout<<L"<DEPENDENCIES>"<<endl;
      PrintDepTree(*is,k);
      wcout<<L"</DEPENDENCIES>"<<endl;
      wcout<<L"</BEST_SEQUENCE>"<<endl;
    }
    
    wcout <<endl;
  }

}

//---------------------------------------------
// print morphological information
//---------------------------------------------

void PrintMorfo(const sentence &s, int k) {
  for (sentence::const_iterator w=s.begin(); w!=s.end(); w++) 
    wcout<<w->get_form()<<L" "<<w->get_lemma(k)<<L" "<<w->get_tag(k)<<endl;
}

//---------------------------------------------
// print syntactic tree
//---------------------------------------------

void rec_PrintTree(parse_tree::const_iterator n, int k, int depth) {
  parse_tree::const_sibling_iterator d;
  
  wcout<<wstring(depth*3,' '); 
  if (n.num_children()==0) { 
    const word & w=n->get_word();
    wcout<<(n->is_head()? L"+" : L"")<< L"("<<w.get_form()<<L" "<<w.get_lemma(k)<<L" "<<w.get_tag(k)<<")"<<endl;
  }
  else { 
    wcout<<(n->is_head()? L"+" : L"")<<n->get_label()<<L"_["<<endl;
    for (d=n.sibling_begin(); d!=n.sibling_end(); d++)
      rec_PrintTree(d, k, depth+1);

    wcout<<wstring(depth*3,' ')<<L"]"<<endl;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - -
void PrintTree(const sentence &s, int k) {
  rec_PrintTree(s.get_parse_tree(k).begin(), k, 0);
}

//---------------------------------------------
// print dependency tree
//---------------------------------------------

void rec_PrintDepTree(dep_tree::const_iterator n, int k, int depth) {
  dep_tree::const_sibling_iterator d,dm;
  int last, min;
  bool trob;
  
    wcout<<wstring(depth*3,' '); 

    wcout<<n->get_link()->get_label()<<L"/"
         <<n->get_label()<<"/";
    const word & w=n->get_word();
    wcout<<L"("<<w.get_form()<<L" "<<w.get_lemma(k)<<L" "<<w.get_tag(k)<<L")";

    if (n.num_children()>0) { 
       wcout<<" ["<<endl;

       //Print Nodes
       for (d=n.sibling_begin(); d!=n.sibling_end(); ++d)
         if(!d->is_chunk())
           rec_PrintDepTree(d, k, depth+1);
       
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
         if (trob) rec_PrintDepTree(dm, k, depth+1);
         last=min;
       }       
       wcout<<wstring(depth*3,' ')<<L"]"; 
    }

    wcout<<endl;
    
}

// - - - - - - - - - - - - - - - - - - - - - - -
void PrintDepTree(const sentence &s, int k) {
  rec_PrintDepTree(s.get_dep_tree(k).begin(), k, 0);
}




