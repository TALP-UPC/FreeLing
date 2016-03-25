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
#include <sstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/quantities.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"QUANTITIES"
#define MOD_TRACECODE QUANT_TRACE


  ///////////////////////////////////////////////////////////////
  ///  Create the appropriate quantities_module (according to
  /// received options), and create a wrapper to access it.
  //////////////////////////////////////////////////////////////

  quantities::quantities(const std::wstring &Lang, const std::wstring &currFile) {

    // Spanish quantities handler
    if (Lang==L"es") who = new quantities_es(currFile);
    // Catalan quantities handler 
    else if (Lang==L"ca") who = new quantities_ca(currFile);
    // Galician quantities handler 
    else if (Lang==L"gl") who = new quantities_gl(currFile);
    // Portuguese quantities handler 
    else if (Lang==L"pt") who = new quantities_pt(currFile);
    // English quantities handler 
    else if (Lang==L"en") who = new quantities_en(currFile);
    // Russian quantities handler 
    else if (Lang==L"ru") who = new quantities_ru(currFile);

    // Default quantities handler.
    else 
      who = new quantities_default(currFile);  
  }


  ///////////////////////////////////////////////////////////////
  ///  Destructor. Do nothing (the pointer is freed by the factory)
  ///////////////////////////////////////////////////////////////

  quantities::~quantities() {}

} // namespace
