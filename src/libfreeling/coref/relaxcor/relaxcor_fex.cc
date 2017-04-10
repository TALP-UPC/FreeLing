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

#include <iostream>
#include <string>

#include "freeling/morfo/util.h"
#include "freeling/morfo/relaxcor_fex.h"

using namespace std;

namespace freeling {

  //////////////////////////////////////////////////////////////////
  /// Class relaxcor_fex is a wrapper providing transparent access
  /// to either relaxcor_fex_dep or relaxcor_fex_constit
  //////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////
  /// constructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex::relaxcor_fex(const relaxcor_model &m) : model(m) {}
    
  //////////////////////////////////////////////////////////////////
  /// destructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex::~relaxcor_fex() {}


  unsigned int relaxcor_fex::ID(const std::wstring &x) const {
    return model.feature_name_id(x);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Print the detected features
  /////////////////////////////////////////////////////////////////////////////

  void relaxcor_fex::print(relaxcor_fex::Mfeatures &M, unsigned int nment) const {
    for (unsigned int i=1; i<nment; i++) {
      for (unsigned int j=0; j<i; j++) {
	wcerr << i << L":" << j << L" ";
	wstring mp = util::int2wstring(i);
	mp += L":";
	mp += util::int2wstring(j);

        wcerr << relaxcor_model::print(M[mp]) << endl;
      }
    }
  }

}
