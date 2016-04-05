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

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dates.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"DATES"
#define MOD_TRACECODE DATES_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create the appropriate dates_module (according to
  /// received options), and create a wrapper to access it.
  //////////////////////////////////////////////////////////////

  dates::dates(const std::wstring &Lang) {
    // Spanish dates handler
    if (Lang==L"es") who = new dates_es();
    // Catalan dates handler 
    else if (Lang==L"ca") who = new dates_ca();
    // Galician dates handler 
    else if (Lang==L"gl") who = new dates_gl();
    // Portuguese dates handler 
    else if (Lang==L"pt") who = new dates_pt();
    // English dates handler
    else if (Lang==L"en") who = new dates_en();
    // Russian dates handler
    else if (Lang==L"ru") who = new dates_ru();
    // French dates handler
    else if (Lang==L"fr") who = new dates_fr();
    // Welsh dates handler (not implemented yet)
    //else if (Lang==L"cy") who = new dates_cy();
    // German dates handler
    else if (Lang==L"de") who = new dates_de();
    // Default dates handler.
    else       
      who = new dates_default();
  }            
             
             
  ///////////////////////////////////////////////////////////////
  ///  Destructor. Do nothing (the pointer is freed by the factory)
  ///////////////////////////////////////////////////////////////

  dates::~dates() {}


} // namespace
