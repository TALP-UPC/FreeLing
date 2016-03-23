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
 * \file   fgen-ftemplates-dep2.h
 * \brief  Definition of FGenFTemplatesDep2
 * \author Xavier Carreras
 */


#ifndef DEP_FGENFTEMPLATESDEP2_H
#define DEP_FGENFTEMPLATESDEP2_H

#include <string>
#include <list>

#include "treeler/base/feature-idx-v0.h"
#include "treeler/dep/fgen-ftemplate-types.h"


namespace treeler {

  /** 
   * \brief Static feature-extraction templates for second-order dependencies
   *
   * \author Xavier Carreras, Terry Koo
   */
  class FGenFTemplatesDep2 {
  public:    
    
    template <typename X>
    static void phi_dep2(const X& x, 
			 const int head, const int mod, const int child, 
			 const int word_limit, bool use_pos, bool use_trigrams, 
			 std::list<FeatureIdx>& f);
  };
  
  
  
  template <typename X>
  void FGenFTemplatesDep2::phi_dep2(const X& x,
				    const int head, const int mod, const int child,
				    const int word_limit, bool use_pos, bool use_trigrams, 
				    std::list<FeatureIdx>& f) {
    // dependency direction: is the modifier before the head in the sentence?
    const int mod_before = (mod < head ? 1 : 0);
    
    // get head info, possibly being the ROOT
    int hword, hctag;
    
    // this is the ROOT dependency
    if(head < 0) {
      /* special null tokens instead of -1 */
      hword = feature_idx_nullw;
      hctag = feature_idx_nullp;
    }
    else {
      const typename X::Token& ht = x.get_token(head);
      //      hword = (ht.word() < word_limit ? ht.word() : word_limit);
      hword = (ht.word() < word_limit ? ht.word() : -1);
      hctag = ht.coarse_pos();
    }
    
    // the modifier is always a real token
    const typename X::Token& mt = x.get_token(mod);
    //    const int mword = (mt.word() < word_limit ? mt.word() : word_limit);
    const int mword = (mt.word() < word_limit ? mt.word() : -1);
    const int mctag = mt.coarse_pos();
    
    int cword, cctag;
    if (mod==child or child<0) {
      // means that there is no child, with two possibilities :
      // - mod is the first child of the head
      // - mod has no child to the left or to the right
      /* special null tokens instead of -1 */
      cword = feature_idx_nullw;
      cctag = feature_idx_nullp;
    }
    else {
      const int cw = x.get_token(child).word();
      //      cword = (cw < word_limit ? cw : word_limit);
      cword = (cw < word_limit ? cw : -1);
      cctag = x.get_token(child).coarse_pos();
    }
    
    /* this implementation attempts to reuse partially-constructed
       feature_idxs instead of creating each one from scratch */
#define addfeat(type, ft) {					     \
      f.push_back(feature_idx_catt(feature_idx_catd(ft, mod_before), \
				   FG_O2_ ## type));		     \
    }
    
    
    const FeatureIdx ft_cw = (cword == -1 ? 0 : feature_idx_catw(feature_idx_zero, cword));
    if(cword != -1) {
      if(hword != -1) {
	if (use_trigrams and mword!=-1) {
	  const FeatureIdx ft_cw_hw = feature_idx_catw(ft_cw, hword);
	  addfeat(HW_CW, ft_cw_hw);
	  addfeat(HW_MW_CW, feature_idx_catw(ft_cw_hw, mword));
	}
	else {
	  addfeat(HW_CW, feature_idx_catw(ft_cw, hword));
	}
      }
      if(mword != -1) { addfeat(MW_CW, feature_idx_catw(ft_cw, mword)); }
    }
    
    if(use_pos) {
      if(cword != -1) {
	addfeat(HT_CW, feature_idx_catp(ft_cw, hctag));
	addfeat(MT_CW, feature_idx_catp(ft_cw, mctag));
      }
      
      const FeatureIdx ft_ct = feature_idx_catp(feature_idx_zero, cctag);
      if(hword != -1) { addfeat(HW_CT, feature_idx_catw(ft_ct, hword)); }
      if(mword != -1) { addfeat(MW_CT, feature_idx_catw(ft_ct, mword)); }
      
      addfeat(MT_CT, feature_idx_catp(ft_ct, mctag));
      const FeatureIdx ft_ct_ht = feature_idx_catp(ft_ct, hctag);
      addfeat(HT_CT, ft_ct_ht);
      addfeat(HT_MT_CT, feature_idx_catp(ft_ct_ht, mctag));
    }
  }
  
}
#undef addfeat

#endif
