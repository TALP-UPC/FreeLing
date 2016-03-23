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
 * \file   ftemplates-dep1-v1.h
 * \brief  Feature templates for dependency parsing
 * \author 
 */

#ifndef DEP_FTEMPLATES_DEP1_V1_H
#define DEP_FTEMPLATES_DEP1_V1_H

#include <string>
#include "treeler/base/fidx.h"
#include "treeler/dep/dep-symbols.h"
#include "treeler/dep/fgen-ftemplate-types.h"

namespace treeler {

  class FTemplatesDep1V1 {
  public:

    struct Configuration {
      bool use_pos; 

    };

    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract(const DepSymbols& symbols, const X& x, const R& r, Functor& F) {
      //if root, extract root
      //call to the old baseline implementation
      extract_dep_mcdo<FIdx, X, R, Functor>(symbols, x, r, F);      
    }
    
    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_dep(const DepSymbols& symbols, const X& x, const R& r, 
			    const bool use_pos, Functor& F) {
      assert(false); //function to remove
      //phi_dependency
      //phi_dependency_context
      //phi_dependency_distance
      //phi_dependency_between
      assert(r.head() >= -1);
      assert(r.mod() != -1);
      extract_dep_std<FIdx, X, R, Functor>(symbols, x, r, F);
      extract_dep_ctx<FIdx, X, R, Functor>(symbols, x, r, F);
      extract_dep_distance<FIdx, X, R, Functor>(symbols, x, r, use_pos, F);
      extract_dep_between<FIdx, X, R, Functor>(symbols, x, r, F);
      
      //dependency context may use cache
      //dependency dist and between may be extracted at once
    }

    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_dep_se(const DepSymbols& symbols, const X& x, const int s, const int e, 
			       const bool use_pos, Functor& F_se, Functor& F_es) {
      //phi_dependency
      //phi_dependency_context
      //phi_dependency_distance
      //phi_dependency_between
      assert(s >= -1);
      assert(e >= 0);
      extract_dep_std_se<FIdx, X, R, Functor>(symbols, x, s, e, F_se, F_es);
      extract_dep_ctx_se<FIdx, X, R, Functor>(symbols, x, s, e, F_se, F_es);
      extract_dep_distance_se<FIdx, X, R, Functor>(symbols, x, s, e, use_pos, F_se, F_es);
      extract_dep_between_se<FIdx, X, R, Functor>(symbols, x, s, e, F_se, F_es);
      
      //dependency context may use cache
      //dependency dist and between may be extracted at once
    }
    

    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_token(const DepSymbols& symbols, const X& x, const int token, 
			      const bool use_pos, Functor& F) {
      assert(false); // function to remove
      //phi_token
      //phi_root_token
      //phi_token_context
      
      //std feats
      const int num_words = x.size();
      assert(token <= num_words);
      assert(token >= -1);
      
      if (token == -1){
	extract_token_root<FIdx, X, R, Functor>(symbols, x, token, use_pos, F);
      } else {
	extract_token_std<FIdx, X, R, Functor>(symbols, x, token, use_pos, F);
      }
      //token context
      extract_token_context<FIdx, X, R, Functor>(symbols, x, token, use_pos, F);
    }
    
    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_token_std(const DepSymbols& symbols, const X& x, const int i,
				  const bool use_pos, Functor& F) {
      
      //this function is not to extract feats from root
      const int num_words = x.size();
      assert(i>= 0 and i < num_words);
      
      //an idx to fill
      FIdx idx;      
      
      /* the token to be inspected */
      const typename X::Token& tok = x.get_token(i);
      //get token fields
      
      typename FIdx::Tag tok_cpos = 
	FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(tok.coarse_pos()));
      
      typename FIdx::Tag tok_fpos = 
	FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(tok.fine_pos()));
      
      typename FIdx::Word tok_word = 
	FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(tok.word()));
      
      typename FIdx::Word tok_lemma =
	FIdx::word(symbols.map_field<LEMMA,typename FIdx::Word>(tok.lemma()));
      
      
      //const int realword = tok.word();
      //const int w = ((realword < word_limit) ? realword : -1);
      
      //if(w != -1) {
      F(FIdx() << tok_word << FIdx::code(FG_TOK_WORD, "FG_TOK_WORD"));
      //}
      
      //if(tok.lemma() >= 0) {
      F(FIdx() << tok_lemma << FIdx::code(FG_TOK_LEMMA, "FG_TOK_LEMMA"));
      //}
      
      assert(use_pos);
      //add coarse tag
      F(FIdx() << tok_cpos << FIdx::code(FG_TOK_CTAG, "FG_TOK_CTAG"));
      
      //add fine tag
      F(FIdx() << tok_fpos << FIdx::code(FG_TOK_FTAG, "FG_TOK_FTAG"));
      
      //if (tok.fine_pos() >= 0){
      F(FIdx() << tok_cpos << tok_word << 
	FIdx::code(FG_TOK_WORD_CTAG, "FG_TOK_WORD_CTAG"));
      //}
      
    }


    template <typename FIdx, typename X, typename R, typename Functor>
    static void show_sentence(const DepSymbols& symbols, const X& x){
      /* the token to be inspected */
      const int num_words = x.size();
      for (int i = 0; i < num_words; ++i){
	cerr << x.get_token(i).word() << endl;
      }
    }

    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_token_root(const DepSymbols& symbols, const X& x, const int i, 
				   const bool use_pos, Functor& F) {
      
      //this function is not to extract feats from root
      assert(i == -1);
      
      //an idx to fill
      FIdx idx;      
      
      //get token fields
      
      typename FIdx::Tag tok_cpos = FIdx::rootTag();
      
      typename FIdx::Word tok_word = FIdx::rootWord();
      
      typename FIdx::Word tok_lemma = FIdx::rootWord();
      
      
      F(FIdx() << tok_word << FIdx::code(FG_TOK_WORD, "FG_TOK_WORD"));
      
      F(FIdx() << tok_lemma << FIdx::code(FG_TOK_LEMMA, "FG_TOK_LEMMA"));
      
      assert(use_pos);
      //add coarse tag
      F(FIdx() << tok_cpos << FIdx::code(FG_TOK_CTAG, "FG_TOK_CTAG"));
      
      F(FIdx() << tok_cpos << FIdx::code(FG_TOK_FTAG, "FG_TOK_FTAG"));
      
    }


    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_previous_or_next_token(const DepSymbols& symbols, const X& x, const int i,
					       const bool use_pos, const typename FIdx::Code& feat_code,
					       Functor& F) {
      assert(false); //function to remove (non cached)
      //extract the feats for the token i
      Functor prev_or_next_feats;
      const int num_words = x.size();
      assert(i>= 0 or i < num_words);
      extract_token_std<FIdx, X, R, Functor>(symbols, x, i, use_pos, prev_or_next_feats);
      
      for (auto it = prev_or_next_feats.begin(); it != prev_or_next_feats.end(); ++it){
	FIdx fidx = *it;
	//concat every feat with the feature code
	fidx << feat_code;
	//add to the main functor
	F(fidx);
      }
      
    }

    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_previous_or_next_token_cached(const DepSymbols& symbols, 
						      const X& x, const int i,
						      const bool use_pos, const typename FIdx::Code& feat_code,
						      const vector<Functor>& cached_token_feats,
						      Functor& F) {
      
      //extract the feats for the token i
      const int num_words = x.size();
      assert(i>= 0 or i < num_words);
      const Functor& prev_or_next_feats = cached_token_feats.at(i);
      
      for (auto it = prev_or_next_feats.begin(); 
	   it != prev_or_next_feats.end(); ++it){
	//copy
	FIdx fidx = *it;
	//concat every feat with the feature code
	fidx << feat_code;
	//add to the main functor
	F(fidx);
      }
      
    }


    //extact token context assuming caching
    template <typename FIdx, typename X, typename R, typename Functor>
    static void extract_token_context_cached(const DepSymbols& symbols, 
					     const X& x, const int i, const bool
					     use_pos, const vector<Functor>& cached_token_std, Functor& F) {
      
      const int num_words = x.size();
      //context size n = 2
      
      //an idx to fill
      FIdx idx;      
      
      
      //get token fields
      typename FIdx::Tag tok_cpos = (i >= 0)?
	FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(i).coarse_pos())):
	FIdx::rootTag();
      
      assert(i >= -1);
      assert(i < num_words);
      
      //features for the +1/-1 token
      if (i > 0){
	//....PREV1
	extract_previous_or_next_token_cached<FIdx, X, R, Functor>(symbols, x, i-1, use_pos,
								   FIdx::code(FG_TOK_PREV1, "FG_TOK_PREV1"), cached_token_std, F);
	
      } else {
	F(FIdx() << FIdx::tag(1)<< FIdx::code(FG_TOK_PREVNULL, "FG_TOK_PREVNULL"));
      }
      if (i < num_words -1){
	//...NEXT1
	extract_previous_or_next_token_cached<FIdx, X, R, Functor>(symbols, x, i+1, use_pos,
								   FIdx::code(FG_TOK_NEXT1, "FG_TOK_NEXT1"), cached_token_std, F);
	//assert(false);
      } else {
	F(FIdx() << FIdx::tag(1)<< FIdx::code(FG_TOK_NEXTNULL, "FG_TOK_NEXTNULL"));
      }
      

      //features for the +2/-2 token
      if (i >1){
	//...PREV2
	extract_previous_or_next_token_cached<FIdx, X, R, Functor>(symbols, x, i-2, use_pos,
								   FIdx::code(FG_TOK_PREV2, "FG_TOK_PREV2"), cached_token_std, F);
      } else {
	F(FIdx() << FIdx::tag(2)<< FIdx::code(FG_TOK_PREVNULL, "FG_TOK_PREVNULL"));
      }
      if (i < num_words -2){
	//...NEXT2
	extract_previous_or_next_token_cached<FIdx, X, R, Functor>(symbols, x, i+2, use_pos,
								   FIdx::code(FG_TOK_NEXT2, "FG_TOK_NEXT2"), cached_token_std, F);
      } else {
	F(FIdx() <<  FIdx::tag(2)<< FIdx::code(FG_TOK_NEXTNULL, "FG_TOK_NEXTNULL"));
      }
      

      //then insert bigram and trigram feats
      assert(use_pos);
      if (i > 0){
	
	typename FIdx::Tag tok_cpos_minus_1 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i - 1).coarse_pos()));
	
	F(FIdx() << tok_cpos_minus_1 << tok_cpos << 
	  FIdx::code(FG_TOK_PREV_CTAG_BIGRAM, "FG_TOK_PREV_CTAG_BIGRAM"));
      } else if (i == 0 ) {
	F(FIdx() <<  FIdx::rootTag() << tok_cpos <<
	  FIdx::code(FG_TOK_PREV_CTAG_BIGRAM, "FG_TOK_PREV_CTAG_BIGRAM"));
      }
      
      if (i < num_words - 1){
	
	typename FIdx::Tag tok_cpos_plus_1 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i + 1).coarse_pos()));
	
	F(FIdx() <<  tok_cpos_plus_1 << tok_cpos <<
	  FIdx::code(FG_TOK_NEXT_CTAG_BIGRAM, "FG_TOK_NEXT_CTAG_BIGRAM"));
      } else if ( i == num_words - 1){
	F(FIdx() <<  FIdx::rootTag() << tok_cpos <<
	  FIdx::code(FG_TOK_NEXT_CTAG_BIGRAM, "FG_TOK_NEXT_CTAG_BIGRAM"));
      }

        
      if (i > 1){
	typename FIdx::Tag tok_cpos_minus_1 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i - 1).coarse_pos()));
	
	typename FIdx::Tag tok_cpos_minus_2 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i - 2).coarse_pos()));
	
	F(FIdx() << tok_cpos_minus_2 << tok_cpos_minus_1 << tok_cpos <<
	  FIdx::code(FG_TOK_PREV_CTAG_TRIGRAM, "FG_TOK_PREV_CTAG_TRIGRAM"));
      } else if ( i == 1 ) {
	typename FIdx::Tag tok_cpos_minus_1 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i - 1).coarse_pos()));
	
	F(FIdx() << FIdx::rootTag() << tok_cpos_minus_1 << tok_cpos <<
	  FIdx::code(FG_TOK_PREV_CTAG_TRIGRAM, "FG_TOK_PREV_CTAG_TRIGRAM"));
      }
      
      if (i < num_words - 2){
	typename FIdx::Tag tok_cpos_plus_1 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i + 1).coarse_pos()));
	
	typename FIdx::Tag tok_cpos_plus_2 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i + 2).coarse_pos()));
	
	
	F(FIdx() << tok_cpos_plus_2 << tok_cpos_plus_1 <<  tok_cpos << 
	  FIdx::code(FG_TOK_NEXT_CTAG_TRIGRAM, "FG_TOK_NEXT_CTAG_TRIGRAM"));
      } else if (i == num_words - 2){
	typename FIdx::Tag tok_cpos_plus_1 = 
	  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
		    (x.get_token(i + 1).coarse_pos()));
	
	F(FIdx() << FIdx::rootTag() << tok_cpos_plus_1 << tok_cpos <<
          FIdx::code(FG_TOK_NEXT_CTAG_TRIGRAM, "FG_TOK_NEXT_CTAG_TRIGRAM"));
      }

      }



    //phi token context

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_token_context(const DepSymbols& symbols, const X& x, const int i, const bool
      use_pos, Functor& F) {
        assert(false); //function to remove (it is non-cached)

        const int num_words = x.size();
        //context size n = 2


        //an idx to fill
        FIdx idx;      


        //get token fields
        typename FIdx::Tag tok_cpos = (i >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(i).coarse_pos())):
          FIdx::rootTag();

        assert(i >= -1);
        assert(i < num_words);

        //features for the +1/-1 token
        if (i > 0){
          //....PREV1
          extract_previous_or_next_token<FIdx, X, R, Functor>(symbols, x, i-1, use_pos,
          FIdx::code(FG_TOK_PREV1, "FG_TOK_PREV1"), F);

        } else {
          idx << FIdx::tag(1)<< FIdx::code(FG_TOK_PREVNULL, "FG_TOK_PREVNULL");
          F(idx); 
          idx.clear();
        }
        if (i < num_words -1){
          //...NEXT1
          extract_previous_or_next_token<FIdx, X, R, Functor>(symbols, x, i+1, use_pos,
          FIdx::code(FG_TOK_NEXT1, "FG_TOK_NEXT1"), F);
          //assert(false);
        } else {
          idx << FIdx::tag(1)<< FIdx::code(FG_TOK_NEXTNULL, "FG_TOK_NEXTNULL");
          F(idx); 
          idx.clear();
        }


        //features for the +2/-2 token
        if (i >1){
          //...PREV2
          extract_previous_or_next_token<FIdx, X, R, Functor>(symbols, x, i-2, use_pos,
          FIdx::code(FG_TOK_PREV2, "FG_TOK_PREV2"), F);
        } else {
          idx << FIdx::tag(2)<< FIdx::code(FG_TOK_PREVNULL, "FG_TOK_PREVNULL");
          F(idx); 
          idx.clear();
        }
        if (i < num_words -2){
          //...NEXT2
          extract_previous_or_next_token<FIdx, X, R, Functor>(symbols, x, i+2, use_pos,
          FIdx::code(FG_TOK_NEXT2, "FG_TOK_NEXT2"), F);
        } else {
          idx <<  FIdx::tag(2)<< FIdx::code(FG_TOK_NEXTNULL, "FG_TOK_NEXTNULL");
          F(idx); 
          idx.clear();
        }


        //then insert bigram and trigram feats
        assert(use_pos);
        if (i > 0){

          typename FIdx::Tag tok_cpos_minus_1 = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
              (x.get_token(i - 1).coarse_pos()));

          idx << tok_cpos << tok_cpos_minus_1 << 
            FIdx::code(FG_TOK_PREV_CTAG_BIGRAM, "FG_TOK_PREV_CTAG_BIGRAM");
          F(idx); 
          idx.clear();
        }
        if (i < num_words - 1){

          typename FIdx::Tag tok_cpos_plus_1 = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
              (x.get_token(i + 1).coarse_pos()));

          idx << tok_cpos << tok_cpos_plus_1 << 
            FIdx::code(FG_TOK_NEXT_CTAG_BIGRAM, "FG_TOK_NEXT_CTAG_BIGRAM");
          F(idx); 
          idx.clear();
        }

        if (i > 1){

          typename FIdx::Tag tok_cpos_minus_1 = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
              (x.get_token(i - 1).coarse_pos()));

          typename FIdx::Tag tok_cpos_minus_2 = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
              (x.get_token(i - 2).coarse_pos()));

          idx << tok_cpos << tok_cpos_minus_1 << tok_cpos_minus_2 <<
            FIdx::code(FG_TOK_PREV_CTAG_TRIGRAM, "FG_TOK_PREV_CTAG_TRIGRAM");
          F(idx); 
          idx.clear();

        }
        if (i < num_words - 2){
          typename FIdx::Tag tok_cpos_plus_1 = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
              (x.get_token(i + 1).coarse_pos()));

          typename FIdx::Tag tok_cpos_plus_2 = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>
              (x.get_token(i + 2).coarse_pos()));


          idx << tok_cpos << tok_cpos_plus_1 <<  tok_cpos_plus_2 << 
            FIdx::code(FG_TOK_NEXT_CTAG_TRIGRAM, "FG_TOK_NEXT_CTAG_TRIGRAM");
          F(idx); 
          idx.clear();

        }
      }


    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_std_se(const DepSymbols& symbols, const X& x, const int start, const int end, 
        Functor& F_se, Functor& F_es) {
        const int num_words = x.size();
        assert(start >= -1);
        assert(end >= 0);
        assert(start <= end);
        assert(start < num_words);
        assert(end < num_words);

        typename FIdx::Tag start_cpos = (start >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag end_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end).coarse_pos()));


        typename FIdx::Word start_word = (start >= 0)? 
        FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(x.get_token(start).word())):
        FIdx::rootWord();

        typename FIdx::Word end_word = 
          FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(x.get_token(end).word()));


        F_se(FIdx() << start_word << end_word << FIdx::dir(false) << 
          FIdx::code(FG_DEP_SW_EW, "FG_DEP_SW_EW") );
        F_es(FIdx() << start_word << end_word << FIdx::dir(true) << 
          FIdx::code(FG_DEP_SW_EW, "FG_DEP_SW_EW") );

        F_se(FIdx() << start_cpos << end_cpos << FIdx::dir(false) <<
          FIdx::code(FG_DEP_ST_ET, "FG_DEP_ST_ET"));
        F_es(FIdx() << start_cpos << end_cpos << FIdx::dir(true) <<
          FIdx::code(FG_DEP_ST_ET, "FG_DEP_ST_ET"));

        F_se(FIdx() << start_cpos << end_word << end_cpos << FIdx::dir(false) << 
          FIdx::code(FG_DEP_ST_EWT, "FG_DEP_ST_EWT"));
        F_es(FIdx() << start_cpos << end_word << end_cpos << FIdx::dir(true) << 
          FIdx::code(FG_DEP_ST_EWT, "FG_DEP_ST_EWT"));

        F_se(FIdx() << start_word << end_word << end_cpos << FIdx::dir(false) << 
          FIdx::code(FG_DEP_SW_EWT, "FG_DEP_SW_EWT"));
        F_es(FIdx() << start_word << end_word << end_cpos << FIdx::dir(true) << 
          FIdx::code(FG_DEP_SW_EWT, "FG_DEP_SW_EWT"));

        F_se(FIdx() << start_word <<  end_word << start_cpos <<FIdx::dir(false) << 
          FIdx::code(FG_DEP_SWT_EW, "FG_DEP_SWT_EW"));
        F_es(FIdx() << start_word <<  end_word << start_cpos <<FIdx::dir(true) << 
          FIdx::code(FG_DEP_SWT_EW, "FG_DEP_SWT_EW"));

        F_se(FIdx() << start_word << end_cpos << start_cpos << FIdx::dir(false) << 
          FIdx::code(FG_DEP_SWT_ET, "FG_DEP_SWT_ET"));
        F_es(FIdx() << start_word << end_cpos << start_cpos << FIdx::dir(true) << 
          FIdx::code(FG_DEP_SWT_ET, "FG_DEP_SWT_ET"));

        F_se(FIdx() << start_word << end_word << start_cpos << end_cpos << FIdx::dir(false) << 
          FIdx::code(FG_DEP_SWT_EWT, "FG_DEP_SWT_EWT"));
        F_es(FIdx() << start_word << end_word << start_cpos << end_cpos << FIdx::dir(true) << 
          FIdx::code(FG_DEP_SWT_EWT, "FG_DEP_SWT_EWT"));

        //morpho feats
        typedef std::list<typename X::Tag> MorphoList;
        MorphoList empty; 
        const MorphoList& start_morph = (start == -1) ? empty : x.get_token(start).morpho_tags(); 
        const MorphoList& end_morph  = x.get_token(end).morpho_tags(); 

        //loop over mod morph label
        for (auto end_m = end_morph.begin(); end_m != end_morph.end(); ++end_m) {
          typename FIdx::Tag end_m_idx = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*end_m)); 	  
            
          F_se(FIdx() << start_cpos << end_m_idx << FIdx::dir(false) << 
            FIdx::code(FG_DEP_ST_EM, "FG_DEP_ST_EM"));
          F_es(FIdx() << start_cpos << end_m_idx << FIdx::dir(true) << 
            FIdx::code(FG_DEP_ST_EM, "FG_DEP_ST_EM"));

          F_se(FIdx() << start_cpos << end_m_idx << end_cpos << FIdx::dir(false) << 
            FIdx::code(FG_DEP_ST_EMT, "FG_DEP_ST_EMT"));
          F_es(FIdx() << start_cpos << end_m_idx << end_cpos << FIdx::dir(true) << 
            FIdx::code(FG_DEP_ST_EMT, "FG_DEP_ST_EMT"));
        }

        //loop over head morph and also combine with mod morph
        for (auto start_m = start_morph.begin(); start_m != start_morph.end(); ++start_m) {
          typename FIdx::Tag start_m_idx = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*start_m)); 	  
          F_se(FIdx() << start_m_idx << end_cpos << FIdx::dir(false) << 
            FIdx::code(FG_DEP_SM_ET, "FG_DEP_SM_ET"));
          F_es(FIdx() << start_m_idx << end_cpos << FIdx::dir(true) << 
            FIdx::code(FG_DEP_SM_ET, "FG_DEP_SM_ET"));

          F_se(FIdx() << start_m_idx << end_cpos << start_cpos << FIdx::dir(false) << 
            FIdx::code(FG_DEP_SMT_ET, "FG_DEP_SMT_ET"));
          F_es(FIdx() << start_m_idx << end_cpos << start_cpos << FIdx::dir(true) << 
            FIdx::code(FG_DEP_SMT_ET, "FG_DEP_SMT_ET"));

          for (auto end_m = end_morph.begin(); end_m != end_morph.end(); ++end_m) {
            typename FIdx::Tag end_m_idx = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*end_m)); 	  

            //assert(r.head() != 0);
            F_se(FIdx() << start_m_idx << end_m_idx << FIdx::dir(false) << 
              FIdx::code(FG_DEP_SM_EM, "FG_DEP_SM_EM"));
            F_es(FIdx() << start_m_idx << end_m_idx << FIdx::dir(true) << 
              FIdx::code(FG_DEP_SM_EM, "FG_DEP_SM_EM"));

            F_se(FIdx() << start_m_idx << end_m_idx << start_cpos << FIdx::dir(false) << 
              FIdx::code(FG_DEP_SMT_EM, "FG_DEP_SMT_EM"));
            F_es(FIdx() << start_m_idx << end_m_idx << start_cpos << FIdx::dir(true) << 
              FIdx::code(FG_DEP_SMT_EM, "FG_DEP_SMT_EM"));

            F_se(FIdx() << start_m_idx << end_m_idx << end_cpos << FIdx::dir(false) << 
              FIdx::code(FG_DEP_SM_EMT, "FG_DEP_SM_EMT"));
            F_es(FIdx() << start_m_idx << end_m_idx << end_cpos << FIdx::dir(true) << 
              FIdx::code(FG_DEP_SM_EMT, "FG_DEP_SM_EMT"));

            F_se(FIdx() << start_m_idx << end_m_idx << start_cpos << end_cpos << FIdx::dir(false) << 
              FIdx::code(FG_DEP_SMT_EMT, "FG_DEP_SMT_EMT"));
            F_es(FIdx() << start_m_idx << end_m_idx << start_cpos << end_cpos << FIdx::dir(true) << 
              FIdx::code(FG_DEP_SMT_EMT, "FG_DEP_SMT_EMT"));
          }
        }
      }



    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_std(const DepSymbols& symbols, const X& x, const R& r, Functor& F) {
        const int num_words = x.size();
        const int head = r.head();
        const int mod = r.mod();
        assert(head >= -1);
        assert(mod >= 0);
        assert(head < num_words);
        assert(mod < num_words);
        assert(false); //function to remove

        typename FIdx::Tag head_cpos = (head >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag mod_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod).coarse_pos()));


        typename FIdx::Word head_word = (head >= 0)? 
        FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(x.get_token(head).word())):
        FIdx::rootWord();

        typename FIdx::Word mod_word = 
          FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(x.get_token(mod).word()));


        FIdx idx;      
        idx << head_word << mod_word << FIdx::code(FG_DEP_SW_EW, "FG_DEP_SW_EW");
        F(idx); 
        idx.clear();

	    idx << head_cpos << mod_cpos << FIdx::code(FG_DEP_ST_ET, "FG_DEP_ST_ET");
        F(idx); 
        idx.clear();

        idx << head_cpos << mod_word << mod_cpos << FIdx::code(FG_DEP_ST_EWT, "FG_DEP_ST_EWT");
        F(idx); 
        idx.clear();

        idx << head_word << mod_word << mod_cpos << FIdx::code(FG_DEP_SW_EWT, "FG_DEP_SW_EWT");
        F(idx); 
        idx.clear();

        idx << head_word << head_cpos << mod_word << FIdx::code(FG_DEP_SW_EWT, "FG_DEP_SWT_EW");
        F(idx); 
        idx.clear();

        idx << head_word << head_cpos << mod_cpos << FIdx::code(FG_DEP_SW_EWT, "FG_DEP_SWT_ET");
        F(idx); 
        idx.clear();

        idx << head_word << head_cpos << mod_word << mod_cpos << FIdx::code(FG_DEP_SWT_EWT, "FG_DEP_SWT_EWT");
        F(idx); 
        idx.clear();

        //morpho feats
        typedef std::list<typename X::Tag> MorphoList;
        MorphoList empty; 
        const MorphoList& head_morph = (r.head()==-1) ? empty : x.get_token(r.head()).morpho_tags(); 
        const MorphoList& mod_morph  = x.get_token(r.mod()).morpho_tags(); 

        //loop over mod morph label
        for (auto mm = mod_morph.begin(); mm != mod_morph.end(); ++mm) {
          typename FIdx::Tag mm_idx = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*mm)); 	  
          //assert(r.mod() != 0);
          //assert(r.head() != 0);
            
          idx << head_cpos << mm_idx << FIdx::code(FG_DEP_ST_EM, "FG_DEP_ST_EM");
          F(idx); 
          idx.clear();

          idx << head_cpos << mm_idx << mod_cpos << FIdx::code(FG_DEP_ST_EMT, "FG_DEP_ST_EMT");
          F(idx); 
          idx.clear();
        }

        //loop over head morph and also combine with mod morph
        for (auto hm = head_morph.begin(); hm != head_morph.end(); ++hm) {
          typename FIdx::Tag hm_idx = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*hm)); 	  
          idx << hm_idx << mod_cpos << FIdx::code(FG_DEP_SM_ET, "FG_DEP_SM_ET");
          F(idx); 
          idx.clear();

          idx << hm_idx << mod_cpos << head_cpos << FIdx::code(FG_DEP_SMT_ET, "FG_DEP_SMT_ET");
          F(idx); 
          idx.clear();

          for (auto mm = mod_morph.begin(); mm != mod_morph.end(); ++mm) {
            typename FIdx::Tag mm_idx = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*mm)); 	  

            //assert(r.head() != 0);
            
            idx << hm_idx << mm_idx << FIdx::code(FG_DEP_SM_EM, "FG_DEP_SM_EM");
            F(idx); 
            idx.clear();

            idx << hm_idx << mm_idx << head_cpos << FIdx::code(FG_DEP_SMT_EM, "FG_DEP_SMT_EM");
            F(idx); 
            idx.clear();

            idx << hm_idx << mm_idx << mod_cpos << FIdx::code(FG_DEP_SM_EMT, "FG_DEP_SM_EMT");
            F(idx); 
            idx.clear();

            idx << hm_idx << mm_idx << head_cpos << mod_cpos << FIdx::code(FG_DEP_SMT_EMT, "FG_DEP_SMT_EMT");
            F(idx); 
            idx.clear();
          }
        }
      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_ctx_se(const DepSymbols& symbols, const X& x, const int start, 
        const int end, Functor& f_se, Functor& f_es) {
        assert(start >= -1);
        assert(start < x.size());

        assert(end >= 0);
        assert(end < x.size());
        if (start == -1){
          extract_dep_ctx_root_se<FIdx, X, R, Functor>(symbols, x, start, end, f_se); 
        } else {
          extract_dep_ctx_std_se<FIdx, X, R, Functor>(symbols, x, start, end, f_se, f_es);
        }
      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_ctx(const DepSymbols& symbols, const X& x, const R& r, Functor& f) {
        assert(r.head() >= -1);
        assert(r.head() < x.size());
        assert(false); //function to remove

        assert(r.mod() >= 0);
        assert(r.mod() < x.size());
        if (r.head() == -1){
          extract_dep_ctx_root<FIdx, X, R, Functor>(symbols, x, r, f); 
        } else {
          extract_dep_ctx_std<FIdx, X, R, Functor>(symbols, x, r, f);
        }
      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_ctx_root_se(const DepSymbols& symbols, const X& x, const int start, 
        const int end , Functor& F_se) {
        //cerr << "start of ctx root " << end << endl;

        assert(start == -1);
        assert(end >= 0);
        const int num_words = x.size();

        assert(end < num_words);

        //root context do not have a direction

        //typename FIdx::Tag start_cpos = 
        //  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start).coarse_pos()));

        typename FIdx::Tag end_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end).coarse_pos()));

        //typename FIdx::Tag start_cpos_minus_1 = 
        //  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start - 1).coarse_pos()));

        //assert(start + 1 < num_words);
        //typename FIdx::Tag start_cpos_plus_1 = 
        //  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start + 1).coarse_pos()));

        typename FIdx::Tag end_cpos_minus_1 = (end - 1 >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end - 1).coarse_pos())):
          FIdx::rootTag();

        //typename FIdx::Tag end_cpos_minus_2 = (end - 2 >= 0)?
        //  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end - 2).coarse_pos())):
        //  FIdx::rootTag();

        typename FIdx::Tag end_cpos_plus_1 = (end + 1 < num_words)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end + 1).coarse_pos())):
          FIdx::rootTag();

        //first group
        F_se(FIdx() <<  end_cpos_minus_1 << end_cpos <<
          FIdx::code(FG_DEP_CTXT_ROOT_EP, "FG_DEP_CTXT_ROOT_EP"));
        
        F_se(FIdx() <<  end_cpos_plus_1 << end_cpos <<
          FIdx::code(FG_DEP_CTXT_ROOT_EN, "FG_DEP_CTXT_ROOT_EN"));

        F_se(FIdx() << end_cpos_minus_1 << end_cpos_minus_1 <<  end_cpos << 
          FIdx::code(FG_DEP_CTXT_ROOT_EPN, "FG_DEP_CTXT_ROOT_EPN"));

        //second group i > 0!?
        /*if (end > 0){
          cerr << "adding adjacent " << endl;
        F_se(FIdx() << end_cpos << end_cpos_minus_1 << start_cpos_plus_1 << 
          FIdx::code(FG_DEP_CTXT_ADJ_S_EN, "FG_DEP_CTXT_ROOT_ADJ_S_N"));
          cerr << "se " << F_se.back() << endl;

        F_se(FIdx() << end_cpos << end_cpos_minus_1 << end_cpos_minus_2 <<
          FIdx::code(FG_DEP_CTXT_ADJ_SP_E, "FG_DEP_CTXT_ROOT_ADJ_SP_E"));
          cerr << "se " << F_se.back() << endl;

        F_se(FIdx() << end_cpos << end_cpos_minus_1 << end_cpos_minus_2 << end_cpos_plus_1 << 
          FIdx::code(FG_DEP_CTXT_ADJ_SP_EN, "FG_DEP_CTXT_ROOT_ADJ_SP_EN"));
          cerr << "se " << F_se.back() << endl;
         }*/

          //cerr << "end of root " << end << endl;
          //string k; cin >> k;
      }


    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_ctx_root(const DepSymbols& symbols, const X& x, const R& r, Functor& F) {
        const int head = r.head();
        const int mod = r.mod();
        assert(head == -1);
        assert(mod >= 0);
        const int num_words = x.size();
        assert(false); //function to remove

        assert(mod < num_words);

        //typename FIdx::Tag head_cpos = 
        //  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head).coarse_pos()));

        typename FIdx::Tag mod_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod).coarse_pos()));

        //typename FIdx::Tag head_cpos_minus_1 = 
        //  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head - 1).coarse_pos()));

        assert(head + 1 < num_words);
        typename FIdx::Tag head_cpos_plus_1 = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head + 1).coarse_pos()));

        typename FIdx::Tag mod_cpos_minus_1 = (mod - 1 >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod - 1).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag mod_cpos_minus_2 = (mod - 2 >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod - 2).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag mod_cpos_plus_1 = (mod + 1 < num_words)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod + 1).coarse_pos())):
          FIdx::rootTag();

        //first group
        FIdx idx;      
        idx << mod_cpos << mod_cpos_minus_1 << 
          FIdx::code(FG_DEP_CTXT_ROOT_EP, "FG_DEP_CTXT_ROOT_EP");
        F(idx); 
        idx.clear();

        idx << mod_cpos << mod_cpos_plus_1 << 
          FIdx::code(FG_DEP_CTXT_ROOT_EN, "FG_DEP_CTXT_ROOT_EN");
        F(idx); 
        idx.clear();

        idx << mod_cpos << mod_cpos_minus_1 << mod_cpos_minus_1 <<
          FIdx::code(FG_DEP_CTXT_ROOT_EPN, "FG_DEP_CTXT_ROOT_EPN");
        F(idx); 
        idx.clear();

        //second group i > 0!?
        idx << mod_cpos << mod_cpos_minus_1 << head_cpos_plus_1 << 
          FIdx::code(FG_DEP_CTXT_ADJ_S_EN, "FG_DEP_CTXT_ROOT_ADJ_S_N");
        F(idx); 
        idx.clear();

        idx << mod_cpos << mod_cpos_minus_1 << mod_cpos_minus_2 <<
          FIdx::code(FG_DEP_CTXT_ADJ_SP_E, "FG_DEP_CTXT_ROOT_ADJ_SP_E");
        F(idx); 
        idx.clear();

        idx << mod_cpos << mod_cpos_minus_1 << mod_cpos_minus_2 << mod_cpos_plus_1 << 
          FIdx::code(FG_DEP_CTXT_ADJ_SP_EN, "FG_DEP_CTXT_ROOT_ADJ_SP_EN");
        F(idx); 
        idx.clear();
      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_ctx_std(const DepSymbols& symbols, const X& x, const R& r, Functor& F) {
        const int head = r.head();
        const int mod = r.mod();
        assert(head >= 0);
        assert(mod >= 0);
        const int num_words = x.size();
        assert(false); //function to remove

        typename FIdx::Tag head_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head).coarse_pos()));

        typename FIdx::Tag mod_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod).coarse_pos()));

      
        typename FIdx::Tag head_cpos_minus_1 = (head > 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head - 1).coarse_pos())) :
          FIdx::rootTag();

        typename FIdx::Tag head_cpos_plus_1 = (head < num_words -1)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(head + 1).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag mod_cpos_minus_1 = (mod > 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod - 1).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag mod_cpos_plus_1 = (mod < num_words - 1)? 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(mod + 1).coarse_pos())):
          FIdx::rootTag();

        //first group

        FIdx idx;      
        idx << mod_cpos << head_cpos << head_cpos_minus_1 << FIdx::code(FG_DEP_CTXT_SP_E, "FG_DEP_CTXT_SP_E");
        F(idx); 
        idx.clear();

        idx << mod_cpos << head_cpos << head_cpos_plus_1 << FIdx::code(FG_DEP_CTXT_SN_E, "FG_DEP_CTXT_SN_E");
        F(idx); 
        idx.clear();

        //second group

        idx << mod_cpos_minus_1 << head_cpos << FIdx::code(FG_DEP_CTXT_S_EP, "FG_DEP_CTXT_S_EP");
        F(idx); 
        idx.clear();

        idx << mod_cpos_minus_1 << head_cpos << head_cpos_minus_1 << 
          FIdx::code(FG_DEP_CTXT_SP_EP, "FG_DEP_CTXT_SP_EP");
        F(idx); 
        idx.clear();

        idx << mod_cpos_minus_1 << head_cpos << head_cpos_plus_1 << 
          FIdx::code(FG_DEP_CTXT_SN_EP, "FG_DEP_CTXT_SN_EP");
        F(idx); 
        idx.clear();

        //third group
        idx << mod_cpos_plus_1 << head_cpos << FIdx::code(FG_DEP_CTXT_S_EN, "FG_DEP_CTXT_S_EN");
        F(idx); 
        idx.clear();

        idx << mod_cpos_plus_1 << head_cpos << head_cpos_minus_1 << 
          FIdx::code(FG_DEP_CTXT_SP_EN, "FG_DEP_CTXT_SP_EN");
        F(idx); 
        idx.clear();

        idx << mod_cpos_plus_1 << head_cpos << head_cpos_plus_1 << 
          FIdx::code(FG_DEP_CTXT_SN_EN, "FG_DEP_CTXT_SN_EN");
        F(idx); 
        idx.clear();
      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_distance(const DepSymbols& symbols, const X& x, const R& r,
        const bool use_pos, Functor& F) {
        assert(false); //function to remove
        const int dist = r.head()<r.mod() ? r.mod()-r.head() : r.head()-r.mod(); //we do not subtract one
        const int num_words = x.size();
        assert(r.head() >= -1);
        assert(r.mod() >= 0);
        assert(r.head() < num_words);
        assert(r.mod() < num_words);

        typename FIdx::Tag head_cpos = (r.head() >= 0)? 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(r.head()).coarse_pos())): FIdx::rootTag();

        typename FIdx::Tag mod_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(r.mod()).coarse_pos()));


        const int plen = (dist > TREELER_FTEMPLATES_TOKEN_DISTANCE_THRESHOLD
            ? TREELER_FTEMPLATES_TOKEN_DISTANCE_THRESHOLD : dist);

        typename FIdx::Tag plen_idx = FIdx::tag(plen);

        FIdx idx;      
        idx << plen_idx << FIdx::code(FG_DEP_DIST_EXACT, "FG_DEP_DIST_EXACT");
        F(idx); 
        idx.clear();

        //binned distance
        if (dist > 2){
          idx << FIdx::tag(2) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT");
          F(idx); 
          idx.clear();
        }
        if (dist > 5){
          idx << FIdx::tag(5) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT");
          F(idx); 
          idx.clear();
        }
        if (dist > 10){
          idx << FIdx::tag(10) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT");
          F(idx); 
          idx.clear();
        }
        if (dist > 20){
          idx << FIdx::tag(20) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT");
          F(idx); 
          idx.clear();
        }
        if (dist > 30){
          idx << FIdx::tag(30) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT");
          F(idx); 
          idx.clear();
        }
        if (dist > 40){
          idx << FIdx::tag(40) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT");
          F(idx); 
          idx.clear();
        }

        assert(use_pos);
        ///////////////
        if (dist > 2){
          //first feat 
          idx << FIdx::tag(2) << head_cpos << FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG");
          F(idx); 
          idx.clear();
          //second feat
          idx << FIdx::tag(2) << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG");
          F(idx); 
          idx.clear();
          //third feat
          idx << FIdx::tag(2) << head_cpos << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG");
          F(idx); 
          idx.clear();
        }
        if (dist > 5){
          //first feat 
          idx << FIdx::tag(5) << head_cpos << FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG");
          F(idx); 
          idx.clear();
          //second feat
          idx << FIdx::tag(5) << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG");
          F(idx); 
          idx.clear();
          //third feat
          idx << FIdx::tag(5) << head_cpos << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG");
          F(idx); 
          idx.clear();
        }
        if (dist > 10){
          //first feat 
          idx << FIdx::tag(10) << head_cpos << FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG");
          F(idx); 
          idx.clear();
          //second feat
          idx << FIdx::tag(10) << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG");
          F(idx); 
          idx.clear();
          //third feat
          idx << FIdx::tag(10) << head_cpos << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG");
          F(idx); 
          idx.clear();
        }
        if (dist > 20){
          //first feat 
          idx << FIdx::tag(20) << head_cpos << FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG");
          F(idx); 
          idx.clear();
          //second feat
          idx << FIdx::tag(20) << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG");
          F(idx); 
          idx.clear();
          //third feat
          idx << FIdx::tag(20) << head_cpos << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG");
          F(idx); 
          idx.clear();
        }
        if (dist > 30){
          //first feat 
          idx << FIdx::tag(30) << head_cpos << FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG");
          F(idx); 
          idx.clear();
          //second feat
          idx << FIdx::tag(30) << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG");
          F(idx); 
          idx.clear();
          //third feat
          idx << FIdx::tag(30) << head_cpos << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG");
          F(idx); 
          idx.clear();
        }
        if (dist > 40){
          //first feat 
          idx << FIdx::tag(40) << head_cpos << FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG");
          F(idx); 
          idx.clear();
          //second feat
          idx << FIdx::tag(40) << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG");
          F(idx); 
          idx.clear();
          //third feat
          idx << FIdx::tag(40) << head_cpos << mod_cpos << FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG");
          F(idx); 
          idx.clear();
        }

      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_between(const DepSymbols& symbols, const X& x, const R& r, Functor& F) {
        assert(false); // function to remove
        const int head = r.head();
        const int mod = r.mod();
        assert(head >= -1);
        assert(mod >= 0);

        typename FIdx::Tag head_cpos = (head >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(r.head()).coarse_pos())): FIdx::rootTag();

        typename FIdx::Tag mod_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(r.mod()).coarse_pos()));



        
        int start = head;
        int end = mod;
        if (end < start){
          start = mod;
          end = head;
        }
        if (start == -1){
          start = 0;
        }
        FIdx idx;

        //init the counters
        int verb_count = 0;
        int punct_count = 0;
        int coord_count = 0;

        // if there is something between the start and the end

        for (int i = start; i < end; ++i){
          //add the inbetween pos

          typename FIdx::Tag between_cpos = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(i).coarse_pos()));
          idx << head_cpos << mod_cpos << between_cpos <<
            FIdx::code(FG_DEP_BETW_SEB, "FG_DEP_BETW_SEB");
          F(idx); 

          //get the verb count, punct count and coord count
          typename X::Token t = x.get_token(i);
          int fine_pos_tag = symbols.map_field<COARSE_POS,int>(t.fine_pos());
          if(symbols.is_verb(fine_pos_tag))  { verb_count++; }
          if(symbols.is_punc(fine_pos_tag))  { punct_count++; }
          if(symbols.is_coord(fine_pos_tag)) { coord_count++; }

        }



        //limit the counts
        if(verb_count > TREELER_FTEMPLATES_DEP1_VCOUNT_MAX) { 
          verb_count = TREELER_FTEMPLATES_DEP1_VCOUNT_MAX; 
        }
        if(punct_count > TREELER_FTEMPLATES_DEP1_PCOUNT_MAX) { 
          punct_count = TREELER_FTEMPLATES_DEP1_PCOUNT_MAX; 
        }
        if(coord_count > TREELER_FTEMPLATES_DEP1_CCOUNT_MAX) { 
          coord_count = TREELER_FTEMPLATES_DEP1_CCOUNT_MAX; 
        }

        //add the counts
        idx << FIdx::tag(verb_count) <<
          FIdx::code(FG_DEP_BETW_VCNT, "FG_DEP_BETW_VCNT");
        F(idx); 
        idx.clear();

        idx << head_cpos << mod_cpos << FIdx::tag(verb_count) <<
          FIdx::code(FG_DEP_BETW_SE_VCNT, "FG_DEP_BETW_SE_VCNT");
        F(idx); 
        idx.clear();

        idx << FIdx::tag(punct_count) <<
          FIdx::code(FG_DEP_BETW_PCNT, "FG_DEP_BETW_PCNT");
        F(idx); 
        idx.clear();

        idx << head_cpos << mod_cpos << FIdx::tag(punct_count) <<
          FIdx::code(FG_DEP_BETW_SE_PCNT, "FG_DEP_BETW_SE_PCNT");
        F(idx); 
        idx.clear();

        idx << FIdx::tag(coord_count) <<
          FIdx::code(FG_DEP_BETW_CCNT, "FG_DEP_BETW_CCNT");
        F(idx); 
        idx.clear();

        idx << head_cpos << mod_cpos << FIdx::tag(coord_count) <<
          FIdx::code(FG_DEP_BETW_SE_CCNT, "FG_DEP_BETW_SE_CCNT");
        F(idx); 
        idx.clear();
      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_mcdo(const DepSymbols& symbols, const X& x, const R& r, Functor& F) {

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
            hp[i] = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(htok.fine_pos()));
            if (i == 1) hw = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(htok.word()));
          }
          int mi = r.mod()+i-1;
          if (mi < 0 || mi >= x.size()) {
            mp[i] = FIdx::rootTag();
            // 	  if (i == 1) mw = FIdx::rootWord();
          }
          else {
            const typename X::Token& mtok = x.get_token(mi);
            mp[i] = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(mtok.fine_pos()));
            if (i == 1) mw = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(mtok.word()));
          }
        }

        int dist = r.head()<r.mod() ? r.mod()-r.head()-1 : r.head()-r.mod()-1;

        FIdx idx;      
        idx << hp[1] << hw << FIdx::code(FG_DEP_HW_HT, "hw_ht");
        F(idx); 
        idx.clear();

        idx << hw << FIdx::code(FG_DEP_HW, "hw");
        F(idx); 
        idx.clear(); 

        idx << hp[1] << FIdx::code(FG_DEP_HT, "ht");
        F(idx); 
        idx.clear();

        idx << mp[1] << mw << FIdx::code(FG_DEP_MW_MT, "mw_mt");
        F(idx); 
        idx.clear(); 

        idx << mw << FIdx::code(FG_DEP_MW, "mw");
        F(idx); 
        idx.clear();       

        idx << mp[1] << FIdx::code(FG_DEP_MT, "mt");
        F(idx); 
        idx.clear(); 

        idx << mp[1] << mw << hp[1] << hw << FIdx::code(FG_DEP_HW_HT_MW_MT, "hw_ht_mw_mt");
        F(idx); 
        idx.clear(); 

        idx << mp[1] << mw << hp[1] << FIdx::code(FG_DEP_HT_MW_MT, "ht_mw_mt");
        F(idx); 
        idx.clear(); 

        idx << mp[1] << mw << hw << FIdx::code(FG_DEP_HW_MW_MT, "hw_mw_mt");
        F(idx); 
        idx.clear(); 

        idx << mp[1] << hp[1] << hw << FIdx::code(FG_DEP_HW_HT_MT, "hw_ht_mt");
        F(idx); 
        idx.clear(); 

        idx << mw << hp[1] << hw << FIdx::code(FG_DEP_HW_HT_MW, "hw_ht_mw");
        F(idx); 
        idx.clear(); 

        idx << mw << hw << FIdx::code(FG_DEP_HW_MW, "hw_mw");
        F(idx); 
        idx.clear(); 

        idx << mp[1] << hp[1] << FIdx::code(FG_DEP_HT_MT, "ht_mt");
        F(idx); 
        idx.clear(); 

        int i = 13;
        for (int j = 0; j < dist; j++, i++) {
          const typename X::Token& btok = x.get_token(min(r.mod(), r.head())+j+1);
          idx << mp[1] << FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(btok.fine_pos())) << hp[1] << FIdx::code(FG_DEP_BETW, "ht_bt_mt");
          F(idx); 
          idx.clear(); 
        }

        idx << mp[1] << mp[0] << hp[2] << hp[1] << FIdx::code(FG_DEP_SURR_HN_MP, "ht_ht+1_mt-1_mt");
        F(idx); 
        idx.clear(); 

        idx << mp[1] << mp[0] << hp[1] << hp[0] << FIdx::code(FG_DEP_SURR_HP_MP, "ht-1_ht_mt-1_mt");
        F(idx); 
        idx.clear(); 

        idx << mp[2] << mp[1] << hp[2] << hp[1] << FIdx::code(FG_DEP_SURR_HN_MN, "ht_ht+1_mt_mt+1");
        F(idx); 
        idx.clear(); 

        idx << mp[2] << mp[1] << hp[1] << hp[0] << FIdx::code(FG_DEP_SURR_HP_MN, "ht-1_ht_mt_mt+1");
        F(idx); 
        idx.clear(); 


        ////////////////////////

        // FG_DEP_SM_ET,
        // FG_DEP_SMT_ET,

        // FG_DEP_ST_EM,
        // FG_DEP_ST_EMT,


        {
          typedef std::list<typename X::Tag> MorphoList;
          MorphoList empty; 
          const MorphoList& headM = (r.head()==-1) ? empty : x.get_token(r.head()).morpho_tags(); 
          const MorphoList& modM  = x.get_token(r.mod()).morpho_tags(); 

          for (auto hm = headM.begin(); hm != headM.end(); ++hm) {
            typename FIdx::Tag fhm = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*hm)); 	  

            idx << mp[1] << fhm << FIdx::code(FG_DEP_SM_ET, "DEP_SM_ET");
            F(idx); 
            idx.clear(); 	    	  

            idx << mp[1] << hp[1] << fhm << FIdx::code(FG_DEP_SMT_ET, "DEP_SMT_ET");
            F(idx); 
            idx.clear();	  

            for (auto mm = modM.begin(); mm != modM.end(); ++mm) {
              typename FIdx::Tag fmm = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*mm)); 

              idx << fmm << fhm << FIdx::code(FG_DEP_SM_EM, "DEP_SM_EM");
              F(idx); 
              idx.clear(); 	    

              idx << mp[1] << fmm << fhm << FIdx::code(FG_DEP_SM_EM, "DEP_SM_EMT");
              F(idx); 
              idx.clear(); 	    

              idx << fmm << hp[1] << fhm << FIdx::code(FG_DEP_SM_EM, "DEP_SMT_EM");
              F(idx); 
              idx.clear(); 	    

              idx << mp[1] << fmm << hp[1] << fhm << FIdx::code(FG_DEP_SM_EM, "DEP_SMT_EMT");
              F(idx); 
              idx.clear(); 	    
            } // for each mod tag 
          } // for each head tag 

          for (auto mm = modM.begin(); mm != modM.end(); ++mm) {
            typename FIdx::Tag fmm = FIdx::tag(symbols.map_field<MORPHO_TAG,typename FIdx::Tag>(*mm)); 
            idx << fmm << hp[1] << FIdx::code(FG_DEP_SM_ET, "DEP_ST_EM");
            F(idx); 
            idx.clear(); 	    	  

            idx << mp[1] << fmm << hp[1] << FIdx::code(FG_DEP_SMT_ET, "DEP_ST_EMT");
            F(idx); 
            idx.clear();	  	  
          }
        }      

      }

      ///dep features se/es
    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_ctx_std_se(const DepSymbols& symbols, const X& x, const int start,
        const int end, Functor& F_se, Functor& F_es) {

        assert(start >= 0);
        assert(end >= 0);
        const int num_words = x.size();

        typename FIdx::Tag start_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start).coarse_pos()));

        typename FIdx::Tag end_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end).coarse_pos()));

      
        typename FIdx::Tag start_cpos_minus_1 = (start > 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start - 1).coarse_pos())) :
          FIdx::rootTag();

        typename FIdx::Tag start_cpos_plus_1 = (start < num_words -1)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start + 1).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag end_cpos_minus_1 = (end > 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end - 1).coarse_pos())):
          FIdx::rootTag();

        typename FIdx::Tag end_cpos_plus_1 = (end < num_words - 1)? 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end + 1).coarse_pos())):
          FIdx::rootTag();

        //first group

             
        F_se(FIdx() << end_cpos << start_cpos << start_cpos_minus_1 << FIdx::dir(false) << 
          FIdx::code(FG_DEP_CTXT_SP_E, "FG_DEP_CTXT_SP_E"));
        F_es(FIdx() << end_cpos << start_cpos << start_cpos_minus_1 << FIdx::dir(true) << 
          FIdx::code(FG_DEP_CTXT_SP_E, "FG_DEP_CTXT_SP_E"));
        
        F_se(FIdx() << end_cpos << start_cpos << start_cpos_plus_1 << FIdx::dir(false) << 
          FIdx::code(FG_DEP_CTXT_SN_E, "FG_DEP_CTXT_SN_E"));
        F_es(FIdx() << end_cpos << start_cpos << start_cpos_plus_1 << FIdx::dir(true) << 
          FIdx::code(FG_DEP_CTXT_SN_E, "FG_DEP_CTXT_SN_E"));

        //second group
        F_se(FIdx() << end_cpos_minus_1 << start_cpos << FIdx::dir(false) << 
          FIdx::code(FG_DEP_CTXT_S_EP, "FG_DEP_CTXT_S_EP"));
        F_es(FIdx() << end_cpos_minus_1 << start_cpos << FIdx::dir(true) << 
          FIdx::code(FG_DEP_CTXT_S_EP, "FG_DEP_CTXT_S_EP"));
        
        F_se(FIdx() << end_cpos_minus_1 << start_cpos << start_cpos_minus_1 << 
          FIdx::dir(false) << FIdx::code(FG_DEP_CTXT_SP_EP, "FG_DEP_CTXT_SP_EP"));
        F_es(FIdx() << end_cpos_minus_1 << start_cpos << start_cpos_minus_1 << 
          FIdx::dir(true) << FIdx::code(FG_DEP_CTXT_SP_EP, "FG_DEP_CTXT_SP_EP"));
        

        F_se(FIdx() << end_cpos_minus_1 << start_cpos << start_cpos_plus_1 << 
          FIdx::dir(false) << FIdx::code(FG_DEP_CTXT_SN_EP, "FG_DEP_CTXT_SN_EP"));
        F_es(FIdx() << end_cpos_minus_1 << start_cpos << start_cpos_plus_1 << 
          FIdx::dir(true) << FIdx::code(FG_DEP_CTXT_SN_EP, "FG_DEP_CTXT_SN_EP"));
        
        //third group

        F_se(FIdx() << end_cpos << end_cpos_plus_1 << start_cpos << FIdx::dir(false) << 
          FIdx::code(FG_DEP_CTXT_S_EN, "FG_DEP_CTXT_S_EN")); /////
        F_es(FIdx() << end_cpos << end_cpos_plus_1 << start_cpos << FIdx::dir(true) << 
          FIdx::code(FG_DEP_CTXT_S_EN, "FG_DEP_CTXT_S_EN"));

        F_se(FIdx() <<  end_cpos << end_cpos_plus_1 << start_cpos << start_cpos_minus_1 <<
          FIdx::dir(false) << FIdx::code(FG_DEP_CTXT_SP_EN, "FG_DEP_CTXT_SP_EN"));
        F_es(FIdx() <<  end_cpos << end_cpos_plus_1 << start_cpos << start_cpos_minus_1 <<
          FIdx::dir(true) << FIdx::code(FG_DEP_CTXT_SP_EN, "FG_DEP_CTXT_SP_EN"));
        

        F_se(FIdx() <<  end_cpos << end_cpos_plus_1 << start_cpos << start_cpos_plus_1 <<
          FIdx::dir(false) << FIdx::code(FG_DEP_CTXT_SN_EN, "FG_DEP_CTXT_SN_EN"));
        F_es(FIdx() <<  end_cpos << end_cpos_plus_1 << start_cpos << start_cpos_plus_1 <<
          FIdx::dir(true) << FIdx::code(FG_DEP_CTXT_SN_EN, "FG_DEP_CTXT_SN_EN"));
        
      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void build_fgbin_1_se(
        const int len,
        Functor& F_se, Functor& F_es) {
          F_se(FIdx() << FIdx::tag(len) << FIdx::dir(false) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT"));
          F_es(FIdx() << FIdx::tag(len) << FIdx::dir(true) << FIdx::code(FG_DEP_DIST_BINGT, "FG_DEP_DIST_BINGT"));
 

      }


    template <typename FIdx, typename X, typename R, typename Functor>
      static void build_fgbin_2_se(
        const typename FIdx::Tag& start_cpos,
        const typename FIdx::Tag& end_cpos,
        const int len,
        Functor& F_se, Functor& F_es) {

           //first feat 
          F_se(FIdx() << start_cpos << FIdx::tag(2) << FIdx::dir(false) << 
            FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG"));
          F_es(FIdx() << start_cpos << FIdx::tag(2) << FIdx::dir(true) << 
            FIdx::code(FG_DEP_DIST_BINGT_SCTAG, "FG_DEP_DIST_BINGT_SCTAG"));
          
          //second feat
          F_se(FIdx() << end_cpos << FIdx::tag(2) << FIdx::dir(false) << 
            FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG"));
          F_es(FIdx() << end_cpos << FIdx::tag(2) << FIdx::dir(true) << 
            FIdx::code(FG_DEP_DIST_BINGT_ECTAG, "FG_DEP_DIST_BINGT_ECTAG"));
          
          //third feat
          F_se(FIdx() <<  end_cpos << start_cpos << FIdx::tag(2) << FIdx::dir(false) << 
            FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG"));
          F_es(FIdx() << end_cpos << start_cpos << FIdx::tag(2) << FIdx::dir(true) << 
            FIdx::code(FG_DEP_DIST_BINGT_SECTAG, "FG_DEP_DIST_BINGT_SECTAG"));

 }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_distance_se(const DepSymbols& symbols, const X& x, const int start,
        const int end, const bool use_pos, Functor& F_se, Functor& F_es) {
        assert(start <= end);
        const int dist = end - start; //we do not subtract one
        const int num_words = x.size();
        assert(start >= -1);
        assert(end >= 0);
        assert(start < num_words);
        assert(end < num_words);

        typename FIdx::Tag start_cpos = (start >= 0)? 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start).coarse_pos())): FIdx::rootTag();

        typename FIdx::Tag end_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end).coarse_pos()));


        const int plen = (dist > TREELER_FTEMPLATES_TOKEN_DISTANCE_THRESHOLD
            ? TREELER_FTEMPLATES_TOKEN_DISTANCE_THRESHOLD : dist);

        typename FIdx::Tag plen_idx = FIdx::tag(plen);

             
        F_se(FIdx() << plen_idx << FIdx::dir(false) << 
          FIdx::code(FG_DEP_DIST_EXACT, "FG_DEP_DIST_EXACT"));
        F_es(FIdx() << plen_idx << FIdx::dir(true) << 
          FIdx::code(FG_DEP_DIST_EXACT, "FG_DEP_DIST_EXACT"));
        
        

        //binned distance
        if (dist > 2){
          build_fgbin_1_se<FIdx,X,R,Functor>(2, F_se, F_es);
       }
        if (dist > 5){
          build_fgbin_1_se<FIdx,X,R,Functor>(5, F_se, F_es);
        }
        if (dist > 10){
          build_fgbin_1_se<FIdx,X,R,Functor>(10, F_se, F_es);
        }
        if (dist > 20){
          build_fgbin_1_se<FIdx,X,R,Functor>(20, F_se, F_es);
        }
        if (dist > 30){
          build_fgbin_1_se<FIdx,X,R,Functor>(30, F_se, F_es);
        }
        if (dist > 40){
          build_fgbin_1_se<FIdx,X,R,Functor>(40, F_se, F_es);
        }

        assert(use_pos);
        ///////////////
        if (dist > 2){
          build_fgbin_2_se<FIdx,X,R,Functor>(start_cpos, end_cpos, 2, F_se, F_es);
        }
        if (dist > 5){
          build_fgbin_2_se<FIdx,X,R,Functor>(start_cpos, end_cpos, 5, F_se, F_es);
        }
        if (dist > 10){
          build_fgbin_2_se<FIdx,X,R,Functor>(start_cpos, end_cpos, 10, F_se, F_es);
        }
        if (dist > 20){
          build_fgbin_2_se<FIdx,X,R,Functor>(start_cpos, end_cpos, 20, F_se, F_es);
        }
        if (dist > 30){
          build_fgbin_2_se<FIdx,X,R,Functor>(start_cpos, end_cpos, 30, F_se, F_es);
        }
        if (dist > 40){
          build_fgbin_2_se<FIdx,X,R,Functor>(start_cpos, end_cpos, 40, F_se, F_es);
        }

      }

    template <typename FIdx, typename X, typename R, typename Functor>
      static void extract_dep_between_se(const DepSymbols& symbols, const X& x, const int input_start,
        const int end, Functor& F_se, Functor& F_es) {
        int start = input_start;
        assert(start >= -1);
        assert(end >= 0);
        const int num_words = x.size();
        if (input_start == num_words -1) return;// there are no feats for this combination in the original file!

        typename FIdx::Tag start_cpos = (start >= 0)?
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(start).coarse_pos())): FIdx::rootTag();

        typename FIdx::Tag end_cpos = 
          FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(end).coarse_pos()));

        assert(input_start <= end); 
        /*if (start == -1){
          start = 0;
        }*/

        //init the counters
        int verb_count = 0;
        int punct_count = 0;
        int coord_count = 0;

        // if there is something between the start and the end

        for (int i = start + 1; i < end; ++i){
          //add the inbetween pos

          typename FIdx::Tag between_cpos = 
            FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(x.get_token(i).coarse_pos()));
          F_se(FIdx() << start_cpos << end_cpos << between_cpos <<
            FIdx::dir(false) << FIdx::code(FG_DEP_BETW_SEB, "FG_DEP_BETW_SEB"));
          F_es(FIdx() <<  start_cpos << end_cpos << between_cpos <<
            FIdx::dir(true) << FIdx::code(FG_DEP_BETW_SEB, "FG_DEP_BETW_SEB"));

          //get the verb count, punct count and coord count
          typename X::Token t = x.get_token(i);
          int fine_pos_tag = symbols.map_field<COARSE_POS,int>(t.fine_pos());
          if(symbols.is_verb(fine_pos_tag))  { verb_count++; }
          if(symbols.is_punc(fine_pos_tag))  { punct_count++; }
          if(symbols.is_coord(fine_pos_tag)) { coord_count++; }

        }



        //limit the counts
        if(verb_count > TREELER_FTEMPLATES_DEP1_VCOUNT_MAX) { 
          verb_count = TREELER_FTEMPLATES_DEP1_VCOUNT_MAX; 
        }
        if(punct_count > TREELER_FTEMPLATES_DEP1_PCOUNT_MAX) { 
          punct_count = TREELER_FTEMPLATES_DEP1_PCOUNT_MAX; 
        }
        if(coord_count > TREELER_FTEMPLATES_DEP1_CCOUNT_MAX) { 
          coord_count = TREELER_FTEMPLATES_DEP1_CCOUNT_MAX; 
        }


        F_se(FIdx() << start_cpos << end_cpos << FIdx::tag(verb_count) <<
          FIdx::dir(false) << FIdx::code(FG_DEP_BETW_SE_VCNT, "FG_DEP_BETW_SE_VCNT"));
        F_es(FIdx() <<  start_cpos << end_cpos <<  FIdx::tag(verb_count) <<
          FIdx::dir(true) << FIdx::code(FG_DEP_BETW_SE_VCNT, "FG_DEP_BETW_SE_VCNT"));

        F_se(FIdx() << FIdx::tag(verb_count) <<
          FIdx::dir(false) << FIdx::code(FG_DEP_BETW_VCNT, "FG_DEP_BETW_VCNT"));
        F_es(FIdx() << FIdx::tag(verb_count) <<
          FIdx::dir(true) << FIdx::code(FG_DEP_BETW_VCNT, "FG_DEP_BETW_VCNT"));

        
        F_se(FIdx() << start_cpos << end_cpos << FIdx::tag(punct_count) <<
          FIdx::dir(false) << FIdx::code(FG_DEP_BETW_SE_PCNT, "FG_DEP_BETW_SE_PCNT"));
        F_es(FIdx() << start_cpos << end_cpos << FIdx::tag(punct_count) <<
          FIdx::dir(true) << FIdx::code(FG_DEP_BETW_SE_PCNT, "FG_DEP_BETW_SE_PCNT"));

        F_se(FIdx() << FIdx::tag(punct_count) <<
          FIdx::dir(false) << FIdx::code(FG_DEP_BETW_PCNT, "FG_DEP_BETW_PCNT"));
        F_es(FIdx() << FIdx::tag(punct_count) <<
          FIdx::dir(true) << FIdx::code(FG_DEP_BETW_PCNT, "FG_DEP_BETW_PCNT"));
        
        
        F_se(FIdx() << start_cpos << end_cpos <<   FIdx::tag(coord_count) <<
          FIdx::dir(false) << FIdx::code(FG_DEP_BETW_SE_CCNT, "FG_DEP_BETW_SE_CCNT"));
        F_es(FIdx() <<  start_cpos << start_cpos << FIdx::tag(coord_count) <<
          FIdx::dir(true) << FIdx::code(FG_DEP_BETW_SE_CCNT, "FG_DEP_BETW_SE_CCNT"));

        F_se(FIdx() << FIdx::tag(coord_count) <<
          FIdx::dir(false) << FIdx::code(FG_DEP_BETW_CCNT, "FG_DEP_BETW_CCNT"));
        F_es(FIdx() << FIdx::tag(coord_count) <<
          FIdx::dir(true) << FIdx::code(FG_DEP_BETW_CCNT, "FG_DEP_BETW_CCNT"));
 

      }
  };

}



#endif /* DEP_FGENDEP_V1_H */
