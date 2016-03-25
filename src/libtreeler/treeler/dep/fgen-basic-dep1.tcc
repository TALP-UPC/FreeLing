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
 * \file   fgen-basic-dep1.tcc
 * \brief  Implementation of FGenBasicDep1 template methods
 * \author Xavier Carreras
 */

#include <string>
#include <iostream>
using namespace std;

#include <limits.h>
#include "treeler/dep/fgen-ftemplates-dep1.h"
#include "treeler/util/char-utils.h"
#include "treeler/util/options.h"

namespace treeler {

  // Creates a cache of feature vectors for x
  template <typename X>
  void FGenBasicDep1::create_cache(const DepSymbols& symbols, 
				   const Configuration& config,
				   const X& x,
				   FVec*& cache_tok, FVec*& cache_dep) {
    int N = x.size();
    int NN = N*N;
    try {
      cache_tok = new FVec[N + 1];
      cache_dep = new FVec[NN];
    }
    catch(bad_alloc& e) //handle exception thrown from f()
      {
	cerr << "FGenBasicDep1 : exception (" << e.what()
	     << ") : not enough memory to allocate part-wise structures, sentence" << x.id() << " (" << x.size() << " tokens;)\n";
	assert(0);
      }

    /* EXTRACT TOKEN FEATURES */
    list<FeatureIdx>* tok_fidx = NULL;
    try {
      tok_fidx = new list<FeatureIdx>[N];
    }
    catch(bad_alloc &e){
      cerr << "FGenBasicDep1 : not enough memory (sentence " << x.id() << ")" << endl;
      assert(0);
    }
    list<FeatureIdx> root_fidx;
    if(config.use_token) {
      FGenFTemplatesDep1::phi_token(x, config.word_limit, config.use_pos, tok_fidx);
      FGenFTemplatesDep1::phi_root_token(x, config.word_limit, config.use_pos, root_fidx);
      //       if(config.use_chtype || config.affix_level > 0 || config.cluster_limit > 0) {
      //         fgc_token::phi(*_cur_info, tok_ftuples);
      //       }
    }
    if(config.use_token_context) {
      FGenFTemplatesDep1::phi_token_context(x, 2, config.word_limit, config.use_pos, tok_fidx);
      //       if(config.use_chtype || config.affix_level > 0 || config.cluster_limit > 0) {
      //         fgc_token_context::phi(*_cur_info, tok_ftuples, 2);
      //       }
    }

    /* EXTRACT DEPENDENCY FEATURES */
    list<FeatureIdx>* dep_fidx = NULL;
    try {
      dep_fidx = new list<FeatureIdx>[NN];
    }
    catch(bad_alloc& e) {
      cerr << "FGenBasicDep1 : not enough memory (sentence "<< x.id() <<")"<< endl;
      assert(0);
    }

    // for (int j = 0; j < N; j++){
    // const token& t = x->get_token(j);
    //assert(t.fine_tag() < 48);
    //}

    if(config.use_dependency) {
      FGenFTemplatesDep1::phi_dependency(x, config.word_limit, config.use_pos, dep_fidx);
      //       if(_use_chtype || _affix_level > 0 || _cluster_limit > 0) {
      //         fgc_dependency::ft_phi(sent_x, *_cur_info, dep_ftuples);
      //       }
    }
    if(config.use_dependency_context) {
      FGenFTemplatesDep1::phi_dependency_context(x, config.word_limit, config.use_pos, dep_fidx);
      //       if(_use_chtype || _affix_level > 0 || _cluster_limit > 0) {
      //         fgc_dependency_context::ft_phi(*_cur_info, dep_ftuples);
      //      }
    }
    if(config.use_dependency_distance) {
      FGenFTemplatesDep1::phi_dependency_distance(x, config.word_limit, config.use_pos, dep_fidx);
      //       if(_use_chtype || _affix_level > 0 || _cluster_limit > 0) {
      //         fgc_dependency_distance::ft_phi(*_cur_info, dep_ftuples);
      //       }
    }
    if(config.use_dependency_between) {
      FGenFTemplatesDep1::phi_dependency_between(symbols, x, config.word_limit, config.use_pos, dep_fidx);
      //       if(_use_chtype || _affix_level > 0 || _cluster_limit > 0) {
      //         fgc_dependency_between::ft_phi(*_cur_info, dep_ftuples);
      //       }
    }


    /* NOW CREATE THE FEATURE VECTORS IN THE CACHES */
    for(int h = -1; h < N; ++h) {
      { /* token features */
	list<FeatureIdx>& f_tok = (h == -1 ? root_fidx : tok_fidx[h]);
	FeatureIdx* arr_t = NULL;
	try{
	  arr_t =  new FeatureIdx[f_tok.size()];
	}
	catch(bad_alloc& e) {
	  assert(false);
	}
	assert(arr_t != NULL);
	/* copy in the head features */
	list<FeatureIdx>::const_iterator it = f_tok.begin();
	list<FeatureIdx>::const_iterator it_end = f_tok.end();
	FeatureIdx* fj = arr_t;
	for(; it != it_end; ++it, ++fj) { *fj = *it; }
	FVec* fv_t = cache_tok + h + 1;
	fv_t->idx = arr_t;
	fv_t->val = NULL; /* indicator features */
	fv_t->n = f_tok.size();
	fv_t->next = NULL;
	f_tok.clear();
      }
      if(h == -1) { continue; }

      for(int m = 0; m < N; ++m) {
	/* features for the head-token and the dependency */
	const int dep_idx = h + N*m; //! \todo Change this to use the index function?
	list<FeatureIdx>& f_dep = dep_fidx[dep_idx];
	/* allocate the feature array */
	//ftuple* const arr_dep = (ftuple*)malloc(f_dep.size()*sizeof(ftuple));
	FeatureIdx* arr_dep = NULL;
	try{
	  arr_dep = new FeatureIdx[f_dep.size()];
	}
	catch(bad_alloc& e) {
	  assert(false);
	}

	/* copy in the dependency features */
	list<FeatureIdx>::const_iterator it = f_dep.begin();
	list<FeatureIdx>::const_iterator it_end = f_dep.end();
	FeatureIdx* fj = arr_dep;
	for(; it != it_end; ++it, ++fj) { *fj = *it; }
	/* assign values to the output feature vectors */
	FVec* fv = cache_dep + dep_idx;
	fv->idx = arr_dep;
	fv->val = NULL; /* indicator features */
	fv->n = f_dep.size();
	fv->next = NULL;
	f_dep.clear();
      } // for m = ...
    } // for h = ...

    delete [] tok_fidx;
    delete [] dep_fidx;
  }


  // destroy a cache
  // Destroys a cache of feature vectors
  template <typename X>
  void FGenBasicDep1::destroy_cache(const X& x, FVec* cache_tok, FVec* cache_dep) {
    int N = x.size();
    FVec *it_cache_tok = cache_tok;
    FVec *it_cache_dep = cache_dep;
    for (int i = 0; i < N; ++i, ++it_cache_tok) {
      delete[] it_cache_tok->idx;
      for (int j = 0; j < N; ++j, ++it_cache_dep) {
        delete[] it_cache_dep->idx;
      }
    }
    //delete the last (N+1)-th it_cache_tok
    delete[] it_cache_tok->idx;
    // delete the arrays of pointers
    delete[] cache_tok;
    delete[] cache_dep;
  }



  // Returns a copy of a token feature vector from the cache
  template <typename X>
  typename FGenBasicDep1::FVec* FGenBasicDep1::phi_token(const X& x, int t,
							 FVec* cache) {
    assert( t < x.size());
    FVec* fv;
    try {
      fv = new FVec;
    }
    catch (exception e) {
      cerr << "FGenBasicDep1 : not enough memory"<< endl;
      assert(0);
      return NULL;
    }
    FVec* cached = cache + token_index(x.size(), t);
    assert(cached->idx!=NULL);
    fv->idx = cached->idx;
    fv->val = cached->val;
    fv->n = cached->n;
    fv->offset = 0;
    fv->next = NULL;
    return fv;
  }

  // Returns a copy of a dependency feature vector from the cache
  template <typename X>
  typename FGenBasicDep1::FVec* FGenBasicDep1::phi_dependency(const X& x,
							      int h, int m,
							      FVec* cache) {
    FVec* fv;
    try {
      fv = new FVec;
    }
    catch (exception e) {
      cerr << "FGenBasicDep1 : not enough memory"<< endl;
      assert(0);
      return NULL;
    }
    FVec* cached = cache + dep_index(x.size(), h, m);
    if (cached->idx==NULL) {
      cerr << "FGenBasicDep1 : null fvec in cache for pattern x=" << x.id() 
	   << " h="<< h << " m="<< m << " idx="<< dep_index(x.size(), h, m) << endl;
      assert(cached->idx!=NULL);      
      exit(0);
    }
    fv->idx = cached->idx;
    fv->val = cached->val;
    fv->n = cached->n;
    fv->offset = 0;
    fv->next = NULL;
    return fv;
  }





} // namespace treeler
