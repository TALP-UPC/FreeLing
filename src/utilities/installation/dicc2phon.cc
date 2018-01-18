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
#include <fstream>
#include <string>
#include <cstdlib>

#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/phonetics.h"

#if defined WIN32 
#include "iso646.h"
#endif

using namespace std;
using namespace freeling;

void read_config(const wstring &altsFile, wstring &phdic, wstring &phrules) {
  
  wstring path=altsFile.substr(0,altsFile.find_last_of(L"/\\")+1);
  
  enum sections {GENERAL};
  config_file cfg(true);
  cfg.add_section(L"General",GENERAL);
  if (not cfg.open(altsFile)) {
    wcerr << "Error opening file " << altsFile << endl;
    exit(-1);
  }
    
  // load ortographic alternatives configuration
  wstring line;
  while (cfg.get_content_line(line)) {
    wistringstream sin;
    sin.str(line);
    wstring key,value;
    sin>>key>>value;
    
    switch (cfg.get_section()) {
      // Read <General> section contents
    case GENERAL: {
      if (key==L"PhoneticDictionary") phdic = util::absolute(value,path); 
      else if (key==L"PhoneticRules") phrules = util::absolute(value,path); 
      break;
    }
    default: break;
    }
  }
  cfg.close(); 
}


int main(int argc, char *argv[]){

  util::init_locale(L"default");
		
  if (argc!=3) { 
    wcout << L"Usage: "<< util::string2wstring(argv[0]) << " dicc.src alternatives-phon" << endl; 
    exit(1);
  }

  // get phonetic rules file name and target dictionary name
  wstring dic_file, ph_file;
  read_config(util::string2wstring(argv[2]), dic_file, ph_file);

  // create phonetic encoder
  phonetics ph(ph_file);
  
  // remember which words originated each sound
  map<wstring,wstring> orthogr;
 
  std::wifstream dicc;
  util::open_utf8_file(dicc, util::string2wstring(argv[1]));

  wstring line, word; 
  while (getline(dicc,line)) {

    // read the word form the dictionary, ignoring the rest
    wistringstream sin; sin.str(line); sin>>word;
    // get phonetic encoding
    wstring sound=ph.get_sound(word);

    // store phonetic form, and how it is spelled, accounting for homophones
    map<wstring,wstring>::iterator p=orthogr.find(sound);
    if (p==orthogr.end()) orthogr.insert(make_pair(sound,word));
    else p->second = p->second+L" "+word;
  }
  dicc.close();
  
  // dump built dictionary to output
  std::wofstream res;
  util::open_utf8_file(res, dic_file);
  for (map<wstring,wstring>::const_iterator phon = orthogr.begin(); phon!=orthogr.end(); ++phon) 
    res << phon->first << L" " << phon->second<<endl;
  res.close();
}
