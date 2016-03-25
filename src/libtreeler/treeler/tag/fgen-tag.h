#ifndef TAG_FGENTAG_H
#define TAG_FGENTAG_H


#include <list>
#include <string>
#include <vector>

#include "treeler/base/feature-vector.h"
#include "treeler/tag/ftemplates-tag-v1.h"

namespace treeler {

  /** \brief A class that generates features n-gram based tagging
   *  \author Xavier Carreras
   */

  template <typename X, typename R, typename FIdx_p = FIdxBits>
  class FGenTag {  
  public:
    typedef FIdx_p FIdx;
    typedef FeatureVector<FIdx> FVec;

    struct Configuration {
      /* flags for activation of various feature classes */
      bool use_token;
      bool use_token_context;
      /* flags for classes of information */
      int  word_limit;
      bool use_pos;
      bool backoff;
      
      Configuration() 
	: use_token(1), use_token_context(0), word_limit(-1), use_pos(1), backoff(true)
      {}
    };

  private:
    const TagSymbols& _symbols;
    Configuration _config; 
    int _L;
  public:

    FGenTag(const TagSymbols& sym) 
      : _symbols(sym) 
    {
      _L = sym.d_tags.size(); 
    }

    FGenTag(const TagSymbols& sym, const Configuration& config)
      : _symbols(sym), _config(config) 
    {}

    Configuration& config() { return _config; }

    int spaces() const { 
      int n = (_L+1)*_L; // one space per bigram, where the previous tag may be *START*
      if (_config.backoff) {
	n += _L;
      }
      return n; 
    };

    class Features {
    public:  
      friend class FGenTag;
      Features(): _n(0), _L(0), _cache(NULL) {}
      ~Features() {
	clear();
      }
      
      void clear();
      const FVec* phi(const R& r) const; 
      const FVec* operator()(const R& r) const { return phi(r); }
      void discard(const FVec* f) const; 
      
    private:
      int _n;
      int _L;
      FVec* _cache;
      bool _backoff;
    };

    void features(const X& x, Features& f) const; 
  };


  template <typename X, typename R, typename FIdxA, typename FIdxB>
  struct SwitchFIdx<FGenTag<X,R,FIdxA>,FIdxB> {
    typedef FGenTag<X,R,FIdxB> F;
  };



  /***************************************************
   *
   *  IMPLEMENTATION
   *
   ***************************************************/

  // create a cache
  template <typename X, typename R, typename FIdx>
  void FGenTag<X,R,FIdx>::features(const X& x, Features& f) const {
    assert(f._cache==NULL); 
    f._n = x.size(); 
    f._L = _L;
    f._backoff = _config.backoff;
    try {
      f._cache = new FVec[f._n];
    }
    catch(bad_alloc& e) //handle exception thrown from f()
      {
        cerr << "FGenTok : exception (" << e.what()
             << ") : not enough memory to allocate part-wise structures, input " << x.id() << " (" << x.size() << " tokens;)\n";
	exit(0);
      }

    R r; 
    for (r.i=0; r.i<x.size(); ++r.i) {
      CollectFIdx<FIdx> functor; 
      FTemplatesTagV1::extract<FIdx>(_symbols, x, r, functor); 
      FVec* fv_t = f._cache + r.i;
      fv_t->idx = functor.create_array();
      fv_t->val = NULL; /* indicator features */
      fv_t->n = functor.size();
      fv_t->next = NULL;
    }
  }

  // destroy the cache
  template <typename X, typename R, typename FIdx>
  void FGenTag<X,R,FIdx>::Features::clear() {
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
  
  template <typename X, typename R, typename FIdx>
  const typename FGenTag<X,R,FIdx>::FVec* FGenTag<X,R,FIdx>::Features::phi(const R& r) const {
    assert(r.i < _n); 
    FVec* f = NULL;
    try {
      f = new FVec(); 
    }
    catch (exception e) {
      cerr << "FGenTag : not enough memory" << endl; 
      exit(0);
    }
    FVec* cached = _cache + r.i;
    f->n = cached->n; 
    f->idx = cached->idx; 
    f->val = NULL;
    f->offset = r.b * (_L+1) + r.a+1;
    f->next = NULL;

    if (_backoff) {
      FVec* f2 = new FVec(); 
      *f2 = *f;
      f->next = f2; 
      f2->next = NULL;
      f2->offset = (_L+1)*_L + r.b;
    }
    return f;     
  }


  template <typename X, typename R, typename FIdx>
  void FGenTag<X,R,FIdx>::Features::discard(const FVec* f) const {
    if (_backoff) {
      delete f->next; 
    }
    delete f; 
  }


}


#endif /* TAG_FGENTAG_H */
