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

#ifndef _NEC
#define _NEC

#include <list>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/fex.h"

#include "freeling/omlet/classifier.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class nec implements a ML-based NE classificator
  ////////////////////////////////////////////////////////////////

  class WINDLL nec : public processor {
  private: 
    /// feature extractor
    const fex* extractor;
    /// adaboost classifier
    const classifier* classif;
    // tag of NPs to classify
    std::wstring NPtag;

  public:
    /// Constructor
    nec(const std::wstring &); 
    /// Destructor
    ~nec();

    /// analyze given sentence
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif

