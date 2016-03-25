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

#ifndef _NP
#define _NP

#include <set>
#include <map>
#include <freeling/regexp.h>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/ner_module.h"

namespace freeling {

  const std::wstring RE_NA=L"^(NC|AQ)";
  const std::wstring RE_DNP=L"^[FWZ]";
  const std::wstring RE_CLO=L"^[DSC]";

  ////////////////////////////////////////////////////////////////
  ///  The class np implements a simple proper noun recognizer.
  ////////////////////////////////////////////////////////////////

  class WINDLL np: public ner_module {
  
  private: 
    /// set of function words
    std::set<std::wstring> func;
    /// set of special punctuation tags
    std::set<std::wstring> punct;
    /// set of words to be considered possible NPs at sentence beggining
    std::set<std::wstring> names;
    /// set of words/tags to be ignored as NE parts, even if they are capitalized
    std::map<std::wstring,int> ignore_tags;
    std::map<std::wstring,int> ignore_words;
    /// sets of NE affixes
    std::set<std::wstring> prefixes;
    std::set<std::wstring> suffixes;

    freeling::regexp RE_NounAdj;
    freeling::regexp RE_Closed;
    freeling::regexp RE_DateNumPunct;

    int ComputeToken(int,sentence::iterator &, sentence &) const;
    void ResetActions(ner_status *) const;
    void StateActions(int, int, int, sentence::const_iterator,ner_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const ner_status *) const;

  public:
    /// Constructor
    np(const std::wstring &); 
  };

} // namespace

#endif
