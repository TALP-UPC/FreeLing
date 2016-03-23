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

#include "treeler/class/model-mc.h"
#include "treeler/class/fgen-class.h"
#include "treeler/util/options.h"

#include <cmath>
using namespace std;

namespace treeler {

  void ModelMC::usage(const char* const mesg) {
    cerr << "Multiclass model options:" << endl;
    cerr << " --L=<int>   : number of classes" << endl;
    cerr << endl;
    cerr << mesg << endl;
  }

  void ModelMC::process_options(Options& options) {
    _L = -1;
    if(!options.get("L", _L)) {
      usage("please provide the number of classes"); exit(1);
    }
    assert(_L > 0);
    //    if(!options.get("dir", _dir)) {
    //      usage("please provide the model directory"); exit(1);
    //    }
    assert(_L > 0);
    _fgen.process_options(options);
  }

  double ModelMC::argmax(const X& x,
			 const double* const S,
			 Label<R>& y) {
    int maxr = 0;
    double maxs = S[0];
    for (int r=1; r<_L; ++r) {
      if (S[r]>maxs) {
	maxr = r;
	maxs = S[r];
      }
    }
    y.push_back(PartClass(maxr));
    return maxs;
  }

  double ModelMC::partition(const X& x,
			    const double* const S) {
    int R = _L;
    double logZ = S[0];
    for (int r=1; r<R; ++r) {
      if (logZ>S[r]) {
	logZ += log(1+exp(S[r]-logZ));
      }
      else {
	logZ = S[r] + log(1+exp(logZ-S[r]));
      }
    }
    return logZ;
  }

  double ModelMC::marginals(const X& x,
			    const double* const S,
			    double* const out_M) {
    double logZ= partition(x, S);
    int R = _L;
    for (int r=0; r<R; ++r) {
      out_M[r] = exp(S[r]-logZ);
    }
    return logZ;
  }


  
    
}
