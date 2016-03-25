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

#include "freeling/output/output_train.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

//---------------------------------------------
// Constructor
//---------------------------------------------

output_train::output_train() {}

//---------------------------------------------
// Destructor
//---------------------------------------------

output_train::~output_train() {}


//---------------------------------------------
// print retokenization combinations for a word
//---------------------------------------------

list<analysis> output_train::printRetokenizable(wostream &sout, const list<word> &rtk, list<word>::const_iterator w, const wstring &lem, const wstring &tag) const {
  
  list<analysis> s;
  if (w==rtk.end()) 
    s.push_back(analysis(lem.substr(1),tag.substr(1)));
      
  else {
    list<analysis> s1;
    list<word>::const_iterator w1=w; w1++;
    for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
      s1=printRetokenizable(sout, rtk, w1, lem+L"+"+a->get_lemma(), tag+L"+"+a->get_tag());
      s.splice(s.end(),s1);
    }
  }
  return s;
}  


//---------------------------------------------
// print analysis for a word
//---------------------------------------------

void output_train::PrintWord (wostream &sout, const word &w, bool only_sel, bool probs) const {
  word::const_iterator ait;

  word::const_iterator a_beg,a_end;
  if (only_sel) {
    a_beg = w.selected_begin();
    a_end = w.selected_end();
  }
  else {
    a_beg = w.analysis_begin();
    a_end = w.analysis_end();
  }

  for (ait = a_beg; ait != a_end; ait++) {
    if (ait->is_retokenizable ()) {
      const list <word> & rtk = ait->get_retokenizable();
      list <analysis> la=printRetokenizable(sout, rtk, rtk.begin(), L"", L"");
      for (list<analysis>::iterator x=la.begin(); x!=la.end(); x++) {
        sout << L" " << x->get_lemma() << L" " << x->get_tag();
        if (probs) sout << L" " << ait->get_prob()/la.size();
      }
    }
    else {
      sout << L" " << ait->get_lemma() << L" " << ait->get_tag ();
      if (probs) sout << L" " << ait->get_prob ();
    }

  }
}

//---------------------------------------------
// print obtained analysis in classical FreeLing format
//---------------------------------------------

void output_train::PrintResults (wostream &sout, const list<sentence > &ls) const {

  for (list<sentence>::const_iterator is = ls.begin (); is != ls.end (); is++) {
    for (sentence::const_iterator w = is->begin (); w != is->end (); w++) {
      sout << w->get_form();      
      /// Trainig output: selected analysis (no prob) + all analysis (with probs)
      PrintWord(sout,*w,true,false);
      sout<<L" #";
      PrintWord(sout,*w,false,true);      
      sout << endl;   
    }
  }
  // sentence separator: blank line.
  sout << endl;
}

//----------------------------------------------------------
// print global information of the document 
//----------------------------------------------------------

void output_train::PrintResults(wostream &sout, const document &doc) const {
  for (document::const_iterator p = doc.begin (); p != doc.end (); p++) 
    PrintResults(sout,*p);
}


