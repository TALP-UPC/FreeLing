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

///////////////////////////////////////////////
// Author: Stanilovsky Evgeny, stanilovsky@gmail.com
///////////////////////////////////////////////

#include "freeling/regexp.h"
#include <locale>
#include <iostream>

using namespace std;

#if defined USE_XPRESSIVE_REGEX
#define CONTINUOUS boost::xpressive::regex_constants::match_continuous
#define SEARCH boost::xpressive::regex_search
#define MATCH boost::xpressive::regex_match
#else
#define CONTINUOUS boost::match_continuous
#define SEARCH boost::u32regex_search
#define MATCH boost::u32regex_match
#endif

namespace freeling {

  ///////////////////////////////////////////////
  /// Constructor
  ///////////////////////////////////////////////

  regexp::regexp (const wstring &expr, bool icase) {
#if defined USE_XPRESSIVE_REGEX
    /// Some people might want to use boost::xpressive instead of boost::regex
    try {
      boost::xpressive::wsregex_compiler re_compiler;
      re_compiler.imbue(locale()); 

      if (icase) re = re_compiler.compile(expr, boost::xpressive::regex_constants::icase | boost::xpressive::regex_constants::optimize);
      else re = re_compiler.compile(expr, boost::xpressive::regex_constants::optimize);
    }
    catch (const boost::xpressive::regex_error &e) {
      wcerr << L"Error compiling regular expression: " << expr << endl;
      throw e;
    }
#else
    /// In Mac OS, we need to use boost::regex (and boost::locale)
    /// because std::locale doesn't support UTF8 locales
    try {
      if (icase) re = boost::make_u32regex(expr, boost::regex::icase | boost::regex::optimize);
      else re = boost::make_u32regex(expr, boost::regex::optimize);
    }
    catch (const boost::regex_error &e) {
      wcerr << L"Error compiling regular expression: " << expr << endl;
      throw e;
    }    
#endif
  }

  ///////////////////////////////////////////////
  /// Copy constructor
  ///////////////////////////////////////////////

  regexp::regexp (const regexp &r) : re(r.re) {}

  ///////////////////////////////////////////////
  /// Destructor 
  ///////////////////////////////////////////////

  regexp::~regexp () {}

  ///////////////////////////////////////////////
  /// Search for a partial match in a string
  ///////////////////////////////////////////////

  bool regexp::search(const wstring &in, bool continuous) const {
    if (continuous) 
      return SEARCH(in.begin(), in.end(), re, CONTINUOUS);
    else 
      return SEARCH(in.begin(), in.end(), re);
  }

  ///////////////////////////////////////////////
  /// Search for a partial match in a string, return sub matches
  ///////////////////////////////////////////////

  bool regexp::search(const wstring &in, vector<wstring> &out, bool continuous) const {
    return this->search(in.begin(), in.end(), out, continuous);
  }

  ///////////////////////////////////////////////
  /// Search for a partial match in a string, return 
  /// sub matches and positions
  ///////////////////////////////////////////////

  bool regexp::search (const wstring &in, vector<wstring> &out, vector<int> &pos, bool continuous) const {
    return this->search(in.begin(), in.end(), out, pos, continuous);
  }


  ///////////////////////////////////////////////
  /// Search for a partial match in a string, return sub matches
  ///////////////////////////////////////////////

  bool regexp::search (wstring::const_iterator i1, wstring::const_iterator i2, 
                       vector<wstring> &out, bool continuous) const {
    match_type what;
    out.clear();

    bool ok =  (continuous ? SEARCH(i1,i2,what,re,CONTINUOUS)
                : SEARCH(i1,i2,what,re));

    if (ok) extract_matches(what,out);
    return ok;
  }

  ///////////////////////////////////////////////
  /// Search for a partial match in a string, return sub matches and positions
  ///////////////////////////////////////////////

  bool regexp::search (wstring::const_iterator i1, wstring::const_iterator i2, 
                       vector<wstring> &out, vector<int> &pos, bool continuous) const {
    match_type what;
    out.clear();
    pos.clear();

    bool ok = (continuous ? SEARCH(i1,i2,what,re,CONTINUOUS) 
               : SEARCH(i1,i2,what,re));

    if (ok) extract_matches(what,out,pos);
    return ok;
  }


  ///////////////////////////////////////////////
  /// Search for a whole match in a string
  ///////////////////////////////////////////////

  bool regexp::match(const wstring &in) const {
    return MATCH(in.begin(),in.end(),re);
  }

  ///////////////////////////////////////////////
  /// Search for a whole match in a string, return sub matches
  ///////////////////////////////////////////////

  bool regexp::match(const wstring &in, vector<wstring> &out) const {

    match_type what;
    out.clear();

    bool ok = MATCH(in.begin(),in.end(),what,re);
    if (ok) extract_matches(what,out);
    return ok;
  }

  ///////////////////////////////////////////////
  /// private function: convert internal match list to vector<string>
  ///////////////////////////////////////////////

  void regexp::extract_matches(const match_type &what, vector<wstring> &out) const {
    for (size_t i=0; i<what.size(); ++i) {
      out.push_back(what.str(i));
    }
  }

  ///////////////////////////////////////////////
  /// private function: convert internal match list to vector<string> and their positions to vector<int>
  ///////////////////////////////////////////////

  void regexp::extract_matches(const match_type &what, vector<wstring> &out, vector<int> &pos) const {
    for (size_t i=0; i<what.size(); ++i) {
      out.push_back(what.str(i));
      pos.push_back(what.position(i));
    }
  }

}
