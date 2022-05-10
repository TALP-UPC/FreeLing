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


/////////   MAIN SAMPLE PROGRAM  -- begin

int main (int argc, char **argv) {

  /// set locale to an UTF8 comaptible locale
  util::init_locale(L"default");

  wstring ipath;
  if(argc < 2) ipath=L"/usr/local";
  else ipath=util::string2wstring(argv[1]);

  wstring LANG = L"es";
  wstring spath = ipath + L"/share/freeling/";
  wstring lpath = spath + LANG;

  // if FreeLing was compiled with --enable-traces, you can activate
  // the required trace verbosity for the desired modules.
  //   traces::TraceLevel=4;
  //   traces::TraceModule=0xFFFFF;

  // create tokeizer and splitter
  tokenizer tk(lpath+L"/tokenizer.dat"); 
  splitter sp(lpath+L"/splitter.dat");

  // create options set for maco analyzer. 
  analyzer_config op;
  // define creation options for morphological analyzer modules
  op.config_opt.Lang = LANG;
  op.config_opt.MACO_PunctuationFile = spath + L"common/punct.dat";
  op.config_opt.MACO_DictionaryFile = lpath + L"/dicc.src";
  op.config_opt.MACO_AffixFile = lpath + L"/afixos.dat" ;
  op.config_opt.MACO_CompoundFile = lpath + L"/compounds.dat" ;
  op.config_opt.MACO_LocutionsFile = lpath + L"/locucions.dat";
  op.config_opt.MACO_NPDataFile = lpath + L"/np.dat";
  op.config_opt.MACO_QuantitiesFile = lpath + L"/quantities.dat";
  op.config_opt.MACO_ProbabilityFile = lpath + L"/probabilitats.dat";

  // chose which modules among those available will be used by default
  // (can be changed at each call if needed)
  op.invoke_opt.MACO_AffixAnalysis = true;
  op.invoke_opt.MACO_CompoundAnalysis = true;
  op.invoke_opt.MACO_MultiwordsDetection = true;
  op.invoke_opt.MACO_NumbersDetection = true;
  op.invoke_opt.MACO_PunctuationDetection = true ;
  op.invoke_opt.MACO_DatesDetection = true;
  op.invoke_opt.MACO_QuantitiesDetection = true;
  op.invoke_opt.MACO_DictionarySearch = true;
  op.invoke_opt.MACO_ProbabilityAssignment = true;
  op.invoke_opt.MACO_NERecognition = true;
  op.invoke_opt.MACO_RetokContractions = true;
  
  // create morphological analyzer
  maco mf(op);
  
  // create tagger, sense anotator, and parsers
  op.config_opt.TAGGER_HMMFile = lpath + L"/tagger.dat";
  op.invoke_opt.TAGGER_Retokenize = true;
  op.invoke_opt.TAGGER_ForceSelect = freeling::RETOK;
  hmm_tagger tg(op);

  wstring text = L"El gato come pescado. Los niños suben a los árboles.";

  // tokenize input line into a list of words
  list<word> lw = tk.tokenize(text);  
  // accumulate list of words in splitter buffer, returning a list of sentences.
  list<sentence> ls = sp.split(lw);
  
  // perform and output morphosyntactic analysis and disambiguation
  mf.analyze(ls);
  tg.analyze(ls);
  PrintMorfo(ls);
}

/////////   MAIN SAMPLE PROGRAM  -- end


//----------------------------------
/// Result processing functions
//----------------------------------


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
