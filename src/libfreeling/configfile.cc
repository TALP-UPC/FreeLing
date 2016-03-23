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

#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"CONFIG_FILE"
#define MOD_TRACECODE 0xFFFFFFFF

  //////////////////////////////////////////
  ///
  ///  Class to handle loading a config file for 
  /// freeling module, with XML-like sections
  ///
  ///////////////////////////////////////////


  /// -----------------------------------------
  /// Constructor

  config_file::config_file(bool skip, const wstring &comment) : comment_prefix(comment), skip_unknown_sections(skip) {}


  /// -----------------------------------------
  /// Destructor

  config_file::~config_file() {}


  /// -----------------------------------------
  /// See if a line is a section opening

  bool config_file::isopensection(const wstring &s) const {
    return (s.length()>2 and 
            s[0]==L'<' and s[1]!=L'/' and s[s.length()-1]==L'>');
  }


  /// -----------------------------------------
  /// See if a line is a section closing

  bool config_file::isclosesection(const wstring &s) const {
    return (s.length()>3 and 
            s[0]==L'<' and s[1]==L'/' and s[s.length()-1]==L'>');
  }


  /// -----------------------------------------
  /// See if a line is a comment or an empty/blank line

  bool config_file::iscomment(const wstring &s) const {
    bool blankline=true;
    for (wstring::const_iterator c=s.begin(); c!=s.end() and blankline; ++c) 
      blankline = iswblank(*c);

    return (blankline or s.find(comment_prefix)==0);
  }


  /// -----------------------------------------
  /// add a section with corresponding open/close label.
  /// if required, crash when the file does not contain it

  void config_file::add_section(const wstring &key, int section, bool required) {
    sectionsopen.insert(make_pair(L"<"+key+L">", section));
    sectionsclose.insert(make_pair(L"</"+key+L">", section));
    if (required) 
      requiredsections.insert(L"<"+key+L">");
  }


  /// -----------------------------------------
  /// open config file

  bool config_file::open(const wstring &fname){
    filename=fname;
    section=SECTION_NONE;
    line_num=0;
    section_start=false;
    util::open_utf8_file(filestr, fname);
    return (not filestr.fail());
  }

  /// -----------------------------------------
  /// close stream

  void config_file::close(){
    filestr.close();
  }

  /// -----------------------------------------
  /// get current section

  int config_file::get_section() const {return section;}

  /// ----------------------------------------
  /// get current line number

  int config_file::get_line_num() const {return line_num; }

  /// ----------------------------------------
  /// get whether current line is first in its section

  bool config_file::at_section_start() const {return section_start; }

  /// -----------------------------------------
  /// process next line, handling section opening and closing.
  /// returns only content lines

  bool config_file::get_content_line(wstring &line) {

    // get next line
    line_num++;
    section_start = false;

    // read until EOF, tracking section open/close.
    // when a content line is found, the function returns to the caller.
    while (getline(filestr,line)) {
    
      if (section==SECTION_NONE) {
        // we are out of any section. Only opening and comments are accepted
        if (isopensection(line)) {
          map<wstring,int>::const_iterator s=sectionsopen.find(line);
          if (s==sectionsopen.end()) {
            if (not skip_unknown_sections) {
              ERROR_CRASH(L"Opening of unknown section "+line+L" in file "+filename);
            }
            else {
              section=SECTION_UNKNOWN;
              unk_name=line.substr(1,line.length()-2);
              TRACE(8,L"Entering unknown section "+line+L" in file "+filename);
            }
          }
          else {
            section = s->second;
            requiredsections.erase(line); // the section has started, do not expect it anymore
            section_start = true;
            TRACE(8,L"Entering section "+line+L" in file "+filename);
          }
        }

        else if (isclosesection(line)) { 
          ERROR_CRASH(L"Unexpected closing of section "+line+L" in file "+filename);
        }
        else if (not iscomment(line)) {
          WARNING(L"Ignoring unexpected non-comment line outside sections: '"+line+L"' in file "+filename);
        }
        else {
          TRACE(8,L"Skipping comment: "+line);
        }
      }

      else if (section!=SECTION_NONE) {
        // we are inside a section. Appropriate closing may come. 
        if (isclosesection(line)) { // it is a closing
          map<wstring,int>::const_iterator s=sectionsclose.find(line);
          if (s==sectionsclose.end()) { // it is an undeclared closing 

            if (not skip_unknown_sections) { // undeclared sections are not allowed
              ERROR_CRASH(L"Closing of unknown section "+line+L" in file "+filename);
            }
            else if (section==SECTION_UNKNOWN) { // undeclared sections are allowed, and we are inside one of them

              // the undeclared closing does not match the undeclared open section
              if (unk_name!=line.substr(2,line.length()-3)) {
                ERROR_CRASH(L"Unexpected closing of unknown section "+line+L" in file "+filename);
              }
              else { // the undeclared closing matches undeclared open section
                TRACE(8,L"Exiting unknown section "+line+L" in file "+filename);
                section=SECTION_NONE;
              }
            }

            else  { // we are in a declared section, being closed with undeclared tag
              ERROR_CRASH(L"Unexpected section closing "+line+L" in file "+filename);
            }
          }

          // declared section closing, not matching currently opened section
          else if (s->second!=section) {
            ERROR_CRASH(L"Unexpected closing of section "+line+L" in file "+filename);
          }
          else {// expected closing 
            TRACE(8,L"Exiting section "+line+L" in file "+filename);
            section=SECTION_NONE;
          }
        }

        else if (isopensection(line)) {
          ERROR_CRASH(L"Unexpected nested opening of section "+line+L" in file "+filename);
        }

        // not section closing, regular section line. 
        // If it is not a comment, and we are in a declared section, return current line.
        // Otherwise, skip it.
        else if (section!=SECTION_UNKNOWN and not iscomment(line)) 
          return true;
      }

      line_num++;
    }

    // all required sections should have been seen. If some are missing, crash.
    if (not requiredsections.empty()) {
      ERROR_CRASH(L"Required section "+(*requiredsections.begin())+L" missing in file "+filename);
    }

    return bool(filestr); 
  }

}

