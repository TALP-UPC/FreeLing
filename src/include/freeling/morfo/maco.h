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

#ifndef _MACO
#define _MACO

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/maco_options.h"
#include "freeling/morfo/RE_map.h"
#include "freeling/morfo/locutions.h"
#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/numbers.h"
#include "freeling/morfo/dates.h"
#include "freeling/morfo/quantities.h"
#include "freeling/morfo/punts.h"
#include "freeling/morfo/probabilities.h"
#include "freeling/morfo/ner.h"
#include "freeling/morfo/np.h"
#include "freeling/morfo/bioner.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class maco implements the morphological analyzer, which
  /// uses all the specific analyzers: dates, numbers, dictionary, etc.
  ////////////////////////////////////////////////////////////////

  class WINDLL maco : public processor {
  private:
    /// Morhpological analyzer active modules.
    bool MultiwordsDetection, NumbersDetection, PunctuationDetection, 
      DatesDetection, QuantitiesDetection, DictionarySearch,
      ProbabilityAssignment, UserMap, NERecognition;

    locutions* loc;
    dictionary* dico;
    numbers* numb;
    dates* date;
    quantities* quant;
    punts* punt;
    RE_map *user;
    probabilities* prob;
    ner* npm;
      
  public:
    /// Constructor
    maco(const maco_options &); 
    /// Destructor
    ~maco();

    /// change active options for further analysis
    void set_active_options(bool umap, bool num, bool pun, bool dat,
                            bool dic, bool aff, bool comp, bool rtk,
                            bool mw, bool ner, bool qt, bool prb);

    /// analyze given sentence
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif

