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
 * \file   paths_container.h
 * \brief  Has all the paths
 */

#ifndef TREELER_SRL_PATHS_H
#define TREELER_SRL_PATHS_H

#include <vector>
#include "treeler/base/basic-sentence.h"
#include "treeler/dep/dep-tree.h"
#include "treeler/srl/paths-defs.h"

namespace treeler {

  namespace srl {
    /** Computes list of children given a dep vector */
    class Children {
    public:
      Children(const DepVector<string>& dep_vector): kRoot_(-1) {
	//the number of words
	int n = dep_vector.size();
	children_.resize(n);
	
	//root is -1, thus we have to get the children of -1, the root
	for (int i = -1; i < n; ++i) {
	  //get i as mod
	  int head = i;
	  for (int m = 0; m < n; ++m){
	    //get the head of m
	    if (dep_vector.at(m).h == head){
	      if (head == kRoot_){
		children_of_root_.push_back(m);
	      } else {
		children_.at(head).push_back(m);
	      }
	    }
	  }
	}
      }
    
      /** return the children of h */
      const list<int>& at(int h) const {
	if (h == kRoot_){
	  return children_of_root_;
	}
	assert(h >= 0);
	assert(h < static_cast<int>(children_.size()));
	return children_.at(h);
      }
      
    private:
      /** the root position */
      const int kRoot_;
      /** children for the root */
      list<int> children_of_root_;
      /** a vector from node to children of this node */
      vector<list<int>> children_;
    }; //end of class Children
    

    /**
     * \brief A class to have paths
     * \ingroup srl
     */
    class PathsContainer {
    public:
      
      PathsContainer(): kUp_(true), kDown_(false), kRoot_(-1), num_words_(0)
      {}
      
      void Init(const list<int>& pred_list, const DepVector<string>&
        dep_vector){
        num_words_ = dep_vector.size();

        assert(node_paths_.empty());
        const int num_words2 = num_words_ * num_words_;
        node_paths_.resize(num_words2);
        syn_label_paths_.resize(num_words2);
        ud_paths_.resize(num_words2);

        //compute the children of dep_vector
        Children children(dep_vector);

        //build paths starting only from predicates
        for (auto it = pred_list.begin(); it != pred_list.end(); ++it) {
          //explore the paths for this predicate
          const int pred = *it;
          list<int> initial_path;
          initial_path.push_back(pred);
          bool go_only_down = false;
          ComputeAllPaths(pred, pred, dep_vector, children, initial_path, go_only_down);
          //now all paths from pred to all reachable args are filled
          //PrintPaths(pred);

       }
      }

      void PrintPaths(int pred) const {
          cerr << "computing all paths for pred " << pred << endl;
          for (int m = 0; m < num_words_; ++m){
            cerr << "m " << m << " " ;
            cerr << PathToString(pred, m);
          }
      }

      string PathToString(int s, int e) const {
	ostringstream oss; 
        const NodePath& np = get_node_path(s, e);
        for (auto it = np.begin(); it != np.end(); ++it){
          auto e = *it;
          oss << e << " ";
        }
        return oss.str();

        const SynLabelPath& sp = get_syn_path(s, e);
        for (auto it = sp.begin(); it != sp.end(); ++it){
          auto e = *it;
          oss << e << " ";
        }
        oss << oss.str();

        const UpDownPath& ud = get_ud_path(s, e);
        for (auto it = ud.begin(); it != ud.end(); ++it){
          auto e = *it;
          oss << e << " ";
        }
	return oss.str();
      }

      /** Get the path from s to e */
      const NodePath& get_node_path(int s, int e) const{
        const NodePath& p = node_paths_.at(Index(s,e));
        if (p.empty()){
          cerr << "empty path from s " << s << " to " << e << endl;
        }
        assert(!p.empty());
        return p;
      }

      /** Get the syn labels from s to e */
      const SynLabelPath& get_syn_path(int s, int e) const{
        const SynLabelPath& p = syn_label_paths_.at(Index(s,e));
        if ((s != e) and p.empty()){
          cerr << "asking for path from s " << s << " to  " << e << endl;
        }
        if (s != e) assert(!p.empty());
        return p;
      }

      /** Get the up-down path from s to e */
      const UpDownPath& get_ud_path(int s, int e) const{
        const UpDownPath& p = ud_paths_.at(Index(s,e));
        assert(!p.empty());
        return p;
      }



