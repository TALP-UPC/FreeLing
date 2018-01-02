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
 *  along with Treeler.  If not, see <http:://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   fidx-path.h
 * \brief  Conversions from paths to fidx
 * \author 
 */

#ifndef SRL_FIDX_PATH
#define SRL_FIDX_PATH

#include <string>

#include "treeler/base/feature-vector.h"

#include "treeler/srl/srl.h"
#include "treeler/srl/paths-defs.h"


#include <iostream>

namespace treeler {
  namespace srl {


    template <typename FIdx> 
    struct PathFeatures{
    };
    
    template <>
    struct PathFeatures<FIdxBits> {
      
      static inline FIdxBits::BigWord path(const srl::SynLabelPath& syn_string_path, 
					   const srl::SynIntLabelPath& syn_int_path, 
					   const srl::UpDownPath& ud_path) {
	//get the max num of syntactic labels
	//TODO hardcoded as 6 bit
	const int kSynLabelSize = 6;
	//the max path len is 5 thus:
	//5*6=30 + path directions 5*1= 5 = 35 bits
	long res  = 0;
	assert(syn_int_path.size() + 1 == ud_path.size());

	int i = 0;
	auto it_ud = ud_path.begin();
	++it_ud;
	auto it = syn_int_path.begin(); 
	bool end = (it == syn_int_path.end());
	while (!end){
	  //get the syn label
	  int synl = *it; 
	  assert(synl >= 0); (void)synl; // (shut up compiler about unused variables)	 
	  res |= kSynLabelSize;
	  //get the direction
	  bool dir = *it;
	  res <<= 1;
	  res |= dir;

	  //update iterators
	  ++i;
	  ++it;
	  ++it_ud;
	  //end if long path 
	  end = (it == syn_int_path.end());
	  if ( i==5 ) end = true;
	  //make room for the next only if it is not the last
	  if (!end){
	    res <<= (kSynLabelSize + 1);
	  }
	}
	assert(res >= 0);
	return res; 
      }
    };


   
    template <>
    struct PathFeatures<FIdxChars> {
      
      static inline string UpDownToString(bool b){
        if (b){
          return "^";
        } else {
          return "_";
        }
      }

      static inline FIdxChars::BigWord path(const srl::SynLabelPath& syn_string_path, 
					    const srl::SynIntLabelPath& syn_int_path, 
					    const srl::UpDownPath& ud_path) { 
	string res;
	auto it_ud = ud_path.begin();
	
	res += UpDownToString(*it_ud); 
	++it_ud;
	assert(syn_string_path.size() + 1 == ud_path.size());

	for (auto it = syn_string_path.begin(); it != syn_string_path.end(); ++it){
	  assert(it_ud != ud_path.end());
	  res += *it;
	  res += UpDownToString(*it_ud);
	  ++it_ud;
	}
	return res; 
      }
    };

    template <>
    struct PathFeatures<FIdxPair> {
      
      static inline FIdxPair::BigWord path(const srl::SynLabelPath& syn_string_path, 
					   const srl::SynIntLabelPath& syn_int_path, 
					   const srl::UpDownPath& ud_path) { 
	return FIdxPair::BigWord(PathFeatures<FIdxBits>::path(syn_string_path, syn_int_path, ud_path), 
				 PathFeatures<FIdxChars>::path(syn_string_path, syn_int_path, ud_path)); 
      }      
    };
    
  }
}

#endif 
