//////////////////////////////////////////////////////////////////
//
//    Omlet - Open Machine Learning Enhanced Toolkit
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This file is part of the Omlet library
//
//    The Omlet library is free software; you can redistribute it 
//    and/or modify it under the terms of the GNU Affero General Public
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
//    Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110-1301 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx Omega.S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/omlet/svm.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"SVM"
#define MOD_TRACECODE OMLET_TRACE

  boost::mutex svm::svm_sem;

  //---------- Class svm ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  constructor
  ///////////////////////////////////////////////////////////////

  svm::svm(const wstring &modelFile, const wstring &codes) : classifier(codes) {

    // libsvm function svm_load_model is not thread safe. Mutex it.
    svm_sem.lock(); 
    model = svm_load_model(util::wstring2string(modelFile).c_str());
    svm_sem.unlock();

    int* aux = new int[svm_get_nr_class(model)];    // libsvm code for each class
    class_code = new int[svm_get_nr_class(model)];  // class for each libsvm code

    // obtain libsvm codes for each class
    svm_get_labels(model,aux); 
    // reverse to class for each libsvm code
    for (int i=0; i<svm_get_nr_class(model); i++)
      class_code[aux[i]] = i;
  
    delete [] aux;
  }

  ///////////////////////////////////////////////////////////////
  ///  destructor
  ///////////////////////////////////////////////////////////////

  svm::~svm() {
    delete model;  
    delete [] class_code;
  }

  ///////////////////////////////////////////////////////////////
  ///  get number of classes in the model
  ///////////////////////////////////////////////////////////////

  int svm::get_nlabels() const { 
    return svm_get_nr_class(model);
  }

  ///////////////////////////////////////////////////////////////
  ///  classify given example, computing inner product of the
  ///  example by the hiperplane.
  ///////////////////////////////////////////////////////////////

  void svm::classify(const example &ex, double pred[]) const {

    int n_classes=svm_get_nr_class(model);

    // transform parameters to libsvm format
    struct svm_node *exmp = new svm_node[ex.size()+1];
    int k=0;
    for (example::const_iterator f=ex.begin(); f!=ex.end(); f++) {
      exmp[k].index=f->first;
      exmp[k].value=f->second;
      k++;
    }
    exmp[k].index = -1;

    // call libsvm to do the predictions
    double *pr = new double[n_classes];
    svm_predict_probability(model, exmp, pr);
    
    // transform results to C++, and make sure class numberings are allright
    // (since libsvm numbers them in appearance order).
    // Class codes are assumed to be consecutive and starting at zero.
    for (int c=0; c<n_classes; c++) 
      pred[c] = pr[class_code[c]];

    delete [] exmp;
    delete [] pr;
  }


} // namespace
