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

#include "freeling/morfo/nerc_features.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"FEX_RULE"
#define MOD_TRACECODE FEX_TRACE


  ///////////////////////////////////////////////////////////////
  /// NERC-specific feature functions for FEX
  ///////////////////////////////////////////////////////////////

  class fpattern : public feature_function {
  private:
    wstring patro(const wstring &s) const {
      wstring lcs=util::lowercase(s);
      if (lcs==L"de" or lcs==L"del") return L"d";
      if (lcs==L"el" or lcs==L"la" or lcs==L"las" or lcs==L"lo" or lcs==L"los") return L"e";
      if (util::RE_initial_dot.search(s)) return L"S";
      if (util::RE_all_caps_dot.search(s)) return L"A";
      if (util::RE_capitalized_dot.search(s)) return L"M";
      if (util::RE_has_digits.search(s)) return L"9";
      if (util::RE_lowercase_dot.search(s)) return L"w";
      return s;
    }

  public:
    void extract (const sentence &sent, int i, std::list<std::wstring> &res) const {
      if (i>=0 and i<(int)sent.size()) {
        list<wstring> ls=util::wstring2list(sent[i].get_form(),L"_");
        wstring patr;
        for (list<wstring>::iterator s=ls.begin(); s!=ls.end(); s++) 
          patr = patr+patro(*s);
        res.push_back(patr);
      }
    }
  };

  class fnwords : public feature_function {
  public:
    void extract (const sentence &sent, int i, std::list<std::wstring> &res) const {
      res.push_back(util::int2wstring(util::wstring2list(sent[i].get_form(),L"_").size()));
    }
  };

  class fquoted : public feature_function {
  public:
    void extract (const sentence &sent, int i, std::list<std::wstring> &res) const {
      if ( (i>0 and sent[i-1].get_tag()==L"Fe") and
           (i<(int)sent.size()-1 and sent[i+1].get_tag()==L"Fe") )
        res.push_back(L"in_quotes");
    }
  };

  class fparenthesis : public feature_function {
  public:
    void extract (const sentence &sent, int i, std::list<std::wstring> &res) const {
      if ( (i>0 and sent[i-1].get_tag()==L"Fpa") and
           (i<(int)sent.size()-1 and sent[i+1].get_tag()==L"Fpt") )
        res.push_back(L"in_parenthesis");
    }
  };


  /// Set of available NERC-specific feature functions

  const map<wstring,const feature_function*> 
  nerc_features::functions = {{L"pattern",(feature_function *) new fpattern()},
                              {L"quoted", (feature_function *) new fquoted()},
                              {L"nwords", (feature_function *) new fnwords()},
                              {L"parenthesis", (feature_function *) new fparenthesis()}
                             };

} // namespace
