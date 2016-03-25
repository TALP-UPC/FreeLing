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
 * @file registry.cc
 * @brief Implements the class Registry
 * @author Terry Koo
 */

#include "scripts/registry.h"

#include <iostream>
#include <assert.h>
#include <stdlib.h>

using namespace std;

namespace treeler {

  Registry::reg* Registry::_registry = NULL;

  Registry::Registry(const char* type, const char* name, constructor f) {
    /* allocate the registry mapping here.  note that due to issues
       with the order of initialization/construction of static data
       members, it is not reliable to declare _registry as a normal
       object.  specifically, it is possible to arrive at this code
       without having initialized _registry (due to the fact that
       registry objects are instantiated at global scope; see
       Register.h).  therefore, we use a pointer and allocate it on
       demand. */
    if(_registry == NULL) { _registry = new reg(); }
    Registry::add(type, name, f);
  }
  Registry::~Registry() {}


  void Registry::add(const char* ctype, const char* cname, constructor f) {
    reg& R = *_registry;
    string type(ctype);
    string name(cname);
    const consmap::iterator end = R[type].end();
    consmap::iterator it = R[type].find(name);
    if(it != end) {
      cerr << "registry : detected double-registration of "
	   << type << " \"" << name << "\"" << endl;
      exit(1);
    }
    R[type][name] = f;
  }

  void* Registry::construct(const char* ctype, const char* cname) {
    reg& R = *_registry;
    string type(ctype);
    string name(cname);
    // cerr << "registry : constructing " << type
    // << " \"" << name << "\"" << endl;
    const consmap::iterator end = R[type].end();
    consmap::iterator it = R[type].find(name);
    if(it == end) {
      return NULL;
      cerr << "registry : no constructor for " << type
	   << " \"" << name << "\"" << endl;
      exit(1);
    }
    constructor f = R[type][name];
    return f();
  }

  void Registry::getnames(const char* ctype, vector<string>& names) {
    reg& R = *_registry;
    string type(ctype);
    const reg::iterator end = R.end();
    reg::iterator it = R.find(type);
    if(it == end) {
      cerr << "registry : invalid type \"" << type << "\"" << endl;
      exit(1);
    }
    consmap::iterator cit = R[type].begin();
    const consmap::iterator cend = R[type].end();
    for(; cit != cend; ++cit) {
      names.push_back(cit->first);
    }
  }
}
