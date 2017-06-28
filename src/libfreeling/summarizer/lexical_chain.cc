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


#include "freeling/morfo/lexical_chain.h"

using namespace std;

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Constructor
  ////////////////////////////////////////////////////////////////

  lexical_chain::lexical_chain(relation *r, const word &w, const sentence &s,
                               int n_paragraph, int n_sentence, int position) {
    rel = r;
    word_pos * wp = new word_pos(w, s, n_paragraph, n_sentence, position);
    words.push_back(*wp);
    unique_words[w.get_lc_form()] = pair<int, word_pos*>(1, wp);
    score = -1;
  }

  ////////////////////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////////////////////

  lexical_chain::~lexical_chain() { }

  ////////////////////////////////////////////////////////////////
  /// Get the score of the lexical chain
  ////////////////////////////////////////////////////////////////

  double lexical_chain::get_score() {
    // the score is computed only the first time
    if (score < 0)
      score = (double)words.size() * rel->get_homogeneity_index(words, relations, unique_words);
    return score;
  }

  ////////////////////////////////////////////////////////////////
  /// Get all words in the chain embedded in word_pos structs
  ////////////////////////////////////////////////////////////////

  const list<word_pos> &lexical_chain::get_words() const {
    return words;
  }

  ////////////////////////////////////////////////////////////////
  /// Get all the words ordered by frequency
  ////////////////////////////////////////////////////////////////

  list<word_pos> lexical_chain::get_ordered_words() const {
    return rel->order_words_by_weight(this->unique_words);
  }

  ////////////////////////////////////////////////////////////////
  /// Get the number of words inside the lexical chain
  ////////////////////////////////////////////////////////////////

  int lexical_chain::get_number_of_words() const {
    return words.size();
  }

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  wstring lexical_chain::toString() {
    wstring res;
    wstring label = (rel->label==relation::SAME_WORD ? wstring(L"same_word") 
                     : (rel->label==relation::HYPERNYMY ? wstring(L"hypernymy") 
                     : (rel->label==relation::SAME_COREF_GROUP ? wstring(L"same_coref_group") 
                     : wstring(L"unknown") )));
 
    res += L"Lexical Chain. Type="+ label + L" score=" + std::to_wstring(get_score()) + L"\n";
    res += L"	Word list:\n";
    for (list<word_pos>::const_iterator it = words.begin(); it != words.end(); it++) {
      res += L"		" + it->toString() + L"\n";
    }
    res += L"	Relation list:\n";
    for (list<related_words>::const_iterator it = relations.begin(); it != relations.end(); it++) {
      res += L"		" + it->toString() + L"\n";
    }
    return res;
  }

  ////////////////////////////////////////////////////////////////
  /// Adds given word must be to the lexical chain if it is a
  /// match.  Returns a boolean stating whether the word was added
  ////////////////////////////////////////////////////////////////

  bool lexical_chain::compute_word(const word &w, const sentence &s, const document &doc,
                                   int n_paragraph, int n_sentence, int position) {
    score = -1; // score needs to be computed again
    return rel->compute_word(w, s, doc, n_paragraph, n_sentence, position, 
                             this->words, this->relations, this->unique_words);
  }

} // namespace
