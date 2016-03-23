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
 * \file   fgen-srl-v1.h
 * \brief  Standard features for SRL
 * \author 
 */

#ifndef SRL_FTEMPLATES
#define SRL_FTEMPLATES

#include <string>

#include "treeler/base/feature-vector.h"

#include "treeler/srl/srl.h"
#include "treeler/srl/fgen-srl-types.h"
#include "treeler/srl/paths-defs.h"
#include "treeler/srl/fidx-path.h"


#include <iostream>

namespace treeler {
  
  namespace srl {
    
    class FTemplatesSRL {
    public:
      typedef srl::Sentence X; 

    
      struct Configuration {
	bool unlabeled_syn; 
	bool path; 
	bool lluism;
	string kNoLabel; 
      
	Configuration() : 
	  unlabeled_syn(false), 
	  path(true), 	  
	  lluism(true), 
	  kNoLabel("NO_LABEL")
	{}
      };
       
    public:

      template <typename FIdx>
      static void extract(const Configuration& config, const srl::SRLSymbols& symbols, 			  
			  const X& x, int pred, int arg, 
			  FeatureVector<FIdx>* f) {			  
	typedef CollectFIdx<FIdx> Functor; 
	Functor F; 
	extract_<FIdx, Functor>(config, symbols, x, pred, arg, F);
	f->idx = F.create_array(); 
	f->n = F.size();
	return;
      }

      template <typename FIdx, typename Functor>
      static void extract_(const Configuration& config, const SRLSymbols& symbols, const X& x, int pred, int arg, Functor& F) {			  
	//phi secondary feats
	PhiSecondary<FIdx,Functor>(config, symbols, x, pred, arg, F);

	if (config.path) {
	  //phi interdep feats
	  PhiInterdep<FIdx,Functor>(config, symbols, x, pred, arg, F);
	  //phi path token feats
	  PhiPathToken<FIdx,Functor>(config, symbols, x, pred, arg, F);
	  //last phi path token
	  PhiPathLast<FIdx,Functor>(config, symbols, x, pred, arg, F);
	}
      }
      

      //typedef srl::Sentence X;
      //
      /** Johnasson secondary features */
      template <typename FIdx, typename Functor>
      static void PhiSecondary(const Configuration& config, const SRLSymbols& symbols, 
			       const X& x, int pred, int arg, Functor& F) {			       
	/*
	  Predicate word
	  Predicate POS
	  Argument word
	  Argument POS
	  Pred. + arg. words
	  Predicate word + label
	  Predicate POS + label
	  Argument word + label
	  Argument POS + label
	  Pred. + arg. word + label
	*/ 
	const int num_words = x.size();
	assert(pred >= 0);
	assert(arg >= 0);
	assert(pred < num_words);
	assert(arg < num_words);

	const typename X::Token& ptok = x.get_token(pred);
	const typename X::Token& atok = x.get_token(arg);
	//get the pred and arg word and fine pos
	typename FIdx::Tag pred_fpos = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(ptok.fine_pos()));
	typename FIdx::Tag arg_fpos = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(atok.fine_pos()));
	typename FIdx::Word pred_word = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(ptok.word()));
	typename FIdx::Word arg_word = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(atok.word()));
	//get the syn label of arg
	const DepVector<std::string>& dep_vect = x.dependency_vector();
	//check arg is in valid range
	assert(arg < static_cast<int>(dep_vect.size()));
	//get the head label pair for this arg
	const HeadLabelPair<std::string>& hlp = dep_vect.at(arg);
	std::string syn_label_arg = hlp.l;
	typename FIdx::Tag syn_label = FIdx::tag(symbols.map_field<SYNTACTIC_LABEL,typename FIdx::Tag>(syn_label_arg));
	if (config.unlabeled_syn){
	  syn_label = FIdx::tag(symbols.map_field<SYNTACTIC_LABEL,typename FIdx::Tag>(config.kNoLabel));
	}


	//Predicate word
	F( FIdx() << pred_word << FIdx::code(J_PRED_WORD, "J_PRED_WORD"));
	//Predicate POS
	F( FIdx() << pred_fpos <<
	   FIdx::code(J_PRED_POS, "J_PRED_POS"));

	//argument word
	F( FIdx() << arg_word << FIdx::code(J_ARG_WORD, "J_ARG_WORD"));
	//argument POS
	F( FIdx() << arg_fpos << 
	   FIdx::code(J_ARG_POS, "J_ARG_POS"));

	//Predicate + arg words
	F( FIdx() << pred_word << arg_word <<
	   FIdx::code(J_PRED_ARG_POS, "J_PRED_ARG_POS"));

	//same feature with label (syn?)
	//Predicate word + label

	//Predicate word
	F( FIdx() << pred_word << syn_label << FIdx::code(J_PRED_WORD_LABEL, "J_PRED_WORD_LABEL"));
	//Predicate POS
	F( FIdx() << pred_fpos << syn_label << FIdx::code(J_PRED_POS_LABEL, "J_PRED_POS_LABEL"));
	//argument word
	F( FIdx() << arg_word << syn_label << FIdx::code(J_ARG_WORD_LABEL, "J_ARG_WORD_LABEL"));
	//argument POS
	F( FIdx() << arg_fpos << syn_label << FIdx::code(J_ARG_POS_LABEL, "J_ARG_POS_LABEL"));
	//Predicate + arg words
	F( FIdx() << pred_word << arg_word << syn_label << FIdx::code(J_PRED_ARG_POS_LABEL, "J_PRED_ARG_POS_LABEL"));	   
      }

      
      static SynIntLabelPath ToIntPath(const Configuration& config, const SRLSymbols& symbols, 
				       const SynLabelPath& syn_path){
	//TODO precompute
	SynIntLabelPath new_path;
	for (auto it = syn_path.begin(); it != syn_path.end(); ++it){
	  string synl = *it;
	  //map the label
	  //typename FIdx::Tag syn_label = FIdx::tag( symbols.map_field<SYNTACTIC_LABEL,typename FIdx::Tag>(syn_label_arg));
	  int int_syn = symbols.map_field<SYNTACTIC_LABEL, int>(synl);
	  if (config.unlabeled_syn){
	    int_syn = symbols.map_field<SYNTACTIC_LABEL, int>(config.kNoLabel);
	  }
	  new_path.push_back(int_syn);
	}
	return new_path;
      }


      /** Johansson interdep features */
      template <typename FIdx, typename Functor>
      static void PhiInterdep(const Configuration& config, const SRLSymbols& symbols, 
			      const X& x, int pred, int arg, Functor& F) {
	/*
	  Path
	  Path + arg. POS
	  Path + pred. POS
	  Path + arg. word
	  Path + pred. word
	  Path + label
	  Path + arg. POS + label
	  Path + pred. POS + label
	  Path + arg. word + label
	  Path + pred. word + label
	*/

	/*the PATH feature that
	  represents the grammatical relation between pred-
	  icate and argument words. For instance, in Fig-
	  ure 1, we can represent the surface-syntactic re-
	  lation between the tokens fall and prices as the
	  string IM↑OPRD↑OBJ↓. */

	const typename X::Token& ptok = x.get_token(pred);
	const typename X::Token& atok = x.get_token(arg);
	//get the pred and arg word and fine pos
	typename FIdx::Tag pred_pos = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(ptok.fine_pos()));
	typename FIdx::Tag arg_pos = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(atok.fine_pos()));
	typename FIdx::Word pred_word = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(ptok.word()));
	typename FIdx::Word arg_word = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(atok.word()));
	//get the syn label of arg
	const DepVector<std::string>& dep_vect = x.dependency_vector();
	//check arg is in valid range
	assert(arg >= 0);
	assert(arg < static_cast<int>(dep_vect.size()));
	//get the head label pair for this arg
	const HeadLabelPair<std::string>& hlp = dep_vect.at(arg);
	std::string syn_label_arg = hlp.l;
	typename FIdx::Tag syn_label = FIdx::tag(symbols.map_field<SYNTACTIC_LABEL,typename FIdx::Tag>(syn_label_arg));
	if (config.unlabeled_syn){
	  syn_label = FIdx::tag(symbols.map_field<SYNTACTIC_LABEL,typename FIdx::Tag>(config.kNoLabel));
	}

	//get the syn path from pred to arg
	SynLabelPath synl_path = x.get_syn_path(pred, arg);
	SynIntLabelPath synl_int_path = ToIntPath(config, symbols, synl_path);
	UpDownPath ud_path = x.get_ud_path(pred, arg);

	typename FIdx::BigWord syn_ud_path = PathFeatures<FIdx>::path(synl_path, synl_int_path, ud_path);
	//typename FIdx::Path syn_ud_path = FIdx::path(synl_path, synl_int_path, ud_path);
									 
              

	//only the path
	F( FIdx() << syn_ud_path << FIdx::code(J_PATH, "J_PATH"));
	   
	//Path + arg. POS
	F( FIdx() << syn_ud_path << arg_pos << FIdx::code(J_PATH_ARG_POS, "J_PATH_ARG_POS"));
	   
	//Path + pred. POS
	F( FIdx() << syn_ud_path << pred_pos << FIdx::code(J_PATH_PRED_POS, "J_PATH_PRED_POS"));
	   
	//Path + arg. word
	F( FIdx() << syn_ud_path << arg_word << FIdx::code(J_PATH_ARG_WORD, "J_PATH_ARG_WORD"));
	   
	//Path + pred. word
	F( FIdx() << syn_ud_path << pred_word << FIdx::code(J_PATH_PRED_WORD, "J_PATH_PRED_WORD"));
	   

	//same features but with labels
	//Path + label
	F( FIdx() << syn_ud_path << syn_label << FIdx::code(J_PATH_LABEL, "J_PATH_LABEL"));
	   
	//Path + arg. POS + label
	F( FIdx() << syn_ud_path << arg_pos << syn_label << FIdx::code(J_PATH_ARG_POS_LABEL, "J_PATH_ARG_POS_LABEL") );
	   
	//Path + pred. POS + label
	F( FIdx() << syn_ud_path << pred_pos << syn_label << FIdx::code(J_PATH_PRED_POS_LABEL, "J_PATH_PRED_POS_LABEL") );
	   
	//Path + arg. word + label
	F( FIdx() << syn_ud_path << arg_word << syn_label<< FIdx::code(J_PATH_ARG_WORD_LABEL, "J_PATH_ARG_WORD_LABEL") );
	   
	//Path + pred. word + label
	F( FIdx() << syn_ud_path << pred_word << syn_label << FIdx::code(J_PATH_PRED_WORD_LABEL, "J_PATH_PRED_WORD_LABEL") );
	   
      }

      /** Added path token features */
      template <typename FIdx, typename Functor>
      static void PhiPathToken(const Configuration& config, const SRLSymbols& symbols, const X& x, int pred, 
			       int arg, Functor& F) {
	//options
	bool extended_path_token_feats_ = true;
	bool lluism_feats_ = true;
	/* For each dep head-mod  (bigram feats)
	 * word head - word mod
	 * pos head - pos mod
	 * word head of each one
	 * left or right concat with the two first
	 * going up or down
	 */

	const int num_words = x.size();
	typedef std::list<int> NodePath;
	NodePath pt = x.get_node_path(pred, arg);

	//var to check if there are holes 
	bool holes_at_left = false;
	bool holes_at_right = false;
	int max_hole_size = 0;

	//get an iterator
	auto it = pt.begin();
	assert(it != pt.end());
	//get the first node
	int previous = *it;

	//go to the next node
	++it;

	//loop over the nodes
	while (it != pt.end()) {
	  int current = *it;
	  int head = current;
	  int mod = previous;
	  if (current < 0) {
	    head = previous;
	    mod = -current;
	  }
	  //get the codifications
	  //what is the FIdx type: must be int
	  assert(head >= 0);
	  assert(mod >= 0);

	  // NEEDS A FIX
	  if (head <= num_words and mod < num_words) {
	    assert(head <= num_words);
	    assert(mod < num_words);
	    
	    typename FIdx::Tag head_pos = FIdx::rootTag();
	    typename FIdx::Word head_word = FIdx::rootWord();
	    std::string htok_cpos;

	    if (head != num_words){ //the head is NOT the root
	      const typename X::Token& htok = x.get_token(head);
	      assert(htok.fine_pos().size() < 100);
	      head_pos = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(htok.fine_pos()));
	      head_word = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(htok.word()));
	      htok_cpos = (htok.coarse_pos().empty())? htok.fine_pos() : htok.coarse_pos();
	    } else { 
	      //change root code
	      head = -1;
	    }

	    const typename X::Token& mtok = x.get_token(mod);
	    assert(mtok.fine_pos().size() < 100);


	    typename FIdx::Tag mod_pos = FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(mtok.fine_pos()));
	    typename FIdx::Word mod_word = FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(mtok.word()));

	    //cpos
	    //if (htok.coarse_pos().empty()){
	    //cerr << "warning: empty coarse pos " << endl;
	    //expecting a filled cpos, if not
	    //we will use the fine pos instead (dictionaries must match)
	    //}
	    const std::string& mtok_cpos = (mtok.coarse_pos().empty())? mtok.fine_pos() : mtok.coarse_pos();
	    
	    typename FIdx::Tag head_cpos = FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(htok_cpos));
	    typename FIdx::Tag mod_cpos = FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(mtok_cpos));
	    
	    //cpos of -1 and -2
	    if (extended_path_token_feats_) {
	      //add the coarse pos and lemma
	      F( FIdx() << head_cpos << FIdx::code(J_PATH_TOKEN_HEAD_CPOS, "J_PATH_TOKEN_HEAD_CPOS"));
	      F( FIdx() << mod_cpos << FIdx::code(J_PATH_TOKEN_MOD_CPOS, "J_PATH_TOKEN_MOD_CPOS"));
	    }
	    
	    //get individual words features
	    
	    F( FIdx() << head_word << FIdx::code(J_PATH_TOKEN_HEAD_WORD, "J_PATH_TOKEN_HEAD_WORD"));
	    F( FIdx() << head_pos << FIdx::code(J_PATH_TOKEN_HEAD_POS, "J_PATH_TOKEN_HEAD_POS"));
	    F( FIdx() << mod_word << FIdx::code(J_PATH_TOKEN_MOD_WORD, "J_PATH_TOKEN_MOD_WORD"));
	    F( FIdx() << mod_pos << FIdx::code(J_PATH_TOKEN_MOD_POS, "J_PATH_TOKEN_MOD_POS"));
	    
	    //now combinations of word and pos
	    F( FIdx() << head_word << mod_word <<
	       FIdx::code(J_PATH_TOKEN_WORD_WORD, "J_PATH_TOKEN_WORD_WORD"));
	    F( FIdx() << head_pos << mod_pos <<
	       FIdx::code(J_PATH_TOKEN_POS_POS, "J_PATH_TOKEN_POS_POS"));
	    //and direction
	    F( FIdx() << FIdx::dir(head < mod) << FIdx::code(J_PATH_TOKEN_DIR, "J_PATH_TOKEN_DIR"));
	    
	    //add up or down
	    F( FIdx() << FIdx::dir(current < 0) << FIdx::code(J_PATH_TOKEN_DOWN, "J_PATH_TOKEN_DOWN"));
	    
	    //dependency feats
	    if (extended_path_token_feats_) {
	      F( FIdx() << head_word << mod_word << mod_cpos <<
		 FIdx::code(J_PATH_TOKEN_WWP, "J_PATH_TOKEN_WWP"));
	      F( FIdx() << head_word << mod_word << head_cpos <<
		 FIdx::code(J_PATH_TOKEN_PWW, "J_PATH_TOKEN_PWW"));
	      F( FIdx() << head_word << head_cpos << mod_cpos <<
		 FIdx::code(J_PATH_TOKEN_WPP, "J_PATH_TOKEN_WPP"));
	      F( FIdx() << head_word << mod_word << head_cpos << mod_cpos <<
		 FIdx::code(J_PATH_TOKEN_WWPP, "J_PATH_TOKEN_WWPP"));
	    }
	    
	    //context feats
	    if (extended_path_token_feats_) {
	      F( FIdx() <<  FIdx::dir(mod == 0) << 
		 FIdx::code(J_PATH_TOKEN_NO_PREV_MOD, "J_PATH_TOKEN_NO_PREV_MOD"));
	      F( FIdx() << FIdx::dir(head == 0) <<
		 FIdx::code(J_PATH_TOKEN_NO_PREV_HEAD, "J_PATH_TOKEN_NO_PREV_HEAD"));
	      F( FIdx() << FIdx::dir(mod == num_words - 1) <<
		 FIdx::code(J_PATH_TOKEN_LAST, "J_PATH_TOKEN_LAST"));
	      F( FIdx() << FIdx::dir(head == num_words - 1) <<
		 FIdx::code(J_PATH_TOKEN_LAST, "J_PATH_TOKEN_LAST"));
	      
	      F( FIdx() << FIdx::dir(mod == 1) <<
		 FIdx::code(J_PATH_TOKEN_NO_PREV_2, "J_PATH_TOKEN_NO_PREV_2" ));
	      F( FIdx() << FIdx::dir(head == 1) <<
		 FIdx::code(J_PATH_TOKEN_NO_PREV_2, "J_PATH_TOKEN_NO_PREV_2"));
	      F( FIdx() << FIdx::dir(mod == num_words - 2) <<
		 FIdx::code(J_PATH_TOKEN_LAST_2, "J_PATH_TOKEN_LAST_2"));
	      F( FIdx() << FIdx::dir(head == num_words - 2) <<
		 FIdx::code(J_PATH_TOKEN_LAST_2, "J_PATH_TOKEN_LAST_2"));
	      
	      //if possible add a bigrams and trigrams of coarse pos
	      if (mod > 0) {
		const typename X::Token& mod_minus_1 = x.get_token(mod - 1);
		typename FIdx::Tag mod_cpos_minus_1 = 
		  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(mod_minus_1.coarse_pos()));
		F( FIdx() << mod_cpos_minus_1 << mod_cpos <<
		   FIdx::code(J_PATH_TOKEN_CTX_B1, "J_PATH_TOKEN_CTX_B1"));
		if (mod > 1) {
		  const typename X::Token& mod_minus_2 = x.get_token(mod - 2);
		  typename FIdx::Tag mod_cpos_minus_2 = 
		    FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(mod_minus_2.coarse_pos()));
		  F( FIdx() << mod_cpos_minus_1 << mod_cpos_minus_2 <<
		     FIdx::code(J_PATH_TOKEN_CTX_B2, "J_PATH_TOKEN_CTX_B2"));
		}
	      }
	    

	      //same for head
	      if (head > 0) {
		const typename X::Token& head_minus_1 = x.get_token(head - 1);
		typename FIdx::Tag head_cpos_minus_1 = 
		  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(head_minus_1.coarse_pos()));
		F( FIdx() << head_cpos_minus_1 << head_cpos <<
		   FIdx::code(J_PATH_TOKEN_CTX_B1, "J_PATH_TOKEN_CTX_B1"));
		if (head > 1) {
		  const typename X::Token& head_minus_2 = x.get_token(head - 2);
		  typename FIdx::Tag head_cpos_minus_2 = 
		    FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(head_minus_2.coarse_pos()));
		  F( FIdx() <<  head_cpos_minus_1 << head_cpos_minus_2 <<
		     FIdx::code(J_PATH_TOKEN_CTX_B2, "J_PATH_TOKEN_CTX_B2"));
		}
	      }
	    }
	    
	    //dependency distance
	    if (extended_path_token_feats_) {
	      //distance with a maximum only
	      const int dist = abs(mod - head);
	      F( FIdx() << FIdx::tag(dist) <<
		 FIdx::code(J_PATH_TOKEN_DIST, "J_PATH_TOKEN_DIST"));
	      F( FIdx() << mod_cpos << head_cpos << FIdx::tag(dist) << 
		 FIdx::code(J_PATH_TOKEN_DIST, "J_PATH_TOKEN_DIST"));
	      //distance binned concat with both cpos
	    }
	    
	    //dependency between
	    if (extended_path_token_feats_) {
	      //for every token in the middle 
	      //everything is concat with the two cpos
	      //add  cpos of head, cpos of mod, cpos middle
	      //
	      int start = head;
	      //if (start < 0) start  = 0;
	      
	      int end = mod;
	      assert(mod >= 0);
	      
	      if (end < start) {
		start = mod;
		end = head;
	      }
	      assert(start <= end);

	      //count also number of verbs in the middle
	      int num_verbs = 0;
	      //number of punctuation in the middle
	      int num_punct = 0;
	      //number of coordination in the middle
	      int num_coord = 0;
	      for (int i = start + 1; i < end; ++i) {
		assert(i >= 0);
		assert(i < num_words);
		
		//add the middle cpos as a feat
		const typename X::Token& middle = x.get_token(i);
		typename FIdx::Tag middle_cpos = 
		  FIdx::tag(symbols.map_field<COARSE_POS,typename FIdx::Tag>(middle.coarse_pos()));
		
		F( FIdx() << mod_cpos << head_cpos << middle_cpos <<
		   FIdx::code(J_PATH_TOKEN_BTW_POS, "J_PATH_TOKEN_BTW_POS"));
		
		//ask if verb, ask if coord ask if punc
		if (symbols.is_verb(middle.fine_pos()) ) {
		  num_verbs++;
		}
		if (symbols.is_punc(middle.fine_pos()) ) {
		  num_punct++;
		}
		if (symbols.is_coord(middle.fine_pos()) ) {
		  num_coord++;
		}
	      }
	      //then add the features
	      F( FIdx() << mod_cpos << head_cpos << FIdx::tag(num_verbs) <<
		 FIdx::code(J_PATH_TOKEN_BTW_PRED, "J_PATH_TOKEN_BTW_PRED"));
	      F( FIdx() << mod_cpos << head_cpos << FIdx::tag(num_punct) <<
		 FIdx::code(J_PATH_TOKEN_BTW_PUNCT, "J_PATH_TOKEN_BTW_PUNCT"));
	      F( FIdx() << mod_cpos << head_cpos << FIdx::tag(num_coord) <<
		 FIdx::code(J_PATH_TOKEN_BTW_COORD, "J_PATH_TOKEN_BTW_COORD"));
	      
	    } //end of if extended token
	    int hole_size = abs(head - mod);
	    if (hole_size > max_hole_size) max_hole_size = hole_size;
	    
	    //if there is at least one hole
	    if (hole_size > 0){
	      if (mod > head) holes_at_left = true;
	      if (mod < head) holes_at_right = true;
	    }
	  }

	  //go to the next node
	  previous = current;
	  if (previous < 0) previous = -previous;
	  ++it;
	} //end of node loop

	if (lluism_feats_){
	  //add hole feats
	  if (max_hole_size > 0){
	    if (max_hole_size > 6) max_hole_size = 6;
	    int  are_holes = 0;
	    F( FIdx() << FIdx::tag(are_holes) <<
	       FIdx::code(J_PATH_TOKEN_HOLES, "J_PATH_TOKEN_HOLES"));
	    int  max_size = max_hole_size + 4;
	    F( FIdx() << FIdx::tag(max_size) <<
	       FIdx::code(J_PATH_TOKEN_HOLES, "J_PATH_TOKEN_HOLES"));

	    if (holes_at_left and holes_at_right){
	      int are_holes = 1;
	      F( FIdx() << FIdx::tag(are_holes) <<
		 FIdx::code(J_PATH_TOKEN_HOLES, "J_PATH_TOKEN_HOLES"));
	    } else if (holes_at_left){
	      int are_holes = 2;
	      F( FIdx() << FIdx::tag(are_holes) <<
		 FIdx::code(J_PATH_TOKEN_HOLES, "J_PATH_TOKEN_HOLES"));
	    } else {
	      int are_holes = 3;
	      F( FIdx() << FIdx::tag(are_holes) <<
		 FIdx::code(J_PATH_TOKEN_HOLES, "J_PATH_TOKEN_HOLES"));
	    }

	  } else {
	    int are_holes = 4;
	    F( FIdx() << FIdx::tag(are_holes) <<
	       FIdx::code(J_PATH_TOKEN_HOLES, "J_PATH_TOKEN_HOLES"));
	  }

	} //end of last lluism feats

	//check the last feature code fits in 8 bit
	assert(J_PATH_TOKEN_HOLES < 256);

      }

      static bool IsLemmaToBe(const SRLSymbols& symbols, const X& x, int token){
	//get this token
	const X::Token& tok = x.get_token(token);
	//get the lemma
	return tok.lemma().compare("be") == 0;
      }

      static bool IsValidInbetween(const SRLSymbols& symbols, const X& x, int token){
	//get this token
	const X::Token& tok = x.get_token(token);
	//get the fine pos
	return symbols.is_rb(tok.fine_pos()) or symbols.is_to(tok.fine_pos())
	  or symbols.is_modal(tok.fine_pos());
      }

      /** Added path token features */
      template <typename FIdx, typename Functor>
      static void PhiPathLast(const Configuration& config, const SRLSymbols& symbols, 
			      const X& x, int pred, int arg, Functor& F) {			      
	//feature holes
	//get the size of the biggest hole if any and tell if holes are
	//to left, to right or mixed

	//cerr << "phi path last " << endl;
	const typename X::Token& pred_tok = x.get_token(pred);
	const typename X::Token& arg_tok = x.get_token(arg);
	int num_words = x.size();

	//compute voice feat for this pred
	bool passive = false;
	if (symbols.is_past_participle(pred_tok.fine_pos())) { 
	  if (pred > 0) {
	    //if we find at the end a VBN
	    bool end = false;
	    int i = pred - 1;
	    while (!end) {
	      if (IsLemmaToBe(symbols, x, i)) {
		passive = true;
	      }
	      //check is a valid inbetween particle
	      if (IsValidInbetween(symbols, x, i)) {
		//ok
	      } else {
		end = true;
	      }
	      i = i - 1;
	      if (i < 0) end = true;
	    }
	  }
	}

	typename FIdx::Tag arg_pos = 
	  FIdx::tag(symbols.map_field<FINE_POS,typename FIdx::Tag>(arg_tok.fine_pos()));

	F( FIdx() << FIdx::dir(passive) << 
	   FIdx::code(J_PATH_TOKEN_VOICE, "J_PATH_TOKEN_VOICE"));
	if (passive) {
	  F( FIdx() << arg_pos <<
	     FIdx::code(J_PATH_TOKEN_VOICE_POS, "J_PATH_TOKEN_VOICE_POS"));
	  if (arg < pred) {
	    F( FIdx() << FIdx::tag(1) <<
	       FIdx::code(J_PATH_TOKEN_VOICE_DIR, "J_PATH_TOKEN_VOICE_DIR"));
	  } else if (arg == pred) {
	    F( FIdx() << FIdx::tag(2) <<
	       FIdx::code(J_PATH_TOKEN_VOICE_DIR, "J_PATH_TOKEN_VOICE_DIR"));
	  } else {
	    F( FIdx() << FIdx::tag(3) <<
	       FIdx::code(J_PATH_TOKEN_VOICE_DIR, "J_PATH_TOKEN_VOICE_DIR"));
	  }
	}

	//head of PP
	//if the arg is a prep
	if (symbols.is_preposition(arg_tok.fine_pos()) and arg < num_words - 1){
	  //find an NN
	  bool end = false;
	  int noun_found = -1;
	  int i = arg + 1; //at most arg == num_words - 1
	  while (!end){
	    assert(i < num_words);

	    const typename X::Token& i_token = x.get_token(i);
	    bool is_noun = symbols.is_noun(i_token.fine_pos());

	    //if we already found a noun and we find one again
	    //we udate the pointer
	    if (noun_found != -1 and is_noun){
	      //update
	      noun_found = i;
	    }
	    //if never found a noun update
	    else if (noun_found == -1 and is_noun){
	      noun_found = i;
	    }
	    // if we found a noun but now it is not, keep the last and exit
	    else if (noun_found != -1 and !is_noun){
	      end = true;
	    }

	    i++;
	    if (i == num_words) end = true;
	  } //end of finding nouns

	  //if we found the name, add it
	  if (noun_found != -1){
	    const typename X::Token& noun_tok = x.get_token(noun_found);
	    typename FIdx::Word noun_word = 
	      FIdx::word(symbols.map_field<WORD,typename FIdx::Word>(noun_tok.word()));

	    F( FIdx() << noun_word <<
	       FIdx::code(J_PATH_TOKEN_PP_HEAD, "J_PATH_TOKEN_HEAD_WORD" ));
	  } else {
	    F( FIdx() << FIdx::tag(0) <<
	       FIdx::code(J_PATH_TOKEN_PP_HEAD, "J_PATH_TOKEN_PP_HEAD" ));
	  }

	} //end of if there is a PP


      }
    };

    

  }
}

#endif /* FTEMPLATES_SRL */
