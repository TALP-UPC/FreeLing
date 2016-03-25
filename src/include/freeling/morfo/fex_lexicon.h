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

#ifndef _FEX_LEXICON
#define _FEX_LEXICON

#include <map>
#include <set>
#include <string>

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Content of one lexicon entry
  ////////////////////////////////////////////////////////////////

  class lex_entry {
  public:
    unsigned int code;
    unsigned int freq;
    lex_entry(unsigned int c, unsigned int f) {code=c; freq=f;};
  };


  ////////////////////////////////////////////////////////////////
  /// Feature lexicon. Stores feature codes and frequencies
  ////////////////////////////////////////////////////////////////

  class fex_lexicon : std::map<std::wstring,lex_entry> {

  private:
    unsigned int freq_sum;
    unsigned int next_code;
    std::set<unsigned int> known_codes;

  public:
    /// constructor: empty lexicon
    fex_lexicon();
    /// constructor: load file
    fex_lexicon(const std::wstring &);
    /// empty lexicon
    void clear_lexicon(); 
    /// add feature occurrence to lexicon
    void add_occurrence(const std::wstring &);
    /// save lexicon to a file, filtering features with low occurrence rate
    void save_lexicon(const std::wstring &, double) const;
    /// consult: get feature code
    unsigned int get_code(const std::wstring &) const;
    /// consult: get feature frequency
    unsigned int get_freq(const std::wstring &) const;
    /// Find out whether the lexicon contains the given feature code
    bool contains_code(unsigned int) const; 
    /// Find out whether the lexicon is loaded and full
    bool is_empty() const; 
  };

} // namespace

#endif
