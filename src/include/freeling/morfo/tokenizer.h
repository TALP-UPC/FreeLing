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

#ifndef _TOKENIZER
#define _TOKENIZER

#include <set>
#include <map>
#include <list>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/regexp.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class tokenizer implements a token splitter, which
  ///  converts a string into a sequence of word objects, 
  ///  according to  a set of tokenization rules read from 
  ///  aconfiguration file.
  ////////////////////////////////////////////////////////////////

  class WINDLL tokenizer {
  private:
    /// abreviations set (Dr. Mrs. etc. period is not separated)
    std::set<std::wstring> abrevs;
    /// tokenization rules
    std::list<std::pair<std::wstring, freeling::regexp> > rules;
    /// substrings to convert into tokens in each rule
    std::map<std::wstring,int> matches;

  public:
    /// Constructor
    tokenizer(const std::wstring &cfgfile);

    /// tokenize string, leave result in lw
    void tokenize(const std::wstring &text, std::list<word> &lw) const;
    /// tokenize string, return result as list
    std::list<word> tokenize(const std::wstring &text) const;
    /// tokenize string, updating byte offset. Leave results in lw.
    void tokenize(const std::wstring &text, unsigned long &offset, std::list<word> &lw) const;
    /// tokenize string, updating offset, return result as list
    std::list<word> tokenize(const std::wstring &text, unsigned long &offset) const;
  };

} // namespace

#endif

