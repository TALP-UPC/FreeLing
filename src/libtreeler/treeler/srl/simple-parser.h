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
 * \file   simple-parser.h
 * \brief  Declaration of class SimpleParser
 */
#ifndef TREELER_SRL_SIMPLE_PARSER_H
#define TREELER_SRL_SIMPLE_PARSER_H

#include <cassert>
#include <iostream>
#include <algorithm>
#include <set>
#include <list>
#include <limits>

#include "treeler/base/label.h"
#include "treeler/srl/srl.h"
#include "treeler/srl/sentence.h"

using namespace std; 



namespace treeler {

  namespace srl {

    /**
     * \brief A simple SRL parser that looks for arguments in direct
     * modifiers of the predicate ancestors
     */
    class SimpleParser {
    public:
      
      typedef srl::Sentence X; 
      typedef PartSRL R; 
      typedef DepTree<string> DepTreeT; 

      enum SyntacticScope { DIRECT, ANCESTOR, ALL  };
      
      /** 
       *  \brief Configuration parameters of this parser 
       */
      struct Configuration {
        Configuration() 
	  : scope(DIRECT), self(true), L(1), use_gold_syntax(false),
	    apply_pos_filter(false), score_blockwise(true), verbose(0)
	{}
	
        // the scope within the syntactic dependency tree where
        // argument candidates are listed, relative to the predicate
        SyntacticScope scope;

	// whether predicates are candidate arguments
	bool self;
	
        // the number of role labels
        int L;
	
        // whether to use gold syntax -- obsolete
        bool use_gold_syntax;
	
        //whether to apply POS filters
        bool apply_pos_filter;
	
	bool score_blockwise; 
	
	// verbosity of parser
	int verbose;
      };
      
    private:
      Configuration _config;
      const SRLSymbols& symbols_;

      static const int kRoot; // the index of the root token in a dependency tree

    public:

      /** Main constructor with symbols */
      SimpleParser(const SRLSymbols &s)
	: symbols_(s)
      {	
        //load here the number of sem labels
        _config.L = s.d_semantic_labels.size();
      }
      
      /** gives access to the config struct */
      Configuration& config(){
        return _config;
      }      


      /** Returns the predicate-argument set */
      template <typename S>
      double argmax(const X& sentence, S& scores, PredArgSet& y_pred_arg_set) {
        Label<R> y_label;
        return argmax(sentence, scores, y_pred_arg_set, y_label);
      }
      
      /** Returns the predicate-argument set as a collection of parts */
      template <typename S>
      double argmax(const X& sentence, S& scores, Label<R>& y_label) {
        PredArgSet y_pred_arg_set;
        return argmax(sentence, scores, y_pred_arg_set, y_label);
      }
      
      /** Returns the predicate-argument as a structure and as a collection of parts */
      template <typename S>
      double argmax(const X& sentence, S& scores, PredArgSet& y_pred_arg_set, Label<R>& y_label);
      
      /** Turns a PredArgSet into a set of parts */
      void decompose(const X& sentence, const PredArgSet& y, Label<R>& parts) const;


    private: 

      template <typename Scores>
      int predict(const srl::Sentence& s, Scores& scores, int pred, int arg, double *score);
      

      /**
       *  obtains a set of predicates for the sentence
       */
      void get_pred_candidates(const srl::Sentence& s, list<int>& result) const;
      
      void get_arg_candidates(const srl::Sentence& s, const DepTreeT& dtree, 
			      int pred, list<int>& result) const;

      void get_role_candidates(const srl::Sentence& s, const int pred, const int arg, 
			       list<int>& candidates) const;

      /** adds the children of a node and recusivelly calls the same function for the head of the node */	  
      static void GetChildrenOfAncestors(int node, const DepVector<string>& dep_vec, set<int>* candidates);
            
    };



    /**** IMPLEMENTATION ****/

