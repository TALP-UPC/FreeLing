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
/* MCModel : multiclass model

   Author : Xavier Carreras
            carreras@csail.mit.edu */

#ifndef CLASS_MODELMC_H
#define CLASS_MODELMC_H

#include "treeler/base/parameters.h"
#include "treeler/class/class-basic.h"
#include "treeler/class/fgen-class.h"
#include "treeler/util/options.h"

namespace treeler {
  /// A model for multiclass classification
  /// This is a trivial implementation of a structured prediction
  /// model, which is used for standard multiclass classification. In
  /// this case a part directly encodes an output class
  class ModelMC {
  public:
    typedef ClassPattern X;
    typedef PartClass    R;
    typedef Label<R> Y;

    ModelMC() : _L(-1) {}
    ~ModelMC() {}

    void usage(const char* const mesg = "");
    void process_options(Options& options);

    FGenClass& fgen() { return _fgen; }    
    template <typename FIdx>
    double argmax(const X& x, const Parameters<FIdx>& w, Label<R>& y);
    double argmax(const X& x, const double* const S, Label<R>& y);
    double partition(const X& x, const double* const S);
    double marginals(const X& x, const double* const S, double* const M);

  protected:
    int _L;          /* number of classes */
    FGenClass _fgen;

  };


  template <typename FIdx> 
  double ModelMC::argmax(const X& x, const Parameters<FIdx>& w, Label<R>& y) {
    double* S = new double[_L];
    FGenClass fg; 
    for (int l=0; l<_L; ++l) {
      PartClass r(l);
      const FeatureVector<>* f = fg.phi(x,r); 
      S[l] = w.dot(f);      
      fg.discard(f,x,r);
    }
    double s = argmax(x, S, y);
    delete [] S; 
    return s;
  }
  
}

#endif /* CLASS_MODELMC_H */
