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

#include <sstream>
#include <iostream>

#include <map>
#include <vector>

/// headers to call freeling library
#include "freeling.h"

using namespace std;
using namespace freeling;

void PrintDepTree (dep_tree::iterator n, int depth)  {
  dep_tree::sibling_iterator d, dm;
  int last, min;
  bool trob;

  wcout << wstring (depth*2, ' ');

  wcout << n->get_link()->get_label(); 
  wcout <<L"/" << n->get_label() << L"/";

  const word & w = n->get_word();
  wcout << L"(" << w.get_form() << L" " << w.get_lemma() << L" " << w.get_tag ();
  wcout << L")";
  
  if (n.num_children () > 0) {
    wcout << L" [" << endl;
    
    // Print Nodes
    for (d = n.sibling_begin (); d != n.sibling_end (); ++d)
      if (!d->is_chunk ())
        PrintDepTree (d, depth + 1);
    
    // print CHUNKS (in order)
    last = 0;
    trob = true;
    while (trob) {
      // while an unprinted chunk is found look, for the one with lower chunk_ord value
      trob = false;
      min = 9999;
      for (d = n.sibling_begin (); d != n.sibling_end (); ++d) {
        if (d->is_chunk ()) {
          if (d->get_chunk_ord () > last
              && d->get_chunk_ord () < min) {
            min = d->get_chunk_ord ();
            dm = d;
            trob = true;
          }
        }
      }
      if (trob)
        PrintDepTree (dm, depth + 1);
      last = min;
    }
    
    wcout << wstring (depth * 2, ' ') << L"]";
  }
  wcout << endl;
}


void PrintResults (list<sentence> &ls) {
  word::const_iterator ait;
  sentence::const_iterator w;
  list < sentence >::iterator is;
  int nsentence = 0;

  for (is = ls.begin (); is != ls.end (); is++, ++nsentence) {
      dep_tree & dep = is->get_dep_tree ();
      PrintDepTree (dep.begin (), 0);
      wcout << endl;
  }
}



///---------------------------------------------
/// Sample main program
///
///   The following program reads from stdin a PoS-tagged text, and 
///   calls the chart parser to perform a shallow parsing and then
///   the dependency parser to get a dependency tree.
///
///   The input format should be one word per line, with the right lemma-PoS
///   for each word, and a blank line as sentence separator.  For example:
///
///    The the DT 1
///    cats cat NNS 1
///    eat eat VBP 0.25
///    fish fish NN 0.916667
///    . . Fp 1
///  
///    The the DT 1
///    kids kid NNS 0.982759
///    are be VBP 1
///    playing play VBG 1
///    . . Fp 1
/// 
///---------------------------------------------

int main (int argc, char **argv) {
  dependency_parser *dep = NULL;
  chart_parser *parser = NULL;

  /// set locale to an UTF8 compatible locale
  util::init_locale(L"default");

  if(argc < 3) { 
    wcerr << L"fl-parser" << endl;
    wcerr << L"Usage: fl-parser chunk_grammar dep_grammar" << endl;
    wcerr << endl; 
    return 1;
  }
 
  //DepParser=txala
  //DepTxalaFile=$FREELINGSHARE/es/dep/dependences.dat

  parser = new chart_parser(util::string2wstring(argv[1]));
  dep = new dep_txala (util::string2wstring(argv[2]), parser->get_start_symbol ());

  // read and process input
  wstring text, form, lemma, tag, sn, spr;
  sentence av;
  list < sentence > ls;
  unsigned long totlen = 0;

  while (std::getline (std::wcin, text))  {
    
    if (text != L"")  {	// got a word line
      wistringstream sin;
      sin.str (text);
      sin >> form;       // get word form
      
      // build new word
      word w (form);
      w.set_span (totlen, totlen + form.size ());
      totlen += text.size () + 1;

      // process word line, according to input format.
      // add all analysis in line to the word.
      w.clear ();
      while (sin >> lemma >> tag >> spr) {
        sin >> lemma >> tag;
        analysis an (lemma, tag);
        an.set_prob (1.0);
        w.add_analysis (an);
      }
      
      // append new word to sentence
      av.push_back (w);
    }
    else {// blank line, sentence end.
      totlen += 2;
      
      ls.push_back (av);      
      parser->analyze (ls);
      dep->analyze (ls);
      PrintResults (ls);
      
      av.clear ();		// clear list of words for next use
      ls.clear ();		// clear list of sentences for next use
    }
  }
  
  // process last sentence in buffer (if any)
  ls.push_back (av);		// last sentence (may not have blank line after it)
  parser->analyze (ls);
  dep->analyze (ls);
  PrintResults (ls);

  // clean up. 
  delete parser;
  delete dep;

  return 0;
}
