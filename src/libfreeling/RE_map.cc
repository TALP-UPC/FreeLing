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

#include <sstream>
#include <fstream>

#include "freeling/morfo/RE_map.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RE_MAP"
#define MOD_TRACECODE DATABASE_TRACE


  ////////////////////////////////////////////////////////////////
  /// Create a RegExp mapping module
  ////////////////////////////////////////////////////////////////

  RE_map::RE_map(const std::wstring &puntFile)
  { 
    wstring line;

    // opening regexp file
    wifstream fabr;
    util::open_utf8_file(fabr, puntFile);
    if (fabr.fail()) ERROR_CRASH(L"Error opening file "+puntFile);

    while (getline(fabr,line)) {
      wstring key=line.substr(0,line.find(L" "));
      wstring data=line.substr(line.find(L" ")+1);        
      regexps.push_back(RE_map_rule(key,data));
    }
    fabr.close(); 
  
    TRACE(3,L"RE_map succesfully loaded");
  }


  ////////////////////////////////////////////////////////////////
  /// Check given word for regexp matches
  ////////////////////////////////////////////////////////////////

  void RE_map::annotate_word(word &w) const {

    wstring form=w.get_form();
    TRACE(3,L"checking "+form);

    // check word against each regexp in list
    std::list<RE_map_rule>::const_iterator r;  
    wstring data=L"";
    bool found=false;
    vector<wstring> mtch;
    for (r=regexps.begin(); r!=regexps.end() and not found; r++) {
      TRACE(3,L"Checking expression "+r->expression);
      if (r->re.match(form,mtch)) {
        found=true;
        data=r->data;
        TRACE(3,L" ... Match! returning data= "+data);
      }
    }

    if (found) {
      wistringstream sin;
      sin.str(data);
      // extract lemma+tag pairs from recovered data string
      wstring lemma,tag;
      while (sin>>lemma>>tag) {
        // if the lemma should be the RE match, change it.
        if (lemma==L"$$") lemma = mtch[0];  
        // add analysis to the word
        w.add_analysis(analysis(lemma,tag));
      }

      // record that we analyzed this word.
      w.set_analyzed_by(word::USERMAP);
      // prevent any other module from reinterpreting this.
      w.lock_analysis();
    }
  }

  ////////////////////////////////////////////////////////////////
  /// Check words in given sentence for regexp matches
  ////////////////////////////////////////////////////////////////

  void RE_map::analyze(sentence &se) const {
    for (sentence::iterator i=se.begin(); i!=se.end(); ++i) 
      annotate_word(*i);
  }

} // namespace
