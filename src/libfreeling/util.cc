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

#include <sstream>
#include <fstream>
#include <iomanip>  
#include <algorithm>

#include "freeling/morfo/util.h"

#if defined MACOSX
#include <codecvt>
#endif

/// locale-related stuff
#if defined WIN32 || defined WIN64
#include <io.h>     
#include <fcntl.h>  
#include <codecvt>
void setUtf8Mode(FILE* f, char const name[]) {
  int const newMode = _setmode( _fileno(f), _O_U8TEXT );
  if (newMode == -1) throw "Setmode failed.";
}   
#define DEFAULT_LOCALE "eng"
#else
#define DEFAULT_LOCALE "en_US.UTF-8"
#endif

/// get cwd-related stuff
#if defined WIN32 || defined WIN64
#include <direct.h>
#include <stdlib.h>
#define MaxPath _MAX_PATH
#define getCWD _getcwd
#else
#define MaxPath PATH_MAX
#define getCWD getcwd
#endif

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"UTIL"
#define MOD_TRACECODE UTIL_TRACE

  /////////////////////////////////////////////////////////////////////////////
  /// Create useful regexps
  /////////////////////////////////////////////////////////////////////////////

  freeling::regexp util::RE_has_lowercase(L"");
  freeling::regexp util::RE_has_alphanum(L"");
  freeling::regexp util::RE_is_capitalized(L"");
  freeling::regexp util::RE_all_digits(L"");
  freeling::regexp util::RE_all_caps(L"");
  freeling::regexp util::RE_initial_dot(L"");
  freeling::regexp util::RE_all_caps_dot(L"");
  freeling::regexp util::RE_capitalized_dot(L"");
  freeling::regexp util::RE_has_digits(L"");
  freeling::regexp util::RE_lowercase_dot(L"");

  freeling::regexp util::RE_win_absolute_path(L"");

  locale current_locale;
  

  /////////////////////////////////////////////////////////////////////////////
  /// Init the locale of the program. If no parameter given, the default locale
  /// en_US.utf8 is used. If "system" is specified, the system locale is used.
  /// Otherwise, the given locale is used.
  /// In any case the selected locale is used only for alphanumerical functions
  /// (utf8 encoding, tolower, isalpha, etc)
  /// Note that for FreeLing to work with UTF8 texts, the locale must be
  /// set to some UTF-8 locale (e.g "en_US.utf8") installed in the system.
  /////////////////////////////////////////////////////////////////////////////

  void util::init_locale(const wstring &loc) {
 
    string lname;
    if (loc==L"system") lname="";
    else if (loc==L"default") lname=DEFAULT_LOCALE;
    else lname=util::wstring2string(loc);

    try
      {
        // create requested locale using libstdc++.
        // modify a basic "C" locale to use alphabetic functions from chosen locale.
        current_locale = locale(locale::classic(), lname.c_str(), locale::ctype);
        // set locale as global, for all streams.
        locale::global(current_locale);

#if defined MACOSX
        current_locale = locale(current_locale, new std::codecvt_utf8<wchar_t>);

        wcin.imbue(current_locale);
        wcerr.imbue(current_locale);
        wcout.imbue(current_locale);
#endif        

      }
    catch (const std::runtime_error &e)
      {
        ERROR_CRASH(L"Error initializing locale. Locale name '"
                    + util::string2wstring(lname) + L"' is unknown or not installed.\n"
                    + util::string2wstring(e.what()));
      }

    std::ios_base::sync_with_stdio(false);

#if defined WIN32 || defined WIN64
    setUtf8Mode(stdin, "stdin");
    setUtf8Mode(stdout, "stdout");
#endif 

    /// Init useful regexps
    util::RE_has_lowercase = freeling::regexp(L"[[:lower:]]");
    util::RE_has_alphanum = freeling::regexp(L"[[:alnum:]]");
    util::RE_is_capitalized = freeling::regexp(L"^[[:upper:]]");
    util::RE_all_digits = freeling::regexp(L"^[[:digit:]]+$");
    util::RE_all_caps = freeling::regexp(L"^[[:upper:]]+$");
    util::RE_initial_dot = freeling::regexp(L"^[[:upper:]]\\.?$");
    util::RE_all_caps_dot = freeling::regexp(L"^[[:upper:]]+\\.?$");
    util::RE_capitalized_dot = freeling::regexp(L"^([[:upper:]][[:lower:]]+\\.?)+$");
    util::RE_has_digits = freeling::regexp(L"[[:digit:]]+");
    util::RE_lowercase_dot = freeling::regexp(L"^[[:lower:]]+\\.?$");

    util::RE_win_absolute_path = freeling::regexp(L"^([A-Za-z]:)?\\\\");
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Open an utf-8 file for reading
  /////////////////////////////////////////////////////////////////////////////

  void util::open_utf8_file(std::wifstream &fabr, const wstring &file) 
  {
    string fname=util::wstring2string(file);
    fabr.open(fname.c_str());
#if defined WIN32 || defined WIN64
    const std::locale utf8_locale
      = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
    fabr.imbue(utf8_locale);
#endif
#if defined MACOSX
    fabr.imbue(current_locale);
#endif        
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Open an utf-8 file for writting
  /////////////////////////////////////////////////////////////////////////////

  void util::open_utf8_file(std::wofstream &fabr, const wstring &file) {
    string fname=util::wstring2string(file);
    fabr.open(fname.c_str());
#if defined WIN32 || defined WIN64
    const std::locale utf8_locale
      = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
    fabr.imbue(utf8_locale);
#endif
#if defined MACOSX
    fabr.imbue(current_locale);
#endif    
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Lowercase an string, possibly with accents.
  /////////////////////////////////////////////////////////////////////////////

  wstring util::lowercase(const wstring &s) {
    wstring ws=s;
#if defined MACOSX
    const std::ctype<wchar_t>& wchar_facet =std::use_facet<std::ctype<wchar_t> >( current_locale );
    for (int i = 0; i < ws.length(); i++) {
      ws.at(i) = wchar_facet.tolower(ws.at(i));
    }
#else 
    transform( ws.begin(), ws.end(), ws.begin(), towlower); 
#endif
    return ws;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Uppercase an string, possibly with accents.
  /////////////////////////////////////////////////////////////////////////////

  wstring util::uppercase(const wstring &s) {
    wstring ws=s;
#if defined MACOSX
    const std::ctype<wchar_t>& wchar_facet =std::use_facet<std::ctype<wchar_t> >( current_locale );
    for (int i = 0; i < ws.length(); i++) {
      ws.at(i) = wchar_facet.toupper(ws.at(i));
    }
#else 
    transform( ws.begin(), ws.end(), ws.begin(), towupper); 
#endif
    return ws;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// get current working directory, string version
  /////////////////////////////////////////////////////////////////////////////

  string util::get_current_path()  {
    char buff[MaxPath];
    char *pb = getCWD(buff, MaxPath);
    return string(pb);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// check if given path is absolute, wstring version
  /////////////////////////////////////////////////////////////////////////////

  bool util::is_absolute(const wstring &p)  {
    // linux/Mac absolute paths start with "/"
    // Windows absolute paths match regexp "([A-Za-z]:)?\\"
    return p.size()>0 and (p[0]==L'/' or util::RE_win_absolute_path.search(p));
  }

  /////////////////////////////////////////////////////////////////////////////
  /// check if given path is absolute, wstring version
  /////////////////////////////////////////////////////////////////////////////

  bool util::is_absolute(const string &p)  {
    return util::is_absolute(util::string2wstring(p));
  }


  /////////////////////////////////////////////////////////////////////////////
  /// convert a relative path to absolute, string version
  /////////////////////////////////////////////////////////////////////////////

  string util::absolute(const string &fname, const string &path)  {
    string fn=fname;
    // remove " quotes around filename, if any
    if (fn[0]=='"' && fn[fn.size()-1]=='"') fn=fn.substr(1,fn.size()-2);   
    // prepend given path if filename is relative
    if (not util::is_absolute(fn)) fn=path+fn;
    return fn;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// convert a relative path to absolute, wstring version
  /////////////////////////////////////////////////////////////////////////////

  wstring util::absolute(const wstring &fname, const wstring &path)  {
    return util::string2wstring(util::absolute(util::wstring2string(fname),util::wstring2string(path)));
  }

  /////////////////////////////////////////////////////////////////////////////
  /// expand environment variables in a filename or path. string version
  /////////////////////////////////////////////////////////////////////////////

  string util::expand_filename(const string &s) {
    string name = s;
    size_t n=name.find_first_of("$"); 
    if (n!=std::string::npos) {
      size_t i=name.find_first_of("/\\",n+1);
      if (i==std::string::npos) i=name.size();
      char* exp=getenv(name.substr(n+1,i-n-1).c_str());
      if (exp==NULL)
        name = name.substr(0,n) + name.substr(i);
      else
        name = name.substr(0,n) + std::string(exp) + name.substr(i);
    }  

    return name;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// expand environment variables in a filename or path. wstring version.
  /////////////////////////////////////////////////////////////////////////////

  wstring util::expand_filename(const wstring &s) {
    return util::string2wstring(util::expand_filename(util::wstring2string(s)));
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Auxiliar function: delete from text any char present in clist.
  /////////////////////////////////////////////////////////////////////////////

  wstring util::remove_chars(const wstring &text, const wstring &clist) { 
    wstring s=text;
    size_t p=s.find_first_of(clist);
    while (p!=wstring::npos) {
      s.erase(p,1);
      p=s.find_first_of(clist,p);
    }
    return (s);
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Replace all occurrences of s in t by r
  /////////////////////////////////////////////////////////////////////////////

  void util::find_and_replace(wstring &t, const wstring &s, const wstring &r) {
    size_t p=t.find(s); 
    while (p!=wstring::npos) {
      t.replace(p,s.size(),r);
      p=t.find(s,p+r.size());
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Find out capitalizatin pattern:  AAAA vs Aaaaa
  /////////////////////////////////////////////////////////////////////////////

  int util::capitalization(const wstring &s) { 
    int caps=UPPER_NONE; // no uppercase
    if (util::RE_all_caps.search(s))  // all uppercase
      caps=UPPER_ALL;
    else if (util::RE_is_capitalized.search(s))  // first uppercase (and not all uppercase)
      caps=UPPER_1ST; 
    return caps;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Format a string to the specified capitalization pattern
  /////////////////////////////////////////////////////////////////////////////

  wstring util::capitalize(const wstring &form, int caps, bool init) {
    wstring cl=form;
    if (caps==UPPER_ALL) // uppercase all
      cl=util::uppercase(cl);
    else if (caps==UPPER_1ST and init)   // capitalize first letter (if init==true)
      cl[0] = towupper(cl[0]); 
    return cl;
  }
} // namespace
