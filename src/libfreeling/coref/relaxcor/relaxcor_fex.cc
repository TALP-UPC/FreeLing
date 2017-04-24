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
#include "freeling/morfo/relaxcor_fex.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAXCOR_FEX"
#define MOD_TRACECODE COREF_TRACE


  ///////////////////////////////////////////////////////////////
  /// Create a mention detector module, loading appropriate files.
  ///////////////////////////////////////////////////////////////

  relaxcor_fex::relaxcor_fex(const wstring &filename, const relaxcor_model &mod) {

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

    fed = NULL; fec = NULL;
    if (type==DEP) fed = new relaxcor_fex_dep(filename, mod);
    else fec = new relaxcor_fex_constit(filename, mod);
  }

  ///////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  relaxcor_fex::~relaxcor_fex() {
    delete fed;
    delete fec;
  }

  /////////////////////////////////////////////////
  /// Use wrapped detector to detect entity mentions in a given document.
  /////////////////////////////////////////////////

  relaxcor_fex_abs::Mfeatures relaxcor_fex::extract(const std::vector<mention> &ments) const {
    if (type==DEP) 
      return fed->extract(ments);
    else 
      return fec->extract(ments);
  }

} // namespace


