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

#ifndef _CONFIGFILE
#define _CONFIGFILE

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <set>

#include "freeling/windll.h"

namespace freeling {

  //////////////////////////////////////////
  ///
  ///  Class to handle loading a config file for 
  /// freeling module, with XML-like sections
  ///
  ///////////////////////////////////////////

  class WINDLL config_file {

  private:
    std::map<std::wstring,int> sectionsopen, sectionsclose;
    std::set<std::wstring> requiredsections;

    std::wifstream filestr;
    int section;
    static const int SECTION_NONE = -1;
    static const int SECTION_UNKNOWN = -2;

    std::wstring filename;  // for error messages and traces

    // true just after section begin
    bool section_start;
    // prefix that line must have to be considered a comment
    std::wstring comment_prefix;
    // current line num
    int line_num;

    // set at creation time, tells whether to complain about 
    // unknown sections or just skip them.
    bool skip_unknown_sections;
    std::wstring unk_name;

    bool isopensection(const std::wstring &s) const;
    bool isclosesection(const std::wstring &s) const;
    bool iscomment(const std::wstring &s) const;

  public:
    config_file(bool skip=false, const std::wstring &comment=L"##");
    ~config_file();
    void add_section(const std::wstring &key, int section, bool required=false);
    bool open(const std::wstring &fname);
    void close();
    bool get_content_line(std::wstring &line);
    int get_section() const;
    int get_line_num() const;
    bool at_section_start() const;
  };

}

#endif
