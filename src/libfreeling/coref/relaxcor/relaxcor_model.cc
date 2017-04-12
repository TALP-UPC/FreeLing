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

#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/relaxcor_model.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAXCOR"
#define MOD_TRACECODE COREF_TRACE

  ///////////////////////////////////////////////////////////////
  /// Destructor is virtual
  ///////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  /// Creates a relaxcor model from a file of feature names.
  ///////////////////////////////////////////////////////////////

  relaxcor_model::relaxcor_model(const wstring &fmodel) {

    enum sections {FEATURES};
    config_file cfg(true,L"#");  
    cfg.add_section(L"Features",FEATURES,true);

    if (not cfg.open(fmodel))
      ERROR_CRASH(L"Error opening file "+fmodel);

    bool has_features=false;

    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;  
      sin.str(line);
      
      switch (cfg.get_section()) { 
        case FEATURES: {
           has_features=true;
           wstring fname;
           int fcode;
           sin >> fname >> fcode;
           _Feature_names[fname] = fcode;
           break;
        }

        default: break;
      }
    }    
    cfg.close();

    if (not has_features) 
      ERROR_CRASH(L"Feature list not found in relaxcor model file "+fmodel);
  }


  //////////////////////////////////////////////////
  /// checks whether the fature exists, and returns the id if so
  //////////////////////////////////////////////////

  bool relaxcor_model::feature_name_defid(const std::wstring &name, unsigned int &id) const {
    TfeaturesNames::const_iterator p= _Feature_names.find(name);
    if (p == _Feature_names.end()) {
      id = -1;
      return false;
    }
    else {
      id = p->second;
      return true;
    }
  }

  //////////////////////////////////////////////////
  /// returns the id of given feature 
  //////////////////////////////////////////////////

  unsigned int relaxcor_model::feature_name_id(const std::wstring &name) const {
    unsigned int x;
    if (not feature_name_defid(name,x)) {
      ERROR_CRASH("Attempt to get id for non-existing feature '"<<name<<L"'");
    }
    return x;
  }

  //////////////////////////////////////////////////
  /// checks existence of feature
  //////////////////////////////////////////////////

  bool relaxcor_model::is_feature_name(const std::wstring &name) const {
    unsigned int x;
    return feature_name_defid(name,x);
  }

  //////////////////////////////////////////////////////////////////
  /// return the start and end iterators of the features' structure
  //////////////////////////////////////////////////////////////////
  relaxcor_model::TfeaturesNames::const_iterator relaxcor_model::begin_features() const {
    return _Feature_names.begin();
  }
  relaxcor_model::TfeaturesNames::const_iterator relaxcor_model::end_features() const {
    return _Feature_names.end();
  }


  wstring relaxcor_model::print(const relaxcor_model::Tfeatures &f, bool activeonly) {
    wstring s=L"";
    for (relaxcor_model::Tfeatures::const_iterator it=f.begin(); it!=f.end(); it++) {
      if (activeonly) {
         // print only active features, for readability
        if (it->second) {
          if (it!=f.begin()) s += L" ";
          s += util::int2wstring(it->first);
        }
      }
      else {
        if (it!=f.begin()) s += L" ";
        if (not it->second) s += L"!";
        s += util::int2wstring(it->first);
      }
    }
    return s;
  }

 
  //////////////////////////////////////////////////
  /// print all the names and values of the model features
  //////////////////////////////////////////////////

  void relaxcor_model::print_feature_names() const {

    for (map<wstring, unsigned int>::const_iterator it=_Feature_names.begin(); it!=_Feature_names.end(); it++) {
      wcerr << (*it).first << " " << (*it).second << endl;
    }
  }

} //namespace
