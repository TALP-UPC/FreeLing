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

#ifndef _NUMBERS
#define _NUMBERS

#include "freeling/windll.h"
#include "freeling/morfo/factory.h"
#include "freeling/morfo/numbers_modules.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///   Class numbers implements a wrapper to transparently 
  ///   create and access a numbers_module number recognizer
  ///   for the appropriate language.
  ////////////////////////////////////////////////////////////////

  class WINDLL numbers : public factory<numbers_module> {

  public:
    /// Constructor
    numbers(const std::wstring &lang, const std::wstring &decimal, const std::wstring &thousand); 
    /// Destructor
    ~numbers();
  };

} // namespace

#endif

