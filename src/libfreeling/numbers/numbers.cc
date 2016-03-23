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
#include "freeling/morfo/numbers.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NUMBERS"
#define MOD_TRACECODE NUMBERS_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create the appropriate numbers_module (according to
  /// received options).
  //////////////////////////////////////////////////////////////

  numbers::numbers(const std::wstring &Lang, const std::wstring &Decimal, const std::wstring &Thousand) {

    // Spanish numbers handler
    if (Lang==L"es")  who = new numbers_es(Decimal, Thousand);
    // Catalan numbers handler 
    else if (Lang==L"ca") who = new numbers_ca(Decimal, Thousand);
    // Czeck numbers handler 
    else if (Lang==L"cs") who = new numbers_cs(Decimal, Thousand);
    // Galician numbers handler 
    else if (Lang==L"gl") who = new numbers_gl(Decimal, Thousand);
    // Portuguese numbers handler 
    else if (Lang==L"pt") who = new numbers_pt(Decimal, Thousand);
    // Italian numbers handler 
    else if (Lang==L"it") who = new numbers_it(Decimal, Thousand);
    // English numbers handler 
    else if (Lang==L"en") who = new numbers_en(Decimal, Thousand);
    // Russian numbers handler 
    else if (Lang==L"ru") who = new numbers_ru(Decimal, Thousand);

    // Default numbers handler.
    else
      who = new numbers_default(Decimal, Thousand);
  
  }

  //////////////////////////////////////////////////////////////
  /// Destructor (do nothing, the pointer is freed by the factory)
  //////////////////////////////////////////////////////////////

  numbers::~numbers() {}


} // namespace
