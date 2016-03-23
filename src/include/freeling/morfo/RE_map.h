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

#ifndef _RE_MAP
#define _RE_MAP

#include <list>

#include "freeling/windll.h"
#include "freeling/regexp.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Auxiliary class to store a single RE_map rule
  ////////////////////////////////////////////////////////////////

  class RE_map_rule {
  public:
    freeling::regexp re;
    std::wstring expression;
    std::wstring data;
    // constructor
  RE_map_rule(const std::wstring &ex, const std::wstring &dt) : re(ex) {
      expression=ex;
      data=dt;
    }
    // copy constructor
  RE_map_rule(const RE_map_rule & s) : re(s.re) {
      expression=s.expression;
      data=s.data;
    }
  };


  ////////////////////////////////////////////////////////////////
  /// Class tag_map implements a mapping from a regexps to an 
  /// associated data string. Regexps are sequentially checked
  /// until a match is found.
  /// This module allows the user application to directly assign
  /// tags to certain words or patterns.
  ////////////////////////////////////////////////////////////////

  class WINDLL RE_map : public processor {
  private:
    /// list of regex with associated information
    std::list<RE_map_rule> regexps;
    
  public:
    /// Constructor
    RE_map(const std::wstring &); 
 
    /// annotate given word
    void annotate_word(word &) const;

    /// analyze given sentence
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif

