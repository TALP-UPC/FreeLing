//////////////////////////////////////////////////////////////////
//
//    Treeler - Open-source Structured Prediction for NLP
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//                          02111-1307 USA
//
//    contact: Xavier Carreras (carreras@lsi.upc.es)
//             TALP Research Center
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
#include "treeler/class/fgen-class.h"

//#include "Register.h"
//#include "Registry.h"
//REGISTER_FGEN(cfgen, cfgen);

#include "treeler/class/class-basic.h"


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


using namespace std;

namespace treeler {

  void FGenClass::usage(const char* const mesg) {
    cerr << _name << " options:" << endl;
    cerr << " --dim=<int>    : dimensionality of the feature vectors" << endl;
    cerr << " --L=<int>      : number of labels" << endl;
    cerr << endl;
    cerr << mesg << endl;
  }
  
  void FGenClass::process_options(Options& options) {
    _udim = -1;
    if(!options.get("dim", _udim)) {
      usage("please provide the dimensionality of the feature vectors"); exit(1);
    }
    _L = -1;
    if(!options.get("L", _L)) {
      usage("please provide the number of labels"); exit(1);
    }
    _dim = _udim*_L;
    _spaces = _L;
  }


  const FeatureVector<>* FGenClass::phi(const ClassPattern& x, const PartClass& r) {
    FeatureVector<>* F = new FeatureVector<>;
    F->idx = x.idx;
    F->val = x.val;
    F->n = x.n;
    F->offset = r.label();
    F->next = NULL;
    return F;
  }

  void FGenClass::discard(const FeatureVector<>* const F, const ClassPattern& x, const PartClass& r) {
    delete F;
  }


  const FeatureVector<>* FGenClass::phi(const ClassPattern& x) {
    assert(0);
    return NULL;
  }

  void FGenClass::discard(FeatureVector<> const* const F,
  			  ClassPattern const&  x) {
    assert(0);
  }
}
