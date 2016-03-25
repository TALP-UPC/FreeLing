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
 * \brief  Declaration of the class Sentence
 * \author Xavier Carreras, Mihai Surdeanu
 * \note   This file was ported from egstra
 */

#ifndef TREELER_SENTENCE_H
#define TREELER_SENTENCE_H

#include "treeler/base/token.h"
#include <vector>

namespace treeler {

  /**
   * \brief A sentence, implemented as a vector of tokens
   * \author Xavier Carreras, Mihai Surdeanu
   * \ingroup base
   * 
   * \todo In principle this class should be a bit more flexible with
   * respect to the type of tokens it contains. Probably it will be
   * templated, with a default token type.
   * 
   */
  class Sentence  {
  public:
    typedef treeler::Token Token;
    Sentence(){}

    void set_id(int i) { _id = i; }
    int id() const { return _id; }

    /// Adds a token to the end of the sentence 
    void add_token(const Token & t) { _tokens.push_back(t); }

    /// Fetches the token at the given position 
    const Token & get_token(int position) const { return _tokens[position]; }
    
    bool empty() const { return _tokens.empty(); }

    int size() const { return _tokens.size(); }

  private:
    int _id;
    std::vector<Token> _tokens;
  };

}

#endif /* TREELER_SENTENCE_H */
