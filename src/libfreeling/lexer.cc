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

#include "freeling/morfo/lexer.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

  //////////////////////////////////////////
  /// Constructor
  //////////////////////////////////////////

  lexer::lexer(const vector<pair<freeling::regexp,int> > &rs) {  
    rules = rs;
    line = 0;
    // make sure beg==end so we read the first line in getToken
    text = L""; beg = text.begin(); end = beg;
  }


  //////////////////////////////////////////
  /// Get next token from stream (or buffer)
  //////////////////////////////////////////

  int lexer::getToken(wistream &sin) {  

    int token=0;
    while (token==0) { // proceed until a non-ignored token is found

      while (beg==end) {  // if buffer empty, skip empty lines and get a new one
        if (getline(sin,buffer)) { // load next line, if any, into buffer
          line++;
          beg=buffer.begin();  
          end=buffer.end();
        }
        else 
          return(0); // no more lines, return EOF
      }

      bool found=false;
      for (size_t i=0; i<rules.size() and not found; i++) {
        if (rules[i].first.search(beg, end, rem, true)) {
          token = rules[i].second;  // token to return
          text = rem[0];         // matching input string
          beg += rem[0].size();  // move iterator to end of match (consume input);
          found=true;
        }
      } 
      if (not found) {
        token = -1;  // no match found, return error.
        text = wstring(beg,end); 
      }
    }
  
    return token;
  }


  //////////////////////////////////////////
  /// Get text matched by last token found
  //////////////////////////////////////////

  wstring lexer::getText() {
    return text;
  }

  //////////////////////////////////////////
  /// Get number for last line processed 
  //////////////////////////////////////////

  size_t lexer::lineno() {
    return line;
  }
} // namespace
