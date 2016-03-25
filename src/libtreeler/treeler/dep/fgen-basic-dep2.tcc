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
 * \file   fgen-basic-dep2.tcc
 * \brief  Implementation of FGenBasicDep2 template methods
 * \author Xavier Carreras
 */

#include <iostream>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <exception>

#include "treeler/util/options.h"
#include "treeler/util/char-utils.h"
#include "treeler/dep/fgen-ftemplates-dep2.h"

using namespace std;

#define DEPFEATS2_VERBOSE 0

#define CHECK_INDEX(NN,LL,tt,hh,mm,ll,cc)				\
{                                                                                                  \
  int r = DPO2Index::encode(NN,tt,hh,mm,ll,cc);                                                  \
  cout << _name << " :: part [";                                                                  \
  if ( tt==DPO2Index::FO ) cout << "FO";   \
  if ( tt==DPO2Index::CH ) cout << "CH";   \
  if ( tt==DPO2Index::CMO ) cout << "CMI"; \
  if ( tt==DPO2Index::CMI ) cout << "CMO"; \
  cout << "," << hh <<","<< mm <<","<< ll <<","<< cc << "] ";      \
  cout << " index " << r << " : ";                                                                \
  bool ok = true;                                                                                 \
  DPO2Index::dpo2_type dt;                                                                       \
  int dh=-2, dm=-2, dl=-2, dc=-2;                                                                 \
  DPO2Index::decode(NN, LL, r, &dt, &dh, &dm, &dl, &dc);                                         \
  if ( tt!=dt ) { cout << " type " << tt << " " << dt << " !!! "; ok = false; }                   \
  if ( hh!=dh ) { cout << " head " << hh << " " << dh << " !!! "; ok = false; }                   \
  if ( mm!=dm ) { cout << " mod " << mm << " " << dm << " !!! ";  ok = false; }                   \
  if ( ll!=dl ) { cout << " label " << ll << " " << dl << " !!! "; ok = false; }                  \
  if ( tt!=DPO2Index::FO and cc!=dc ) { cout << " child " << cc << " " << dc << " !!! ";  ok = false; } \
  if ( ok ) { cout << "OK"; } \
  cout << endl;  \
}                                                                                      \

namespace treeler {

  
  //! Creates a cache of feature vectors for x
  template <typename X>
  void FGenBasicDep2::create_cache(const DepSymbols& symbols,
				   const Configuration& config,
				   const X& x,				   
				   FeatureVector<>*& cache) {    
    assert(config.on_demand);
    int N = x.size();
    int NNN = N*N*N;
    
    try {
      cache = new FeatureVector<>[NNN];
    }
    catch(bad_alloc& e) //handle exception thrown from f()
      {
	cerr << "FGenBasicDep2 : exception (" << e.what() << ") : not enough memory to allocate part-wise structures on input " << x.id() << " (" << x.size() << " tokens;)\n";
	assert(0);
      }
    FeatureVector<> *tmp = cache;
    // mark that feature vectors are actually empty
    for (int i=0; i<NNN; ++i, ++tmp) tmp->idx = NULL;
  }
  
  //! Destroys a cache of feature vectors
  template <typename X>
  void FGenBasicDep2::destroy_cache(const X& x,
				    FeatureVector<>* cache) {
    int N = x.size();
    int NNN = N*N*N;
    FeatureVector<> *tmp = cache;
    for (int i=0; i<NNN; ++i, ++tmp){
      delete [] tmp->idx;
    }
    delete [] cache;       
  }
  
  template <typename X>
  FeatureVector<>* FGenBasicDep2::phi_dep2(const Configuration& config,
					   const X& x,
					   int h, int m, int c,
					   FeatureVector<>* cache) {
    int N = x.size();
    int hh = h==-1 ? m : h;
    int cc = (c==-1) ? m : c;
    
    int index = hh*N*N + m*N + cc;
    FeatureVector<>* cached = cache + index;
    if (cached->idx==NULL) {
      // GENERATE features for <h,m,c>
      list<FeatureIdx> fidxlist;
      FGenFTemplatesDep2::phi_dep2(x, h, m, c, config.word_limit, config.use_pos, config.use_trigrams, fidxlist);
      // allocate the feature array
      int n = fidxlist.size();
      FeatureIdx* f = new FeatureIdx[n];
      assert(f != NULL);
      /* copy in the ftuples */
      list<FeatureIdx>::const_iterator it = fidxlist.begin();
      list<FeatureIdx>::const_iterator it_end = fidxlist.end();
      FeatureIdx* fj = f;
      for(; it != it_end; ++it, ++fj) { *fj = *it; }
      fidxlist.clear();
      cached->idx = f;
      cached->val = NULL; /* indicator features */
      cached->n = n;
      cached->offset = 0; /* logical offset (set later) */
      cached->next = NULL;
    }
    FeatureVector<>* fv;
    try {
      fv = new FeatureVector<>;
    }
    catch (exception e) {
      cerr << "FGenBasicDep2 : not enough memory"<< endl;
      assert(0);
    }
    fv->idx = cached->idx;
    fv->val = cached->val;
    fv->n = cached->n;
    fv->offset = 0;
    fv->next = NULL;
    return  fv;
  }
 
#undef CHECK_INDEX

}
