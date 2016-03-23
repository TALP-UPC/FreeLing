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
//    version 2.1 of the License, or (at your option) any later version.
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
#include <string>

#include "freeling/regexp.h"
#include "freeling/morfo/tokenizer.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"TOKENIZER"
#define MOD_TRACECODE TOKEN_TRACE

  ///////////////////////////////////////////////////////////////
  /// Create a tokenizer, using the abreviation and patterns 
  /// file indicated in given options.
  ///////////////////////////////////////////////////////////////

  tokenizer::tokenizer(const std::wstring &tokFile) {

    enum sections {MACROS, REGEXPS, ABBREV};  
    config_file cfg;
    cfg.add_section(L"Macros",MACROS);
    cfg.add_section(L"RegExps",REGEXPS);
    cfg.add_section(L"Abbreviations",ABBREV);

    if (not cfg.open(tokFile))
      ERROR_CRASH(L"Error opening file "+tokFile);

    // load and store abreviations file.  
    // At each iteration, a line content is read, and interpreted according to current state.

    list<pair<wstring,wstring> > macros;
    bool rul=false;

    wstring line;
    while (cfg.get_content_line(line)) {
    
      wistringstream sin;
      sin.str(line);

      // proces content line according to current section
      switch (cfg.get_section()) {

        // reading macros.
      case MACROS: {
        if (rul)
          ERROR_CRASH(L"Error reading tokenizer configuration. Macros must be defined before rules.");

        // reading macros
        wstring mname, mvalue;
        sin>>mname>>mvalue;
        macros.push_back(make_pair(mname,mvalue));
        TRACE(3,L"Read macro "+mname+L": "+mvalue);
        break;
      }

        // reading regexps (tokenization rules). Expect section end or a rule
      case REGEXPS: {
        wstring comm, re;
        unsigned int substr;
        // read tokenization rule
        sin>>comm>>substr>>re;
        // at least a rule found
        rul=true;
        // check all macros
        for (list<pair<wstring,wstring> >::iterator i=macros.begin(); i!=macros.end(); i++) {
          wstring mname=L"{"+i->first+L"}"; 
          wstring mvalue=i->second;
          // replace all macro occurrences in the rule with its value
          wstring::size_type p=re.find(mname,0);
          while (p!=wstring::npos) { 
            //re=re.substr(0,p)+mvalue+re.substr(p+mname.length());
            re.replace(p, mname.length(), mvalue);
            p=re.find(mname,p);
          }
        } 
      
        // if there is an extra field "CI", consider regex case insensitive
        wstring ci=L"";
        if (sin>>ci and ci==L"CI") {
          freeling::regexp x(re, true);
          rules.push_back(make_pair(comm,x));
        }
        else {
          freeling::regexp x(re);
          rules.push_back(make_pair(comm,x));
        }
      
        // create and store Regexp rule in rules vector.
        matches.insert(make_pair(comm,substr));
        TRACE(3,L"Stored rule "+comm+L" "+re);
        break;
      }
    
        // reading abbreviations. Expect section end or an abbreviation
      case ABBREV: {
        // store abbreviation
        abrevs.insert(line);
        break;
      }

      default: break;
      }
    }

    cfg.close();

    TRACE(3,L"analyzer succesfully created");
  }

  ///////////////////////////////////////////////////////////////
  /// Split the string into tokens using RegExps from
  /// configuration file, returning a word object list.
  ///////////////////////////////////////////////////////////////

  void tokenizer::tokenize(const std::wstring &p, unsigned long &offset, list<word> &v) const 
  {
    wstring t[10];
    list<pair<wstring, freeling::regexp> >::const_iterator i;
    bool match;
    int j, substr, len=0;
    vector<wstring> results;  // to store match results

    v.clear(); 
    // Loop until line is completely processed. 
    wstring::const_iterator c=p.begin();
    while (c!=p.end()) {
      // find first non-white space and erase leading whitespaces
      while (iswspace(*c)) {
        ++c;
        ++offset;
      }
      
      TRACE(4,L"Tokenizing ["+wstring(c,p.end())+L"]");
      // find first matching rule
      match=false;    
      for (i=rules.begin(); i!=rules.end() && !match; i++) {
        try {
          TRACE(4,L"  Checking rule "+i->first);
          if (i->second.search(c, p.end(), results, true)) {
            // regexp matches, extract substrings
            match=true; len=0;
            substr = matches.find(i->first)->second;
            for (j=(substr==0? 0 : 1); j<=substr && match; j++) {
              // get each requested  substring
              t[j] = results.at(j);
              len += t[j].length();
              TRACE(2,L"Found match "+util::int2wstring(j)+L" ["+t[j]+L"] for rule "+i->first);
              // if special rule, match must be in abbrev file
              if ((i->first)[0]==L'*') {
                wstring lower = util::lowercase(t[j]);
                if (abrevs.find(lower)==abrevs.end()) {
                  match = false;
                  TRACE(2,L"Special rule and found match not in abbrev list. Rule not satisfied");
                }
              }
            }
          }
        }
        catch (...) {
          // boost::regexp rejects to match an expression if the matched string is too long
          WARNING(L"Match too long for boost buffer: Rule "+i->first+L" skipped.");
          WARNING(L"Provided input doesn't look like text.");
        }
      }
    
      if (match) {
        i--;
        // create word for each matched substring and append it to token list
        substr = matches.find(i->first)->second;
        for (j=(substr==0? 0 : 1); j<=substr; j++) {
          if (t[j].length() > 0) {
            TRACE(2,L"Accepting matched substring ["+t[j]+L"]");
            word w(t[j]);
            w.set_span(offset,offset+t[j].length());
            offset += t[j].length();
            v.push_back(w);
          }
          else
            TRACE(2,L"Skipping matched null substring ["+t[j]+L"]");
        } 
        // remaining substring
        c += len;
      }
      else if (c!=p.end()) {
        WARNING(L"No rule matched input substring '"+wstring(c,p.end())+L"'. Character '"+wstring(1,*c)+L"' skipped. Check your tokenization rules.");
        c++;
      }
    }
  
    offset++; // count newline
    TRACE_WORD_LIST(1,v);
  }


  void tokenizer::tokenize(const wstring &p, list<word> &lw) const {
    unsigned long aux=0;
    tokenize(p,aux,lw);
  }


  list<word> tokenizer::tokenize(const wstring &p, unsigned long &offset) const {
    list<word> lw;
    tokenize(p,offset,lw);
    return lw;
  }

  list<word> tokenizer::tokenize(const wstring &p) const {
    unsigned long aux=0;
    list<word> lw;
    tokenize(p,aux,lw);
    return lw;
  }

} // namespace
