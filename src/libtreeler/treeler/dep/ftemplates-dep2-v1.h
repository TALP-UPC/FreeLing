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
 * \file   ftemplates-dep2-v1.h
 * \brief  Feature templates for dependency parsing
 * \author 
 */

#ifndef DEP_FTEMPLATES_DEP2_V1_H
#define DEP_FTEMPLATES_DEP2_V1_H

#include <string>
#include "treeler/base/fidx.h"
#include "treeler/dep/dep-symbols.h"
#include "treeler/dep/fgen-ftemplate-types.h"

namespace treeler {

  class FTemplatesDep2V1 {
  public:
    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract(const DepSymbols& symbols, const X& x, const R& r, Functor& F) {
        //if root, extract root
        const int head = r.head();
        const int mod = r.mod();
        const int child = r.child();

        const bool use_pos = true;
        const int num_words = x.size();

        assert(r.head() >= -1);
        assert(mod >= 0);
        assert(child >= 0);
        assert(head < num_words);
        assert(mod < num_words);
        assert(child < num_words);

        typename FIdx::Word head_word = (head >= 0)? 
        FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(x.get_token(head).word())):
        FIdx::rootWord();

        typename FIdx::Tag head_cpos = (head >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head).coarse_pos())):
          FIdx::rootTag();


        typename FIdx::Word mod_word = 
          FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(x.get_token(mod).word()));
        typename FIdx::Tag mod_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod).coarse_pos()));

        typename FIdx::Word child_word =  (mod == child or child < 0)?
        // means that there is no child, with two possibilities :
      // - mod is the first child of the head
      // - mod has no child to the left or to the right
      /* special null tokens instead of -1 */
          FIdx::rootWord() : 
          FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(x.get_token(mod).word()));
        typename FIdx::Tag child_cpos = (mod == child or child < 0)?
          FIdx::rootTag() :
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod).coarse_pos()));

        assert(use_trigrams);
        assert(use_pos);


        FIdx idx;      
        idx << child_word << head_word << FIdx::code(FG_O2_CW_HW, "FG_O2_CW_HW");
        F(idx); 
        idx.clear();

        idx << child_word << head_word << FIdx::code(FG_O2_CW_MW_HW, "FG_O2_CW_MW_HW");
        F(idx); 
        idx.clear();

        idx << child_word << head_cpos << FIdx::code(FG_O2_HT_CW, "FG_O2_HT_CW");
        F(idx); 
        idx.clear();

        idx << child_word << mod_cpos << FIdx::code(FG_O2_MT_CW, "FG_O2_MT_CW");
        F(idx); 
        idx.clear();

        idx << child_tag << head_word << FIdx::code(FG_O2_HW_CT, "FG_O2_HW_CT");
        F(idx); 
        idx.clear();

        idx << child_tag << mod_word << FIdx::code(FG_O2_MW_CT, "FG_O2_MW_CT");
        F(idx); 
        idx.clear();
      
        idx << child_tag << mod_tag << FIdx::code(FG_O2_MT_CT, "FG_O2_MT_CT");
        F(idx); 
        idx.clear();

        idx << child_tag << head_tag << FIdx::code(FG_O2_HT_CT, "FG_O2_HT_CT");
        F(idx); 
        idx.clear();

        idx << child_tag << head_tag << mod_tag << FIdx::code(FG_O2_HT_MT_CT, "FG_O2_HT_MT_CT");
        F(idx); 
        idx.clear();
      }
  };

}



#endif /* DEP_FGENDEP_V1_H */
