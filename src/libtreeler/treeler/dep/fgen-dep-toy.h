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
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   fgen-dep-toy.h
 * \brief  Declaration of FGenDepToy
 * \author 
 */

#ifndef DEP_FGENDEPTOY_H
#define DEP_FGENDEPTOY_H

#include <string>
#include "treeler/base/feature-vector.h"
#include "treeler/dep/part-dep1.h"

namespace treeler {

  /** 
      \brief A structure of features for an input pattern 
  */
  template <typename X, typename R, typename FIdx>
  class DepFeaturesToy;

  /** 
      \brief This class provides a static method to extract features that depends on FIdx
  */
  template <typename X, typename R, typename FIdx>
  class DepFeaturesToyExtractor;

  /** 
      \brief A "Toy" Feature Generator for dependency parsing

      It is templated on: 
      - X : the type of input structure (i.e. a sentence)
      - R : the type of parts (i.e. a dependency edge)
      - FIdx : the type of feature indices it extracts

      Bla bla ...
  */
  template <typename X, typename R, typename FIdx_par>
  class FGenDepToy { 
  public:
    typedef FIdx_par FIdx;
    struct Configuration {};

    FGenDepToy(int l) : 
      _L(l), _spaces(l)
      {}
    ~FGenDepToy() {};
    
    int spaces() const { return _spaces; };

    // a feature structure for an X instance
    typedef DepFeaturesToy<X,R,FIdx> Features;
    
    // creates features for x in f
    void features(const X& x, Features& f) const; 

  private:
    //! Number of dependency labels
    int _L;
    //! Number of parameter spaces
    int _spaces;
  };


  template <typename X, typename R, typename FIdx>
  void FGenDepToy<X,R,FIdx>::features(const X& x, Features& f) const {
    assert(f._x==NULL);
    f._x = &x;
  }
  
  // the features for all parts of a pattern
  template <typename X, typename R, typename FIdx>
  class DepFeaturesToy {
  public:
    const X* _x; // a pointer to the pattern 

    // creation and destruction
    DepFeaturesToy():  _x(NULL) {} 
    ~DepFeaturesToy() { clear(); }

    // clears the feature structure
    void clear();
    
    //! Returns a feature vector for input x and part r.
    const FeatureVector<FIdx>* phi(const X& x, const R& r); 

    FIdx* extract(const X& x, const R& r) const; 
    
    void discard(const FeatureVector<FIdx>* f, const X& x, const R& r) const; 
  };

  template <typename X, typename R, typename FIdx>
  void DepFeaturesToy<X,R,FIdx>::clear() {
    _x = NULL;
  }

  template <typename X, typename R, typename FIdx>
  const FeatureVector<FIdx>* DepFeaturesToy<X,R,FIdx>::phi(const X& x, const R& r) {
    FeatureVector<FIdx> *f = new FeatureVector<FIdx>;    
    DepFeaturesToyExtractor<X,R,FIdx>::extract(x, r, f);
    f->val = NULL;
    f->next = NULL;
    f->offset = r.label();
    return f;
  }

  template <typename X, typename R, typename FIdx>
  void DepFeaturesToy<X,R,FIdx>::discard(const FeatureVector<FIdx>* const f, const X& x, const R& r) const {
    // recurse if necessary
    if (f->next!=NULL) {
      discard(f->next, x, r);
    }
    delete [] f->idx; 
    delete f;
  }

  template <typename X, typename R, typename FIdx>
  class DepFeaturesToyExtractor {
    public:
    static void extract(const X& x, const R& r, FeatureVector<FIdx>* f) {
      typename FIdx::Code c;
      typename FIdx::Dir dir = FIdx::dir(r.head() < r.mod());
      typename FIdx::Tag hp, mp;
      typename FIdx::Word hw, mw;
      if (r.head() == -1) {
	hp = FIdx::rootTag();
	hw = FIdx::rootWord();
      }
      else {
	const typename X::Token& htok = x.get_token(r.head());
	hp = FIdx::tag(htok.fine_tag());
	hw = FIdx::word(htok.word());
      }
      const typename X::Token& mtok = x.get_token(r.mod());
      mp = FIdx::tag(mtok.fine_tag());
      mw = FIdx::word(mtok.word());
      
      int N = 2;
      FIdx* idx = new FIdx[N];
      c = FIdx::code(1, "hpmp");
      idx[0] << mp << hp << dir << c;
      c = FIdx::code(2, "hwmw");
      idx[1] << mw << hw << dir << c;
      
      f->n = N; 
      f->idx = idx; 
    }
  };

}



#endif /* DEP_FGENDEPTOY_H */
