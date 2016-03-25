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

#ifndef _SUFFIXES
#define _SUFFIXES

#include <string>
#include <set>
#include <map>

#include "freeling/morfo/language.h"
#include "freeling/morfo/accents.h"

#define SUF 0
#define PREF 1

namespace freeling {

  // just predeclaring
  class dictionary;

  ////////////////////////////////////////////////////////////////
  ///  Class suffixes implements suffixation rules and
  ///  dictionary search for suffixed word forms.
  ////////////////////////////////////////////////////////////////

  class affixes {

  private:
    /// Language-specific accent handler
    accents accen;
    const dictionary& dic;

    /// all suffixation/prefixation rules
    std::multimap<std::wstring,sufrule> affix[2];
    /// suffixation/prefixation rules applied unconditionally
    std::multimap<std::wstring,sufrule> affix_always[2];

    /// index of existing suffix/prefixs lengths.
    std::set<unsigned int> ExistingLength[2];
    /// Length of longest suffix/prefix.
    unsigned int Longest[2];

    /// find all applicable affix rules for a word
    void look_for_affixes_in_list (int, const std::multimap<std::wstring,sufrule> &, word &) const;
    /// find all applicable prefix+sufix rules combination for a word
    void look_for_combined_affixes(const std::multimap<std::wstring,sufrule> &, const std::multimap<std::wstring,sufrule> &, word &) const;
    /// generate roots according to rules.
    std::set<std::wstring> GenerateRoots(int, const sufrule &, const std::wstring &) const;
    /// find roots in dictionary and apply matching rules
    void SearchRootsList(std::set<std::wstring> &, const std::wstring &, const sufrule &, word &) const;
    /// actually apply a affix rule
    void ApplyRule(const std::wstring &, const std::list<analysis> &, const std::wstring &, const sufrule &, word &) const;

    /// auxiliary method to deal with retokenization
    void CheckRetokenizable(const sufrule &, const std::wstring &, const std::wstring &, const std::wstring &, std::list<word> &, int) const;

  public:
    /// Constructor
    affixes(const std::wstring &, const std::wstring &, const dictionary &);

    /// look up possible roots of a suffixed/prefixed form
    void look_for_affixes(word &) const;
  };

} // namespace

#endif
