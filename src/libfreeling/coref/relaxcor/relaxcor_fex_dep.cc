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
//   Author: Lluis Padro
/////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//    Class for the feature extractor.
//////////////////////////////////////////////////////////////////

#include <string>
#include <stdlib.h> // abs
#include <ctime>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/relaxcor_fex_dep.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAXCOR_FEX"
#define MOD_TRACECODE COREF_TRACE


  //////////////////////////////////////////////////////////////////
  /// Constructor. Sets defaults
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::relaxcor_fex_dep(const wstring &filename, const relaxcor_model &m) : relaxcor_fex(m) {
    TRACE(2,L"Module successfully loaded");
  }

  //////////////////////////////////////////////////////////////////
  /// Destructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::~relaxcor_fex_dep() {
  }


  ////////////////////////////////////////////////////////////////// 
  ///    Extract the configured features for all mentions  
  //////////////////////////////////////////////////////////////////

  relaxcor_fex::Mfeatures relaxcor_fex_dep::extract(vector<mention> &mentions) const {
    relaxcor_fex::Mfeatures M;
    return M;
  }

} // namespace
