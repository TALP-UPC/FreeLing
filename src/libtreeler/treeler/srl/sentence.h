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
 * \file   sentence.h
 * \brief  Declaration of the class srl::Sentence
 */

#ifndef TREELER_SRL_SENTENCE_H
#define TREELER_SRL_SENTENCE_H

#include <vector>
#include <list>
#include <iostream>
#include "treeler/base/basic-sentence.h"
#include "treeler/dep/dep-tree.h"
#include "treeler/srl/srl.h"
#include "treeler/srl/paths-container.h"

namespace treeler {

  namespace srl {

    /**
     * \brief A sentence for SRL
     * \ingroup srl
     */
    class Sentence : public BasicSentence<string,string> {
    public:
      typedef BasicSentence<string,string> BaseType;
      typedef BaseType::Token Token;
      typedef BaseType::Lex Lex;
      typedef BaseType::Tag Tag;
      
      Sentence()
	: syn_and_preds_loaded_(false)
      {}
      
      /** Adds the syntax and predicates,
      * for lemma_senses we expect as input a vector of _ and lemma.sense
      */

      /** Loads the pred arg set and updates paths */
      void add_gold_srl(const PredArgSet& pred_arg_set){
        //save the predicates list
        pred_list_ = pred_arg_set.predicate_list();
        //check is not previously saved
        assert(gold_pred_args_.empty());
        //copy
        gold_pred_args_ = pred_arg_set;
      }
      
      /** Returns the list of predicates */
      const list<int>& get_predicates() const {
        return pred_list_;
      }

      /** Get the gold pred senses */
      const vector<string> & get_pred_senses() const {
	return gold_senses_;
      }

      /** Get the gold pred arg set */
      const PredArgSet& get_gold_pred_args() const {
        std::cerr << "warning -> gold access !" << std::endl;
        return gold_pred_args_;
      }

      //get path functions
      /** returns the path from s to e as a list of nodes */
      const NodePath& get_node_path(int s, int e) const {
        assert(syn_and_preds_loaded_);
        return paths_.get_node_path(s, e);
      }

      /** returns the path from s to e as a list of syn labels */
      const SynLabelPath& get_syn_path(int s, int e) const {
        assert(syn_and_preds_loaded_);
        return paths_.get_syn_path(s, e);
      }

      /** returns the path from s to e as a list of up/down transitions */
      const UpDownPath& get_ud_path(int s, int e) const {
        assert(syn_and_preds_loaded_);
        return paths_.get_ud_path(s, e);
      }

      /** returns the gold syn deps */
      const DepVector<string>& get_gold_deps() const {
        std::cerr << "warning -> gold access !" << std::endl;
        // the parser should be configured with
        //use_gold_syntax = false; 
        //we do not allow access to gold syntax
        return gold_deps_;
      }

      /** returns the predicted syn deps */
      const DepVector<string>& dependency_vector() const {
        return pred_deps_;
      }


   private:
      /** The gold predicted dependency structure, in the form of a
       vector of dependencies */
      DepVector<string> gold_deps_;
      /** The predicted dependency structure, in the form of a
	  vector of dependencies */
      DepVector<string> pred_deps_;
      /** the gold pred arg set */
      PredArgSet gold_pred_args_;

    public:
      /** Vector of readed or gold senses */
      std::vector<std::string> gold_senses_;
      /** Vector of expected roles for each predicate */
      std::vector<std::string> expected_roles_;
      /** the paths from all predicates to argument candidates */
      PathsContainer paths_;

    private:
      /** the list of already identified predicates */
      list<int> pred_list_;
      /** flag the syntax and predicates are loaded */
      bool syn_and_preds_loaded_;

    private: // methods

      /** Builds paths. */
      void build_paths_from_syn_and_preds() {
        assert(!syn_and_preds_loaded_); // do not read twice
        paths_.Init(pred_list_, dependency_vector());
        syn_and_preds_loaded_ = true;
      }

    public:

      void add_syn_and_preds(const DepVector<string>& pred_deps,
                             const PossiblePreds &lemma_senses) {
        pred_deps_ = pred_deps;

        gold_senses_ = vector<string>(pred_deps.size(),"_");
        expected_roles_ = vector<string>(pred_deps.size(),"_");

        // for each word that is a predicate
        pred_list_.clear();
        for (auto i=lemma_senses.begin(); i!=lemma_senses.end(); i++) {
          // add word position to predicate list
          pred_list_.push_back(i->first);

          // add word senses into gold_senses (format "eat.01|ingest.02")  <-- XAVI, aixo canvia-ho si vols
          string& sense = gold_senses_[i->first];
	  {
	    bool first = true;
	    for (auto j=i->second.predsense.begin(); j!=i->second.predsense.end(); j++) {
	      if (first) {
		sense = j->first+"."+j->second;
		first = false;
	      }
	      else {
		sense = sense + "|" + j->first + "." + j->second;
	      }
	    }
	  }

          // add roles expected for this predicate into expected_roles (format "A0 A1 A3")  <-- XAVI, aixo canvia-ho si vols
          string& roles = expected_roles_[i->first];
	  {
	    bool first = true;
	    for (auto j=i->second.possible_args.begin(); j!=i->second.possible_args.end(); j++) {
	      if (first) {
		roles = j->first;
		first = false;
	      }
	      else {
		roles += " " + j->first;
	      }
	    }
	  }
	}
	  
	build_paths_from_syn_and_preds();
      }


    }; //class Sentence
    




  } //namespace srl
} //namespace treeler

#endif /* TREELER_SRL_SENTENCE_H */
