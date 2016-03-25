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
 * \file   part-dep1.h
 * \brief  Declaration of PartDep1
 * \author Xavier Carreras
 */

#ifndef DEP_PARTDEP1_H
#define DEP_PARTDEP1_H

#include <iostream>
#include <cassert>
#include <iterator>
#include <algorithm>

#include "treeler/base/windll.h"

#include "treeler/base/label.h" 
#include "treeler/dep/dep-symbols.h"
#include "treeler/dep/dep-tree.h"

namespace treeler {

  class PartDep1; 

  std::ostream& operator<<(std::ostream & o, const PartDep1& p);

  template <typename R, typename T, typename INDEX = typename R::Index>
  class array_map;

  
  /**
   * \brief A first-order part for dependency parsing
   * \author Xavier Carreras
   * 
   *  A first order part <h,m,l> where
   *      h, m are head and modifier positions  
   *      l is the label of the dependency (for unlabeled, constant to 0)
   *  
   *  It defines an iterator to loop over parts
   * 
   *  It also defines an per-sentence indexing where
   *     index(h,m,l) = h + N*m + N*N*l
   *  assuming that N is the number of tokens of the sentence
   *
   */
  class PartDep1 {
  public:
    int h, m, l;
  public:
    
    //! a configuration struct to generate parts
    struct Configuration {
      int L;
      Configuration() : L(1) {}
      Configuration(int l) : L(l) {}
    };
    
    // returns the number of parts in an input of size c.N
    template <typename X>
    static int num_parts(const Configuration& c, const X& x) { 
      const int N =  x.size(); 
      return N*N*c.L*2; 
    } 
    
    //! construct a null part
    PartDep1() : h(-1), m(-1), l(-1) {}
    
    //! construct an unlabeled dependency 
    PartDep1(int h0, int m0) : h(h0), m(m0), l(0) {
      if (h==m) h=-1;
    }

    //! construct a labeled dependency 
    PartDep1(int h0, int m0, int l0) : h(h0), m(m0), l(l0) {
      if (h==m) h=-1;
    }    

    inline int head() const { return h; }
    inline int mod() const { return m; }
    inline int label() const { return l; }
    
    bool operator==(const PartDep1& other) const {
      if (l==-1 and other.l==-1) return true;
      if (h != other.h) return false;
      if (m != other.m) return false;
      if (l != other.l) return false;
      return true;      
    }
    
    bool operator<(const PartDep1& other) const {
      if (h < other.h) return true;
      else if (h == other.h) {
	if (m < other.m) return true;
	else if (m == other.m) {
	  if (l < other.l) return true;
	  else return false;
	}
      }
      return false;
    }
    
    // increments the part by i steps
    // mod is the major coordinate, head is the mid coordinate, label is the lower coordinate
    void increment(int N, int L, int i) {
      l += i;
      if (l>=L) {
	h+=l/L;       
	l=l%L;
	if (m==h) ++h;
	if (h>=N) {
	  m+=(h+1)/(N+1);
	  h=(h+1)%(N+1)-1;
	  if (m>=N) { h=-1; m=-1; l=-1; }
	  else if (m==h) ++m;	  
	}
      }	
    }

    // head is the major coordinate, mod is the mid coordinate, label is the lower coordinate
    void increment_hmajor(int N, int L, int i) {
      l += i;
      if (l>=L) {
	m+=l/L;       
	l=l%L;
	if (m==h) ++m;
	if (m>=N) {
	  h+=m/N;
	  m=m%N;
	  if (h>=N) { h=-1; m=-1; l=-1; }
	  else if (m==h) ++m;	  
	}
      }	
    }
    
    //! An iterator of parts
    class iterator: public std::iterator<std::input_iterator_tag, PartDep1> {
      Configuration _c;
      int _N;
      PartDep1* _r;       
    public:
      iterator() 
	: _c(0), _N(0), _r(new PartDep1(-1,-1,-1)) {}

      iterator(int N, int L, int h, int m, int l) 
	: _c(L), _N(N), _r(new PartDep1(h, m, l)) {}

      iterator(const iterator& other) = delete;
      //	: _c(other._c.L), _N(other._N), _r(new PartDep1(*other._r)) {}

      // move 
      iterator(iterator&& other)
      : _c(other._c.L), _N(other._N), _r(other._r) 
      {
	cerr << "(moving iterator " << _r << ")" << flush; 
	other._r = NULL;
      }

      ~iterator() { delete _r; }	


    private : 
      iterator& operator=(const iterator& other) 
      {
	_N = other._N; 
	_c.L = other._c.L; 
	if (_r==NULL) _r = new PartDep1(other._r->h, other._r->m, other._r->l);
	else {
	  _r->h = other._r->h; 
	  _r->m = other._r->m; 
	  _r->l = other._r->l; 
	}	
	return *this;
      };

    public: 
      
      iterator& operator++() { 
	(*_r).increment(_N, _c.L, 1);
	return *this;
      }
      
      bool operator==(const iterator& rhs) const {return *_r==*(rhs._r);}
      bool operator!=(const iterator& rhs) const { return not (*this==rhs);}
      PartDep1& operator*() {return *_r;}     
      const PartDep1& operator*() const {return *_r;}
    };


    template <typename X> 
    static iterator begin(const Configuration& c, const X& x) { return (iterator(x.size(), c.L, -1, 0, 0)); } 

    static iterator begin(const Configuration& c, int N) { return iterator(N, c.L, -1, 0, 0); } 

    template <typename X> 
    static iterator begin(const X& x, int L) { return iterator(x.size(), L, -1, 0, 0); } 

    static iterator end() { return iterator(0, 0, -1, -1, -1); }


    // COMPOSITION AND DECOMPOSITION
    

    static void decompose(const DepVector<int>& yin, Label<PartDep1>& yout);

    static Label<PartDep1>&& decompose(const DepVector<int>& y);

    template <typename X>
    static void decompose(const DepSymbols& sym, const X& x, const Label<PartDep1>& y, Label<PartDep1>& parts) {
      parts = y;
    }

    template <typename X, typename LabelT>
    static void decompose(const DepSymbols& sym, const X& x, const DepVector<LabelT>& y, Label<PartDep1>& parts) {
      parts.clear(); 
      for (size_t m = 0; m < y.size(); ++m) {
	const HeadLabelPair<LabelT>& r = y[m];
	parts.push_back(PartDep1(r.h, m, sym.map_field<SYNTACTIC_LABEL,int>(r.l))); 
      }
    }
    
    template <typename X, typename LabelT>
    static Label<PartDep1>&& decompose(const DepSymbols& sym, const X& x, const DepVector<LabelT>& y) {
      Label<PartDep1> parts; 
      for (size_t m = 0; m < y.size(); ++m) {
	parts.push_back(PartDep1(y[m].h, m, sym.map_field<SYNTACTIC_LABEL,int>(y[m].l))); 
      }
      return std::move(parts);
    }


    static void compose(const Label<PartDep1>& yin, DepVector<int>& yout);

    template <typename X> 
    static void compose(const X& x, const Label<PartDep1>& yin, DepVector<int>& yout) {
      yout.resize(x.size(), HeadLabelPair<int>()); 
      auto r = yin.begin(); 
      auto r_end = yin.end(); 
      for (; r!=r_end; ++r) {
	HeadLabelPair<int>& hl = yout[r->m];
	hl.h = r->h; 
	hl.l = r->l; 
      }    
    }
    
    template <typename X, typename LabelT> 
    static void compose(const DepSymbols& sym, const X& x, const Label<PartDep1>& yin, DepVector<LabelT>& yout) {
      yout.resize(x.size(), HeadLabelPair<LabelT>()); 
      auto r = yin.begin(); 
      auto r_end = yin.end(); 
      for (; r!=r_end; ++r) {
	HeadLabelPair<LabelT>& hl = yout[r->m];
	hl.h = r->h; 
	hl.l = sym.map_field<SYNTACTIC_LABEL,LabelT>(r->l); 
      }    
    }



    // INDEXING 

    /**
     *  \brief An operator that retunrs unique indices for the parts of one
     *   pattern. 
     * 
     *  Indices are non-zero integers between 0 and T-1, where T is the
     *  total number of parts for the pattern.
     * 
     */
    struct Index {
    private:
      int _L; 
      int _N, _NN; 
    public:
      
      // constructors, by directly giving the appropriate dimensions or by 
      // giving a configuration struct and/or a pattern
      //
      Index(int num_labels) : _L(num_labels), _N(0), _NN(_N*_N) {}

      Index(int num_labels, int size_x) : _L(num_labels), _N(size_x), _NN(_N*_N) {}
    
      Index(PartDep1::Configuration c, int size_x) : _L(c.L), _N(size_x), _NN(_N*_N) {}
    
      template <typename X>
      Index(int num_labels, const X& x) : _L(num_labels), _N(x.size()), _NN(_N*_N) {}
    
      template <typename X>
      Index(PartDep1::Configuration c, const X& x) : _L(c.L), _N(x.size()), _NN(_N*_N) {}
  
      void resize(int size_x) { _N = size_x; _NN = _N*_N; }

      size_t num_values() const { return _NN*_L; }

      size_t size() const { return _N; }

      size_t num_labels() const { return _L; }
    
      inline
      size_t operator()(const PartDep1& r) const {
	return (*this)(r.h, r.m, r.l); 
      }

      inline
      size_t operator()(int h, int m, int l) const {
	if (h==-1) h=m;
	return h + _N*m + _NN*l;
      }

      inline
      void decode(size_t idx, int& h, int& m, int& l) const {
	l = idx / _NN;
	idx = idx % _NN;
	m = idx / _N;
	h = idx % _N;
	if (h==m) h=-1;
      }
      
      inline
      void decode(size_t idx, PartDep1& r) const {
	decode(idx, r.h, r.m, r.l); 
      }
      
    };

    template<typename T, typename INDEX=Index>
    class map : public array_map<PartDep1,T,INDEX>
    {
    public:
      //      using array_map<PartDep1,T,Index>::array_map<PartDep1,T,Index>;
      map<T,INDEX>() : array_map<PartDep1,T,INDEX>() {}
      map<T,INDEX>(const Configuration& c, int n, const T& ini) : array_map<PartDep1,T,INDEX>(c, n, ini) {}
      
      template <typename X>
      map<T,INDEX>(const Configuration& c, const X& x, const T& ini) : array_map<PartDep1,T,INDEX>(c, x, ini) {}
    };
    
    
  };



  // STORAGE (indexed by parts)
  
  //! A map indexed by parts
  template <typename R, typename T, typename INDEX>
  class array_map {
  private:    
    typename R::Configuration _c;
    INDEX _idx;
    T* _array;
  public:
    array_map<R,T,INDEX>() : _c(0), _idx(0), _array(nullptr) 
    {}	
    
    array_map<R,T,INDEX>(const typename R::Configuration& c, int n, const T& ini) : _c(c), _idx(_c,n) {	
      _array = new T[_idx.num_values()];
      std::fill(_array, _array+_idx.num_values(), ini); 
    }       
    
    template <typename X>
    array_map<R,T,INDEX>(const typename R::Configuration& c, const X& x, const T& ini) : _c(c), _idx(_c,x.size()) {
      _array = new T[_idx.num_values()];
      std::fill(_array, _array+_idx.num_values(), ini); 
    }       
    
    ~array_map<R,T,INDEX>() {
      delete [] _array;
    }
    
    /** 
     * initializes the map from scratch, with a new config and size
     */
    void initialize(const typename R::Configuration& c, int n, const T& ini) {
      _c = c;
      _idx = INDEX(_c, n); 	
      if (_array!=nullptr) delete [] _array;
      _array = new T[_idx.num_values()];
      std::fill(_array, _array + _idx.num_values(), ini); 
    }        
      
    template<typename X>
    void initialize(const typename R::Configuration& c, const X& x, const T& ini) {
      initialize(c, x.size(), ini); 
    }       

    void clear() {
      if (_array!=nullptr) {
	delete [] _array;
	_array = nullptr;
      }
      _c = R::Configuration(0); 
      _idx = INDEX(0);
    }

    
    /** 
     * recreates the table of T values, deleting all current values
     */
    void resize(int n, const T& ini) {
      delete [] _array;
      _idx.resize(n); 	
      _array = new T[_idx.num_values()];
      std::fill(_array, _array + _idx.num_values(), ini); 
    }
    
    const T& operator() (const R& r) const {
      return _array[_idx(r)];
    }
    
    const T& operator() (int h, int m, int l) const {
      return _array[_idx(h,m,l)];
    }      
    
    T& operator()(const R& r) {
      return _array[_idx(r)];
    }
    
    T& operator()(int h, int m, int l) {
      return _array[_idx(h,m,l)];
    }      
    
    //! A non-const iterator over the contents of the map
    class iterator : public std::iterator<std::input_iterator_tag, pair<const R&,T&> > {
      typedef pair<const R&,T&> value_type;
    private:
      INDEX* _index;
      R _key;
      T* _value;
      size_t _limit, _i;
      value_type* _kv;
      
    public:
      iterator(array_map<R,T,INDEX>& m) 
	: _index(&m._idx), _key(), _value(m._array), _limit(_index->num_values()), _i(0), _kv(NULL)
      {
	if (_i < _limit) {
	  _index->decode(_i, _key);
	  _kv = new value_type(_key, *_value);
	}	  
      }
	
      iterator() 
	: _index(NULL), _value(NULL), _limit(-1), _i(-1), _kv(NULL)
      {
      }
		
      typename array_map<R,T,INDEX>::iterator& operator++() { 
	++_i;
	if (_i < _limit) {
	  _index->decode(_i, _key);
	  ++_value;
	  delete _kv; 
	  _kv = new value_type(_key, *_value); 
	}
	else {
	  delete _kv; 
	  _i = -1;
	}
	return *this;
      }
      
      value_type* operator->() { return _kv; }
      
      string tostring() {
	ostringstream o; 
	if (_kv == NULL) {
	  o << "[map::it NULL]"; 
	}
	else {
	  o << "[" << _kv->first << " => " << *_value << "]"; 
	}
	  return o.str(); 
      }

      bool operator==(const iterator& rhs) const { return _i==(rhs._i);}
      bool operator!=(const iterator& rhs) const { return _i!=(rhs._i);}
    };
    

    typename array_map<R,T,INDEX>::iterator begin() { return array_map<R,T,INDEX>::iterator(*this); } 
    
    typename array_map<R,T,INDEX>::iterator end() { return array_map<R,T,INDEX>::iterator(); }
    
  };


}



#endif

