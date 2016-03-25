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


#include <iostream>

#include "freeling.h"

using namespace std;
using namespace freeling;

/// predeclarations
void print_corefs(const document &doc);


//////////////// MAIN SAMPLE PROGRAM /////////////////////

int main (int argc, char **argv) {
  /// set locale to an UTF8 compatible locale
  util::init_locale(L"default");
  
  wstring ipath;
  if(argc < 2) ipath=L"/usr/local";
  else ipath=util::string2wstring(argv[1]);

  wstring path=ipath+L"/share/freeling/en/";
  
  // create analyzers
  tokenizer tk(path+L"tokenizer.dat");
  splitter sp(path+L"splitter.dat");

  // morphological analysis has a lot of options, and for simplicity they are packed up
  // in a maco_options object. First, create the maco_options object with default values.
  maco_options opt(L"es");
  opt.UserMapFile=L"";
  opt.LocutionsFile=path+L"locucions.dat"; opt.AffixFile=path+L"afixos.dat";
  opt.ProbabilityFile=path+L"probabilitats.dat"; opt.DictionaryFile=path+L"dicc.src";
  opt.NPdataFile=path+L"np.dat"; opt.PunctuationFile=path+L"../common/punct.dat"; 

  // create the analyzer with the just build set of maco_options
  maco morfo(opt);
  // enable/disable desired modules
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
                             true, //  QuantitiesDetection,
                             true);  //  ProbabilityAssignment
  
  // create a hmm tagger for spanish (with retokenization ability, and forced
  // to choose only one tag per word)
  hmm_tagger tagger(path+L"tagger.dat", true, true);
  // create a shallow parser
  chart_parser parser(path+L"chunker/grammar-chunk.dat");  
  // create a NE classifier
  nec neclass(path+L"nerc/nec/nec-ab-poor1.dat");
  // create a rule-based dep parser
  dep_txala txala(path+L"dep_txala/dependences.dat", parser.get_start_symbol ());
  // statistical dep-parser and SRL
  dep_treeler treeler(path+L"dep_treeler/labeled/dependences.dat");
  // create a coreference solver
  relaxcor coref(path+L"coref/relaxcor/relaxcor.dat)");

  wstring text,line;
  // get plain text input lines while not EOF.
  while (getline(wcin,text)) 
    text = text + line + L"\n";

  // tokenize and split text into a list of sentences
  list<word> lw = tk.tokenize(text);  
  splitter::session_id sid=sp.open_session();
  list<sentence> ls = sp.split(sid, lw, false);
  sp.close_session(sid);

  // load all sentences as a single paragraph in a document 
  // (Might be more than one paragraph if that information is
  // avaliable from input format) 
  document doc;
  doc.insert(doc.end(),ls);
    
  // Process text in the document with all required modules
  morfo.analyze(doc);
  tagger.analyze(doc);
  neclass.analyze(doc);
  parser.analyze(doc);
  txala.complete_parse_tree(doc);
  treeler.analyze(doc);
  coref.analyze(doc);

  // output results
  print_corefs(doc);

}


/////////////////////////////////////////////////////
/// print all coreference groups found in a document

void print_corefs(const document &doc) {
  // Output detected corefence groups
  for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {
    wcout << L"CorefGroup " << *g+1 << endl;      
    list<int> mentions = doc.get_coref_id_mentions(*g);
    int nm=1;
    for (list<int>::iterator m=mentions.begin(); m!=mentions.end(); m++) {
      const mention & ment = doc.get_mention(*m);
      const sentence &sent = (*ment.get_sentence());
      wstring sid = util::int2wstring(ment.get_n_sentence()+1);
      
      int j = ment.get_pos_begin();
      wstring words = sent[j].get_form();
      for (j=ment.get_pos_begin()+1; j<=ment.get_pos_end(); j++) 
        words = words + L" " + sent[j].get_form();

      wcout << L"     Mention "<< *g+1 << L"." << nm << endl;
      wcout << L"          start: " << util::int2wstring(sid)+L"."+util::int2wstring(ment.get_pos_begin()+1) << endl;
      wcout << L"          end: " << util::int2wstring(sid)+L"."+util::int2wstring(ment.get_pos_end()+1) << endl;
      wcout << L"          words: \"" << words << "\"" << endl;
      nm++;
    }      
    wcout << endl;
  }  
}
