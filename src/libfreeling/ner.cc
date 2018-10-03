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

#include <fstream>  

#include "freeling/morfo/ner.h"
#include "freeling/morfo/np.h"
#include "freeling/morfo/bioner.h"
#include "freeling/morfo/crf_nerc.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NP"
#define MOD_TRACECODE NP_TRACE


  ///////////////////////////////////////////////////////////////
  ///  Create the appropriate numbers_module (according to
  /// received options), and create a wrapper to access it.
  //////////////////////////////////////////////////////////////

  ner::ner(const std::wstring &npFile) {

    enum sections {NER_TYPE};
    config_file cfg(true);  
    cfg.add_section(L"Type",NER_TYPE);

    if (not cfg.open(npFile))
      ERROR_CRASH(L"Error opening file "+npFile);

    wstring ner_type= L"";

    // read config file to find out type of NER
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {
      case NER_TYPE: {
        ner_type = util::lowercase(line);
        break;
      }
      default: break;
      }
    }
    cfg.close(); 

    // create apropriate kind of ner module
    if (ner_type==L"basic")  
      who = new np(npFile);
    else if (ner_type==L"bio") 
      who = new bioner(npFile);
    else if (ner_type==L"crf") 
      who = new crf_nerc(npFile);
    else
      ERROR_CRASH (L"Unknown or missing NER type '"+ner_type+L"' in file "+npFile);
  
  }

  ///////////////////////////////////////////////////////////////
  ///  Destructor. Do nothing (the pointer is freed by the factory)
  ///////////////////////////////////////////////////////////////

  ner::~ner() {}

} // namespace
