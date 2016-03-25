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
 * \file   conllstream.h
 * \brief  Declaration of class CoNLLStream
 * \author Xavier Carreras
 */
#ifndef TREELER_CONLLSTREAM_H
#define TREELER_CONLLSTREAM_H


#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "treeler/base/exception.h"
#include "treeler/base/basic-sentence.h"
#include "treeler/base/sentence.h"
#include "treeler/dep/dep-tree.h"
#include "treeler/base/label.h"

using namespace std;

namespace treeler {

  /** 
   * \brief A single column of values
   *
   * \author Xavier Carreras
   *
   */
  template <typename charT>
  class CoNLLBasicColumn : public vector<basic_string<charT>> {
  public: 
    typedef basic_string<charT> T; 
    CoNLLBasicColumn() : vector<T>() {}
    CoNLLBasicColumn(int n) : vector<T>(n) {}
    CoNLLBasicColumn(int n, const T& a) : vector<T>(n,a) {}          
  };
  
  // convenience types
  typedef CoNLLBasicColumn<char> CoNLLColumn; 
  typedef CoNLLBasicColumn<wchar_t> CoNLLWColumn; 


  /**
   * \brief A basic stream of CoNLL annotations   
   * \author Xavier Carreas
   *
   * This class consists of a vector of annotations (i.e. a vector of
   * vectors) that acts as a buffer of annotations. In this code, we
   * use variable m to refer to the number of columns and variable n
   * to the number of rows. In NLP annotations, where CoNLL is highly
   * used, rows correspond to tokens of a sentence and columns
   * correspond to annotation layers that consist of a value for each
   * token, hence the matricial form of this structure.
   * 
   * First, this class defines operators to read a CoNLLStream from a
   * standard input stream (\c istream) and to write a CoNLLStream to
   * a standard output stream (\c ostream). So we can write:
   *
   *    CoNLLStream conll; 
   *    cin >> conll; 
   *    cout << conll; 
   * 
   * In addition, this class defines shift operations to write new
   * columns to the stream, or to read columns from the stream (where
   * it maintains a pointer to the last column read). So we can write:
   * 
   *    DepSentence s; 
   *    conll >> s; 
   *    DepTree d = parse(s);
   *    conll << d; 
   * 
   * An important condition of the current implementation is that all
   * the columns need to be of the same size. The code of this class,
   * in general, does not check this; hence, for example, adding
   * columns of different sizes to a CoNLL stream will probably result
   * in a segfault crash.  An piece of code that checks this condition
   * is the read method (or the operator istream >> CoNLLStream) that
   * throws an exception if a new row differs from the previous rows
   * in the number of columns. Future versions may make this class
   * more robust; for now, it is the programmer user of this class
   * that needs to satisfy this constraint.
   * 
   * It should be easy to write extensions so that new types can be
   * read/write to the stream in the form of columns. This should be
   * done by implementing the corresponding shif operators for the new
   * class. See the offered operators for an idea of how to write new
   * ones.
   * 
   * \todo Template on charT, for now set to char (should be wchar_t). 
   * 
   */
  template <typename Mapping, typename Format, typename CharT=char>
  class CoNLLBasicStream {
  public:    
    // convenience typedefs
    typedef CharT CharType; 
    typedef basic_string<CharT> StringType; 
    typedef CoNLLBasicColumn<CharType> ColumnType; 

  private:
    Mapping* ___m;

  public:
    Mapping& mapping;
    Format   format;

    // the matrix of annotations
    vector<ColumnType> A; 

    ///// THE FOLLOWING VARIABLES REPRESENT THE STATE OF THE STREAM

    /**
     * \brief The separator string (for input/output)
     */
    string SEP;
    /**
     * \brief A special string for EMPTY values (for input/output)
     */
    string EMPTY;
    /**
     * \brief A prefix of the CoNLL block (for output)
     *
     * \note Perhaps it makes sense to extend this for input?
     * (i.e. skip prefix when reading)
     */
    string prefix;
    /**
     * Whether to add padding (for output)
     * 
     * \note current limitations: this supposes that SEP is one char
     * wide; also this fails for wchars
     */
    bool add_padding;
    /** 
     * \brief An offset for positional columns
     * 
     * Internally, a sequence of N tokens has id positions from 0 to N-1
     * with the offset, position i appears as offset+i
     */
    int offset;    
    /** 
     * \brief The index of the next column to be read
     */
    size_t read_ptr;
    /**
     * \brief Indicates if the stream is in a good state
     * 
     * A stream is not in good state if it reached the end of file, or
     * if some operation failed and left the internal buffer in a
     * strange state
     */
    bool good;
    

    ///// OPERATIONS

    // constructor    
    CoNLLBasicStream()
      : ___m(new Mapping()), mapping(*___m), format(),
	SEP(" "), EMPTY("_"), prefix(""), add_padding(false), offset(1)
    {}

    CoNLLBasicStream(Mapping& m)
      : ___m(NULL), mapping(m), format(),
	SEP(" "), EMPTY("_"), prefix(""), add_padding(false), offset(1)
    {}

    CoNLLBasicStream(Mapping& m, const Format& f) 
      : ___m(NULL), mapping(m), format(f),
	SEP(" "), EMPTY("_"), prefix(""), add_padding(false), offset(1)
    {}

    ~CoNLLBasicStream() {
      if (___m!=NULL) delete ___m;
    }

    // basic methods
    size_t num_columns() const { return A.size(); }
    size_t num_rows() const { 
      if (A.empty()) {
	return 0; 
      }
      return A[0].size(); 
    }
    
    // sets the number of columns to m
    void resize(int m) {      
      A.resize(m, ColumnType(num_rows()));
    }

    // sets the number of columns to m, creating new columns with n
    // values if necessary
    void resize(int m, int n) {            
      A.resize(m, ColumnType(n));
    }

    // sets the number of columns to n, adding col as the default
    // column for new values
    void resize(int m, ColumnType col) {
      A.resize(m, col); 
    }
    
    const ColumnType& operator[](size_t i) const {
      return A[i];
    }

    ColumnType& operator[](size_t i) {
      return A[i];
    }

    
    bool read(istream& i);
    void write(ostream& o) const;
    void add_ids(int n) {
      add_ids(n, this->offset); 
    }
    void add_ids(int n, int off);

    operator void*() const {
      return this->good ? const_cast<CoNLLBasicStream*>(this) : 0;
    }

    void rewind() {
      read_ptr = 0;
    }
    
    void clear() {
      A.clear();
      read_ptr = 0;
      good = true;
    }

  public:
  };

  struct CoNLLFormat {
    /* sentence block */
    int word;
    int lemma;
    int coarse_pos;
    int fine_pos;
    int morpho_tags;
    int sentence_end;

    /* tag block */
    int tag; 
    int tag_end; 

    /* dependency block */
    int head;
    int syntactic_label;
    int dependency_end;

    /* semantic role block */
    bool predicate_indicator; 

    CoNLLFormat() 
      : word(0), lemma(1), coarse_pos(2), fine_pos(3), morpho_tags(4), sentence_end(-1),
	tag(0), tag_end(-1),
	head(0), syntactic_label(1), dependency_end(-1), 
	predicate_indicator(false)
    {}

    string to_string() const {
      ostringstream oss; 
      oss << "[ " 
	  << "w(" << word << ") " 
	  << "l(" << lemma << ") " 
	  << "cp(" << coarse_pos << ") " 
	  << "fp(" << fine_pos << ") " 
	  << "mo(" << morpho_tags << ") " 
	  << "send(" << sentence_end << ") " 
	  << "tag(" << tag << ") "
 	  << "tend(" << tag_end << ") " 
 	  << "h(" << head << ") " 
 	  << "s(" << syntactic_label << ") " 
 	  << "dend(" << dependency_end << ") " 
 	  << "pi(" << predicate_indicator << ") " 
	  << "]";
      return oss.str();
    }

  };

  


  struct SimpleMapping {    
  private:
    template <typename T1, typename T2> 
    struct __map_field {
      static inline T1 map(const T2& x) {
	T1 y; 
	stringstream m; 
	m << x; 
	m >> y; 
	return y; 	
      }
    };

    template <typename T> 
    struct __map_field<T,T> {
      static inline T map(const T& x) {
	return x; 
      }
    };
    
  public:
    template <int FIELD, typename T1, typename T2>
    inline 
    T1 map_field(const T2& x) const {
      return __map_field<T1,T2>::map(x);
    }    
  };
  
  // convenience type
  typedef CoNLLBasicStream<SimpleMapping,CoNLLFormat> CoNLLStream; 


  // /** 
  //  * \brief A CoNLLStream that uses Mapping to map values from/to strings
  //  * 
  //  * This is experimental. The goal of it is to associate a Mapping
  //  * object with a basic CoNLL stream, so that it be used during
  //  * reading and writing of values into the stream. 
  //  * 
  //  * For example Mapping may be a set of dictionaries to map values from/to.
  //  * 
  //  * 
  //  */
  // template <typename Mapping, typename Format=CoNLLFormat>
  // class CoNLLStreamM : public CoNLLStream {
  // public:
  //   Mapping& mapping;
  //   Format   format;

  //   CoNLLStreamM(Mapping& m) : mapping(m) {}
  //   CoNLLStreamM(Mapping& m, const Format& f) : mapping(m), format(f) {}
  // };



  
  //-----------------------------------------------------------------------------
  // THE FOLLOWING OPERATIONS DEFINE READ/WRITE OPERATORS FOR SEVERAL KNOWN TYPES
  //-----------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Scalars and vectors

  /** 
   * \brief Writes a constant value t into the CoNLL stream, creating
   * a new full column with as many t values as the number of rows
   */  
  template <typename M, typename F, typename C, typename T>
  inline
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& s, const T& t);

  /** 
   * \brief Writes a vector of values into the CoNLL stream, creating
   * a new column 
   */  
  template <typename M, typename F, typename C, typename T>
  inline
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& s, const vector<T>& v); 
  
  /** 
   * \brief Reads a vector of values from a CoNLL stream
   */  
  template <typename M, typename F, typename C, typename T>
  inline
  CoNLLBasicStream<M,F,C>& operator>>(CoNLLBasicStream<M,F,C>& strm, vector<T>& v);

}



namespace std {

  template<typename C>
  ostream& operator<<(ostream& o, const treeler::CoNLLBasicColumn<C>& a) {
    bool first = true;
    for (auto it = a.begin(); it!=a.end(); ++it) {
      if (!first) o << " ";
      o << *it;
      first = false;
    }
    return o;
  }
 
  template<typename M, typename F, typename C>
  ostream& operator<<(ostream& o, const treeler::CoNLLBasicStream<M,F,C>& s) {
    s.write(o);
    return o;    
  }
  
  template<typename M, typename F, typename C>
  istream& operator>>(istream& i, treeler::CoNLLBasicStream<M,F,C>& s) {
    bool b = s.read(i);
    if (!b) {
      i.setstate(std::ios::failbit);
    }
    return i;    
  }
}


/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
 *******************************************************************************
 * Implementation of template methods
 */

namespace treeler {

  template<typename M, typename F, typename C>
  bool CoNLLBasicStream<M,F,C>::read(istream& in) {
    A.clear(); 
    good = true;
    read_ptr = 0;
    int m = 0;   // num rows so far
    int N = 0;   // num cols so far
    while (1) {
      StringType line; 
      if (!getline(in, line)) return false;
      //      cerr << "ioconll : line is " << line << endl;
      basic_istringstream<CharType> iss(line); 
      int n = 0;   // num fields
      // put line in vector row
      vector<StringType> row; 
      StringType t; 
      while (iss >> t) {
	++n;
	row.push_back(t);
      }
      // empty line
      if (n==0) {
	good = m!=0;
	return good;
      }
      if (m==0) {
	N = n;
	A.resize(N);
      }
      if (m>0 and n!=N) {
	good = false;
	basic_ostringstream<CharType> oss; 
	oss << "wrong number of fields in line! (current " << n << ", previous " << N << ")" <<  endl << ">>>" << line << "<<<";
	TreelerException e("conllstream", oss.str());
	throw e;
      }
      for (int i=0; i<N; ++i) {
	A[i].push_back(row[i]);
      }
      ++m;
    }
  }

  template<typename M, typename F, typename C>
  void CoNLLBasicStream<M,F,C>::write(ostream& o) const {
    if (this->add_padding) {
      size_t m = this->num_columns();
      
      // compute the max width of each column
      vector<size_t> maxes(m, this->EMPTY.length());
      for (size_t i=0; i<m; ++i) {
	const CoNLLBasicColumn<C>& c = this->A[i];
	size_t& max = maxes[i]; 
	for (unsigned int j=0; j<c.size(); ++j) {
	  if (c[j].length()>max) max = c[j].length();
	}
      }
      
      // write the columns
      size_t n = this->num_rows();
      for (size_t i=0; i<n; ++i) {
	for (size_t j=0; j<m; ++j) {
	  const StringType& v = this->A[j][i];
	  if (j==0 and this->prefix!="") o << this->prefix << this->SEP;
	  if (j>0) o << this->SEP;
	  size_t k; 
	  if (v.empty()) {
	    o << this->EMPTY; 
	    k = this->EMPTY.length();
	  }
	  else {
	    o << v;
	    k = v.length();
	  }
	  for (; k<maxes[j]; ++k) {
	    o << this->SEP;
	  }
	}
	o << endl;
      }
      o << this->prefix << endl;
    }
    else {
      int m = this->num_columns();
      int n = this->num_rows();
      for (int i=0; i<n; ++i) {
	for (int j=0; j<m; ++j) {
	  if (j==0 and this->prefix!="") o << this->prefix << this->SEP;
	  if (j>0) o << this->SEP;
	  const StringType& v = this->A[j][i];
	  if (v.empty()) {
	    o << this->EMPTY;
	  }
	  else {
	    o << v; 
	  }
	}
	o << endl;
      }
      o << this->prefix << endl;
    }
  }

  template<typename M, typename F, typename C>
  void CoNLLBasicStream<M,F,C>::add_ids(int n, int off) {
    A.push_back(CoNLLBasicColumn<C>(n));
    CoNLLBasicColumn<C>& ids = A.back();
    basic_ostringstream<CharType> oss;
    for (int i=0; i<n; ++i) {
      oss << i+off;
      ids[i] = oss.str();
      oss.str("");
    }
  }


  //--------------------------------------------------------------------------

  template <typename M, typename F, typename C, typename T>
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& s, const T& t) {
    size_t n = s.num_rows();
    if (n==0) n=1;
    s.A.emplace_back( CoNLLBasicColumn<C>(n, t) );
    return s;
  }

  template <typename M, typename F, typename C, typename T>
  inline
  CoNLLBasicStream<M,F,C>& operator<<(CoNLLBasicStream<M,F,C>& s, const vector<T>& v) {
    // cerr << ">> operator<<(mapping, vector<" << typeid(T).name() << ">)" << endl; 
    size_t n = s.num_rows();
    if (n==0) n=v.size();
    s.A.emplace_back( CoNLLBasicColumn<C>(n) );
    typename vector<T>::const_iterator it = v.begin();
    const typename vector<T>::const_iterator it_end = v.begin()+n;
    typename CoNLLBasicColumn<C>::iterator dest = s.A.back().begin();	 
    for (; it != it_end; ++it, ++dest) {
      basic_stringstream<typename CoNLLBasicStream<M,F,C>::CharType> ss; 
      ss << *it;
      ss >> *dest;
    }
    return s;
  }
  
  template <typename M, typename F, typename C, typename T>
  inline
  CoNLLBasicStream<M,F,C>& operator>>(CoNLLBasicStream<M,F,C>& strm, vector<T>& v) {
    v.clear();
    if (strm.read_ptr >= strm.num_columns()) {
      strm.good = false;
      return strm;
    }
    int n = strm.num_rows();
    v.resize(n);
    const CoNLLBasicColumn<C>& a = strm.A[strm.read_ptr];
    typename CoNLLBasicColumn<C>::const_iterator it = a.begin();
    const typename CoNLLBasicColumn<C>::const_iterator it_end = a.end();
    typename vector<T>::iterator dest = v.begin();
    for (; it!=it_end; ++it, ++dest) {
      basic_stringstream<typename CoNLLBasicStream<M,F,C>::CharType> ss; 
      ss << *it;
      ss >> *dest;
    }
    ++strm.read_ptr;
    return strm;
  }


  //--------------------------------------------------------------------------


}


#endif /* TREELER_CONLLSTREAM_H */
