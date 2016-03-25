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
 * \file   io-dep.h
 * \brief  routines for IO with dependency structures
 * \author Xavier Carreras
 */

#ifndef IO_DEP_H
#define IO_DEP_H

#include <iostream>
#include <cassert>
#include <iterator>
#include <algorithm>

#include "treeler/base/windll.h"

#include "treeler/dep/part-dep1.h" 
#include "treeler/dep/dep-symbols.h"
#include "treeler/dep/dep-tree.h"
#include "treeler/io/conllstream.h"

namespace treeler {
  


  //--------------------------------------------------------------------------
  // Dependency Trees

  template <typename Mapping, typename Format, typename CharT>
  CoNLLBasicStream<Mapping,Format,CharT>& operator<<(CoNLLBasicStream<Mapping,Format,CharT>& strm, const Label<PartDep1>& y) {
    DepVector<int> tmp;
    PartDep1::compose(y,tmp);
    strm << tmp; 
    return strm;     
  }

  /** 
   * \brief Reads a Dependency Tree Vector from a CoNLL stream
   */      
  template<typename M, typename F, typename C, typename LabelT>
  inline
  CoNLLBasicStream<M,F,C>& operator>>(CoNLLBasicStream<M,F,C>& strm, DepVector<LabelT>& x);
    
  /** 
   * \brief Writes a Dependency Tree Vector into a CoNLL stream
   */    
  template<typename M, typename F, typename C, typename LabelT>
  inline
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& strm, const DepVector<LabelT>& x);  


  template<typename M, typename F, typename C, typename LabelT>
  inline
  CoNLLBasicStream<M,F,C>& operator>>(CoNLLBasicStream<M,F,C>& stream, DepVector<LabelT>& x) {
    size_t m = stream.read_ptr;
    const F& format = stream.format; 
    int l = format.dependency_end >=0 ? format.dependency_end + 1 : 1 + std::max(-1, std::max(format.head, format.syntactic_label)); 
    if (m  + l > stream.num_columns()) {
      stream.good = false;
      return stream;
    }
    CoNLLBasicColumn<C>& heads = format.head>=0 ? stream[m+format.head] : stream[0];
    CoNLLBasicColumn<C>& labels = format.syntactic_label>=0 ? stream[m+format.syntactic_label] : stream[0];
    stream.read_ptr += l;
    size_t n = stream.num_rows();
    x.resize(n);
    for (size_t i=0; i<n; ++i) {
      HeadLabelPair<LabelT>& hl = x[i];
      if (format.head>=0) { stringstream m; m << heads[i]; m >> hl.h; hl.h -= stream.offset; }
      else hl.h = -stream.offset; 
      if (format.syntactic_label>=0) { hl.l = stream.mapping.template map_field<SYNTACTIC_LABEL,LabelT>(labels[i]); }      
    }
    return stream;
  }
  
  template<typename M, typename F, typename C, typename LabelT>
  inline
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& stream, const DepVector<LabelT>& x) {
    size_t m = stream.num_columns();
    size_t n = x.size();
    const F& format = stream.format;
    int l = format.dependency_end >=0 ? format.dependency_end + 1 : 1 + std::max(-1, std::max(format.head, format.syntactic_label)); 
    if (l<=0) return stream;
    stream.resize(m+l, CoNLLBasicColumn<C>(n)); 
    CoNLLBasicColumn<C>& heads = format.head>=0 ? stream[m + format.head] : stream[m];
    CoNLLBasicColumn<C>& labels = format.syntactic_label >= 0 ? stream[m + format.syntactic_label] : stream[m];
    for (size_t i=0; i<n; ++i) {      
      const HeadLabelPair<LabelT>& hl = x[i];
      if (format.head>=0) { stringstream m; m << hl.h + stream.offset; m >> heads[i]; }
      if (format.syntactic_label>=0) {
	labels[i] = stream.mapping.template map_field<SYNTACTIC_LABEL,string>(hl.l);
      }
    }    
    return stream;
  }

}

#endif
