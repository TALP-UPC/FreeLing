/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2014   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   dictionary.cc
 * \brief  Implementation of class Dictionary
 * \author Xavier Carreras
 */

#include "treeler/base/dictionary.h"
#include <fstream>
#include <sstream>
#include <cassert>

using namespace std;

namespace treeler {
  
  void Dictionary::load(const string& file) {
    _str2int.clear();
    _int2str.clear();
    _n=0;
    ifstream f(file.c_str());
    if (!f.good()) {
      TreelerException e("dictionary", "error opening file \"" + file + "\"");
      throw(e); 
    }
    string line;
    while (getline(f,line)) {
      istringstream iss(line);
      int id;
      string str;
      if (iss >> id >> str) {
	if (id!=_n) {
	  ostringstream oss; 
	  oss << "invalid id in file \"" << file << "\", line number " << _n+1; 
	  TreelerException e("dictionary", oss.str());
	  throw(e); 
	}
	_str2int[str] = id;
	_int2str.push_back(str);
	++_n;
      }
      else {
	ostringstream oss; 
	oss << "invalid id in file \"" << file << "\", line number " << _n+1; 
	TreelerException e("dictionary", oss.str());
	throw(e); 
      }
    }
    f.close();
    _id_unknowns = _n;
  }

  void Dictionary::dump(ostream& o) const {
    if (_universal) {
      assert(0); // no support to dump universal dictionaries
    }
    int2str_t::const_iterator it = _int2str.begin(); 
    int2str_t::const_iterator itend = _int2str.end(); 
    for (int id=0; it!=itend; ++it, ++id) {
      o << id << " " << *it << endl;
    }
  }
  
  string Dictionary::map(const int& vint) const {
    if (_universal or vint<0 or vint==_id_unknowns) return _str_unknowns;
    //    if (vint >= _n) {
    //      log << "Dictionary: fatal error, vint=" << vint << " n=" << _n << " id_unknowns=" << _id_unknowns << endl; 
    //    }
    assert(vint<_n);
    return _int2str[vint];
  }

  int Dictionary::map(const string& vstr) const {
    if (_universal) { return _id_unknowns; }
    str2int_t::const_iterator i = _str2int.find(vstr);
    if (i==_str2int.end()) { return _id_unknowns; }
    return i->second;
  }

  int Dictionary::map_or_create(const string& vstr) {
    if (_universal) { return _id_unknowns; }
    str2int_t::const_iterator i = _str2int.find(vstr);
    if (i==_str2int.end()) {
      _str2int[vstr] = _n;
      _int2str.push_back(vstr);
      ++_n;
      _id_unknowns = _n; 
      return _n-1;
    }
    return i->second;
  }

}
