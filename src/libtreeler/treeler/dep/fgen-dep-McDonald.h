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
 * \file   fgen-dep-McDonald.h
 * \brief  Declaration of FGenDepMcDonald
 * \author 
 */

#ifndef DEP_FGENDEPMCDONALD_H
#define DEP_FGENDEPMCDONALD_H

#include <string>
#include "treeler/base/feature-vector.h"
#include "treeler/dep/part-dep1.h"

namespace treeler {

  /** 
      \brief A structure of features for an input pattern 
  */
  template <typename X, typename R, typename FIdx>
  class DepFeaturesMcDonald;

  /** 
      \brief This class provides a static method to extract features that depends on FIdx
  */
  template <typename X, typename R, typename FIdx>
  class DepFeaturesMcDonaldExtractor;

  /** 
      \brief A "McDonald" Feature Generator for dependency parsing

      It is templated on: 
      - X : the type of input structure (i.e. a sentence)
      - R : the type of parts (i.e. a dependency edge)
      - FIdx : the type of feature indices it extracts

      Bla bla ...
  */
  template <typename X, typename R, typename FIdx_par>
  class FGenDepMcDonald { 
  public:
    typedef FIdx_par FIdx;
    struct Configuration {};

    FGenDepMcDonald(int l) : 
      _L(l), _spaces(l)
      {}
    ~FGenDepMcDonald() {};
    
    int spaces() const { return _spaces; };

    // a feature structure for an X instance
    typedef DepFeaturesMcDonald<X,R,FIdx> Features;
    
    // creates features for x in f
    void features(const X& x, Features& f) const; 

  private:
    //! Number of dependency labels
    int _L;
    //! Number of parameter spaces
    int _spaces;
  };


  template <typename X, typename R, typename FIdx>
  void FGenDepMcDonald<X,R,FIdx>::features(const X& x, Features& f) const {
    assert(f._x==NULL);
    f._x = &x;
  }
  
  // the features for all parts of a pattern
  template <typename X, typename R, typename FIdx>
  class DepFeaturesMcDonald {
  public:
    const X* _x; // a pointer to the pattern 

    // creation and destruction
    DepFeaturesMcDonald():  _x(NULL) {} 
    ~DepFeaturesMcDonald() { clear(); }

    // clears the feature structure
    void clear();
    
    //! Returns a feature vector for input x and part r.
    const FeatureVector<FIdx>* phi(const X& x, const R& r); 

    FIdx* extract(const X& x, const R& r) const; 
    
    void discard(const FeatureVector<FIdx>* f, const X& x, const R& r) const; 
  };

  template <typename X, typename R, typename FIdx>
  void DepFeaturesMcDonald<X,R,FIdx>::clear() {
    _x = NULL;
  }

  template <typename X, typename R, typename FIdx>
  const FeatureVector<FIdx>* DepFeaturesMcDonald<X,R,FIdx>::phi(const X& x, const R& r) {
    FeatureVector<FIdx> *f = new FeatureVector<FIdx>;    
    DepFeaturesMcDonaldExtractor<X,R,FIdx>::extract(x, r, f);
    f->val = NULL;
    f->next = NULL;
    f->offset = r.label();
    return f;
  }

  template <typename X, typename R, typename FIdx>
  void DepFeaturesMcDonald<X,R,FIdx>::discard(const FeatureVector<FIdx>* const f, const X& x, const R& r) const {
    // recurse if necessary
    if (f->next!=NULL) {
      discard(f->next, x, r);
    }
    delete [] f->idx; 
    delete f;
  }

