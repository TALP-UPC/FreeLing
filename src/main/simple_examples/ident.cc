//////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public License
//    (GNU AGPL) as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License 
//    along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Muntsa Padro (mpadro@lsi.upc.edu)
//             TALP Research Center
//             despatx Omega.S107 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/lang_ident.h"

using namespace std;

int main(int argc, char* argv[]){

  //traces::TraceLevel=4;
  //traces::TraceModule=LANGIDENT_TRACE;

  freeling::util::init_locale(L"default");

  if (argc!=2) {
    wcerr<<L"Usage:  ident config-file  <text" << endl; 
    exit(1);
  }
   
  // creates a language identifier with the given config file
  wstring cfgFile = freeling::util::string2wstring(argv[1]);
  freeling::lang_ident di(cfgFile);
  
  set<wstring> candidates=set<wstring>(); // list of languages to consider. 
                                          // Empty -> all known languages
  vector<pair<double,wstring> > result;
  wstring line;
  while (getline(wcin,line)) {
    // classificate the text and obtain a sorted vector <code_language, probability>
    di.rank_languages(result, line, candidates);
      //output most likely language
    wcout<<L"Most likely: "<<result[0].second<<endl;  
    // output sorted list from more to less probability
    wcout<<L"Decreasing probability list:"<<endl;
    vector<pair<double,wstring> >::iterator i;
    for (i=result.begin(); i!=result.end(); i++)
      wcout<<i->second<<L" "<<i->first<<endl;     

    // alternatively, you can use the funcion identify_language, which
    // will return only the code for the best language, or "none" if 
    // no model yields a large enough probabilitiy
    wstring best_l = di.identify_language (line, candidates);
    wcout << L"Best language "<<best_l<<endl;
  }
}



