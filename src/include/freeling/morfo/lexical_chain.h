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

#ifndef _LEXCHAIN
#define _LEXCHAIN

#include "freeling/morfo/relation.h"
#include "freeling/morfo/language.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class lexical_chain represents a lexical chain and computes 
  /// words and stores (or not) them into the structures.
  ////////////////////////////////////////////////////////////////

  class lexical_chain {
  public:

    /// Constructor
    lexical_chain(relation *r, const freeling::word &w, const freeling::sentence &s,
                  int n_paragraph, int n_sentence, int position);

    /// Destructor
    ~lexical_chain();

    /// Computes a word, if the word an be added to the lexical chain, this method stores it in its
    /// structures and return true. Otherwise, it does nothing and returns false.
    bool compute_word(const freeling::word &w, const freeling::sentence &s, const freeling::document &doc,
                      int n_paragraph, int n_sentence, int position);

    /// Get the score of the lexical chain
    double get_score();

    /// Get the number of words inside the lexical chain
    int get_number_of_words() const;

    /// Get all the words embedded in a word_pos struct of the lexical chain
    const std::list<word_pos> &get_words() const;

    /// Get all the words ordered by frequency
    std::list<word_pos> get_ordered_words() const;

    /// Get a string representation of the lexical chain to debug
    std::wstring toString();

  private:
    /// Lexical chains score.
    double score;
    /// Structure to keep track of the words frequency
    std::unordered_map<std::wstring, std::pair<int, word_pos*> > unique_words;
    /// List of words embedded in a word_pos struct
    std::list<word_pos> words;
    /// Pointer to the relation that this lexical chain uses
    relation * rel;
    /// List of relations between words
    std::list<related_words> relations;
  };

} // namespace

#endif

