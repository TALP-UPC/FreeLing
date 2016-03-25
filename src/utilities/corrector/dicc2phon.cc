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

////////////////////////////////////////////////////////////////
//   
//   From a dictionary of freeling create a text that indexdict can
//
//   use to create a database with 
//  
//
//      key1(consonant fonemas)  data1(words that match) 
//
//	The dictionary is passed as stdin and the text is the stdout
//
//	The paramaters are the 3 necesary files soundChangeRules
//	soundChangeDicFile and sampaFile
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <cstdlib>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/phonetics.h"

using namespace std;
using namespace freeling;

int main(int argc, char *argv[]){

  util::init_locale(L"default");
		
  if (argc<2) { 
    wcout << L"You need to specify a phonetics configuration file." << endl; 
    exit(1);
  }

  // create phonetic encoder
  wstring phFile = util::string2wstring(string(argv[1]));	
  phonetics ph(phFile);
  
  // remember which words originated each sound
  map<wstring,wstring> orthogr;

  wstring line;
  wstring word; 
  while (getline(wcin,line)) {

    // read the word form the dictionary, ignoring the rest
    wistringstream sin; sin.str(line); sin>>word;
    // get phonetic encoding
    wstring sound=ph.get_sound(word);

    // store phonetic form, and how it is spelled, accounting for homophones
    map<wstring,wstring>::iterator p=orthogr.find(sound);
    if (p==orthogr.end()) 
      orthogr.insert(make_pair(sound,word));
    else 
      p->second = p->second+L" "+word;
  }
  
  // dump built dictionary to output
  for (map<wstring,wstring>::const_iterator phon = orthogr.begin(); phon!=orthogr.end(); ++phon) 
    wcout<<phon->first<<L" "<<phon->second<<endl;
  
}
