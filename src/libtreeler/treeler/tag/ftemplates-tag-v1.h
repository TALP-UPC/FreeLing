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
 * \file   ftemplates-tag-v1.h
 * \brief  Feature templates for tagging
 * \author 
 */

#ifndef DEP_FTEMPLATES_TAG_V1_H
#define DEP_FTEMPLATES_TAG_V1_H

#include <string>
#include "treeler/base/fidx.h"
#include "treeler/tag/tag-symbols.h"

namespace treeler {

  enum FTypesTag {
    FGEN_TAG_ONE, 
    FGEN_TAG_P, 
    FGEN_TAG_W, 
    FGEN_TAG_P_W, 
    FGEN_TAG_Pm1_P,
    FGEN_TAG_P_Pp1,
    FGEN_TAG_Wm1_W,
    FGEN_TAG_W_Wp1,
    FGEN_TAG_MORPHO,
    FGEN_TAG_P_MORPHO
  };


  class FTemplatesTagV1 {
  public:

    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract(const TagSymbols& symbols, const X& x, const R& r, Functor& F) {

      const int WIDTH = 3;
      const int CIDX = 1; 
      typename FIdx::Tag word[WIDTH];  
      typename FIdx::Word ftag[WIDTH];
      
      for (int i = 0 ; i < WIDTH; i++) {
	int idx = i-CIDX + r.i;
	if (idx < 0 || idx >= x.size()) {
	  ftag[i] = FIdx::rootTag();
	  word[i] = FIdx::rootWord();
	}
	else {
	  const typename X::Token& tok = x.get_token(idx);
	  ftag[i]  = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(tok.fine_pos()));
	  word[i] = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(tok.word()));
	}
      }
      
      F( FIdx() << FIdx::code(FGEN_TAG_ONE, "true") ) ; 
      F( FIdx() << ftag[CIDX] << FIdx::code(FGEN_TAG_P, "p") ); 
      F( FIdx() << word[CIDX] << FIdx::code(FGEN_TAG_W, "w") ); 
      F( FIdx() << word[CIDX] << ftag[CIDX] << FIdx::code(FGEN_TAG_P_W, "p_w") ); 

      F( FIdx() << ftag[CIDX] << ftag[CIDX-1] << FIdx::code(FGEN_TAG_Pm1_P, "p-1:p0") ); 
      F( FIdx() << ftag[CIDX+1] << ftag[CIDX] << FIdx::code(FGEN_TAG_P_Pp1, "p0:p1") ); 

      F( FIdx() << ftag[CIDX] << ftag[CIDX-1] << FIdx::code(FGEN_TAG_Wm1_W, "w-1:w0") ); 
      F( FIdx() << word[CIDX+1] << word[CIDX] << FIdx::code(FGEN_TAG_W_Wp1, "w0:w1") ); 


      {
 	typedef std::list<typename X::Tag> MorphoList;
      	const MorphoList& ml  = x.get_token(r.i).morpho_tags(); 
	
      	for (auto mm = ml.begin(); mm != ml.end(); ++mm) {
      	  typename FIdx::Tag fmm = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*mm)); 
	  F( FIdx() << fmm << FIdx::code(FGEN_TAG_MORPHO, "m") ); 
	  F( FIdx() << fmm << ftag[CIDX] << FIdx::code(FGEN_TAG_P_MORPHO, "p_m") ); 
      	}
      }      

    }
  };

}



#endif /* TAG_FTEMPLATES_V1_H */
