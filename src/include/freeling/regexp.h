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
//
//   Author: Stanilovsky Evgeny, stanilovsky@gmail.com
//
//
//   This class is just a wrapper to a regular expression engine.
//   All Freeling modules access regexps via this class.
//
//   Currently, the engine is boost::xpressive, but can be changed
//   just writting a new version of this class (with the same API),
//   with no need to alter any other freeling module.
//
///////////////////////////////////////////////

#ifndef _FL_REGEXP_H_
#define _FL_REGEXP_H_

#if defined USE_XPRESSIVE_REGEX
#include <boost/xpressive/xpressive.hpp>
#else
#include <boost/regex/icu.hpp>
#endif

#include <string>
#include <vector>

namespace freeling {

  class regexp {

  private:
#if defined USE_XPRESSIVE_REGEX
    typedef boost::xpressive::wsregex regex_type;
    typedef boost::xpressive::wsmatch match_type;
#else
    typedef boost::u32regex regex_type;
    typedef boost::wsmatch match_type;
#endif

    // internal regular expression
    regex_type re;

    // private function: convert internal match list to vector<string>
    void extract_matches(const match_type &, std::vector<std::wstring> &) const;
    // private function: convert internal match list to vector<string> and positions to vector<int>
    void extract_matches(const match_type &, std::vector<std::wstring> &, std::vector<int> &) const;

  public:
    regexp (const regexp&);
    regexp (const std::wstring &expr, bool icase=false);
    ~regexp (); 
    /// Search for a partial match in a string
    bool search (const std::wstring &in, bool continuous=false) const;
    /// Search for a partial match in a string, return sub matches
    bool search (const std::wstring &in, std::vector<std::wstring> &out, bool continuous=false) const;
    /// Search for a partial match in a string, return sub matches and positions
    bool search (const std::wstring &in, std::vector<std::wstring> &out, 
                 std::vector<int> &pos, bool continuous=false) const;
    /// Search for a partial match in a string, return sub matches 
    bool search (std::wstring::const_iterator i1, std::wstring::const_iterator i2, 
                 std::vector<std::wstring> &out, bool continuous=false) const;
    /// Search for a partial match in a string, return sub matches and positions
    bool search (std::wstring::const_iterator i1, std::wstring::const_iterator i2, 
                 std::vector<std::wstring> &out, std::vector<int> &pos, bool continuous=false) const;
    /// Search for a whole match in a string
    bool match (const std::wstring &in) const;
    /// Search for a whole match in a string, return sub matches
    bool match (const std::wstring &in, std::vector<std::wstring> &out) const;
  };
}

#endif
