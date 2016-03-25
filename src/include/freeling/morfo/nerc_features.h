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

#ifndef _NERC_FEATURES
#define _NERC_FEATURES

#include <map>
#include "freeling/regexp.h"
#include "freeling/morfo/fex_rule.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// available NERC-specific feature extraction functions.
  ////////////////////////////////////////////////////////////////

  class nerc_features {  
  public:
    /// available NERC-specific feature extraction functions.
    static const std::map<std::wstring,const feature_function *> functions;
  };

} // namespace

#endif
