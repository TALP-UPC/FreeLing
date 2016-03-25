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
 * \file   io-fvec.h
 * \brief  Declaration of class IOFVec
 * \author Xavier Carreras
 */
#ifndef TREELER_IO_FVEC
#define TREELER_IO_FVEC

#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <algorithm>

#include <iostream>
#include "treeler/base/feature-vector.h"
// #include "egstra/FTVec.h"

using namespace std;

namespace treeler {

  class IOFVec {

  public:
    template<typename FIdx>
    static void write(std::ostream& out, const FeatureVector<FIdx>& f);

    template<typename FIdx>
    static void pretty_print(std::ostream& out, const FeatureVector<FIdx>& f, const string& prefix="");
  };


  namespace iofvec {

    /**
     *  Special traits are necessary for printing FIdx of type
     *  FIdxV0. Because FIdxV0 is a typedef to a long long unsigned
     *  int, and not a class, the operator<< uses the built-in
     *  version, which prints such integers in decimal form. 
     *  
     *  With these traits, FIdxV0 values are converted into strings
     *  representing the FIdxV0 in hexadecimal form, while other types
     *  of FIdx are left unchanged.
     *
     */  
    template <typename FIdx>
      struct Traits {
	inline static const FIdx& value(const FIdx& v) {
	  return v;
	}
      };

    template <>
      struct Traits<FIdxV0> {
	inline static const string value(const FIdxV0& v) {
	  char tmpidx[17];
	  tmpidx[16] = '\0';
	  feature_idx_sprint(tmpidx, v);
	  return string(tmpidx);
	}
      };
  }
  
  template<typename FIdx>
  void IOFVec::write(ostream& out, const FeatureVector<FIdx>& f) {
    vector<string> strfeats;
    const FeatureVector<FIdx>* ff = &f;
    while (ff != NULL) {
      for (int i = 0; i < ff->n; ++i) {
	ostringstream oss;
	if (ff->val == NULL) {
	  oss << ff->offset << ":" << iofvec::Traits<FIdx>::value(ff->idx[i]);
	}
	else {
	  oss << ff->offset << ":" << iofvec::Traits<FIdx>::value(ff->idx[i]) << ":" << ff->val[i];
	}
	strfeats.push_back(oss.str());
      }
      ff = ff->next;
    }
    
    sort(strfeats.begin(), strfeats.end()); 
    
    out << "[ (" << strfeats.size() << ") "; 
    vector<string>::const_iterator i = strfeats.begin(), i_end = strfeats.end();
    while (i!=i_end) {
      out << *i << " "; 
      ++i;
    }
    out << "]";
  }


  template<typename FIdx>
  void IOFVec::pretty_print(ostream& out, const FeatureVector<FIdx>& f, const string& prefix) {
    vector<string> strfeats;
    const FeatureVector<FIdx>* ff = &f;
    while (ff != NULL) {
      for (int i = 0; i < ff->n; ++i) {
	ostringstream oss;
	if (ff->val == NULL) {
	  oss << ff->offset << ":" << iofvec::Traits<FIdx>::value(ff->idx[i]);
	}
	else {
	  oss << ff->offset << ":" << iofvec::Traits<FIdx>::value(ff->idx[i]) << ":" << ff->val[i];
	}
	strfeats.push_back(oss.str());
      }
      ff = ff->next;
    }
    
    sort(strfeats.begin(), strfeats.end()); 
    

    ostringstream oss; 
    oss << "[ (" << strfeats.size() << ") "; 
    string preftmp = prefix + string(oss.str().length(), ' ');
    out << oss.str();
    vector<string>::const_iterator i = strfeats.begin(), i_end = strfeats.end();
    bool first = true;
    while (i!=i_end) {
      if (first) {
	out << *i << " ";
	first = false;
      }
      else {
	out << endl << preftmp << *i << " ";
      }
      ++i;
    }
    out << "]";
  }


}

#endif
