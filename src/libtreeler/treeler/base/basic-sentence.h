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

/** 
 * \file   basic-sentence.h
 * \brief  Declaration of the class BasicSentence
 */

#ifndef TREELER_BASIC_SENTENCE_H
#define TREELER_BASIC_SENTENCE_H

#include <vector>
#include <list>

namespace treeler {

  enum SentenceFields {
    WORD, 
    LEMMA, 
    COARSE_POS, 
    FINE_POS, 
    MORPHO_TAG, 
    __EXTENSION
  };


  /**  
   * \brief A token for NLP tasks
   * \author Xavier Carreras
   * \ingroup base
   * 
   * A token for NLP tasks, with word, lemma, coarse pos, fine pos and a set of morpho tags
   * 
   * \
   */
  template <typename __LexT, typename __TagT>
  class BasicToken {
  public:
    typedef __LexT Lex; 
    typedef __TagT Tag; 

  protected:
    Lex _word;
    Lex _lemma;
    Tag _coarse_pos;
    Tag _fine_pos;
    std::list<Tag> _morpho;

  public:
    BasicToken() {}    
    BasicToken(const Lex& word, const Lex& lemma, const Tag& cpos, const Tag& fpos)
      : _word(word), _lemma(lemma), _coarse_pos(cpos), _fine_pos(fpos)
    {}

    const Lex& word() const { return _word; }
    Lex& word() { return _word; }

    const Lex& lemma() const { return _lemma; }
    Lex& lemma() { return _lemma; }

    const Tag& coarse_pos() const { return _coarse_pos; }
    Tag& coarse_pos() { return _coarse_pos; }

    const Tag& fine_pos() const { return _fine_pos; }
    Tag& fine_pos() { return _fine_pos; }

    const std::list<Tag>& morpho_tags() const { return _morpho; }
    typename std::list<Tag>::const_iterator morpho_begin() const { return _morpho.begin(); }
    typename std::list<Tag>::const_iterator morpho_end() const { return _morpho.end(); }
    
    void morpho_clear() { _morpho.clear(); }
    void morpho_push(const Tag& tag) { _morpho.push_back(tag); }

  };  


  template<typename T>
  class BasicSentenceTraits {

  };

  template<>
  class BasicSentenceTraits<int> {
  public:
    static int null() { return -1; }
  };

  template<>
  class BasicSentenceTraits<std::string> {
  public:
    static std::string null() { return ""; }
  };


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
  template <typename __LexT, typename __TagT, typename __TokenT = BasicToken<__LexT,__TagT> >
  class BasicSentence  {
  public:
    typedef __LexT Lex; 
    typedef __TagT Tag; 
    typedef __TokenT Token;

    BasicSentence(){}

    void set_id(int i) { _id = i; }
    int id() const { return _id; }

    /// Adds a token to the end of the sentence 
    void add_token(const Token & t) { _tokens.push_back(t); }

    /// Fetches the token at the given position 
    const Token & get_token(int position) const { return _tokens[position]; }
    const Token & operator[](int position) const { return _tokens[position]; }
    const Token& first() const { return _tokens.first(); }
    const Token& back() const { return _tokens.back(); }

    Token& operator[](int position) { return _tokens[position]; }
    Token& first() { return _tokens.first(); }
    Token& back() { return _tokens.back(); }
    
    bool empty() const { return _tokens.empty(); }
    int size() const { return _tokens.size(); }

  void clear() { _tokens.clear(); }
  
  void resize(size_t n) { _tokens.resize(n); }


  protected:
    int _id;
    std::vector<Token> _tokens;
  };

}

#endif /* TREELER_BASIC_SENTENCE_H */
