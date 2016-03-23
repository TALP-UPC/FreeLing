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
 * \file   token.h
 * \brief  Declaration of the class Token
 * \author Xavier Carreras, Mihai Surdeanu
 * \note   This file was ported from egstra
 */
#ifndef TREELER_TOKEN_H
#define TREELER_TOKEN_H

#include <list>
#include <string>
#include <cassert>

namespace treeler {

  /**  
   * \brief A token (i.e an occurrence of a word in a sentence)
   * \author Xavier Carreras, Mihai Surdeanu
   * \ingroup base
   * 
   * Currently the attributes of a token are integers (word, postag,
   * etc). In future we will probably relax this.
   * 
   * \todo Add documentation to each method
   */
  class Token {
  public:
    Token()
    : _word(-1), _lemma(-1), _coarse_tag(-1), _fine_tag(-1){}
    
    Token(int word,int lemma, int coarseTag, int fineTag,int feats);

    int word() const { return _word; }
    int lemma() const { return _lemma; }

    // \todo This method should be obsolete because it is ambiguous;
    // either choose coarse or fine tag
    int pos() const { return _coarse_tag; }
    int coarse_tag() const { return _coarse_tag; }
    int fine_tag() const { return _fine_tag; }

    int coarse_pos() const { return _coarse_tag; }
    int fine_pos() const { return _fine_tag; }

    const std::list<int> & morpho_tags() const { return _morpho_feats; }
    std::list<int> & morpho_tags() { return _morpho_feats; }

    /* create the tag mappings */
    /* static void load_tag_map(const std::string& path); */

    /* static int ntags() { return _ntags; } */

    /* /\* return the largest ftag index found in the dataset *\/ */
    /* static int maxftag() { return _maxftag; } */
    /* /\* return the largest ctag index found in the dataset *\/ */
    /* static int maxctag() { return _maxctag; } */
  
  private:
    int _word;
    int _lemma;
    int _coarse_tag;
    int _fine_tag;
    int _feats;
    std::list<int> _morpho_feats;

  };  
} 

#endif /* TREELER_TOKEN_H */

