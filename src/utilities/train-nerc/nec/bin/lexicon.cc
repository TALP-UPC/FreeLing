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

// ####  Convert a corpus prepared for NER into features and produce
// ####  lexicon files.  See TRAIN.sh in this directory to learn more.

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <map>

#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/nerc_features.h"

using namespace std;
using namespace freeling;

fex *extractor;
fex_lexicon *lex;
bool create_lexicon;


// ------- Extract sentence features, print them, add to lexicon.

void process_sentence(sentence &s) {
  // extract features each word
  vector<set<wstring> > feats;
  extractor->encode_name(s, feats);      
  
  // store features in lexicon, and print encoded corpus 
  int i=0;
  for (sentence::const_iterator w=s.begin(); w!=s.end(); w++,i++) {
    if (w->get_tag()==L"NP00000") {
      wcout<<w->user[0];
      for (set<wstring>::iterator j=feats[i].begin(); j!=feats[i].end(); j++) {
        if (create_lexicon) lex->add_occurrence(*j);
        wcout<<L" "<<*j;
      }
      wcout<<endl;
    }
  }
}


// ----- Main program: read a sentence, encode it and store features appearing

int main(int argc, char* argv[]) {

  wstring text,form,lemma,tag, BIOtag, prob;
  sentence av;

  //traces::TraceModule=FEX_TRACE;
  //traces::TraceLevel=5;

  if (argc!=3) {
    wcerr<<L"Usage:  lexicon rules.rgf output_file <analyzed_text >encoded_text"<<endl;
    exit(1);
  }

  util::init_locale(L"default");

  wstring rgfFile = util::string2wstring(argv[1]);
  wstring lexFile = util::string2wstring(argv[2]);
  create_lexicon = (lexFile != L"-nolex");

  extractor = new fex(rgfFile, L"", nerc_features::functions);
  if (create_lexicon) lex = new fex_lexicon;
  
  while (getline(wcin,text)) {

    if (text != L"") { // got a word line
      wistringstream sin; 
      sin.str(text);

      sin>>form;    // get word form

      word w(form); // build new word

      // read analysis
      sin>>lemma>>tag;

      if (tag.substr(0,2)==L"NP") {
	w.user.push_back(tag); // remember right tag
	tag=L"NP00000";
      }
      analysis an(lemma,tag);
      w.add_analysis(an);
    
      // append new word to sentence
      av.push_back(w);
    }
    else { // blank line, sentence end.
      process_sentence(av);
      av.clear(); // clear list of words for next use
    }
  }
  
  if (not av.empty()) {  // no blank line after last sentence
    process_sentence(av);
    av.clear(); 
  }

  // --- Extraction process finished.
  // --- Save the lexicon to disk --if required--, with different filters
  if (create_lexicon) {
    // no features filtered, whole lexicon
    lex->save_lexicon(lexFile+L"-all.lex", 0);
    
    // filter out features occurring 5 or less/3 or less/1 or less times
    lex->save_lexicon(lexFile+L"-5abs.lex", 5);
    lex->save_lexicon(lexFile+L"-4abs.lex", 4);
    lex->save_lexicon(lexFile+L"-3abs.lex", 3);
    lex->save_lexicon(lexFile+L"-2abs.lex", 2);
    lex->save_lexicon(lexFile+L"-1abs.lex", 1);
  }
}


