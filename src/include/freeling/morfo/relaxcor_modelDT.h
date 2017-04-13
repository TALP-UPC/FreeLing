//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
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
//////////////////////////////////////////////////////////////////

#ifndef RELAXCOR_MODEL_DT_H
#define RELAXCOR_MODEL_DT_H

#include "freeling/windll.h"
#include "freeling/morfo/relaxcor_model.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The basic class relaxcor_modelDT implements the mention-pair model 
  ///  based on constraints derived from DTs
  ////////////////////////////////////////////////////////////////

  class relaxcor_modelDT: virtual public relaxcor_model {
  private:

    class constraint {
    private:  
      Tfeatures conditions;
      double compatibility;

    public:
      constraint();
      ~constraint();
      void set_compatibility(double);
      void set_condition(unsigned int, bool);
      double get_compatibility() const;
      bool satisfies(const Tfeatures &) const;
      const Tfeatures & get_conditions() const;
    };
    
    /// set of constraints from the DT model
    std::vector< relaxcor_modelDT::constraint > _Constraints;

  public:
    relaxcor_modelDT(const std::wstring &fmodel);
    ~relaxcor_modelDT();
    /// DEPRECATED
    /// relaxcor_modelDT(const std::wstring &fmodel, int Balance, bool training=false);
    /// DEPRECATED
    /// void load_constraints(const std::wstring &fmodel, int Balance, bool wipe=true);

    /// returns the weight measuring the coreference degree of a mention-pair
    /// (positive or negative weight) by taking into account the constraints from the DT model
    /// Each mention-pair is given as the set of its features    
    double weight(const Tfeatures&) const;
    void dump() const;

  };

} //namespace

#endif
