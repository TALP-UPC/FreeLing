#ifndef TAG_FGENTTAG_H
#define TAG_FGENTTAG_H


#include <list>
#include <string>
#include <vector>

#include "treeler/base/feature-vector.h"
#include "treeler/dep/fgen-ftemplates-dep1.h"
#include "treeler/tag/tag-symbols.h"

namespace treeler {

  /** \brief A class that generates features for tuple inputs
   *  \author Xavier Carreras
   */

  template <typename X, typename R>
  class FGenTTag {  
  public:
    typedef FIdxV0 FIdx;
    typedef FeatureVector<FIdx> FVec;
    
    struct Configuration {
      int L;
      
      Configuration() 
	: L(0)
      {}
    };

  private:
    const TagSymbols& _symbols; 
    Configuration _config;

  public:
    
    FGenTTag(const TagSymbols& s) 
      : _symbols(s) 
    {}

    Configuration& config() { return _config; }

    int spaces() const { return (_config.L+1)*_config.L; };

    class Features {
    public:  
      friend class FGenTTag;
      Features(): _n(0), _L(0), _cache(NULL) {}
      ~Features() {
	clear();
      }
      
      void clear();
      const FVec* phi(const X& x, const R& r) const; 
      void discard(const FVec* f, const X& x, const R& r) const; 
      
    private:
      int _n;
      int _L;
      FVec* _cache;
    };

    void features(const X& x, Features& f) const; 
  };



  /***************************************************
   *
   *  IMPLEMENTATION
   *
   ***************************************************/

  // create a cache
  template <typename X, typename R>
  void FGenTTag<X,R>::features(const X& x, Features& f) const {
    assert(f._cache==NULL); 
    f._n = x.size(); 
    f._L = _config.L;
    try {
      f._cache = new FVec[f._n];
    }
    catch(bad_alloc& e) //handle exception thrown from f()
      {
        cerr << "FGenTTag : exception (" << e.what()
             << ") : not enough memory to allocate part-wise structures, input " << x.id() << " (" << x.size() << " tokens;)\n";
	exit(0);
      }

    /* EXTRACT TOKEN FEATURES */
    list<FeatureIdx>* tok_fidx = NULL;
    try {
      tok_fidx = new list<FeatureIdx>[f._n];
    }
    catch(bad_alloc& e){
      cerr << "FGenTTag : not enough memory (input " << x.id() << ")" << endl;
      exit(0);
    }

    /* 
       The following block creates a feature index for every dimension
       and value of the tuple. Each feature index consists of a type,
       which is set to the dimension, and a value, which is the value
       itself at that dimension. Negative values in a tuple, which
       stand for unknown values, are mapped to a special null value in
       the feature index.
    */
    {            
      for (int i=0; i<f._n; ++i) {
	const typename X::Tuple& tuple = x.tuple(i);
	list<FeatureIdx>& idxs = tok_fidx[i];
	for (int d=0; d<x.dim(); ++d) {
	  int v = tuple[d];
	  if (v<0) {
	    idxs.push_back(feature_idx_w(d, feature_idx_nullw)); 
	  }
	  else {
	    idxs.push_back(feature_idx_w(d, v)); 
	  }
	}
      }
    }

    /* NOW CREATE THE FEATURE VECTORS IN THE CACHES */    
    for(int i = 0; i < f._n; ++i) {
      { /* token features */
        list<FeatureIdx>& f_tok = tok_fidx[i];
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
        FVec* fv_t = f._cache + i;
        fv_t->idx = arr_t;
        fv_t->val = NULL; /* indicator features */
        fv_t->n = f_tok.size();
        fv_t->next = NULL;
        f_tok.clear();
      }
    }
  }

  // destroy the cache
  template <typename X, typename R>
  void FGenTTag<X,R>::Features::clear() {
    if (_cache!=NULL) {
      FVec* f_r = _cache; 
      for (int i=0; i<_n; ++i) {
	delete [] f_r->idx; 
	++f_r;
      }
      delete [] _cache;
      _cache=NULL;
    }    
  }
  
  template <typename X, typename R>
  const typename FGenTTag<X,R>::FVec* FGenTTag<X,R>::Features::phi(const X& x, const R& r) const {
    assert(_n == x.size()); 
    assert(r.i < _n); 
    FVec* f = NULL;
    try {
      f = new FVec(); 
    }
    catch (exception e) {
      cerr << "FGenTTag : not enough memory" << endl; 
      exit(0);
    }
    FVec* cached = _cache + r.i;
    f->n = cached->n; 
    f->idx = cached->idx; 
    f->val = NULL;
    f->offset = r.b * (_L+1) + r.a+1;
    f->next = NULL;
    return f;     
  }

  template <typename X, typename R>
  void FGenTTag<X,R>::Features::discard(const FVec* f, const X& x, const R& r) const {
    delete f; 
  }


}


#endif /* TAG_FGENTTAG_H */
