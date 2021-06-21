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
#include "freeling/morfo/analyzer_config.h"
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

    // store configuration options used to create the module
    analyzer_config initial_options;

   // invoke options to be used in subsequent calls (defaults to 
   // initial_options, but can be changed)
   analyzer_invoke_options current_invoke_options;

   // sumbodules to be used for analysis
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
    maco(const analyzer_config &opts); 
    /// Destructor
    ~maco();

    /// convenience:  retrieve options used at creation time (e.g. to reset current config)
    const analyzer_config& get_initial_options() const;
    /// set configuration to be used by default
    void set_current_invoke_options(const analyzer_invoke_options &opt);
    /// get configuration being used by default
    const analyzer_invoke_options& get_current_invoke_options() const;

    /// alternative for set_current_invoke_options
    void set_active_options(bool umap, bool num, bool pun, bool dat,
                            bool dic, bool aff, bool comp, bool rtk,
                            bool mw, bool ner, bool qt, bool prb);

    /// analyze given sentence with given options
    void analyze(sentence &s, const analyzer_invoke_options &opts) const;
    /// analyze given sentence with default options
    void analyze(sentence &s) const;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif

