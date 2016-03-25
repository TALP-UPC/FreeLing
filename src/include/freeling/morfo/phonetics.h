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

#ifndef _PHONETICS
#define _PHONETICS

#include <string>
#include <map>
#include <vector>

#include "freeling/safe_map.h"
#include "freeling/regexp.h"
#include "freeling/morfo/processor.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Auxiliar class to store a phonetic change rule
  ////////////////////////////////////////////////////////////////

  class ph_rule {
  public:
    /// what to search and replace
    std::wstring from;
    /// what is the replacement
    std::wstring to;
    /// in which context
    std::wstring env;
    freeling::regexp re_env;

    ph_rule();
    ~ph_rule();
  };


  class rule_set {
  public:
    /// variables and values extracted from rule file
    std::map<std::wstring,std::wstring> Vars;
    /// sound conversion rules
    std::vector<ph_rule> Rules;
  };


  ////////////////////////////////////////////////////////////////
  /// This class is a will calculate the phonetic translation of a word 
  ////////////////////////////////////////////////////////////////

  class WINDLL phonetics : public processor {

  private:  
    /// rulesets
    std::vector<rule_set> RuleSets;

    /// internal caches with phonetic translation of already 
    /// seen words. One cache for each ruleset
    std::vector<safe_map<std::wstring, std::wstring> *> Cache;

    /// exceptions. Words with direct sound encoding
    std::map<std::wstring, std::wstring> Exceptions;

    /// insert a new rule in rules list
    void add_rule(const std::wstring &, const std::wstring &, const std::wstring &);
    /// Apply given rule to given string, performing required changes
    void apply_rule(const ph_rule&, std::wstring &) const;
  
  public:
    /// Constructor, given config file
    phonetics(const std::wstring&);
    /// Destructor
    ~phonetics();
  
    /// Returns the phonetic sound of the word
    std::wstring get_sound(const std::wstring &) const;

    /// analyze given sentence
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif
 
