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

#ifndef _POS_TAGGER
#define _POS_TAGGER

#include <list> 
#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/analyzer_config.h"

#define FORCE_NONE   0  // no selection forced
#define FORCE_TAGGER 1  // force select after tagger
#define FORCE_RETOK  2  // force select after retokenization

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///
  ///  The class POS_tagger is just an abstract class generalizing a PoS tagger.
  ///
  ////////////////////////////////////////////////////////////////

  class WINDLL POS_tagger : public processor {

  private:
    // store configuration options used to create the module
    analyzer_config initial_options;

    // invoke options to be used in subsequent calls (defaults to 
    // initial_options, but can be changed)
    analyzer_invoke_options current_invoke_options;

  protected: 
    // retokenize words that may need it after tagging
    void retokenize(sentence &) const;
    // force the selection of only one Pos tag per word
    void force_select(sentence &) const;

  public:
    POS_tagger(const analyzer_config &opt);
    virtual ~POS_tagger() {};

    /// convenience:  retrieve options used at creation time (e.g. to reset current config)
    const analyzer_config& get_initial_options() const;
    /// set configuration to be used by default
    void set_current_invoke_options(const analyzer_invoke_options &opt);
    /// get configuration being used by default
    const analyzer_invoke_options& get_current_invoke_options() const;

    /// Do actual disambiguation
    virtual void annotate(sentence &, const analyzer_invoke_options &opt) const =0;

    /// analyze given sentence
    void analyze(sentence &) const ;
    /// analyze given sentence with given options
    void analyze(sentence &, const analyzer_invoke_options &opt) const ;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif
