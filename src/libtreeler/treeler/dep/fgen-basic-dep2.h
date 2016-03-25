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
 * \file   fgen-basic-dep2.h
 * \brief  Declaration of FGenBasicDep2
 * \author Xavier Carreras
 */


#ifndef DEP_FGENBASICDEP2_H
#define DEP_FGENBASICDEP2_H

#include "treeler/dep/dep-symbols.h"

using namespace std;

namespace treeler {

  typedef int64_t dim_int;
  typedef int64_t dim_std_int;

  /**
   * \brief Feature generator for second-order dependency parsing
   * \author Xavier Carreras and Terry Koo 
   *
   * Feature vectors are of the form phi(x, h, m, c) where x is a
   * sentence, h and m are head-modifier dependency indices, and c is an
   * index representing a second-order relation of the dependency.
   *
   * We consider two types of second-order vectors:
   *
   *   inwards: if h<m, then h<c<m
   *            if m<h, then m<c<h
   *
   *   outwards: if h<m, then h<m<c
   *             if m<h, then c<m<h
   *
   * Inwards feature vectors may be used to represent sibling
   * relations, where c is a sibling of m; or grandparental relations,
   * where c is a child of m in the same direction as h. 
   *
   * Outwards feature vectors may be used to represent grandparental
   * relations, where c is a child of m in the opposite direction of h.
   * 
   * Special values:
   *
   *   h=-1 means root dependency
   *   c=m means null child
   */    
  class FGenBasicDep2 {
  public:
    struct Configuration {
      /* flags for activation of various feature classes */

      /* flags for classes of information */
      int  word_limit;
      bool use_trigrams;
      bool use_pos;
      bool use_o2;  
      /* create feature vectors on demand */
      bool on_demand;
    };

    //! Creates a cache of feature vectors for x
    template <typename X>
    static void create_cache(const DepSymbols& symbols,
			     const Configuration& config,
			     const X& x,			     
			     FeatureVector<>*& cache);

    //! Destroys a cache of feature vectors
    template <typename X>
    static void destroy_cache(const X& x,
			      FeatureVector<>* cache);

    /**   
     * Returns a copy of a dependency feature vector from the cache.
     *
     * Generation is done on demand: if the fvec is not in the cache, 
     * it is created and stored for future use.
     */    
    template <typename X>
    static FeatureVector<>* phi_dep2(const Configuration& config,
				     const X& x,
				     int h, int m, int c,
				     FeatureVector<>* cache);
    
  };
}

#include "treeler/dep/fgen-basic-dep2.tcc"

#endif /* DEP_FGENBASICDEP2_H */

