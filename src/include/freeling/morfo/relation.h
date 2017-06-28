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

#ifndef _LEXCHAIN_RELATION
#define _LEXCHAIN_RELATION

#include <set>
#include <string>
#include <vector>
#include <unordered_map>

#include "freeling/morfo/semdb.h"
#include "freeling/morfo/language.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Struct that allow us to compare words easily.
  ////////////////////////////////////////////////////////////////

  struct word_pos {
    const freeling::word &w;
    const freeling::sentence &s;
    double ch_score;
    int n_paragraph;
    int n_sentence;
    int position;

    word_pos(const freeling::word &w_p, const freeling::sentence &s_p,
             int n_paragraph, int n_sentence, int position);

    /// Two words are equal if they are in the same position, phrase and paragraph.
    bool operator==(const word_pos &other) const;

    /// One word is smaller than other one if it appears later in the text.
    bool operator<(const word_pos &other) const;

    /// One word is greater than other one if it appears sooner in the text.
    bool operator>(const word_pos &other) const;

    /// Get a string representation of the word_pos to debug
    std::wstring toString() const;
  };

  ////////////////////////////////////////////////////////////////
  /// Struct that represents a relationship between two words.
  ////////////////////////////////////////////////////////////////

  struct related_words {
    const word_pos &w1;
    const word_pos &w2;
    /// Relatedness represents the strength of the relationship.
    double relatedness;

    related_words(const word_pos &w_p1, const word_pos &w_p2, double relatedness);

    /// Get a string representation of the relationship to debug
    std::wstring toString() const;
  };

  ////////////////////////////////////////////////////////////////
  ///  Class relation is a non-instantiable class which defines
  /// many virtual methods to check if a word is compatible with
  /// the relation or if a word can be stored in the structures
  /// of a lexical chain.
  ////////////////////////////////////////////////////////////////

  class relation {
  public:

    // types of relations
    typedef enum {SAME_WORD, SAME_COREF_GROUP, HYPERNYMY} RelType;

    /// Label with the name of the relation. It is used for debugging.
    const RelType label;
    /// The maximum distance in phrases between two words to be related
    static int max_distance;
    static freeling::regexp re_np;
    static freeling::regexp re_nn;

    /// Constructor
    relation(RelType s, const std::wstring &t, const std::wstring &e);

    /// Destructor
    virtual ~relation();

    /// True if the words tag is compatible with the relation
    bool is_compatible(const freeling::word &w) const;

    /// Returns true and stores the word w in the list words, list relations and
    /// unordered_map unique_words if w is compatible with the words in these
    /// structures using this relation.
    virtual bool compute_word (const freeling::word &w, const freeling::sentence &s, const freeling::document &doc, 
                               int n_paragraph, int n_sentence, int position, 
                               std::list<word_pos> &words,
                               std::list<related_words> &relations, 
                               std::unordered_map<std::wstring,std::pair<int, word_pos*> > &unique_words) const;

    virtual int matching_word(const word &w1, const word &w2) const;

    virtual double get_homogeneity_index(const std::list<word_pos> &words,
                                         const std::list<related_words> &relations,
                                         const std::unordered_map<std::wstring,
                                         std::pair<int, word_pos*> > &unique_words) const = 0;

    virtual std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
                                                      std::pair<int, word_pos*> > &unique_words) const = 0;


  protected:

    /// If a word tag matchs with compatible_tag, then the word is compatible
    /// with the relation.
    const freeling::regexp compatible_tag;
    const freeling::regexp excluded_lemmas;

  };

  ////////////////////////////////////////////////////////////////
  ///  Class same_word represents the same word relation: two words
  /// are related if they are the same word.
  ////////////////////////////////////////////////////////////////

  class same_word : public relation {

  public:

    /// Constructor
    same_word(const std::wstring &expr, const std::wstring &excluded);

    /// Computes the homogeinity index of the given structures using the specific formula
    /// of this relation.
    double get_homogeneity_index(const std::list<word_pos> &words,
                                 const std::list<related_words> &relations,
                                 const std::unordered_map<std::wstring,
                                 std::pair<int, word_pos*> > &unique_words) const;

    /// see if w1 can be added to the same chain than w2, according to the relation
    int matching_word(const word &w1, const word &w2) const;

    /// In same_word, the words in unique_words are not sorted because there is just
    /// one word. It returns the word_pos in unique_words in a list.
    std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
                                              std::pair<int, word_pos*> > &unique_words) const;

  };

  ////////////////////////////////////////////////////////////////
  ///  Class hypernymy represents the hypernymy relation: two words
  /// are related if one is an hypernymy of the other and the
  /// hypernymy depth is smaller or equal than a given maximum.
  ////////////////////////////////////////////////////////////////

  class hypernymy : public relation {

  public:

    /// Constructor
    hypernymy(const std::wstring &expr, const std::wstring &excluded, int dp, double alpha, const std::wstring &semfile);

    /// Computes the homogeinity index of the given structures using the specific formula
    /// of this relation.
    double get_homogeneity_index(const std::list<word_pos> &words,
                                 const std::list<related_words> &relations,
                                 const std::unordered_map<std::wstring,
                                 std::pair<int, word_pos*> > &unique_words) const;

    /// see if w1 can be added to the same chain than w2, according to the relation
    int matching_word(const word &w1, const word &w2) const;

    /// Sorts the words in unique_words by word frequency and returns a list with them.
    std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
                                              std::pair<int, word_pos*> > &unique_words) const;

    // get/set depth
    void set_depth(int d);
    int get_depth() const;

  private:

    /// Semantic DB to check the hypernymy.
    freeling::semanticDB * semdb;
    /// Maximum depth.
    int depth;
    /// Used to compute the homogeinity index.
    double alpha;

    /// Recursive auxiliar function to check if two words are related.
    int hypernymyAux(const std::wstring &s1, const std::wstring &s2, int k) const;

    /// Gets the word which appears in most relations.
    const word_pos &count_relations(int n, const std::list<related_words> &relations) const;
  };

  ////////////////////////////////////////////////////////////////
  ///  Class same_coref_group represents the same coreference group
  /// relation: two words are related if they are in the same
  /// coreference group.
  ////////////////////////////////////////////////////////////////

  class same_coref_group : public relation {

  public:

    /// Constructor
    same_coref_group(const std::wstring &expr, const std::wstring &excluded);

    /// Computes the homogeinity index of the given structures using the specific formula
    /// of this relation.
    double get_homogeneity_index(const std::list<word_pos> &words,
                                 const std::list<related_words> &relations,
                                 const std::unordered_map<std::wstring,
                                 std::pair<int, word_pos*> > &unique_words) const;

    /// Returns true and stores the word w in the list words, list relations and
    /// unordered_map unique_words if w is compatible with the words in these
    /// structures using this relation.
    bool compute_word (const freeling::word &w, const freeling::sentence &s,
                       const freeling::document &doc, int n_paragraph, int n_sentence,
                       int position, std::list<word_pos> &words, std::list<related_words> &relations,
                       std::unordered_map<std::wstring, std::pair<int, word_pos*> > &unique_words) const;

    /// Sorts the words in unique_words by word frequency and returns a list with them.
    std::list<word_pos> order_words_by_weight(const std::unordered_map<std::wstring,
                                              std::pair<int, word_pos*> > &unique_words) const;
  };

}

#endif

