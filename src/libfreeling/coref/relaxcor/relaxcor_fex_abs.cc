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

///////////////////////////////////////////////
//   Author: Lluis Padro
/////////////////////////////////////////////////

#include <iostream>
#include <string>

#include "freeling/morfo/util.h"
#include "freeling/morfo/relaxcor_fex_abs.h"

using namespace std;

namespace freeling {


  /////////////////////////////////////////////
  /// Auxiliary class to store already computed mention features
  /////////////////////////////////////////////

  feature_cache::feature_cache() {}
  feature_cache::~feature_cache() {}

  void feature_cache::set_feature(int id, mentionFeature f, unsigned int v) {
    i_features[id][f]=v;
  }
  void feature_cache::set_feature(int id, mentionWsFeature f, const vector<wstring>& v) {
    vs_features[id][f]=v;
  }
  void feature_cache::set_feature(const wstring &id, const wstring &v) {
    str_features[id]=v;
  }
  void feature_cache::set_feature(const wstring &id, int v) {
    int_features[id]=v;
  }
  void feature_cache::set_feature(const wstring &id, bool v) {
    bool_features[id]=v;
  }
  unsigned int feature_cache::get_feature(int id, mentionFeature f) const {
    return i_features.find(id)->second.find(f)->second;
  }
  const vector<wstring>& feature_cache::get_feature(int id, mentionWsFeature f) const {
    return vs_features.find(id)->second.find(f)->second;
  }
  
  bool feature_cache::get_str_feature(const wstring &id, wstring &val) const {
    map<wstring,wstring>::const_iterator p = str_features.find(id);
    bool found = (p!=str_features.end());
    if (found) val = p->second;
    return found;
  }
  bool feature_cache::get_int_feature(const wstring &id, int &val) const {
    map<wstring,int>::const_iterator p = int_features.find(id);
    bool found = (p!=int_features.end());
    if (found) val = p->second;
    return found;
  }
  bool feature_cache::get_bool_feature(const wstring &id, bool &val) const {
    map<wstring,bool>::const_iterator p = bool_features.find(id);
    bool found = (p!=bool_features.end());
    if (found) val = p->second;
    return found;
  }
  bool feature_cache::computed_feature(int id, mentionFeature f) const {
    return i_features.find(id)!=i_features.end() and i_features.find(id)->second.find(f)!=i_features.find(id)->second.end();
  }
  bool feature_cache::computed_feature(int id, mentionWsFeature f) const {
    return vs_features.find(id)!=vs_features.end() and vs_features.find(id)->second.find(f)!=vs_features.find(id)->second.end();
  }



  //////////////////////////////////////////////////////////////////
  /// Class relaxcor_fex_abs is an abstract class for a relaxcor
  ///  feature extractor
  //////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////
  /// constructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_abs::relaxcor_fex_abs(const relaxcor_model &m) : model(m) {}
    
  //////////////////////////////////////////////////////////////////
  /// destructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_abs::~relaxcor_fex_abs() {}


  //////////////////////////////////////////////////////////////////
  // get feature existence and id from model
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_abs::defid(const std::wstring &name, unsigned int &id) const { 
    return model.feature_name_defid(name,id);
  }


  //////////////////////////////////////////////////////////////////
  // get feature id from model
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_abs::fid(const std::wstring &name) const { 
    return model.feature_name_id(name);
  }

  //////////////////////////////////////////////////////////////////
  // get feature existence from model
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_abs::def(const std::wstring &name) const { 
    return model.is_feature_name(name);
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Print the detected features
  /////////////////////////////////////////////////////////////////////////////

  void relaxcor_fex_abs::print(relaxcor_fex_abs::Mfeatures &M, unsigned int nment) const {
    for (unsigned int i=1; i<nment; i++) {
      for (unsigned int j=0; j<i; j++) {
	wcerr << i << L":" << j << L" ";
	wstring mp = util::int2wstring(i);
	mp += L":";
	mp += util::int2wstring(j);

        wcerr << model.print(M[mp],true) << L" " << model.print(M[mp],false) << endl;
      }
    }
  }

}
