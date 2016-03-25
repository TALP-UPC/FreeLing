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
#include "freeling/morfo/util.h"
#include "freeling/output/output_handler.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;


//---------------------------------------------
// Constructor
//---------------------------------------------

output_handler::output_handler() : io_handler() {}

//---------------------------------------------
// Destructor
//---------------------------------------------

output_handler::~output_handler() { delete Tags; }


//---------------------------------------------
// auxiliary to compute token id 
//---------------------------------------------

wstring output_handler::get_token_id(const wstring &sid, int w, const wstring &pref) {
  return pref + sid + L"." + util::int2wstring(w);
}


//---------------------------------------------
// compute retokenization combinations for a word
//---------------------------------------------

list<analysis> output_handler::compute_retokenization(const list<word> &rtk, list<word>::const_iterator w, 
                                                      const wstring &lem, const wstring &tag) {
  
  list<analysis> s;
  if (w==rtk.end()) 
    s.push_back(analysis(lem.substr(1),tag.substr(1)));
      
  else {
    list<analysis> s1;
    list<word>::const_iterator w1=w; w1++;
    for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
      s1=compute_retokenization(rtk, w1, lem+L"+"+a->get_lemma(), tag+L"+"+a->get_tag());
      s.splice(s.end(),s1);
    }
  }
  return s;
}  


///--------------------------------------------
/// Sort dependency children by word position 
///--------------------------------------------

bool output_handler::ascending_position(const dep_tree::const_sibling_iterator &n1, 
                                        const dep_tree::const_sibling_iterator &n2) {

  // if n1 is not chunk and n2 is, then n1<n2
  // else if they are both chunks or non-chunks, word position decides.
  return  (not n1->is_chunk() and n2->is_chunk()) or
          (n1->is_chunk() == n2->is_chunk() 
           and n1->get_word().get_position()<n2->get_word().get_position());
}


//----------------------------------------------------------
// Default header is empty
//----------------------------------------------------------

void output_handler::PrintHeader(std::wostream &sout) const {}

//----------------------------------------------------------
// Default footer is empty
//----------------------------------------------------------

void output_handler::PrintFooter(std::wostream &sout) const {} 


//----------------------------------------------------------
// Use wostream printer to print to a string
//----------------------------------------------------------

std::wstring output_handler::PrintResults (const std::list<freeling::sentence> &ls) const {
  wostringstream sss;
  PrintResults(sss,ls);
  return sss.str();
}

//----------------------------------------------------------
// Use wostream printer to print to a string
//----------------------------------------------------------

std::wstring output_handler::PrintResults (const freeling::document &doc) const {
  wostringstream sss;
  PrintResults(sss,doc);
  return sss.str();
}

//----------------------------------------------------------
/// replace a XML reserved token with its right escape char.
//----------------------------------------------------------

wstring output_handler::escapeXML(const wstring &s) {
 wstring r;
 for (wstring::const_iterator c=s.begin(); c!=s.end(); c++) {
   if (*c==L'"') r += L"&quot;";
   else if (*c==L'&') r += L"&amp;";
   else if (*c==L'>') r += L"&gt;";
   else if (*c==L'<') r += L"&lt;";
   else if (*c==L'\'') r += L"&apos;";
   else r += *c;
 }
 return r;
}


//----------------------------------------------------------
/// escape quotes and backslash for JSON
//----------------------------------------------------------

wstring output_handler::escapeJSON(const wstring &s) {
 wstring r;
 for (wstring::const_iterator c=s.begin(); c!=s.end(); c++) {
   if (*c==L'"') r += L"\\\"";
   else if (*c==L'\'') r += L"\\\\";
   else r += *c;
 }
 return r;
}
