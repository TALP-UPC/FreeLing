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

///////////////////////////////////////////////
//
//   Author: Jordi Turmo (turmo@lsi.upc.edu)
//
///////////////////////////////////////////////

#include <string>
#include <fstream>
#include <sstream>

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/relaxcor_modelDT.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAXCOR"
#define MOD_TRACECODE COREF_TRACE

  ///////////////////
  /// Constraint class
  ///////////////////
  
  relaxcor_modelDT::constraint::constraint() {}
  relaxcor_modelDT::constraint::~constraint() {}

  void relaxcor_modelDT::constraint::set_compatibility(double c) { compatibility=c; }
  double relaxcor_modelDT::constraint::get_compatibility() const { return compatibility; }

  void relaxcor_modelDT::constraint::set_condition(unsigned int feature, bool value) {
    if (conditions.find(feature)!=conditions.end() and conditions[feature]!=value)
      ERROR_CRASH(L"Feature used twice in a constraint with opposite values.");
    conditions[feature]=value;
  }

  bool relaxcor_modelDT::constraint::satisfies(const Tfeatures &pairwise_features) const {
    bool sat = true;
    for(Tfeatures::const_iterator it=conditions.begin(); it!=conditions.end() and sat ; it++) {
      sat = it->second == pairwise_features.find(it->first)->second; 
    }
    return sat;
  }

  wstring relaxcor_modelDT::constraint::print() const {
    return util::double2wstring(compatibility) + L" " + relaxcor_model::print(conditions);
  }
  
  ///////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  relaxcor_modelDT::~relaxcor_modelDT() {}

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// Creates a relaxcor model loading a model file, to be used in prediction mode
  //////////////////////////////////////////////////////////////////////////////////////////////

  relaxcor_modelDT::relaxcor_modelDT(const wstring &fmodel) : relaxcor_model(fmodel) {

    // load constraints from config file
    enum sections {CONSTRAINTS};
    config_file cfg(true,L"#");  
    cfg.add_section(L"Constraints",CONSTRAINTS,true);

    if (not cfg.open(fmodel))
      ERROR_CRASH(L"Error opening file "+fmodel);

    wstring line;
    //???
    int i=0;
    while (cfg.get_content_line(line)) {

      wistringstream sin;  
      sin.str(line);
      
      switch (cfg.get_section()) { 
        case CONSTRAINTS: {
          constraint c;

          // read constaint weight
          double weight;  sin >> weight;
          c.set_compatibility(weight);

          // read conditions, taking into account negations
          wstring cond;
	  // ???
	  i++;
          while (sin >> cond) {
            bool positive = true;
            if (cond[0]==L'!') {
              positive = false;
              cond = cond.substr(1);
            }

            if (not is_feature_name(cond))
              ERROR_CRASH(L"Unknown feature '" + cond + L"' used in relaxcor contraint at line "
                          + util::int2wstring(cfg.get_line_num()) + L" in file " + fmodel);
            c.set_condition(_Feature_names[cond], positive);
          }

          // add constraint to the model
          _Constraints.push_back(c);          
          break;
        }

        default: break;
      }
    }

    cfg.close();
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// Creates a relaxcor model from files of feature names, constraints and weights.
  /// the model can be loaded in training mode (no constraints yet) or tuning mode
  /// applying a balance to constraint weights
  /// DEPRECATED
  //////////////////////////////////////////////////////////////////////////////////////////////

  // relaxcor_modelDT::relaxcor_modelDT(const wstring &fmodel, int Balance, bool training) : relaxcor_model(fmodel) {
  //   if (not training)
  //     load_constraints(fmodel,Balance,true);
  // }


  ////////////////////////////////////////////////////////////////////////////////////////////////
  /// Load constraint section from a rule file, applying given balance.
  /// The boolean controls whether any existing constraint is wiped clean before loading
  /// DEPRECATED
  //////////////////////////////////////////////////////////////////////////////////////////////
  
//   void relaxcor_modelDT::load_constraints(const wstring &fmodel, int Balance, bool wipe) {

//     // clean if needed
//     if (wipe) _Constraints.clear();

//     // load constraints from config file
//     enum sections {CONSTRAINTS};
//     config_file cfg(true,L"#");  
//     cfg.add_section(L"Constraints",CONSTRAINTS,true);

//     if (not cfg.open(fmodel))
//       ERROR_CRASH(L"Error opening file "+fmodel);

//     wstring line;
//     while (cfg.get_content_line(line)) {

//       wistringstream sin;  
//       sin.str(line);
      
//       switch (cfg.get_section()) { 
//         case CONSTRAINTS: {
//           constraint c;

//           // read constaint weight
//           double weight;  sin >> weight;
//           c.set_compatibility(weight-Balance);

//           // read conditions, taking into account negations
//           wstring cond;
//           while (sin >> cond) {
//             bool positive = true;
//             if (cond[0]==L'!') {
//               positive = false;
//               cond=cond.substr(1);
//             }

//             if (not is_feature_name(cond))
//               ERROR_CRASH(L"Unknown feature '" + cond + L"' used in relaxcor contraint at line "
//                           + util::int2wstring(cfg.get_line_num()) + L" in file " + fmodel);

//             c.set_condition(_Feature_names[cond], positive);
//           }

//           // add constraint to the model
//           _Constraints.push_back(c);          
//           break;
//         }

//         default: break;
//       }
//     }

//     cfg.close();
// }


////////////////////////////////////////////////////////////////////////////////////////////

  double relaxcor_modelDT::weight(const Tfeatures& F) const {

    double w = 0;    
    for(vector<constraint>::const_iterator it=_Constraints.begin(); it!=_Constraints.end(); it++)
      if (it->satisfies(F)) {
        double c = it->get_compatibility();
	w += c;
        TRACE(7,L"  Applying constraint: "+it->print());
        TRACE(7,L"      Accumulated w="+util::double2wstring(w));
      }
    return w;
  }

/////////////////////////////////////////////////////////////////////////////////////////////

  void relaxcor_modelDT::print() const {
    
    print_feature_names();

    for(vector<constraint>::const_iterator it=_Constraints.begin(); it<_Constraints.end(); it++) {
      wcerr << it->print() << endl;
    }

  }

} //namespace
