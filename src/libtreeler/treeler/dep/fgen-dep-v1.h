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
 * \file   fgen-dep-v1.h
 * \brief  Standard features for dependency parsing
 * \author 
 */

#ifndef DEP_FGENDEP_V1_H
#define DEP_FGENDEP_V1_H

#include <string>
#include <list>
#include "treeler/base/feature-vector.h"
#include "treeler/dep/dep-symbols.h"
#include "treeler/dep/ftemplates-dep1-v1.h"

namespace treeler {

  /** 
      \brief A structure of features for an input pattern 
  */
  template <typename X, typename R, typename FIdx>
  class DepFeaturesV1;

  /** 
      \brief This class provides a static method to extract features that depends on FIdx
  */
  template <typename X, typename R, typename FIdx>
  class DepFeaturesV1Extractor;

  template <typename X, typename R, typename FIdx, typename CachedExtractor>
  class FeaturesCache;

  template <typename X, typename R, typename FIdx>
  class Dep1InnerCache;

  /** 
      \brief A feature generator for dependency parsing

      It is templated on: 
      - X : the type of input structure (i.e. a sentence)
      - R : the type of parts (i.e. a dependency edge)
      - FIdx : the type of feature indices it extracts      
  */
  template <typename X, typename R, typename FIdx_par = FIdxBits>
  class FGenDepV1 { 
  public:
    typedef FIdx_par FIdx;
    // a feature structure for an X instance
    typedef DepFeaturesV1<X,R,FIdx> Features;

    struct Configuration {
      //! Number of dependency labels
      int L;

      //! dictionary directory 
      std::string dictdir;
      std::string posfile;
      std::string name;        

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

      Configuration() 
	: L(1), dictdir(""), posfile(""), name("") {}
    };

  public:

    // members

    const DepSymbols& symbols;
    Configuration config;

    // methods

    FGenDepV1(const DepSymbols& sym) : 
      symbols(sym)
    {}

    ~FGenDepV1() 
    {}

    int spaces() const { 
      assert(config.L >= 0);
      return 3*config.L; // head tok, mod tok, dep
    };
    
    // creates features for x in f
    void features(const X& x, Features& f) const; 

  };


  template <typename X, typename R, typename FIdxA, typename FIdxB>
  struct SwitchFIdx<FGenDepV1<X,R,FIdxA>,FIdxB> {
    typedef FGenDepV1<X,R,FIdxB> F;
  };


  template <typename X, typename R, typename FIdx>
  void FGenDepV1<X,R,FIdx>::features(const X& x, Features& f) const {
    f.init(x, symbols);
    //assert(f._x==NULL);
    //f._x = &x;
    //f.symbols = &symbols;
  }

  // the features for all parts of a pattern
  template <typename X, typename R, typename FIdx>
  class DepFeaturesV1 {
  public:

    // creation and destruction
    DepFeaturesV1():   symbols_(NULL), x_(NULL) {} 
    ~DepFeaturesV1() { 
      clear(); }

    //loads the params
    void init(const X& x, const DepSymbols& symbols);

    // clears the feature structure
    void clear();

    //! Returns a feature vector for input x and part r
    const FeatureVector<FIdx>* phi(const R& r); 
    const FeatureVector<FIdx>* operator()(const R& r) { return phi(r); } 

    FIdx* extract(const X& x, const R& r) const; 

    void discard(const FeatureVector<FIdx>* f) const; 

  private:
    const DepSymbols* symbols_;
    const X* x_; // a pointer to the pattern 
    typedef Dep1InnerCache<X,R,FIdx> InnerCacheType;
    FeaturesCache<X,R,FIdx, InnerCacheType > features_cache_;

  };

  template <typename X, typename R, typename FIdx>
  void DepFeaturesV1<X,R,FIdx>::init(const X& x, const DepSymbols& symbols){
    assert(x_ == NULL);
    x_ = &x;
    symbols_ = &symbols;
    features_cache_.init(x, symbols);
  }

  template <typename X, typename R, typename FIdx>
  void DepFeaturesV1<X,R,FIdx>::clear() {
    x_ = NULL;
  }
  
  template <typename X, typename R, typename FIdx>
  const FeatureVector<FIdx>* DepFeaturesV1<X,R,FIdx>::phi(const R& r) {
    FeatureVector<FIdx> *f_h = new FeatureVector<FIdx>;    
    FeatureVector<FIdx> *f_m = new FeatureVector<FIdx>;    
    FeatureVector<FIdx> *f_dep = new FeatureVector<FIdx>;    

    features_cache_.extract(*symbols_, *x_, r, f_h, f_m, f_dep); 
    f_dep->val = NULL;
    f_dep->next = NULL; 
    assert(f_h->idx != NULL);
    f_dep->offset = r.label()*3; 

    f_h->val = NULL;
    f_h->next = NULL;
    assert(f_h->idx != NULL);
    f_h->offset = r.label()*3 + 1; 

    f_m->val = NULL;
    f_m->next = NULL; 
    assert(f_m->idx != NULL);
    f_m->offset = r.label()*3 + 2; 

    f_dep->next = f_h;
    f_h->next = f_m;

    return f_dep;
  }

  template <typename X, typename R, typename FIdx>
  void DepFeaturesV1<X,R,FIdx>::discard(const FeatureVector<FIdx>* const f) const {
    // recurse if necessary
    if (f->next != NULL){
      discard(f->next);
    }
    //delete the inner f->idx in the cache manager if required
    features_cache_.discard(f);
    //we delete the FeatureVector f we created
    delete f;
  }

  /** This class is parameterized by the feature extractor and has
      a map<FeatureVector<FIdx> to cache features */
  template <typename X, typename R, typename FIdx, typename CachedExtractor>
  class FeaturesCache {

  private:
    //static std::unordered_map<R, FeatureVector<FIdx>* > cache_;
    FIdx const * cache_token_root_;
    int cache_token_root_size_;
    vector<FIdx const *> cache_token_;
    vector<int> cache_token_size_;
    typename R::template map<FIdx const *> cache_map_dep_;
    typename R::template map<int> cache_map_dep_size_;
    /** the current sentence */
    const X* current_sentence_;
    /** a flag to check if it is correctly initialized */
    bool started_;
    /** saves the extracted features */
    std::list<const FIdx*> extracted_list_;
    /** if true we compute all feats at init */
    bool prefill_;
    /** the inner feature extractor*/
    CachedExtractor inner_extractor_;



  public:
    FeaturesCache(): current_sentence_(NULL), started_(false), prefill_(true) {
      //clean the cache
    }
    
    ~FeaturesCache(){
      //clean the cache
      //iterate the cache and delete f->idx array
      //the FeatureVector is not a pointer, no delete is required
      started_ = false;
      
      for (auto it = extracted_list_.begin(); it != extracted_list_.end(); ++it){
	const FIdx* fidx = *it;
	delete[] fidx;
      }
    }
    
    void init(const X& x, const DepSymbols& symbols)  {
      assert(!started_);
      FIdx* ini = NULL;
      const int n = x.size();
      typename R::Configuration c;
      c.L = symbols.d_syntactic_labels.size();

      cache_token_.resize(n);
      cache_token_size_.resize(n, 0);
      cache_token_root_ = NULL;
      cache_token_root_size_ = 0;
      
      cache_map_dep_.initialize(c, n, ini);
      cache_map_dep_size_.initialize(c, n, 0);
      current_sentence_ = &x;
      
      if (prefill_){
	inner_extractor_.init(x, symbols);
      }
      
      started_ = true;
    }
    

    void extract(const DepSymbols& symbols, const X& x, 
		 const R& r_0, 
		 FeatureVector<FIdx>* f_h,
		 FeatureVector<FIdx>* f_m,
		 FeatureVector<FIdx>* f_dep
		 ) {
      //when cache is indexing is getting the label
      //we just want one feature set for each head, mod
      R r(r_0.head(), r_0.mod(), 0);
      
      assert(started_);
      
      if (&x != current_sentence_){
	current_sentence_ = &x;
	assert(false);
      }
      
      // all feature vectors are already created
      
      // if the entry is null 
      const FIdx* cache_contents = cache_map_dep_.operator()(r);
      if ( cache_contents == NULL){
	// if this cache is prefilled all feats are already computed
	// call the extractor
	// was not in cache, add it
	InsertIntoCache(symbols, x, r);
      } 
      
      // get the pointer from the cache
      // cache contents has the pointer
      RetrieveFromCache(r, f_h, f_m, f_dep);

    }

    void InsertIntoCache(const DepSymbols& symbols, const X& x,
			 const R &r){

      assert(r.label() == 0); // only have feats for label 0
          assert(prefill_);

      //we want to extract fixs h, m, dep and n
      int n_dep = 0;

      FIdx* fidx_dep = inner_extractor_.extract_dep(symbols, x, r, &n_dep);

      cache_map_dep_(r) = fidx_dep;
      cache_map_dep_size_(r) = n_dep;
      extracted_list_.push_front(fidx_dep);

      const int head = r.head();
      if (head == -1){
	if (cache_token_root_ == NULL){
	  int n_h = 0;
	  FIdx* fidx_h = inner_extractor_.extract_token(symbols, x, head, &n_h);
	  cache_token_root_ = fidx_h;
	  cache_token_root_size_ = n_h;
	  extracted_list_.push_front(fidx_h);
	} 
      }
      else{
	if (cache_token_.at(head) == NULL){
	  int n_h = 0;
	  FIdx* fidx_h = inner_extractor_.extract_token(symbols, x, head, &n_h);
	  assert(head < static_cast<int>(cache_token_.size()));
	  assert(head < static_cast<int>(cache_token_size_.size()));
	  cache_token_.at(head) = fidx_h;
	  cache_token_size_.at(head) = n_h;
	  extracted_list_.push_front(fidx_h);
	} 
      }

      const int mod = r.mod();
      assert(mod >= 0);
      if (cache_token_.at(mod) == NULL){
	int n_m = 0;
	FIdx* fidx_m = inner_extractor_.extract_token(symbols, x, mod, &n_m);
	assert(mod < static_cast<int>(cache_token_.size()));
	assert(mod < static_cast<int>(cache_token_size_.size()));
	cache_token_.at(mod) = fidx_m;
	cache_token_size_.at(mod) = n_m;
	extracted_list_.push_front(fidx_m);
      }
    }

    void RetrieveFromCache (
			    const R& r,
			    FeatureVector<FIdx>* f_h,
			    FeatureVector<FIdx>* f_m,
			    FeatureVector<FIdx>* f_dep
			    ) const {
      const int head = r.head();
      const int mod = r.mod();
      assert(r.label() == 0); // only have feats for label 0

      f_dep->idx = cache_map_dep_.operator()(r);
      f_dep->n = cache_map_dep_size_.operator()(r);

      if (head == -1){
	f_h->idx = cache_token_root_;
	f_h->n = cache_token_root_size_;
      } else{
	f_h->idx = cache_token_.at(head);
	f_h->n = cache_token_size_.at(head);
      }
      f_m->idx = cache_token_.at(mod);
      f_m->n = cache_token_size_.at(mod);
    }

    void discard(const FeatureVector<FIdx>* const f) const {
      //do nothing, only discard feats at the end of the sentence
      //when the destructor is called
    }
  };



  /** This class is parameterized by the feature extractor and has
      a map<FeatureVector<FIdx> to cache features */
  template <typename X, typename R, typename FIdx>
  class Dep1InnerCache {
  public:
    typedef typename DepFeaturesV1Extractor<X,R,FIdx>::ExtractFunctor FunctorType;

    Dep1InnerCache(): current_sentence_(NULL), started_(false) {
      //clean the cache
    }

    ~Dep1InnerCache(){
      //clean the cache, features an in list, automatically cleaned
      started_ = false;
    }

    void init(const X& x, const DepSymbols& symbols)  {
      assert(!started_);
      const int num_words = x.size();
      const bool use_pos = true;

      current_sentence_ = &x;

      //individually extract and fill the set of features
      cache_token_.resize(num_words);
      //cache_token_root is of size 1, single Extractor
      cache_token_context_.resize(num_words);
      //cache_token_context_root is of size 1, single Extractor
      cache_dep_.resize(num_words*num_words);
      cache_dep_root_.resize(num_words);

      //first extract token feats
      //root
      FTemplatesDep1V1::extract_token_root<FIdx, X, R, FunctorType>(symbols, x, -1, use_pos, cache_token_root_);
      for (int i = 0; i < num_words; ++i){
	FTemplatesDep1V1::extract_token_std<FIdx, X, R, FunctorType>(symbols, x, i, use_pos, cache_token_.at(i));  
      }

      //token context feats

      FTemplatesDep1V1::extract_token_context_cached<FIdx, X, R, FunctorType>(symbols, x, -1, use_pos, 
									      cache_token_, cache_token_context_root_);

      for (int i = 0; i < num_words; ++i){
	FTemplatesDep1V1::extract_token_context_cached<FIdx, X, R, FunctorType>(symbols, x, i, use_pos, 
										cache_token_, cache_token_context_.at(i) );
      }

      //token dep feats
      for (int e = 1; e < num_words; ++e){
	for (int s = 0; s < e; ++s){ //extract feats for e=s?
	  assert(s <= e);
	  //get the cache
	  FunctorType &part_cache_se = cache_dep_.at(s*num_words + e);
	  FunctorType &part_cache_es = cache_dep_.at(e*num_words + s);
	  FTemplatesDep1V1::extract_dep_se<FIdx, X, R, FunctorType>
	    (symbols, x, s, e, use_pos, part_cache_se, part_cache_es);
	}
      }
      //extract deps considering the head is the root = -1
      for (int m = 0; m < num_words; ++m){
	//build the part 
	//R part(-1,m);
	//get the cache
	FunctorType dummy;
	FTemplatesDep1V1::extract_dep_se<FIdx, X, R, FunctorType>
	  (symbols, x, -1, m, use_pos, cache_dep_root_.at(m), dummy );
      }
      
      started_ = true;
    }
    

