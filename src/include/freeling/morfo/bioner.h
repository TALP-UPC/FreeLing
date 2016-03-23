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

#ifndef _BIONER
#define _BIONER

#include <map>

#include "freeling/windll.h"

#include "freeling/omlet/adaboost.h"
#include "freeling/omlet/svm.h"
#include "freeling/omlet/viterbi.h"

#include "freeling/morfo/language.h"
#include "freeling/morfo/ner_module.h"
#include "freeling/morfo/fex.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class bioner implements an AdaBoost-Based NE recognizer
  ////////////////////////////////////////////////////////////////

  class WINDLL bioner: public ner_module {
  
  private:
    /// feature extractor
    const fex* extractor;
    /// adaboost classifier
    const classifier* classif;
    /// Viterbi solver 
    vis_viterbi vit;

  public:
    /// Constructor
    bioner(const std::wstring &);
    /// Destructor
    ~bioner();

    void SetMultiwordAnalysis(sentence::iterator) const;

    /// Recognize NEs in given sentence
    void analyze ( sentence & ) const;

    /// inherit other methods
    using processor::analyze;
    
  };

} // namespace

#endif

