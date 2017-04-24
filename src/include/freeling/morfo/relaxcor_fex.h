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
//   Author: Lluís Padro
//
///////////////////////////////////////////////


#ifndef RELAXCOR_FEX_H
#define RELAXCOR_FEX_H

#include <map>
#include <set>
#include <algorithm>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/relaxcor_fex_constit.h"
#include "freeling/morfo/relaxcor_fex_dep.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class relaxcor_fex is a wrapper to provide unique
  ///  access to either relaxcor_fex_constit or relaxcor_fex_dep
  ////////////////////////////////////////////////////////////////

  class WINDLL relaxcor_fex {

  private:
    // type of the wrapped mention detectors
    typedef enum {CONSTIT, DEP} feType;
    feType type;
    // pointers to wrapped mention detector (only one is used, depending on "type")
    relaxcor_fex_constit * fec;
    relaxcor_fex_dep * fed;

  public:
    /// Constructor
    relaxcor_fex(const std::wstring &, const relaxcor_model &);
    /// Destructor
    ~relaxcor_fex();
  
    relaxcor_fex_abs::Mfeatures extract(const std::vector<mention> &ments) const;
  };

} // namespace

#endif
