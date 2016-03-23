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

#ifndef _LEXER_H
#define _LEXER_H

#include <vector>
#include "freeling/regexp.h"

namespace freeling {

  //////////////////////////////////////////////
  ///  Simple RegEx lexer to parse UTF8  grammar files
  ///  without depending on flex (or the like)
  /////////////////////////////////////////////

  class lexer {
  private:  
    /// to store regexps and associated tokens
    std::vector<std::pair<freeling::regexp,int> > rules;

    /// keep state of the parsing
    std::wstring buffer;
    std::wstring::const_iterator beg, end;
    size_t line;

    /// resulting token
    std::wstring text;
    std::vector<std::wstring> rem;   

  public:
    /// constructor
    lexer(const std::vector<std::pair<freeling::regexp,int> > &);    
    /// get next token from stream (or in the buffer)
    int getToken(std::wistream &);
    /// get text for last token matched
    std::wstring getText();
    /// get last line parsed
    size_t lineno();
  };

} // namespace

#endif