    private:
      /** arc upwards */
      const bool kUp_;
      /** arc downwards */
      const bool kDown_;
      /** the root node identifier*/
      const int kRoot_;
      /** all paths of nodes*/
      vector<NodePath> node_paths_;
      /** all paths of syn labels */
      vector<SynLabelPath> syn_label_paths_;
      /** all up-down paths */
      vector<UpDownPath> ud_paths_;
      /** the number of words */
      int num_words_;
      /** vector of predicates */
      vector<bool> is_predicate_;

      int Index(int s, int e) const {
        assert(s >= 0);
        assert(e >= 0);
        assert(s < num_words_);
        assert(e < num_words_);
        return s*num_words_ + e;
      }

      /** Compute paths from the depvector */
      void ComputeAllPaths(int source, int node, const DepVector<string>& dep_vector, const
        Children& children, const list<int>& prev_path, bool go_only_down){
        if (node == -1) assert (go_only_down);
        assert(node >= -1);
        assert(node < num_words_);
        //add the initial path to the path list
        assert(prev_path.size() <= 100); //hardcoded max path len

        if (node != -1){ //we will not need paths to the root
          AddPath(source, node, prev_path, dep_vector);
        }
        int last_node = prev_path.back();
        if (last_node < 0) last_node = -last_node;
        //check the path ends where we expect to
        assert(last_node == node or (node == -1 and last_node == num_words_) );

        if (!go_only_down){
          //go up and recursively call
          if (node != kRoot_){
            assert(node >= 0);
            assert(node <= static_cast<int>(dep_vector.size()));

            int head = dep_vector.at(node).h;
            if (head != kRoot_){ //for czech multiroots
              //add the head as a path node
              list<int> current_path = prev_path;
              current_path.push_back(head);
              //cerr << "going upwards " << endl;
              ComputeAllPaths(source, head, dep_vector, children, current_path, false);
            } else {
              list<int> current_path = prev_path;
              current_path.push_back(num_words_); //special code for root (it can't be -1)
              //cerr << "going upwards " << endl;
              ComputeAllPaths(source, head, dep_vector, children, current_path, true);
            }
          }
        } //end of go only down

        //call go downs
        for (auto it = children.at(node).begin(); it != children.at(node).end(); ++it){
          //do not revisit the source
          const int child = *it;

          
            //get a copy of the path and add the child
            list<int> current_path = prev_path;
            //flag as minus the descendant nodes
            current_path.push_back(-child);
            //AddPath(source, child, current_path, dep_vector);

            //Call go down
            ComputeAllPaths(source, child, dep_vector, children, current_path, true);
        } //end for all children


      }

      /** Adds a node path and computes the syn label and up-down path */
      void AddPath(int source, int end, const list<int>& node_path, 
        const DepVector<string>& dep_vector){

        assert(!node_path.empty());
        assert(source == node_path.front());
        assert(-end == node_path.back() or end == node_path.back());

        
        const int idx = Index(source, end);
        assert(idx < static_cast<int>(node_paths_.size()));

        bool update = false;
        if (node_paths_.at(idx).empty()) {
          node_paths_.at( idx ) = node_path;
          update = true;
        } else if (node_paths_.at(idx).size() > node_path.size()){
          node_paths_.at( idx ) = node_path;
          update = true;
        }


        //build the up/down path
        //build the syn label path
        list<bool> updown_path;
        list<string> syn_path;

        int prev_node = source;
        for (auto it = node_path.begin(); it != node_path.end(); ++it){
          int node = *it;
          
          assert(prev_node >= 0);
          assert(prev_node <= static_cast<int>(dep_vector.size()));
          //the node is a std node
          if (node >= 0){
            //add as synlabel the label of the previous one
            //if we are at first node the prev is not valid
            //syn paths are always shorter
            if (prev_node != node){
              if (prev_node == static_cast<int>(dep_vector.size())){
                syn_path.push_back("root");
              } else{
                string synl = dep_vector.at(prev_node).l;
                syn_path.push_back(synl);
              }
            }
            //ud down and syn paths
            updown_path.push_back(kUp_);
            prev_node = node;
          } else { //the node is the child of the previous
            //add as synlabel the label of the current
            string synl = dep_vector.at(-node).l;
            syn_path.push_back(synl);
            updown_path.push_back(kDown_);
            prev_node = -node;
          }
        }
        assert(idx < static_cast<int>(syn_label_paths_.size()));
        if (update) syn_label_paths_.at(idx) = syn_path;
        assert(idx < static_cast<int>(ud_paths_.size()));
        if (update) ud_paths_.at(idx) = updown_path;

      }
    };


  } //end of namespace srl

} // end of treeler namespace

#endif /* TREELER_SRL_PATHS_H */