    template <typename S>
    double SimpleParser::argmax(const X& x, S& scores, 
				PredArgSet& y_pred_arg_set, Label<R>& y_label)    
    {
      double score_sum = 0;

      DepTree<string> dtree = DepTree<string>::convert(x.dependency_vector()); 

      y_pred_arg_set.sentence_token_count = x.size();
      
      list<int> preds;
      // simply gets all sentence preds as candidates
      get_pred_candidates(x, preds);
      
      for (auto pred = preds.begin(); pred != preds.end(); ++pred) {
	
	//first assign the pred sense
	//get the token lemma
	std::string lemma = x.get_token(*pred).lemma();
	//assign a default .01 sense
	y_pred_arg_set[*pred].sense = x.get_pred_senses()[*pred];

	if (_config.verbose) {
	  cerr << "simple-parser : sentence " << x.id() << " predicate " << *pred << " "  << x.get_token(*pred).word() << " sense : " <<  x.gold_senses_[*pred] << endl;
	  cerr << "simple-parser : sentence " << x.id() << " predicate " << *pred << " "  << x.get_token(*pred).word() << " expected roles : " <<  x.expected_roles_[*pred] << endl;
	}

	
	//get the candidate args, considering the scope
	list<int> args;
	get_arg_candidates(x, dtree, *pred, args);
	for (auto arg = args.begin(); arg != args.end(); ++arg) {
	  double part_score = 0;
	  //get the best label for this candidate arg
	  const int rolelabel = predict(x, scores, *pred, *arg, &part_score);
	  if (rolelabel != 0) {
	    const string& string_role = symbols_.d_semantic_labels.map(rolelabel);
	    y_pred_arg_set[*pred][*arg] = string_role;	    
	  }	
	  //add to the list of parts
	  R part(*pred, *arg, rolelabel);
	  y_label.push_back(part);
	  
	  score_sum = score_sum + part_score;
	}//end of arg candidates
      }//end of for all preds
      
      return score_sum;
    }


    /**
     * predict a srl label for a pred and arg in a sentence
     */
    template <typename Scores>
    int SimpleParser::predict(const srl::Sentence& x, Scores& scores, int pred, int arg, double *score) {
      int selected_role = 0;
      
      //the pos of the arg token
      list<int> role_candidates;
      get_role_candidates(x, pred, arg, role_candidates);

      if (_config.verbose==1) {
	cerr << "simple-parser : " << x.id() << " " << pred << " "  << arg << " : " << x.get_token(pred).word() << " " <<  x.get_token(arg).word() << " : " << flush;
      }

      if (_config.score_blockwise) {
	PartSRL p(pred, arg, 0);
	vector<double> s = scores.score_block(p, role_candidates); 

	// search for max
	double max_score = -numeric_limits<double>::max();
	size_t i = 0; 
	for (int role : role_candidates) {
	  if (s[i] > max_score){
	    max_score = s[i];
	    selected_role = role;
	  }	  
	  if (_config.verbose>1) {
	    string rstr = symbols_.d_semantic_labels.map(role);
	    cerr << "simple-parser (B) : " << x.id() << " " << pred << " "  << arg << " " << i << "/" << role << "/" << rstr << " " 
		 << s[i] << " " << (s[i]==max_score ? "*" : "") << endl; 	    
	  }	  
	  ++i;
	}
	if (_config.verbose==1) {
	  for (int role : role_candidates) {
	    string rstr = symbols_.d_semantic_labels.map(role);
	    string best = role==selected_role ? " *" : "";
	    cerr << " " << rstr << " (" << s[role] << best << ")";
	  }
	}
      }
      else {
	double max_score = -numeric_limits<double>::max();
	for (auto it = role_candidates.begin(); it != role_candidates.end(); ++it){
	  //build a part
	  const int role = *it;
	  PartSRL p(pred, arg, role);
	  
	  //score this part
	  double sc = scores(p);
	  if (sc > max_score){
	    max_score = sc;
	    selected_role = p.rolelabel();
	  }
	  
	  if (_config.verbose>1) {
	    string rstr = symbols_.d_semantic_labels.map(role);
	    //string rstr = symbols_.map_field<SEMANTIC_LABEL,string>(role);
	    cerr << "simple-parser : " << x.id() << " " << pred << " "  << arg << " " << role << "/" << rstr << " " 
		 << sc << " " << (sc==max_score ? "*" : "") << endl; 	    
	  }
	}
      }
      if (_config.verbose==1) cerr << endl;

      return selected_role;
    }
 
  } //end of namespace srl

} //end of namespace treeler

#endif
