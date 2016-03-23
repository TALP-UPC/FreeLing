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

#include "freeling/morfo/accents_modules.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

#define MOD_TRACENAME L"ACCENTS"
#define MOD_TRACECODE AFF_TRACE

using namespace std;

namespace freeling {

  ///////////////////////////////////////////////////////////////
  ///  Abstract class constructor.
  ///////////////////////////////////////////////////////////////

  accents_module::accents_module() {}


  //---------------------------------------------------------//
  //           default (English) accentuation                //
  //---------------------------------------------------------//

  ///////////////////////////////////////////////////////////////
  ///  Create a default (null) accent handler (eg for English).
  ///////////////////////////////////////////////////////////////

  accents_default::accents_default(): accents_module() {
    TRACE(3,L"created default accent handler ");
  }


  ///////////////////////////////////////////////////////////////
  /// default behaviour: Do nothing.
  ///////////////////////////////////////////////////////////////

  void accents_default::fix_accentuation(std::set<std::wstring> &candidates, const sufrule &suf) const {
    TRACE(3,L"Default accentuation. Candidates: "+util::int2wstring(candidates.size()));
  }

} // namespace
