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

/// Author:  Francis Tyers

#include <iostream>

/// headers to call freeling library
#include "freeling.h"

using namespace std;
using namespace freeling;

void PrintResults (list<sentence> &ls) {
  word::const_iterator ait;
  sentence::const_iterator w;
  list < sentence >::iterator is;
  int nsentence = 0;

  for (is = ls.begin (); is != ls.end (); is++, ++nsentence) {

      for (w = is->begin (); w != is->end (); w++) {
        wcout << w->get_form ();
	
        for (ait = w->selected_begin (); ait != w->selected_end (); ait++) {
          if (ait->is_retokenizable ()) {
            const list <word> &rtk = ait->get_retokenizable ();
            list <word>::const_iterator r;
            wstring lem, par;
            for (r = rtk.begin (); r != rtk.end (); r++) {
              lem = lem + L"+" + r->get_lemma ();
              par = par + L"+" + r->get_tag ();
            }
            wcout << L" " << lem.substr (1) << L" " 
                  << par.substr (1) << L" " << ait->get_prob ();
          }
          else {
            wcout << L" " << ait->get_lemma () << L" " << ait->
              get_tag () << L" " << ait->get_prob ();
          }
        }
        wcout << endl;	
      }
      // sentence separator: blank line.
      wcout << endl;
  }
}



///---------------------------------------------
/// Sample main program
///
///   The following program reads from stdin a plain text, tokenizes it, splits on
///   sentence boundaries, and looks up each word in the dictionary
///  
///---------------------------------------------
int main (int argc, char **argv) {
  tokenizer *tk = NULL;
  splitter *sp = NULL;
  maco *morfo = NULL;
  
  /// set locale to an UTF8 compatible locale
  util::init_locale(L"default");
  
  if(argc < 4) { 
    wcerr << L"fl1-morph" << endl;
    wcerr << L"Usage: fl-morph tokenizerFile splitterFile dictFile probabilitiesFile" << endl;
    wcerr << endl; 
    return 1;
  }
  
  tk = new tokenizer (util::string2wstring(argv[1]));
  sp = new splitter (util::string2wstring(argv[2]));
  splitter::session_id sid=sp->open_session();
  
  // "xx" should be the language of the input text, but since this is a 
  // very simple example, it doesn't matter much.
  maco_options opt (L"xx");     

  // and provide files for morphological submodules. Note that it is not necessary
  // to set opt.QuantitiesFile, since Quantities module was deactivated.
  opt.DictionaryFile=util::string2wstring(argv[3]);
  opt.ProbabilityFile=util::string2wstring(argv[4]);

  // create the analyzer with the above options.
  morfo = new maco (opt);
  // We use the bare minimum of modules: only dictionary search and probability assignment
  // then, set required options on/off  
  morfo->set_active_options (false,// UserMap
                             false, // NumbersDetection,
                             false, //  PunctuationDetection,
                             false, //  DatesDetection,
                             true, //  DictionarySearch,
                             false, //  AffixAnalysis,
                             false, //  CompoundAnalysis,
                             false, //  RetokContractions,
                             false, //  MultiwordsDetection,
                             false, //  NERecognition,
                             false, //  QuantitiesDetection,
                             true);  //  ProbabilityAssignment

  // read and process text
  wstring text;
  unsigned long offs = 0;
  list < word > av;
  list < word >::const_iterator i;
  list < sentence > ls;
  
  while (std::getline (std::wcin, text))  {
    av = tk->tokenize (text, offs);
    ls = sp->split (sid, av, false);
    morfo->analyze (ls);
    PrintResults (ls);
    
    av.clear ();		// clear list of words for next use
    ls.clear ();		// clear list of sentences for next use
  }
  
  // process last sentence in buffer (if any)
  ls = sp->split (sid, av, false);	//flush splitter buffer
  morfo->analyze (ls);
  PrintResults (ls);
  
  // clean up. 
  delete tk;
  sp->close_session(sid);
  delete sp;
  delete morfo;

  return 0;
}
