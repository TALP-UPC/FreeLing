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

#ifndef _SVM
#define _SVM

#include <string>

#define BOOST_SYSTEM_NO_DEPRECATED
#include <boost/thread/mutex.hpp>

#include "freeling/omlet/example.h"
#include "freeling/omlet/classifier.h"
#include "freeling/omlet/libsvm.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class svm implements a bianry SVM classifier
  ////////////////////////////////////////////////////////////////

  class svm : public classifier {
  private:
    svm_model *model;
    int *class_code;
    static boost::mutex svm_sem;
 
  public:
    // constructor
    svm(const std::wstring &, const std::wstring &);
    // destructor
    ~svm();
   
    ///  classify given example, returning probabilities
    void classify(const example &, double[]) const;
    /// get number of classes on the model
    int get_nlabels() const;
  };

} // namespace

#endif