    FIdx* extract_dep(const DepSymbols& symbols, const X& x, 
		      const R& r, 
		      int* n) {

      assert(started_);

      if (&x != current_sentence_){
	current_sentence_ = &x;
	assert(false);
      }
      const int mod = r.mod();
      const int head = r.head();
      assert(head != mod) ; //map uses this position as root
      const int num_words = x.size();
      assert(mod <= num_words);
      assert(mod >= 0);
      //get and concat the features from the cache tables  
      if (r.head() == -1){
	//get token, token context and dep for root
	*n = cache_dep_root_.at(mod).size();
	return cache_dep_root_.at(mod).create_array();

      }
      else {
	*n = cache_dep_.at(head*num_words + mod).size();
	return cache_dep_.at(head*num_words + mod).create_array();
      }

      /*
	DepFeaturesV1Extractor<X,R,FIdx>::extract(symbols, x, r, f);

	ExtractFunctor F; 
	FTemplatesDep1V1::extract<FIdx>(symbols, x, r, F);
	f->idx = F.create_array(); 
	f->n = F.size();
      */
    }

    FIdx* extract_token(const DepSymbols& symbols, const X& x, 
			const int token,
			int* n
			) {

      assert(started_);

      if (&x != current_sentence_){
	current_sentence_ = &x;
	assert(false);
      }
      assert(token >= -1);
      //get and concat the features from the cache tables  
      if (token == -1){
	//get token, token context and dep for root
	return FunctorType::join_and_create_array(cache_token_root_,
						  cache_token_context_root_, n);
      }
      else {
	return FunctorType::join_and_create_array(cache_token_.at(token), 
						  cache_token_context_.at(token), n);
      }
    }



    void discard(const FeatureVector<FIdx>* const f, const X& x, 
		 const R& r) const {
      //do nothing, only discard feats at the end of the sentence
      //when the destructor is called
    }

  private:
    /** the caches */
    vector<FunctorType> cache_token_;
    FunctorType cache_token_root_;
    vector<FunctorType> cache_token_context_;
    FunctorType cache_token_context_root_;
    vector<FunctorType> cache_dep_;
    vector<FunctorType> cache_dep_root_;

    /** the current sentence */
    const X* current_sentence_;
    /** a flag to check if it is correctly initialized */
    bool started_;
  };


  template <typename X, typename R, typename FIdx>
  class DepFeaturesV1Extractor {
  public:   

    struct ExtractFunctor {
    public:
      std::list<FIdx> idxlist;

      void operator()(FIdx& i) {
	idxlist.push_back(i);
      }

      void add_all(const ExtractFunctor& f){
	for (auto it = f.idxlist.begin(); it != f.idxlist.end(); ++it){
	  this->operator()(*it);
	}
      }

      typename std::list<FIdx>::const_iterator begin() const{
	return idxlist.begin();
      }

      typename std::list<FIdx>::const_iterator end() const {
	return idxlist.end();
      }

      typename std::list<FIdx>::iterator begin(){
	return idxlist.begin();
      }

      typename std::list<FIdx>::iterator end(){
	return idxlist.end();
      }

      size_t size() const { return idxlist.size(); }

      FIdx* create_array() const {
	FIdx* idx = new FIdx[idxlist.size()];
	FIdx* d = idx;
	for (auto s = idxlist.begin(); s!=idxlist.end(); ++s, ++d) {
	  *d = *s;
	}
	return idx; 
      }

      static FIdx* join_and_create_array(const ExtractFunctor& e1, const
					 ExtractFunctor& e2, 
					 int *size
					 ){
	//token feats head are 1 and 2 token ad token context

	*size = e1.size() + e2.size();

	FIdx* idx = new FIdx[*size];
	FIdx* d = idx;
	for (auto s = e1.idxlist.begin(); s!=e1.idxlist.end(); ++s, ++d) {
	  *d = *s;
	}
	for (auto s = e2.idxlist.begin(); s!=e2.idxlist.end(); ++s, ++d) {
	  *d = *s;
	}
	return idx;
      }
    };



    static void extract(const DepSymbols& symbols, const X& x, const R& r, FeatureVector<FIdx>* f) {
      ExtractFunctor F; 
      FTemplatesDep1V1::extract<FIdx>(symbols, x, r, F);
      f->idx = F.create_array(); 
      f->n = F.size();
      return;

    }
  };

}




#endif /* DEP_FGENDEP_V1_H */
