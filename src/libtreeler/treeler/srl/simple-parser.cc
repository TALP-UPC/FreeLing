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


#include "treeler/srl/simple-parser.h"

namespace treeler {

  namespace srl {

    const int SimpleParser::kRoot = -1;

    /**
     * for the moment, the preds already read.
     * should change to a kind of prediction in the case predicate tokens are not marked or to predicate mark detection in case they are
     */
    void SimpleParser::get_pred_candidates(const srl::Sentence& x, list<int>& result) const {
      result = x.get_predicates();
    }
    

    /** Returns the list of possible role labels for the pred arg candidate */
    void SimpleParser::get_role_candidates(const srl::Sentence& x, const int pred, const int arg, list<int>& candidates) const {
      //not using filters, returning all possible labels
      if (not _config.apply_pos_filter){
	for (int i = 0; i < _config.L; ++i){
	  candidates.push_back(i);
	}
      } 
      else {
	const string& arg_pos = x.get_token(arg).fine_pos();
	const string& pred_pos = x.get_token(pred).fine_pos();
	if (symbols_.is_pos_allowed(pred_pos, arg_pos)){
	  for (int i = 0; i < _config.L; ++i){
	    const std::string& role_str = symbols_.d_semantic_labels.map(i);
	    if (i == 0 or symbols_.is_pos_arg_allowed(pred_pos, arg_pos, role_str)) {						      
	      candidates.push_back(i);
	    }
	  }
	} else {
	  //only allow 0 (null) role
	  candidates.push_back(0);
	}
      }
    }

    /** 
     * Get arg candidates for a predicate. Resets result and adds those candidates
     */  
    void SimpleParser::get_arg_candidates(const srl::Sentence& x, const DepTreeT& dtree, 
					  int pred, list<int>& result) const {
      
      const DepVector<string>& dvec = x.dependency_vector();      

      if (_config.scope==DIRECT or _config.scope==ANCESTOR) {
	set<int> candidates;
	GetChildrenOfAncestors(pred, dvec, &candidates);
	assert(result.empty());
	result.insert(result.end(), candidates.begin(), candidates.end());	
      }
      else if (_config.scope==DIRECT) { // or _config.scope==ANCESTOR) {

	if (_config.self) result.push_back(pred);

	const DepTree<string>* ptree = dtree.terminal(pred); 
	int lastidx = -1; // tracks the index of the token we ascend from in the tree
	while (ptree != NULL) {
	  for (int i=0; i<ptree->nlc(); ++i) {
	    const DepTreeT* ctree = ptree->lc(i); 
	    int cidx = ctree->idx(); 
	    if (cidx != lastidx) 
	      result.push_back(cidx);
	  }
	  for (int i=0; i<ptree->nrc(); ++i) {
	    const DepTreeT* ctree = ptree->rc(i); 
	    int cidx = ctree->idx(); 
	    if (cidx != lastidx) 
	      result.push_back(cidx);
	  }
	  lastidx = ptree->idx();
	  if (_config.scope==DIRECT or not ptree->has_parent()) {
	    ptree = NULL;
	  }
	  else {
	    ptree = ptree->parent(); 
	  }
	  // ptree = (_config.scope==ANCESTOR and ptree->has_parent()) ? ptree->parent() : NULL; 
	}
      }
      else { // _config.scope == ALL
	const int num_words = dvec.size();
	for (int i = 0; i < num_words; ++i){
	  if (not _config.self or i!=pred) {
	    result.push_back(i);
	  }
	}
      }
    }
    
    /**
     *  adds the children of node and recusivelly calls the same function for
     *  the head of the node 
     */
    void SimpleParser::GetChildrenOfAncestors(int node, const DepVector<string>& dep_vec, set<int>* candidates) {
      //get all the sons of node and add them
      assert(node != kRoot);
      
      //loop and get the sons
      int mod = 0;
      //for each mod
      for (auto it = dep_vec.begin(); it != dep_vec.end(); ++it){
	//get the head of this given mod
	int head = it->h;
	if (head == node){
	  //if the head of the mod is the node, mod is a son, add it
	  candidates->insert(mod);
	}
	mod++;
      }
      
      //if we are not at the root go upwards
      int head_of_node = dep_vec.at(node).h;
      if (head_of_node != kRoot){
	//recursive call
	GetChildrenOfAncestors(head_of_node, dep_vec, candidates);
      }
    }
    
    

    // a decompose function
    // X is a sentence,
    // Y a pred args set
    // Label a vector of SRLPart parts
    void SimpleParser::decompose(const X& sentence, const PredArgSet& y, Label<R>& parts) const {
      const PredArgSet& pas = y;
      //loop over candidates
      //generate null parts
      
      DepTree<string> dtree = DepTree<string>::convert(sentence.dependency_vector()); 

      list<int> preds;
      //simply gets all sentence preds as candidates
      get_pred_candidates(sentence, preds);
      
      for (auto pred = preds.begin(); pred != preds.end(); ++pred) {
	//get the arg candidates
	list<int> args;
	get_arg_candidates(sentence, dtree, *pred, args);
	//                        cout << "pred: " << *pred << "\nargs: ";
	//                        for_each(args.begin(), args.end(), print_elem<int>);
	//                        cout << "\n";
	for (auto arg = args.begin(); arg != args.end(); ++arg) {
	  //                                cout << "getting for: " << *pred << ", ";
	  //                                cout << *arg << "\n";
	  
	  std::string str_role = "_";
	  int int_role = symbols_.d_semantic_labels.map(str_role);
	  //ask to pas if there is an arg
	  auto it = pas.find(*pred);
	  if (it != pas.end()){
	    auto it_arg = it->second.find(*arg);
	    if (it_arg != it->second.end()){
	      str_role = it_arg->second;
	      int_role = symbols_.d_semantic_labels.map(str_role);
	    }
	  }
	  //cerr << "str " << str_role << " int " << int_role << endl;
	  
	  PartSRL p(*pred, *arg, int_role);
	  parts.push_back(p);
	}
      }
      return;
      
      //iterate over pred-args
      for (auto it = pas.begin(); it != pas.end(); ++it){
	int pred_pos = it->first;
	for (map<int, string>::const_iterator arg = it->second.begin(); arg != it->second.end(); ++arg) {
	  int arg_pos = arg->first;
	  string arg_role = arg->second;
	  //cerr << "pred " << pred_pos << " arg " << arg_pos << " " << arg_role;
	  int role_int = symbols_.d_semantic_labels.map(arg_role);
	  //cerr << " " << role_int << endl;
	  //cerr << "arg role " << arg_role << endl;
	  //check we are mapping to a correct value
	  assert(role_int >= 0);
	  assert(role_int < _config.L);
	  
	  //build the SRLPart
	  PartSRL p(pred_pos, arg_pos, role_int);
	  parts.push_back(p);
	}
      }
    }

  }
}
