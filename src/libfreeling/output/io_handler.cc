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

#include <iostream> 
#include "freeling/morfo/util.h"
#include "freeling/output/io_handler.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

//---------------------------------------------
// Constructor
//---------------------------------------------

io_handler::io_handler() : Tags(NULL), Lang(L"") {}

//---------------------------------------------
// Destructor
//---------------------------------------------

io_handler::~io_handler() { delete Tags; }

///--------------------------------------------
/// load tagset rules for PoS shortening and MSD descriptions
///--------------------------------------------

void io_handler::load_tagset(const std::wstring &ftag) { 
  if (ftag.empty()) 
    Tags = NULL;
  else {
    delete Tags;
    Tags = new tagset(ftag); 
  }
}

///--------------------------------------------
/// provide access to loaded tagset handler
///--------------------------------------------

const freeling::tagset* io_handler::get_tagset() const {
  return Tags;
}


///--------------------------------------------
/// set language 
///--------------------------------------------

void io_handler::set_language(const std::wstring &lg) { Lang = lg; }