  template <typename X, typename R, typename FIdx>
  class DepFeaturesMcDonaldExtractor {
    public:
    static void extract(const X& x, const R& r, FeatureVector<FIdx>* f) {
      typename FIdx::Tag hp[3], mp[3]; //0 -> left of parent; 1 -> parent; 2 -> right of parent
      typename FIdx::Word hw = FIdx::rootWord(), mw = FIdx::rootWord(); //inicialitzo per evitar warning
      
      for (int i = 0 ; i < 3; i++) {
	int hi = r.head()+i-1;
	if (hi < 0 || hi >= x.size()) {
	  hp[i] = FIdx::rootTag();
// 	  if (i == 1) hw = FIdx::rootWord();
	}
	else {
	  const typename X::Token& htok = x.get_token(hi);
	  hp[i] = FIdx::tag(htok.fine_tag());
	  if (i == 1) hw = FIdx::word(htok.word());
	}
	int mi = r.mod()+i-1;
	if (mi < 0 || mi >= x.size()) {
	  mp[i] = FIdx::rootTag();
// 	  if (i == 1) mw = FIdx::rootWord();
	}
	else {
	  const typename X::Token& mtok = x.get_token(mi);
	  mp[i] = FIdx::tag(mtok.fine_tag());
	  if (i == 1) mw = FIdx::word(mtok.word());
	}
      }
      int dist = r.head()<r.mod() ? r.mod()-r.head()-1 : r.head()-r.mod()-1;
      int N = 17 + dist;
      FIdx* idx = new FIdx[N];
      idx[0] << hp[1] << hw << FIdx::code(FG_DEP_HW_HT, "hw_ht");
      idx[1] << hw << FIdx::code(FG_DEP_HW, "hw");
      idx[2] << hp[1] << FIdx::code(FG_DEP_HT, "ht");
      idx[3] << mp[1] << mw << FIdx::code(FG_DEP_MW_MT, "mw_mt");
      idx[4] << mw << FIdx::code(FG_DEP_MW, "mw");
      idx[5] << mp[1] << FIdx::code(FG_DEP_MT, "mt");
      idx[6] << mp[1] << mw << hp[1] << hw << FIdx::code(FG_DEP_HW_HT_MW_MT, "hw_ht_mw_mt");
      idx[7] << mp[1] << mw << hp[1] << FIdx::code(FG_DEP_HT_MW_MT, "ht_mw_mt");
      idx[8] << mp[1] << mw << hw << FIdx::code(FG_DEP_HW_MW_MT, "hw_mw_mt");
      idx[9] << mp[1] << hp[1] << hw << FIdx::code(FG_DEP_HW_HT_MT, "hw_ht_mt");
      idx[10] << mw << hp[1] << hw << FIdx::code(FG_DEP_HW_HT_MW, "hw_ht_mw");
      idx[11] << mw << hw << FIdx::code(FG_DEP_HW_MW, "hw_mw");
      idx[12] << mp[1] << hp[1] << FIdx::code(FG_DEP_HT_MT, "ht_mt");
      int i = 13;
      for (int j = 0; j < dist; j++, i++) {
	const typename X::Token& btok = x.get_token(min(r.mod(), r.head())+j+1);
	idx[i] << mp[1] << FIdx::tag(btok.fine_tag()) << hp[1] << FIdx::code(FG_DEP_BETW, "ht_bt_mt");
      }
      idx[i] << mp[1] << mp[0] << hp[2] << hp[1] << FIdx::code(FG_DEP_SURR_HN_MP, "ht_ht+1_mt-1_mt");
      idx[i+1] << mp[1] << mp[0] << hp[1] << hp[0] << FIdx::code(FG_DEP_SURR_HP_MP, "ht-1_ht_mt-1_mt");
      idx[i+2] << mp[2] << mp[1] << hp[2] << hp[1] << FIdx::code(FG_DEP_SURR_HN_MN, "ht_ht+1_mt_mt+1");
      idx[i+3] << mp[2] << mp[1] << hp[1] << hp[0] << FIdx::code(FG_DEP_SURR_HP_MN, "ht-1_ht_mt_mt+1");
      
      f->n = N; 
      f->idx = idx; 
    }
  };

}



#endif /* DEP_FGENDEPMCDONALD_H */
