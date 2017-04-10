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

#ifndef RELAXCOR_H
#define RELAXCOR_H

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/mention_detector.h"
#include "freeling/morfo/relaxcor_fex_constit.h"
#include "freeling/morfo/relaxcor_modelDT.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class mention_detector implements a rule-based entity
  ///  mention detector
  ////////////////////////////////////////////////////////////////


  class WINDLL relaxcor : public processor {
  public:

    typedef relaxcor_modelDT coref_model;
    
    /// Constructor
    relaxcor(const std::wstring &fname);

    /// Constructor for tuning stage. 
    /// DEPRECATED
    /// relaxcor(const std::wstring &fname, int Nprune, int Balance, const std::wstring &fconstr);

    /// Destructor
    ~relaxcor();

    // set provide_singletons
    void set_provide_singletons(bool);
    // get provide_singletons
    bool get_provide_singletons() const;

   /// Finds the coreferent mentions in a document
    void analyze(document&) const;

    /// defined only because it is virtual in abstract class
    void analyze(sentence&) const;

  private:
    /// relax parameters
    int _Max_iter;
    double _Scale_factor;
    double _Epsilon;
    /// factor for singleton tendency
    double _Single_factor;
    /// maximum number of edges per vertex
    unsigned int _Nprune;
    
    /// singletons
    bool provide_singletons;

    /// model
    coref_model *model;
    /// mention detector
    mention_detector *detector;
    /// mention-pair features extractor
    relaxcor_fex_constit *extractor;

    
    /// auxiliary for constructors
    /// DEPRECATED
    /// void load_config_file(const std::wstring &filename, bool tuning=false);

    static int offset(const mention&, unsigned int);

  };

} // namespace

#endif
