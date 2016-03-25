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

#include <fstream>

#include "freeling/morfo/fex_lexicon.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"FEX_LEXICON"
#define MOD_TRACECODE FEX_TRACE


  ////////////////////////////////////////////////////////////////
  /// create empty lexicon
  ////////////////////////////////////////////////////////////////

  fex_lexicon::fex_lexicon() {
    clear_lexicon();
  }

  ////////////////////////////////////////////////////////////////
  /// create lexicon loading from a file.
  /// if lexFile=="", create empty lexicon.
  ////////////////////////////////////////////////////////////////

  fex_lexicon::fex_lexicon(const std::wstring &lexFile) {
    unsigned int num, freq;
    wstring name;

    clear_lexicon();
    if (not lexFile.empty()) {
      wifstream flex;
      util::open_utf8_file(flex, lexFile);
      if (flex.fail()) ERROR_CRASH(L"Error opening file "+lexFile);
    
      freq_sum=0;
      next_code=1;
      while (flex>>num>>name>>freq) {
        this->insert(make_pair(name,lex_entry(num,freq)));      
        known_codes.insert(num);

        if (num+1>next_code) next_code=num+1;
        freq_sum += freq;
      }
      flex.close();
    }
  }


  ////////////////////////////////////////////////////////////////
  /// empty lexicon
  ////////////////////////////////////////////////////////////////

  void fex_lexicon::clear_lexicon() {
    this->clear();
    freq_sum=0;
    next_code=1;
  }

  ////////////////////////////////////////////////////////////////
  /// add feature occurrence to lexicon
  ////////////////////////////////////////////////////////////////

  void fex_lexicon::add_occurrence(const std::wstring &name) {

    map<wstring,lex_entry>::iterator p;
    p = this->find(name);
    if (p != this->end()) {
      // if feature exists increase count
      p->second.freq++;
      TRACE(4,L"Increasing count of feature "+name);
    }
    else {
      // if doesn't exist, add new entry with count to 1.
      this->insert(make_pair(name,lex_entry(next_code,1)));
      known_codes.insert(next_code);
      next_code++;
      TRACE(4,L"Creating new entry for feature "+name);
    }

    // increase total count;
    freq_sum++;
  }

  ////////////////////////////////////////////////////////////////
  /// save lexicon to a file, filtering features with low occurrence rate
  ////////////////////////////////////////////////////////////////

  void fex_lexicon::save_lexicon(const std::wstring &lexFile, double min=0.0) const {

    // if threshold given in relative frequencies, convert to absolute 
    if (min<1) min = min*freq_sum; 
    // new feature codes
    int n=1;

    wofstream lex;
    util::open_utf8_file(lex,lexFile);
    for ( map<wstring,lex_entry>::const_iterator i=this->begin(); i!=this->end(); i++) {
      // save only relevant features, renumbering them.
      if (i->second.freq > min) {
        lex << n++ <<L" "<< i->first <<L" "<< i->second.freq << endl;
        TRACE(4,L"Saved feature "+i->first+L". freq="+util::int2wstring(i->second.freq)+L"  min="+util::double2wstring(min) );
      }
      else 
        TRACE(4,L"NOT saving feature "+i->first+L". freq="+util::int2wstring(i->second.freq)+L"  min="+util::double2wstring(min) );
    }
    lex.close();
  }


  ////////////////////////////////////////////////////////////////
  /// consult: get feature code
  ////////////////////////////////////////////////////////////////

  unsigned int fex_lexicon::get_code(const std::wstring &name) const {
    map<wstring,lex_entry>::const_iterator p;
    p = this->find(name);
    if (p != this->end()) return p->second.code;
    else return 0;  
  }


  ////////////////////////////////////////////////////////////////
  /// consult: get feature frequency
  ////////////////////////////////////////////////////////////////

  unsigned int fex_lexicon::get_freq(const std::wstring &name ) const {
    map<wstring,lex_entry>::const_iterator p;
    p = this->find(name);
    if (p != this->end()) return p->second.freq;
    else return 0;
  }

  ////////////////////////////////////////////////////////////////
  /// Find out whether the lexicon contains the given feature code
  ////////////////////////////////////////////////////////////////

  bool fex_lexicon::contains_code(unsigned int c) const {
    return (known_codes.find(c)!=known_codes.end());
  }


  ////////////////////////////////////////////////////////////////
  /// Find out whether the lexicon is loaded and full
  ////////////////////////////////////////////////////////////////

  bool fex_lexicon::is_empty() const {
    return this->empty();
  }
} // namespace
