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
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \author Xavier Carreras
 */
#ifndef TREELER_TAGSYMBOLS_H
#define TREELER_TAGSYMBOLS_H

#include <string>
using namespace std; 

#include "treeler/base/dictionary.h"
#include "treeler/dep/dep-symbols.h"


namespace treeler {

  enum TagFields {
    TAG = DepFields::SYNTACTIC_LABEL+1
  };


  namespace _dep_symbols_internal {

    template <> 
    struct FieldMap<TAG> {
      template <typename SYM, typename T1, typename T2>
      static inline T1 map_field(const SYM& Sym, const T2& t) { 
	return _dep_symbols_internal::map_value<T1,T2>(Sym.d_tags, t); 
      }
    };   

  }
  

  /**********
   *
   *
   ****/
  class TagSymbols : public DepSymbols {
  public:
    Dictionary d_tags;    

    template <int FIELD, typename T1, typename T2> 
    inline T1 map_field(const T2& t) const {
      typedef _dep_symbols_internal::FieldMap<FIELD> FieldMap; 
      return FieldMap::template map_field<TagSymbols,T1,T2>(*this, t); 
    }
    
  };
  


}

#endif

