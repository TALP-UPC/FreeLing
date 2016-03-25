/*
 * basic-srl.h
 *
 *  Created on: 13/12/2012
 *      Author: Usuari
 */

#ifndef TREELER_SRL_BASIC
#define TREELER_SRL_BASIC

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>


#include "treeler/dep/dep-tree.h"
#include "treeler/dep/dep-symbols.h"
#include "treeler/srl/part-srl0.h"

using namespace std; 

namespace treeler {

  /**
   * \defgroup srl Semantic Role Labeling
   * This module implements models and algorithms for dependency parsing.
   * @{
   */


  namespace srl {

    enum SRLFields {
      SEMANTIC_LABEL = DepFields::SYNTACTIC_LABEL+1
    };



    /**
     * POS-POS filter, returns list of allowed role tags 
     */
    class POSPOSFilter{
    public:
      
      /** load the file s */
      void load_pos_filter(const string &s){
        cerr << "POS-POS Filter: loading \"" << s << "\" ... ";
        assert(pos_to_arg_and_freq_.empty()); // do not reread
        //open the file
        //for each line load, int freq and string pos
        std::ifstream infile(s.c_str());
        std::string postag;
        int i = 0;
        while (infile >> postag){
          std::string twopoints;
          infile >> twopoints;
          assert(twopoints.compare(":") == 0);

          int num_entries = 0;
          assert(infile >> num_entries);

          //read the list of pairs role tag - freq
          for (int j = 0; j < num_entries; ++j){
            std::string role;
            int freq=0;
            assert(infile >> role);
            assert(infile >> freq);
            process_pair(postag, role, freq);
          }

          ++i;
        }
        cerr << i << " entries" << endl;
      }

      /** return true if the combination predicate_ppos - argument_ppos was present on the filter
      * with any frequency */
      bool is_pos_allowed(const std::string &pos_pred, const std::string &pos_arg) const {
        const std::string s = pos_pred + "." + pos_arg;
        //is pos in the map? 
        if (pos_to_arg_and_freq_.find(s) == pos_to_arg_and_freq_.end()){
          //assume freq is 0
          return false;
        }

        //return true if there is at least 1 entry for this pos tag
        int mapsize = pos_to_arg_and_freq_.find(s)->second.size();
        if (mapsize > 0) return true;
        else return false;
      }

      /** return true if the combination predicate_ppos - argument_ppos was present on the filter
      * and with the minimum frequency, hardcoded to 1 */
      bool is_pos_arg_allowed(const std::string postag_pred, 
        const std::string &postag_arg, const std::string& arg) const {
         const std::string pospos = postag_pred + "." + postag_arg;
        //return true;
        if (pos_to_arg_and_freq_.find(pospos) == pos_to_arg_and_freq_.end()){
          //assume freq is 0
          return false;
        }

        //return true if there is at least 1 entry for this pos tag
        const std::map<std::string,int>& arg_to_freq = pos_to_arg_and_freq_.find(pospos)->second;
        if (arg_to_freq.empty()) return false;

        if (arg_to_freq.find(arg) == arg_to_freq.end()) return false;
        
        return true;
        //ask for 5 or more //the combination pos-arg has low freq so set to 1
        //if (arg_to_freq.find(arg)->second < 5) return false;
        //else return false;

      }

      private:
      /** map from predicate_ppos.arg_ppos to a set of role(str) - freq */
      map<std::string, map<std::string,int> > pos_to_arg_and_freq_;
      
      /** inserts the combination in the map */
      void process_pair(const std::string &postag, const std::string &arg, const int freq){
        //is there a map for this pos tag?
        if (pos_to_arg_and_freq_.find(postag) == pos_to_arg_and_freq_.end()){
          pos_to_arg_and_freq_.insert(std::pair<std::string, std::map<std::string,int>>(postag, std::map<std::string,int>()));
        }
        //get the map
        map<std::string,int>& arg_to_freq = pos_to_arg_and_freq_.find(postag)->second;
	
        arg_to_freq.insert(std::pair<std::string,int>(arg, freq));
      }
    };
    

    /**
     * A symbols dictionary for SRL
     */
    class SRLSymbols : public DepSymbols {
    public:
      Dictionary d_semantic_labels;
      
      void load_pos_filter(const std::string &s){
        pos_filter_.load_pos_filter(s);
      }

