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

#include "freeling/morfo/accents.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"ACCENTS"
#define MOD_TRACECODE SUFF_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create the appropriate accents_module (according to
  /// received options), and create a wrapper to access it.
  //////////////////////////////////////////////////////////////

  accents::accents(const std::wstring &Lang)
  {
    if (Lang==L"es")
      /// Create Spanish accent handler
      who = new accents_es();
    else if (Lang==L"gl")
      /// Create Galician accent handler
      who = new accents_gl();
    else if (Lang==L"ca")
      /// Create Catalan accent handler !! To be fixed !!
      who = new accents_default();
    else
      /// Create Default (null) accent handler. Ok for english.
      who = new accents_default();
  }


  ///////////////////////////////////////////////////////////////
  ///  Destroy the wrapper and the wrapped accents_module.
  ///////////////////////////////////////////////////////////////

  accents::~accents()
  {
    delete(who);
  }


  ///////////////////////////////////////////////////////////////
  /// wrapper methods: just call the wrapped accents_module.
  ///////////////////////////////////////////////////////////////

  void accents::fix_accentuation(std::set<std::wstring> &candidates, const sufrule &suf) const
  {
    who->fix_accentuation(candidates,suf);
  }

} // namespace
