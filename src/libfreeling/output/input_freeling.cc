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


//////////////////////////////////////////////////////////
///  Auxiliary functions to print several analysis results
//////////////////////////////////////////////////////////

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/output/input_freeling.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;


//---------------------------------------------
// Constructor
//---------------------------------------------

input_freeling::input_freeling() : input_handler() {}

//---------------------------------------------
// Destructor
//---------------------------------------------

input_freeling::~input_freeling() {}


//---------------------------------------------
// load sentences in Freeling format
//---------------------------------------------

void input_freeling::input_sentences (const wstring &lines, list<sentence> &ls) const {

  wstringstream sin;
  sin.str(lines);
  sentence s;
  wstring line;

  size_t sp=0;
  while (getline(sin,line)) {
    wstringstream lin;
    lin.str(line);

    if (line.empty()) { 
      ls.push_back(s);
      s.clear();
    }

    else {
      wstring form; 
      lin>>form;
      word w(form);
      // simulate span from input 
      size_t sp1 = sp; 
      size_t sp2 = sp + form.length(); 
      sp = sp2 + 1; 
      w.set_span(sp1,sp2);
      
      wstring lemma,tag;
      double prob;
      while (lin>>lemma>>tag>>prob) {
	analysis a(lemma,tag);
	a.set_prob(prob);
	w.add_analysis(a);
      }
      w.select_all_analysis();
      s.push_back(w);
    }
  }
}




