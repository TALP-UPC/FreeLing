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
 * \file   fgen-basic-dep1.h
 * \brief  Declaration of FGenBasicDep1v05
 * \author Xavier Carreras
 */

#ifndef DEP_FGENBASICDEP1_V05_H
#define DEP_FGENBASICDEP1_V05_H


#include <list>
#include <string>
#include <vector>

#include "treeler/base/feature-vector.h"
#include "treeler/dep/dep-symbols.h"

namespace treeler {

  /** \brief A class that generates first-order features for dependencies 
   *  \author Xavier Carreras
   *
   *  \todo{add documentation}
   *
   *   Templated on input pattern X. A class X must define:
   *      - A subtype X::Token
   *      - size(), returning the number of tokens
   *      - get_token(i), returning a const ref to a Token, where i ranges from 0 to size()-1
   *      - Token defines the following methods: int word(), int cpos(), 
   *      int fpos(), list<int> morphos()
   *
   *
   */
  class FGenBasicDep1v05 {
  public:

    typedef FeatureVector<FIdxV0> FVec;

    struct Configuration {
      /* flags for activation of various feature classes */
      bool use_token;
      bool use_token_context;
      bool use_dependency;
      bool use_dependency_context;
      bool use_dependency_between;
      bool use_dependency_distance;

      /* flags for classes of information */
      int  word_limit;
      bool use_pos;
      bool use_o2;  
      /* count cutoff for features */
      int  cutoff;
    };
    
    //! Creates a cache of feature vectors for x
    template <typename X>
      static void create_cache(const DepSymbols& symbols,
			       const Configuration& config,
			       const X& x,
			       FVec*& cache_tok, FVec*& cache_dep);
    
    //! Destroys a cache of feature vectors
    template <typename X>
      static void destroy_cache(const X& x,
				FVec* cache_tok, FVec* cache_dep);
    
    //! Returns a copy of a token feature vector from the cache
    template <typename X>
      static FVec* phi_token(const X& x,
			     int t,
			     FVec* cache);
    
    //! Returns a copy of a dependency feature vector from the cache
    template <typename X>
      static FVec* phi_dependency(const X& x,
				  int h, int m,
				  FVec* cache);
    

    /* // THE METHODS BELOW SHOULD BE REVISITED */

    /* static void phi(const sentence* const x, */
    /* 		    ftuple_dictionary* dict, */
    /* 		    std::vector<int> F[], */
    /* 		    const configuration& config) { */
    /*   const int N = x->size(); */
    /*   const int NN = N*N; */
    /*   std::list<ftuple>* FT = new std::list<ftuple>[NN]; */
    /*   ft_phi(x, FT, config); */
    /*   for(int i = 0; i < NN; ++i) { */
    /* 	dict->map_all(F[i], FT[i]); FT[i].clear(); */
    /*   } */
    /*   delete [] FT; */
    /* } */


    /* //! Token features */
    /* template<class X> */
    /* static void phi_token(const X& x,  */
    /* 			  const configuration& config, */
    /* 			  std::list<FeatureIdx> F[]); */
    
    /* //! Dependency features */
    /* template<class X> */
    /* static void phi_dependency(const X& x, */
    /* 			       const configuration& config, */
    /* 			       std::list<FeatureIdx> F[]);     */
    

    /* static void phi(const sentence* const x, std::vector<int>& f, */
    /* 		    ftuple_dictionary* dict, */
    /* 		    const configuration& config, */
    /* 		    const int head, const int mod) { */
    /*   std::list<ftuple> tmp; */
    /*   ft_phi(x, tmp, config, head, mod); */
    /*   dict->map_all(f, tmp); */
    /* } */


    /* static void ft_phi(const sentence* const x, */
    /* 		       std::list<ftuple> F[], */
    /* 		       const configuration& config); */


    /* static void ft_phi(const sentence* const x, std::list<ftuple>& f, */
    /* 		       const configuration& config, */
    /* 		       const int head, const int mod); */


  private:

    // returns an index for a token in [0, N+1]
    // t is in [-1, N-1]
    static int token_index(int N, int t) {
      assert(t < N);
      return t+1;
    }

    // returns an index for a dependency in [0, NN-1]
    // h is in [-1, N-1]
    // m is in [0, N-1]
    static int dep_index(int N, int h, int m) {
      assert(h < N);
      assert(m < N);
      int hh = (h==-1) ? m : h;
      return m*N + hh;
    }

  };
}

#include "treeler/dep/fgen-basic-dep1-v0-5.tcc"

#endif /* DEP_FGENBASICDEP1_H */
