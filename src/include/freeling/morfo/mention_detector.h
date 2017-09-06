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


#ifndef MENTION_DETECTOR_H
#define MENTION_DETECTOR_H

#include <map>
#include <set>
#include <algorithm>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/mention_detector_constit.h"
#include "freeling/morfo/mention_detector_dep.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class mention_detector is a wrapper to provide unique
  ///  access to either mention_detector_constit or mention_detector_dep
  ////////////////////////////////////////////////////////////////

  class WINDLL mention_detector {

  private:
    // type of the wrapped mention detectors
    typedef enum {CONSTIT, DEP} mdType;
    mdType type;
    // pointers to wrapped mention detector (only one is used, depending on "type")
    mention_detector_constit * mdc;
    mention_detector_dep * mdd;

  public:
    /// Constructor
    mention_detector(const std::wstring &);
    /// Destructor
    ~mention_detector();
  
    /// Detects entity mentions from parse tree nodes of sentences, and fills given vector
    std::vector<mention> detect(const document &) const;    
  };

} // namespace

#endif
