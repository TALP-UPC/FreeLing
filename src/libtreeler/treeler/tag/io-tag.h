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

#ifndef TREELER_TAG_IO
#define TREELER_TAG_IO

#include "treeler/io/conllstream.h"
#include "treeler/tag/tag-seq.h"
#include "treeler/tag/tag-symbols.h"
#include "treeler/tag/part-tag.h"

using namespace std;

namespace treeler {


  CoNLLStream& operator<<(CoNLLStream& strm, const TagSeq& y);
  CoNLLStream& operator<<(CoNLLStream& strm, const Label<PartTag>& y);

  template<typename Mapping,typename Format, typename CharT>
  CoNLLBasicStream<Mapping,Format,CharT>& operator>>(CoNLLBasicStream<Mapping,Format,CharT>& strm, TagSeq& y) {
    if (strm.read_ptr >= strm.num_columns()) {
      strm.good = false;
      return strm;
    }
    const CoNLLColumn& a = strm.A[strm.read_ptr];
    y.resize(a.size());
    
    CoNLLColumn::const_iterator it = a.begin();
    const CoNLLColumn::const_iterator it_end = a.end();
    typename vector<int>::iterator dest = y.begin();
    for (; it!=it_end; ++it, ++dest) {
      *dest = strm.mapping.template map_field<TAG,int>(*it);
    }
    ++strm.read_ptr;
    return strm;
  }

  template<typename Mapping,typename Format, typename CharT>
  inline 
  CoNLLBasicStream<Mapping,Format,CharT>& operator<<(CoNLLBasicStream<Mapping,Format,CharT>& strm, const TagSeq& y) {
    // cerr << ">> operator<<(mapping, tagset)" << endl; 
    size_t n = strm.num_rows();
    if (n==0) n=y.size();
    strm.A.emplace_back( CoNLLColumn(n) );
    typename TagSeq::const_iterator it = y.begin();
    const typename TagSeq::const_iterator it_end = y.begin()+n;
    CoNLLColumn::iterator dest = strm.A.back().begin();	 
    for (; it != it_end; ++it, ++dest) {
      *dest = strm.mapping.template map_field<TAG,std::string>(*it);
    }
    return strm;
  }

  template<typename Mapping,typename Format, typename CharT>
  inline 
  CoNLLBasicStream<Mapping,Format,CharT>& operator<<(CoNLLBasicStream<Mapping,Format,CharT>& strm, const Label<PartTag>& y) {
    // cerr << ">> operator<<(mapping, Label<PartTag>)" << endl; 
    int n = strm.num_rows();
    if (n==0) n=y.size();
    strm.A.emplace_back( CoNLLColumn(n) );
    CoNLLColumn& dest = strm.A.back(); 
    for (auto r = y.begin(); r!=y.end(); ++r) {
      if (r->i>=0 and r->i<n) {
	dest[r->i] = strm.mapping.template map_field<TAG,std::string>(r->b);
      }
    }    
    return strm;
  }

       
}


#endif