      bool is_pos_allowed(const std::string pos_pred, const std::string& pos_arg) const {
        return pos_filter_.is_pos_allowed(pos_pred, pos_arg);
      }

      bool is_pos_arg_allowed(const std::string &pos_pred, const std::string& pos_arg, 
        const std::string& arg) const {
        return pos_filter_.is_pos_arg_allowed(pos_pred, pos_arg, arg);
      }

    private:
      POSPOSFilter pos_filter_;
    };


    /**
     * \brief Annotation of a predicate, consisting of the predicate
     * sense and the set of arguments, each labeled with its role.
     *
     * \author Manu
     *
     * The set of arguments is implemented as an std::map, since it is
     * typically a sparse structure.
     *
     * \note Xavier: cal clarificar que els arguments estan indexats
     * segons la posicio de l'argument a l'oracio. Tambe cal
     * clarificar que només s'indexen els arguments (es a dir, els
     * tokens que no son arguments del predicat (=rol nul) no hi han
     * d'apareixer).
     *
     * \note Xavier: No esta clar que es the sense. Es (a) el predicat
     * amb el sentit concatenat? O (b) nomes el sentit del predicat?
     * En cas (a), crec que es millor separar-ho en dos camps (caldra
     * fer-ho més endavant de cara a calcular feature). Si es (b), cal
     * afegir un camp per la identitat del predicat.
     *
     */
    class PredArgStructure : public map<int,string> {
    public:
      string sense;

      PredArgStructure()
        : sense("")
      {}

      PredArgStructure(const string& thesense)
        : sense(thesense)
      {}

      /**
       *  \brief Returns the role associated with the token_id, or an
       *  empty string if that id is not an argument
       */
      inline
      string operator()(const int& token_id) const {
        auto a = this->find(token_id);
        if (a != this->end()) {
          return a->second;
        }
        return "";
      }


      /* /\** */
      /*  *  This is experimental, it's probably a bad idea not being able to assign  */
      /*  *\/ */
      /* inline */
      /* string operator[](const int& token_id) const { */
      /*         cerr << "operator [] being called " << endl;  */
      /*         return (*this)(token_id);  */
      /* } */

    };


    /**
     * \brief A set of predicate-argument structures, indexed by the
     * position of the predicate token in the sentence
     * \note The "position" of the predicate of the sentence is the token id minus
     * one, e.g., thus the predicate at token id 13 is indexed by 12!
     *
     * \author Manu
     */
    class PredArgSet : public map<int,PredArgStructure> {
    public:
      // needed for writing methods. We must know the number of "_" to place.
      unsigned sentence_token_count;

      /* template <class... Args> */
      /* void insert(Args&&... args) { */
      /*   bool a = this->std::map<int,PredArgStructure>::empty(); */
      /*   if (a) cerr << "empty!!" << endl;  */
      /*   //        this->std::map<int,PredArgStructure>::emplace(args...); */
      /* } */


      /**
       * returns a sorted list of predicate indices
       */
      list<int> predicate_list() const {
        list<int> pred_list;
        for (auto it = this->begin(); it != this->end(); ++it){
          pred_list.push_back(it->first);
        }
        return std::move(pred_list);
      }
      
    private:
    };

    /**
     * \brief Auxiliary class to store a list of lemmas/sense and a list of arguments for a predicate
     * \note predsense contains a list of pairs (lemma,sense), synonyms (e.g. eat.01, ingest.02)
     *       possible_args contains a list of expected arguments for the predicate. They are
     *       pairs syntactic_role, semantic_role (e.g. A0:Agent, A1:Patient, ..)
     */
    class PossiblePredArgs {
    public:
      // store list of possible predicate-sense pairs (eat.01, ingest.03, ...)
      std::list<std::pair<std::string,std::string> > predsense;
      // store list of syntactic and semantic roles (A0:Agent, A1:Theme, ...)
      std::list<std::pair<std::string,std::string> > possible_args;
    };


    /**
     * \brief Map relating a position in a sentece with a PossiblePredArgs for the predicate expressed by that word
     */
    typedef std::map<int,treeler::srl::PossiblePredArgs> PossiblePreds;

  }


  /**
   * @}
   */

}


#endif /* TREELER_SRL_BASIC */
