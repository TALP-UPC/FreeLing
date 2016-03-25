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

  protected: 
    // remember whether retokenization is active
    bool retok;
    // remember whether the user asked to force only one tag per word
    unsigned int force;
    // retokenize words that may need it after tagging
    void retokenize(sentence &) const;
    // force the selection of only one Pos tag per word
    void force_select(sentence &) const;

  public:
    POS_tagger(bool,unsigned int);
    virtual ~POS_tagger() {};

    /// Do actual disambiguation
    virtual void annotate(sentence &) const =0;

    /// analyze given sentence
    void analyze(sentence &) const ;

    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif
