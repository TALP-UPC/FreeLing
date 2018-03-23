//////////////////////////////////////////////////////////////////
//
//    Fries - Feature Retriever for Intensional Encoding of Sentences
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This file is part of the Fries library
//
//    The Fries library is free software; you can redistribute it 
//    and/or modify it under the terms of the GNU Affero General Public
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
//    Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110-1301 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx Omega.S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _UTIL
#define _UTIL

#include <cstdio>
#include <list>
#include <string>
#include <vector>
#include <set>

#include <locale>
#include <iostream>
#include <fstream>
#include <sstream>
#include "freeling/utf8/utf8.h"

#include "freeling/regexp.h"
#include "freeling/windll.h"
#include "freeling/morfo/traces.h"

// Capitalization patterns
#define UPPER_NONE 0
#define UPPER_1ST 1
#define UPPER_ALL 2
 
namespace freeling {

#define MOD_TRACENAME L"UTIL"
#define MOD_TRACECODE UTIL_TRACE

  ////////////////////////////////////////////////////////////////
  ///  Class util implements some utilities for NLP analyzers:
  ///  "tolower" for latin alfabets, tags manipulation,
  ///  wstring2number and viceversa conversions, etc.
  /////////////////////////////////////////////////////////////

  class WINDLL util {

  public:
    /// useful regexps
    static regexp RE_has_lowercase;   // wstring contains lowercase chars
    static regexp RE_has_alphanum;    // wstring contains alphanum chars
    static regexp RE_is_capitalized;  // wstring is capitalized
    static regexp RE_all_digits;      // wstring is all digits
    static regexp RE_all_caps;        // wstring is uppercase
    static regexp RE_initial_dot;     // wstring is an initial plus optional dot
    static regexp RE_all_caps_dot;    // wstring is uppercase plus optional dot
    static regexp RE_capitalized_dot; // wstring is capitalized plus optional dot
    static regexp RE_has_digits;      // wstring contains digits
    static regexp RE_lowercase_dot;   // wstring is lowercase plus optional dot

    static regexp RE_win_absolute_path; // to detect absolute paths in windows

    /// Init the locale of the program, to properly handle unicode
    static void init_locale(const std::wstring &s=L"default");
    /// open an UTF8 file for reading
    static void open_utf8_file(std::wifstream &, const std::wstring &);
    /// open an UTF8 file for writting
    static void open_utf8_file(std::wofstream &, const std::wstring &);
    /// Lowercase a wstring, even with latin characters
    static std::wstring lowercase(const std::wstring &);
    /// uppercase a wstring, even with latin characters
    static std::wstring uppercase(const std::wstring &);
    /// check if given path is absolute,
    static bool is_absolute(const std::string &p);
    /// check if given path is absolute,
    static bool is_absolute(const std::wstring &p);
    /// filename management: get current working directory
    static std::string get_current_path(); 
    /// filename management: absolutize a maybe relative path
    static std::string absolute(const std::string &, const std::string &);
    /// filename management: absolutize a maybe relative path
    static std::wstring absolute(const std::wstring &, const std::wstring &);
    /// filename management: expand environment variables in a path
    static std::string expand_filename(const std::string &);
    /// filename management: expand environment variables in a path
    static std::wstring expand_filename(const std::wstring &);
    /// filename management: get unique tempfile name
    //static std::wstring new_tempfile_name();
    /// remove occurrences of given chars
    static std::wstring remove_chars(const std::wstring &, const std::wstring &);
    /// wstring handling
    static void find_and_replace(std::wstring &, const std::wstring &, const std::wstring &);

    /// conversion utilities
    static int wstring2int(const std::wstring &);
    static double wstring2double(const std::wstring &);
    static long double wstring2longdouble(const std::wstring &);

    template<class C> static std::wstring wstring_from(const C&, const std::wstring &);
    template<class C> static std::wstring wstring_from(const C&);
    template<class C> static std::wstring wstring_from(const C*);
    template<class C, class T> static C wstring_to(const std::wstring &, const std::wstring &, bool mcsep=true);
    template<class C> static C wstring_to(const std::wstring &, const std::wstring &, bool mcsep=true);
    template<class C> static C wstring_to(const std::wstring &);

    template<class P1,class P2> static std::wstring pairlist2wstring(const std::list<std::pair<P1,P2> > &, const std::wstring &, const std::wstring &);
    template<class P1,class P2> static std::list<std::pair<P1,P2> > wstring2pairlist(const std::wstring &, const std::wstring &, const std::wstring &);

    template<class K, class V> static void file2map(const std::wstring &, std::map<K,V> &);

    static int capitalization(const std::wstring &);
    static std::wstring capitalize(const std::wstring &, int, bool);

    /// sorting criteria for lists of pairs
    template<class T1,class T2> static bool ascending_first(const std::pair<T1,T2> &, const std::pair<T1,T2> &);
    template<class T1,class T2> static bool ascending_second(const std::pair<T1,T2> &, const std::pair<T1,T2> &);
    template<class T1,class T2> static bool descending_first(const std::pair<T1,T2> &, const std::pair<T1,T2> &);
    template<class T1,class T2> static bool descending_second(const std::pair<T1,T2> &, const std::pair<T1,T2> &);


  private:
    // auxiliary for wstring to list/set/vector<T>
    template<class T> static void extract(const std::wstring &, T&);
  };


  /////////////////////////////////////////////////////////////////////////////
  /// Convert a set/vector/list<T> into a wstring with separators
  /////////////////////////////////////////////////////////////////////////////

  template<class C>
    inline std::wstring util::wstring_from(const C& ls, const std::wstring &sep) {
    // if nothing to convert, we are done
    if (ls.empty()) return L"";  

    std::wostringstream ss;
    // print first element to output
    typename C::const_iterator i=ls.begin();
    ss << (*i);
    // print all remaining elements, adding separators
    while (++i!=ls.end()) ss << sep << (*i);
    // return resulting string
    return ss.str();
  }
 
  /////////////////////////////////////////////////////////////////////////////
  /// Convert a number (int, double) to wstring
  /////////////////////////////////////////////////////////////////////////////

  template<class C>
    inline std::wstring util::wstring_from(const C & x) {
    std::wostringstream ss;
    ss<<std::fixed<<x;
    return ss.str();
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Convert a long double to wstring, removing trailing zeros
  /////////////////////////////////////////////////////////////////////////////

  template<>
    inline std::wstring util::wstring_from(const long double &x) {
    std::wostringstream ss;
    ss<<std::fixed<<x;
    // remove decimal digits if all zeros.
    std::wstring s(ss.str());
    std::wstring::size_type pos = s.find(L'.');
    std::wstring::size_type posLast = s.find_last_not_of(L"0");
    if ((pos != s.npos) && (posLast != s.npos) && (posLast >= pos)) {
      if (posLast == pos) s.erase(pos);
      else s.erase(posLast+1);
    }
    return s;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Convert a string (possibly with utf8 chars) to a wstring 
  /////////////////////////////////////////////////////////////////////////////

  template<>
    inline std::wstring util::wstring_from(const std::string &s) {
    std::wstring ws;
    if (sizeof(std::wstring::value_type)==2) 
      utf8::utf8to16(s.begin(), s.end(), back_inserter(ws));
    else if (sizeof(std::wstring::value_type)==4) 
      utf8::utf8to32(s.begin(), s.end(), back_inserter(ws));
    else 
      WARNING(L"Unexpected wchar size "+wstring_from<int>(sizeof(std::wstring::value_type)));
    return ws; 
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Convert a char* (possibly with utf8 chars) to a wstring 
  /////////////////////////////////////////////////////////////////////////////

  template<>
    inline std::wstring util::wstring_from(const char *cp) {
    return wstring_from<std::string>(std::string(cp));
  }

  /////////////////////////////////////////////////////////////////////////////
  /// auxiliary for wstring to list/set/vector<T>

  template<class T>
    inline void util::extract(const std::wstring &s, T &x) {
    std::wistringstream ss;
    ss.str(s);  
    ss>>x;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// auxiliary for wstring to list/set/vector<wstring>

  template<>
    inline void util::extract(const std::wstring &s, std::wstring &x) {
    std::wistringstream ss;
    ss.str(s);  
    getline(ss,x);
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Convert a wstring with separators into a set/vector/list<T>.
  /// C is the type of container (e.g. list<int>) and T the type of element (e.g. int)
  /// If mcsep=true, "sep" is treated as a single (multichar) separator 
  /// If mcsep=false, "sep" is treated as a set of possible separator chars.
  /////////////////////////////////////////////////////////////////////////////

  template<class C, class T>
    inline C util::wstring_to(const std::wstring &ws, const std::wstring &sep, bool mcsep) {
    C ls;
    if (ws.empty()) return ls;

    // at each occurence of separator "sep" in string "s", cut and insert at the end of the container
    size_t step = (mcsep? sep.size() : 1);
    size_t p=0; 
    while (p != std::wstring::npos) {
      size_t q = (mcsep? ws.find(sep,p) : ws.find_first_of(sep,p));
      T x;
      extract(ws.substr(p,q-p), x);
      ls.insert(ls.end(),x);

      p = (q==std::wstring::npos ? q : q+step);
    }
    return(ls);    
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Convert a wstring to int/double/longdouble
  /////////////////////////////////////////////////////////////////////////////

  template<class C>
    inline C util::wstring_to(const std::wstring &ws) {
    long double x;
    std::wistringstream ss; ss.str(ws); 
    ss>>x;
    // if original wstring hasn't been fully emptied return default value
    std::wstring r;
    if (ss>>r) x= -99999;
    return static_cast<C>(x);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Convert a wstring (likely of 1 character) to wchar_t
  /////////////////////////////////////////////////////////////////////////////

  template<>
    inline wchar_t util::wstring_to(const std::wstring &ws) {
    if (ws.empty()) return 0;
    else return ws[0];
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Convert a wstring to a string (possibly with utf8 chars) 
  /////////////////////////////////////////////////////////////////////////////

  template<>
    inline std::string util::wstring_to(const std::wstring &ws) {
    std::string s;
    if (sizeof(std::wstring::value_type)==2) 
      utf8::utf16to8(ws.begin(), ws.end(), back_inserter(s));
    else if (sizeof(std::wstring::value_type)==4) 
      utf8::utf32to8(ws.begin(), ws.end(), back_inserter(s));
    else 
      WARNING(L"Unexpected wchar size "+wstring_from<int>(sizeof(std::wstring::value_type)));

    return s;
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Create a single wstring concatenatig all elements in given 
  /// list with given separators (one for list elements, one for pair elements)
  /////////////////////////////////////////////////////////////////////////////

  template<class P1,class P2> 
    inline std::wstring util::pairlist2wstring(const std::list<std::pair<P1,P2> > &ls, const std::wstring &sep_pair, const std::wstring &sep_list) {
    // if nothing to convert, we are done
    if (ls.empty()) return L"";  
    // print first element to output
    typename std::list<std::pair<P1,P2> >::const_iterator i=ls.begin();
    std::wstringstream ss;  ss << i->first << sep_pair << i->second;
    // concatenate elements in list<pair>
    while (++i!=ls.end()) ss << sep_list << i->first << sep_pair << i->second;
    // return resulting string
    return(ss.str());
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Built a list of pairs from given string and separators
  /////////////////////////////////////////////////////////////////////////////

  template<class P1,class P2> 
    inline std::list<std::pair<P1,P2> > util::wstring2pairlist(const std::wstring &s, const std::wstring &sep_pair, const std::wstring &sep_list) {
    // split string at sep_list
    std::list<std::wstring> ls = util::wstring_to<std::list<std::wstring>,std::wstring>(s,sep_list);
    // split each pair in ls at sep_pair, and store to lps
    std::list<std::pair<P1,P2> > lps;
    P1 elem1;
    P2 elem2;
    for (std::list<std::wstring>::const_iterator i=ls.begin(); i!=ls.end(); i++) {
      std::wstring::size_type p = i->find(sep_pair);
      std::wstringstream ss1(i->substr(0,p)); ss1 >> elem1;
      std::wstringstream ss2(i->substr(p+1)); ss2 >> elem2;
      lps.push_back(make_pair(elem1,elem2));
    }

    return(lps);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Load a file of lines with pairs key, value into a map
  /////////////////////////////////////////////////////////////////////////////

  template<class K, class V>
    inline void util::file2map(const std::wstring &fname, std::map<K,V> &res) {
    std::wifstream f;
    util::open_utf8_file(f, fname);
    if (f.fail()) ERROR_CRASH(L"Error opening file "+fname);
    std::wstring line;    
    while (getline(f,line)) {
      std::wistringstream sin; sin.str(line);
      K key; V val;
      sin >> key >> val;
      res.insert(make_pair(key,val));
    }
    f.close();
  }

  
  /////////////////////////////////////////////////////////////////////////////
  /// Sort lists of pairs by ascending first component
  /////////////////////////////////////////////////////////////////////////////

  template<class T1,class T2> inline bool util::ascending_first(const std::pair<T1,T2> &p1, const std::pair<T1,T2> &p2) {
    return (p1.first<p2.first or (p1.first==p2.first and p1.second<p2.second));
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Sort lists of pairs by ascending second component
  /////////////////////////////////////////////////////////////////////////////

  template<class T1,class T2> inline bool util::ascending_second(const std::pair<T1,T2> &p1, const std::pair<T1,T2> &p2) {
    return (p1.second<p2.second or (p1.second==p2.second and p1.first<p2.first));
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Sort lists of pairs by descending first component
  /////////////////////////////////////////////////////////////////////////////

  template<class T1,class T2> inline bool util::descending_first(const std::pair<T1,T2> &p1, const std::pair<T1,T2> &p2) {
    return (p1.first>p2.first or (p1.first==p2.first and p1.second>p2.second));
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Sort lists of pairs by descending second component
  /////////////////////////////////////////////////////////////////////////////

  template<class T1,class T2> inline bool util::descending_second(const std::pair<T1,T2> &p1, const std::pair<T1,T2> &p2) {
    return (p1.second>p2.second or (p1.second==p2.second and p1.first>p2.first));
  }
  
  /////////////////////////////////////////////////////////////////////////////
  /// Macros for convenience (and back-compatibility)

#define wstring2vector(x,y) wstring_to<std::vector<std::wstring>,std::wstring>(x,y)
#define wstring2list(x,y) wstring_to<std::list<std::wstring>,std::wstring>(x,y)
#define wstring2set(x,y) wstring_to<std::set<std::wstring>,std::wstring>(x,y)

#define wstring2string(x) wstring_to<std::string>(x)
#define wstring2int(x) wstring_to<int>(x) 
#define wstring2double(x) wstring_to<double>(x) 
#define wstring2longdouble(x) wstring_to<long double>(x) 

#define vector2wstring(x,y) wstring_from(x,y)
#define list2wstring(x,y) wstring_from(x,y)
#define set2wstring(x,y) wstring_from(x,y)
#define string2wstring(x) wstring_from(x)
#define int2wstring(x) wstring_from(x)
#define double2wstring(x) wstring_from(x)
#define longdouble2wstring(x) wstring_from(x)

#define wstring2pairlist(x,y,z) wstring2pairlist<std::wstring,std::wstring>(x,y,z)

#undef MOD_TRACENAME
#undef MOD_TRACECODE

} //namespace

#endif
