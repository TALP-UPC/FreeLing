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
 * \file   fgen-dep.h
 * \brief  Declaration of FGenDepV05
 * \author Xavier Carreras
 */

#ifndef DEP_FGENDEP_V05_H
#define DEP_FGENDEP_V05_H

#include <string>

#include "treeler/base/feature-vector.h"
#include "treeler/base/basic-sentence.h"
#include "treeler/dep/dep-symbols.h"
#include "treeler/dep/part-dep1.h"
#include "treeler/dep/part-dep2.h"

/* subordinate feature generation functions */
#include "treeler/dep/fgen-basic-dep1.h"
#include "treeler/dep/fgen-basic-dep1-v0-5.h"
//#include "treeler/dep/fgen-basic-dep2.h"



namespace treeler {

//include previously fgen-basic-dep.h

/*  namespace __fgen_dep_internal {

    template <typename XX>
      struct CachedX {
        const XX& x; 
        CachedX(const DepSymbols& sym, const XX& xx) 
          : x(xx) {}
      };

    template <>
      struct CachedX<BasicSentence<string,string>> {
        BasicSentence<int,int> x;       
        CachedX(const DepSymbols& sym, const BasicSentence<string,string>& xx) 
        {
          sym.map(xx, x); 
        }
      };

  };*/

  // forward declaration 
  template <typename X, typename R>
    class DepFeaturesV05;

  /** 
    \brief Implements the FeatureGenerator interface for dependency parsing
    \author Xavier Carreras

    Computes features for first-order dependencies. Dependencies can
    be labeled via feature offsets.

    The features templates are based on standard work in
    discriminative dependency parsing.

   */
  template <typename X, typename R>
    class FGenDepV05 { 
      public:
        typedef FIdxV0 FIdx;

        struct Configuration {
          //! Number of dependency labels
          int L;

          // Configuration of 1st order feature vectors 
          FGenBasicDep1v05::Configuration config_dep1;

          // Configuration of 2nd order feature vectors 
          FGenBasicDep2::Configuration config_dep2;

          //! dictionary directory 
          std::string dictdir;
          std::string posfile;
          std::string name;        

          Configuration() 
            : L(1), dictdir(""), posfile(""), name("") {}
        };

      public:
        DepSymbols& symbols;
        Configuration config; 

        // constructor, destructor

        FGenDepV05(DepSymbols& sym0)  
          : symbols(sym0)
        {}

        ~FGenDepV05() {};


        //! prints options of the module
        void usage(const char* const mesg = "");
        //! prints options of the module
        void process_options();

        int spaces() const { 
          int spaces = 3 * config.L; // head tok, mod tok, dep
          if (config.config_dep2.use_o2) {
            spaces += 3 * config.L; // sib, gc inwards, gc outwards
          }
          return spaces; 
        };

        typedef DepFeaturesV05<X,R> Features;

        // creates features for x in f
        void features(const X& x, Features& f) const; 



      private:

        //! computes full dimensionality of f(x,r)
        void compute_dim();

    };



  template <typename X, typename R>
    void FGenDepV05<X,R>::features(const X& x, Features& f) const {
      assert(f._x==NULL);
      assert(f._cache_token==NULL);
      assert(f._cache_dep1==NULL);
      assert(f._cache_dep2==NULL);
      f._x = new __fgen_dep_internal::CachedX<X>(symbols,x);
      f._config_dep1 = config.config_dep1; 
      f._config_dep2 = config.config_dep2;

      FGenBasicDep1v05::create_cache(symbols, f._config_dep1, (*f._x).x, f._cache_token, f._cache_dep1);
      if (f._config_dep2.use_o2) {
        FGenBasicDep2::create_cache(symbols, f._config_dep2, (*f._x).x, f._cache_dep2);
      }        
    }



  // the features for all parts of a pattern
  template <typename X, typename R>
    class DepFeaturesV05 {
      friend class FGenDepV05<X,R>;
      private:
      __fgen_dep_internal::CachedX<X>* _x;
      FGenBasicDep1v05::Configuration _config_dep1;
      FGenBasicDep2::Configuration _config_dep2;
      // fvec caches for part-wise feature generation 
      FeatureVector<> *_cache_token, *_cache_dep1, *_cache_dep2;      
      public:
      DepFeaturesV05():  _x(NULL), _cache_token(NULL), _cache_dep1(NULL), _cache_dep2(NULL) {}
      ~DepFeaturesV05() { clear(); }
      // clears the feature structure
      void clear();

      //! Returns a feature vector for input x and part r.
      //! The function is non-const because the fv may be cached for future use.
      const FeatureVector<>* phi(const X& x, const R& r); 

      void discard(const FeatureVector<>* f, const X& x, const R& r) const; 
    };

  template <typename X, typename R>
    void DepFeaturesV05<X,R>::clear() {
      FGenBasicDep1v05::destroy_cache((*_x).x, _cache_token, _cache_dep1);
      if (_cache_dep2 != NULL) {
        FGenBasicDep2::destroy_cache((*_x).x, _cache_dep2);
      }
      _x = NULL;
      _cache_token = _cache_dep1 = _cache_dep2 = NULL;
    }


  template <typename X, typename R>
    const FeatureVector<>* DepFeaturesV05<X,R>::phi(const X& x, const R& r) {
      assert(_cache_token!=NULL);
      assert(_cache_dep1!=NULL);

      const int NSPACES = 3;    
      FeatureVector<> *fv, *fvnext;
      fvnext = FGenBasicDep1v05::phi_token((*_x).x, r.mod(), _cache_token);
      fvnext->offset = NSPACES * r.label() + 2;
      fv = FGenBasicDep1v05::phi_token((*_x).x, r.head(), _cache_token);
      fv->offset = NSPACES * r.label() + 1;
      fv->next = fvnext;
      fvnext = fv;
      fv = FGenBasicDep1v05::phi_dependency((*_x).x, r.head(), r.mod(), _cache_dep1);
      fv->offset = NSPACES * r.label();
      fv->next = fvnext;
      return fv;
    }

  template <typename X, typename R>
    void DepFeaturesV05<X,R>::discard(const FeatureVector<>* const f, const X& x, const R& r) const {
      // recurse if necessary
      if (f->next!=NULL) {
        discard(f->next, x, r);
      }
      delete f;
    }


  /// SPECIALIZATION FOR PartDep2

  // the features for all parts of a pattern
  template <typename X>
    class DepFeaturesV05<X,PartDep2> {
      friend class FGenDepV05<X,PartDep2>;
      private:
      __fgen_dep_internal::CachedX<X>* _x;
      FGenBasicDep1v05::Configuration _config_dep1;
      FGenBasicDep2::Configuration _config_dep2;
      // fvec caches for part-wise feature generation 
      FeatureVector<> *_cache_token, *_cache_dep1, *_cache_dep2;      
      public:
      DepFeaturesV05():  _x(NULL), _cache_token(NULL), _cache_dep1(NULL), _cache_dep2(NULL) {}
      ~DepFeaturesV05() { clear(); }
      // clears the feature structure
      void clear();

      //! Returns a feature vector for input x and part r.
      //! The function is non-const because the fv may be cached for future use.
      const FeatureVector<>* phi(const X& x, const PartDep2& r); 

      void discard(const FeatureVector<>* f, const X& x, const PartDep2& r) const; 
    };

  template <typename X>
    const FeatureVector<>* DepFeaturesV05<X,PartDep2>::phi(const X& x, const PartDep2& r) {
      assert(_cache_token!=NULL);
      assert(_cache_dep1!=NULL);

      int NSPACES = 3;
      if (_cache_dep2!=NULL) {
        NSPACES = 6;
      }
      if (r.type == PartDep2::FO) {
        FeatureVector<> *fv, *fvnext;
        fvnext = FGenBasicDep1v05::phi_token(x, r.mod(), _cache_token);
        fvnext->offset = NSPACES * r.label() + 2;
        fv = FGenBasicDep1v05::phi_token(x, r.head(), _cache_token);
        fv->offset = NSPACES * r.label() + 1;
        fv->next = fvnext;
        fvnext = fv;
        fv = FGenBasicDep1v05::phi_dependency(x, r.head(), r.mod(), _cache_dep1);
        fv->offset = NSPACES * r.label();
        fv->next = fvnext;
        return fv;
      }
      else {
        if (_cache_dep2!=NULL) {
          FeatureVector<> *fv;
          fv = FGenBasicDep2::phi_dep2(_config_dep2, (*_x).x, r.head(), r.mod(), r.child(), _cache_dep2);
          if (r.type == PartDep2::SIB)      fv->offset = NSPACES * r.label() + 3;
          else if (r.type == PartDep2::CMI) fv->offset = NSPACES * r.label() + 4;
          else if (r.type == PartDep2::CMO) fv->offset = NSPACES * r.label() + 5;
          else assert(0);
          fv->next = NULL;
          return fv;
        }
        else {
          FeatureVector<>* fv = new FeatureVector<>;
          fv->idx = NULL;
          fv->val = NULL;
          fv->n = 0;
          fv->offset = -1;
          fv->next = NULL;
          return fv;
        }
      }
    }

  template <typename X>
    void DepFeaturesV05<X,PartDep2>::clear() {
      FGenBasicDep1v05::destroy_cache((*_x).x, _cache_token, _cache_dep1);
      if (_cache_dep2 != NULL) {
        FGenBasicDep2::destroy_cache((*_x).x, _cache_dep2);
      }
      _x = NULL;
      _cache_token = _cache_dep1 = _cache_dep2 = NULL;
    }

  template <typename X>
    void DepFeaturesV05<X,PartDep2>::discard(const FeatureVector<>* const f, const X& x, const PartDep2& r) const {
      // recurse if necessary
      if (f->next!=NULL) {
        discard(f->next, x, r);
      }
      delete f;
    }

}


#endif /* DEP_FGENDEP_H */
