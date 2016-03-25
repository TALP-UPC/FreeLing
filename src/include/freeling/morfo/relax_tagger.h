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

#ifndef _RELAX_TAGGER
#define _RELAX_TAGGER

#include <list> 
#include <string>

#include "freeling/windll.h"
#include "freeling/regexp.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/tagger.h"
#include "freeling/morfo/relax.h"
#include "freeling/morfo/constraint_grammar.h"

namespace freeling {

  // RegExp to detect user field usage
  const std::wstring USER_RE=L"^u\\.([0-9]+)=(.+)$";

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class relax_tagger implements a PoS tagger based on
  /// relaxation labelling algorithm. It does so using the 
  /// generic solver implemented by class relax.
  ///
  ////////////////////////////////////////////////////////////////

  class WINDLL relax_tagger : public POS_tagger {
  private:
    /// Generic solver instance
    relax solver;
    /// PoS constraints.
    constraint_grammar c_gram;

    // RegExp to detect user field usage. It's stored here precompiled for efficiency.
    freeling::regexp RE_user;

    /// check a condition of a RuleCG.
    /// Add to the given constraint& solver-encoded constraint info for the condition
    bool CheckCondition(const sentence &, sentence::const_iterator, int, 
                        const condition &, std::list<std::list<std::pair<int,int> > > &) const;
    /// check whether a word matches a simple list of terms.
    /// Return (via list<pair<int,int>>&) a solver-encoded term for the condition
    bool CheckWordMatchCondition(const std::list<std::wstring> &, bool, int, sentence::const_iterator,
                                 std::list<std::pair<int,int> > &) const;
    /// check whether a word matches one of all possible condition patterns
    bool check_possible_matching(const std::wstring &, word::const_iterator,
                                 sentence::const_iterator) const; 
    /// check a match of two literals, taking into account lemma, tag, wildcards, etc.
    bool check_match(const std::wstring &, const std::wstring &) const;

  public:
    /// Constructor, given the constraint file and config parameters
    relax_tagger(const std::wstring &, int, double, double, bool, unsigned int);

    /// analyze sentences with default options
    void annotate(sentence &) const;
  };

} // namespace

#endif
