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

//////////////////////////////////////////////////////////////////
//
//    Author: Jordi Turmo
//    e-mail: turmo@lsi.upc.edu
//
//    This is an implementation based on the PhD Thesis of Emili Sapena
//
//////////////////////////////////////////////////////////////////

#ifndef RELAXCOR_FEX_DEP_H
#define RELAXCOR_FEX_DEP_H

#include <list>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "freeling/morfo/language.h"
#include "freeling/morfo/semdb.h"
#include "freeling/morfo/tagset.h"
#include "freeling/morfo/relaxcor_fex.h"

namespace freeling {

  // -- Feature group codes    
#define RCF_SET_STRUCTURAL      0x00000001
#define RCF_SET_LEXICAL         0x00000002
#define RCF_SET_MORPHO          0x00000004
#define RCF_SET_SYNTACTIC       0x00000008
#define RCF_SET_SEMANTIC        0x00000010
#define RCF_SET_DISCOURSE       0x00000020
  //                                                                          
#define RCF_SET_ALL             0xFFFFFFFF


  //////////////////////////////////////////////////////////////////
  ///    Class for the feature extractor.
  //////////////////////////////////////////////////////////////////

  class relaxcor_fex_dep : public relaxcor_fex {
  public:

    relaxcor_fex_dep(const std::wstring&, const relaxcor_model &);
    ~relaxcor_fex_dep();

    relaxcor_fex::Mfeatures extract(std::vector<mention>&) const;    
  };

} // namespace

#endif
