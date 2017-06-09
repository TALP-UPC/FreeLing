//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _SUMMARIZER
#define _SUMMARIZER

#include "freeling/windll.h"
#include "freeling/morfo/lexical_chain.h"
#include "freeling/morfo/language.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  summarizer class summarizes a document using the lexical
  /// chains method.
  ////////////////////////////////////////////////////////////////

  class WINDLL summarizer {
  public:
    /// Constructor
    summarizer(const std::wstring &datFile);

    /// Destructor
    ~summarizer();

    /// Summarizes a document and returns the list of sentences that composes the summary.
    std::list<const freeling::sentence*> summarize(const freeling::document &doc,
                                                   int num_words) const;

    typedef enum {FIRST_WORD,FIRST_MOST_WEIGHT,WEIGHT_SUM} Heuristics;

    void set_heuristic(Heuristics h);
    Heuristics get_heuristic() const;

    void set_only_strong(bool s);
    bool get_only_strong() const;

    void set_remove_used_chains(bool s);
    bool get_remove_used_chains() const;

    void enable_all_relations();
    void disable_all_relations();
    void enable_relation(relation::RelType);
    void disable_relation(relation::RelType);

  private:

    /// If true, the used lexical_chains will be removed.
    bool remove_used_lexical_chains;
    /// If true, the summarizer will use only strong chains
    bool only_strong;
    /// Maximum hypernymy depth
    int hypernymy_depth;
    /// Parameter to compute the homogeinity index in the hypernymy relation.
    double alpha;
    /// Path to the semantic DB.
    std::wstring semdb_path;
    /// A set with active relations 
    std::set<relation*> used_relations;
    /// A set with available but disabled relations
    std::set<relation*> unused_relations;
    /// A string that indicates the heuristic that will be used.
    Heuristics heuristic;

    // relation to use if none is defined
    static relation* default_relation;

    /// Builds all the lexical chains.
    std::map<relation::RelType, std::list<lexical_chain>> build_lexical_chains(const freeling::document &doc) const;

    /// Remove the lexical chains with only one word
    void remove_one_word_lexical_chains(std::map<relation::RelType, std::list<lexical_chain>> &chains) const;

    /// Remove the lexical chains which does not satisfy the strength criterion.
    void remove_weak_lexical_chains(std::map<relation::RelType, std::list<lexical_chain>> &chains) const;
    
    /// Print the lexical chains. Only for debugging.
    void print_lexical_chains(std::map<relation::RelType, std::list<lexical_chain>> &chains) const;
    
    /// Counts the number of occurences of the word w in the document doc.
    int count_occurences(const freeling::word &w, const freeling::document &doc) const;
    
    /// Computes and returns the average scores of the lexical chains.
    double average_scores(std::map<relation::RelType, std::list<lexical_chain> > &chains_type) const;
    
    /// Computes and returns the standard deviation of the lexical chains scores.
    double standard_deviation_scores(std::map<relation::RelType, std::list<lexical_chain> > &chains_type,
                                     const double avg) const;
    
    /// Concatenate all the lists in the map chains_type into a single list.
    std::list<lexical_chain> map_to_lists(std::map<relation::RelType,
                                          std::list<lexical_chain> > &chains_type) const;
    
    /// Auxiliar function for first_word and first_most_weighted_word function. Computes
    /// a sentence to include it in the summary or not.
    void compute_sentence(const std::list<word_pos> &wps, std::list<word_pos> &wp_list,
                          std::set<const freeling::sentence*> &sent_set, int &acc_n_words,
                          int num_words) const;
    
    /// Returns the list of sentences embedded in a word_pos struct which composes the
    /// summary using the heuristic FirstWord.
    std::list<word_pos> first_word(std::map<relation::RelType,
                                   std::list<lexical_chain> > &chains_type, int num_words) const;
    
    /// Returns the list of sentences embedded in a word_pos struct which composes the
    /// summary using the heuristic FirstMostWeightedWord.
    std:: list<word_pos> first_most_weighted_word(std::map<relation::RelType, 
                                                  std::list<lexical_chain> > &chains, 
                                                  int num_words) const;
    
    /// Returns the list of sentences embedded in a word_pos struct which composes the
    /// summary using the heuristic SumOfChainWeights.
    std::list<word_pos> sum_of_chain_weights(std::map<relation::RelType, 
                                             std::list<lexical_chain> > &chains, 
                                             int num_words) const;

    // move relations of given type from list "from" to list "to"
    void move_relations(relation::RelType t, std::set<relation*> &from, std::set<relation*> &to);
  };

} // namespace

#endif
