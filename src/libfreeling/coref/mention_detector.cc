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

///////////////////////////////////////////////
//
//   Author: Lluis Padro
//
///////////////////////////////////////////////

#include <fstream>
#include <sstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/mention_detector.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"MENTION_DETECTOR"
#define MOD_TRACECODE MENTIONS_TRACE


  ///////////////////////////////////////////////////////////////
  /// Create a mention detector module, loading appropriate files.
  ///////////////////////////////////////////////////////////////

  mention_detector::mention_detector(const wstring &filename) {

    // read configuration file and store information
    enum sections {TYPE};
    config_file cfg(true,L"%");

    cfg.add_section(L"Type", TYPE, true);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

    map<unsigned int, bool> exists_section;
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {

      case TYPE: {
	// Read the type
        wstring s;
        sin>>s;
        if (s==L"Dependencies") type = DEP;
        else if (s==L"Constituents") type = CONSTIT;
        else {
          WARNING(L"Invalid mention detection type "<<s<<". Using default (Dependencies)");
          type = DEP;
        }
	break;
      }
      default: break;
      }
    }
    cfg.close();

    mdd = NULL; mdc = NULL;
    if (type==DEP) mdd = new mention_detector_dep(filename);
    else mdc = new mention_detector_constit(filename);

    TRACE(3,L"mention detector succesfully created");
  }

  ///////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  mention_detector::~mention_detector() {
    delete mdd;
    delete mdc;
  }

  /////////////////////////////////////////////////
  /// Use wrapped detector to detect entity mentions in a given document.
  /////////////////////////////////////////////////

  vector<mention> mention_detector::detect(const document &doc) const {
    if (type==DEP) 
      return mdd->detect(doc);
    else 
      return mdc->detect(doc);
  }

} // namespace


