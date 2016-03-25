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
 * \file   io-sentence.h
 * \brief  routines for IO with sentences
 * \author Xavier Carreras
 */

#ifndef IO_SENTENCE_H
#define IO_SENTENCE_H

#include <iostream>
#include <cassert>
#include <iterator>
#include <algorithm>

#include "treeler/base/windll.h"
#include "treeler/base/sentence.h"
#include "treeler/base/basic-sentence.h"
#include "treeler/io/conllstream.h"

namespace treeler {


  //--------------------------------------------------------------------------
  // Sentence

  /** 
   * \brief Reads a Sentence from a CoNLL stream using a mapping
   */    
  template<typename M, typename F, typename C, typename LexT, typename TagT>
  inline 
  CoNLLBasicStream<M,F,C>& operator>>(CoNLLBasicStream<M,F,C>& strm, BasicSentence<LexT,TagT>& x);

  /** 
   * \brief Writes a Sentence from a CoNLL stream using a mapping
   */    
  template<typename M, typename F, typename C, typename LexT, typename TagT>
  inline
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& strm, const BasicSentence<LexT,TagT>& s);
  
  

  //--------------------------------------------------------------------------
  // Sentence (old)

  /** 
   * \brief Writes an old Sentence into a CoNLL stream
   */    
  template<typename M, typename F, typename C, typename LexT, typename TagT>
  inline
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& c, const Sentence& s) {
    vector<string> v(s.size(), "--a_sentence_deprecated--"); 
    c << v; 
    return c; 
  }



  //--------------------------------------------------------------------------
  // IMPLEMENTATION


  template <typename M, typename F, typename C, typename LexT, typename TagT>
  CoNLLBasicStream<M,F,C>& operator>>(CoNLLBasicStream<M,F,C>& strm, BasicSentence<LexT,TagT>& x) {
    size_t m = strm.read_ptr; 
    const F& format = strm.format;    
    int l = format.sentence_end>=0 ? format.sentence_end +1 : 1 + std::max(-1, std::max(format.word, std::max(format.lemma, std::max(format.coarse_pos, std::max(format.fine_pos, format.morpho_tags))))); 
    if ((m + l) > strm.num_columns()) {
      strm.good = false;
      return strm;
    }    
    CoNLLBasicColumn<C>& W = (format.word>=0) ?  strm.A[m+format.word] : strm.A[0];
    CoNLLBasicColumn<C>& L = (format.lemma>=0) ? strm.A[m+format.lemma] : strm.A[0];
    CoNLLBasicColumn<C>& CP = (format.coarse_pos>=0) ? strm.A[m+format.coarse_pos] : strm.A[0];
    CoNLLBasicColumn<C>& FP = (format.fine_pos>=0) ? strm.A[m+format.fine_pos] : strm.A[0];
    CoNLLBasicColumn<C>& MO = (format.morpho_tags>=0) ? strm.A[m+format.morpho_tags] : strm.A[0];
    strm.read_ptr += l;
    int n = strm.num_rows();
    x.clear();
    for (int i=0; i<n; ++i) {
      LexT w = BasicSentenceTraits<LexT>::null();
      LexT le = BasicSentenceTraits<LexT>::null();
      TagT cp = BasicSentenceTraits<TagT>::null(); 
      TagT fp = BasicSentenceTraits<TagT>::null(); 
      if (format.word>=0)  { w = strm.mapping.template map_field<WORD,LexT>(W[i]); }
      if (format.lemma>=0) { le = strm.mapping.template map_field<LEMMA,LexT>(L[i]); }
      if (format.coarse_pos>=0)  { cp = strm.mapping.template map_field<COARSE_POS,TagT>(CP[i]); } 
      if (format.fine_pos>=0)  { fp = strm.mapping.template map_field<FINE_POS,TagT>(FP[i]); } 
      x.add_token(BasicToken<LexT,TagT>(w, le, cp, fp));
      if (format.morpho_tags>=0) {
        typename CoNLLBasicStream<M,F,C>::StringType lmorpho = MO[i];
        if (lmorpho!=strm.EMPTY) {
          for (size_t i=0; i<lmorpho.length(); ++i) {
            if (lmorpho[i]=='|') lmorpho[i] = ' ';
          }
          BasicToken<LexT,TagT>& t = x.back();
          basic_stringstream<typename CoNLLBasicStream<M,F,C>::CharType> mapper;
          mapper << lmorpho;
          TagT m;
          while (mapper >> m) {
            t.morpho_push(strm.mapping.template map_field<MORPHO_TAG,TagT>(m) );
          }
        }
      }
    }
    return strm;
  }
  

  template<typename M, typename F, typename C, typename LexT, typename TagT>
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& stream, const BasicSentence<LexT,TagT>& s) {
    // cerr << "+ write sent with mapping" << endl; 
    int n = s.size();
    int m = stream.num_columns(); 
    if (m==0) {
      stream.add_ids(n);
      m++;
    }
    const F& format = stream.format; 
    int l = format.sentence_end>=0 ? 
      format.sentence_end +1 : 
      1 + std::max(-1, 
		   std::max(format.word, 
			    std::max(format.lemma, 
				     std::max(format.coarse_pos, 
					      std::max(format.fine_pos, format.morpho_tags))))); 
    if (l==0) return stream; 
    stream.A.resize(m+l, CoNLLColumn(n));

    CoNLLBasicColumn<C>& wo = (format.word<0)        ? stream[m] : stream[m+format.word];
    CoNLLBasicColumn<C>& le = (format.lemma<0)       ? stream[m] : stream[m+format.lemma];
    CoNLLBasicColumn<C>& ct = (format.coarse_pos<0)  ? stream[m] : stream[m+format.coarse_pos];
    CoNLLBasicColumn<C>& ft = (format.fine_pos<0)    ? stream[m] : stream[m+format.fine_pos];
    CoNLLBasicColumn<C>& mt = (format.morpho_tags<0) ? stream[m] : stream[m+format.morpho_tags];

    typedef typename CoNLLBasicStream<M,F,C>::StringType StringType;
    for (int i=0; i<n; ++i) {
      const typename BasicSentence<LexT,TagT>::Token& t = s.get_token(i);
      if (format.word>=0)       wo[i] = (StringType) stream.mapping.template map_field<WORD,string>(t.word());
      if (format.lemma>=0)      le[i] = (StringType) stream.mapping.template map_field<LEMMA,string>(t.lemma());
      if (format.coarse_pos>=0) ct[i] = (StringType) stream.mapping.template map_field<COARSE_POS,string>(t.coarse_pos());
      if (format.fine_pos>=0)   ft[i] = (StringType) stream.mapping.template map_field<FINE_POS,string>(t.fine_pos());
      
      auto mi = t.morpho_begin();
      auto me = t.morpho_end();
      if (format.morpho_tags<0) {
      }
      else if (mi==me)  {
	mt[i] = "_";
      }
      else {
	ostringstream oss;
	bool first = true;
	for (; mi!=me; ++mi) {
	  if (!first) oss << "|";
	  oss << stream.mapping.template map_field<MORPHO_TAG,string>(*mi);
	  first = false;
	}
	mt[i] = oss.str();
	oss.str("");
      }     
    }
    return stream;
  }



}

#endif
