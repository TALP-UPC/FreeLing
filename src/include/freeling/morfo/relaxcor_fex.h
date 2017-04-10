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


#ifndef RELAXCOR_FEX_H
#define RELAXCOR_FEX_H

#include <string>
#include <vector>

#include "freeling/morfo/language.h"
#include "freeling/morfo/relaxcor_model.h"

namespace freeling {

  //////////////////////////////////////////////////////////////////
  /// Class relaxcor_fex is a wrapper providing transparent access
  /// to either relaxcor_fex_dep or relaxcor_fex_constit
  //////////////////////////////////////////////////////////////////

  class relaxcor_fex {
  public:
    typedef std::map<std::wstring, relaxcor_model::Tfeatures > Mfeatures;
    relaxcor_fex(const relaxcor_model &m);
    ~relaxcor_fex();

  protected:
    const relaxcor_model &model;
    unsigned int ID(const std::wstring &) const;

  private: 
    /// extract features for all mention pairs in given vector.
    virtual Mfeatures extract(std::vector<mention>&) const = 0;
    /// dump extracted features for debugging
    virtual void print(Mfeatures&, unsigned int) const;
  };

}

#endif
