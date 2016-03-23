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
 * \file   part-tag.h
 * \brief  Declaration of class PartTag 
 * \author Xavier Carreras
 */

#ifndef TAG_PARTTAG_H
#define TAG_PARTTAG_H

#include <iostream>
#include <cassert>
#include <iterator>

#include "treeler/base/windll.h"

#include "treeler/tag/tag-seq.h"
#include "treeler/tag/tag-symbols.h"
#include "treeler/base/label.h"

namespace treeler {

  /*
   * \brief A bigram part for tagging
   * 
   */
  class PartTag {
  public:
    // position
    int i; 
    // previous and current tags
    int a, b;
  public:
    
    //! a configuration struct to generate parts
    struct Configuration {
      int L;
      Configuration() : L(-1) {}
      Configuration(int l) : L(l) {}
    };

    // returns the number of parts in an input of size c.N
    template <typename X>
    static int num_parts(const Configuration& c, const X& x) { 
      return x.size()*c.L*c.L; 
    } 
    
    //! construct a null part
    PartTag() : i(-1), a(-1), b(-1) {}
    
    PartTag(int ii, int aa, int bb) : i(ii), a(aa), b(bb) {}
    
    //! construct by decoding
    PartTag(const Configuration& c, int q) {
      decode(c, q, this->i, this->a, this->b);
    }
    
    // i is the major coordinate, b is the mid coordinate, a is the lower coordinate
    static int index(const Configuration& c, int i, int a, int b) { return i*c.L*c.L + b*c.L + a; }
    static int index(const Configuration& c, const PartTag& r) { return index(c, r.i, r.a, r.b); }
    static void decode(const Configuration& c, int q, int& i, int& a, int& b) {
      const int LL = c.L*c.L; 
      i = q / LL;
      q = q % LL;
      b = q / c.L;
      a = q & c.L;
    }

    bool operator==(const PartTag& other) const {
      if (i != other.i) return false;
      if (b != other.b) return false;
      if (a != other.a) return false;
      return true;      
    }
        
    // i is the major coordinate, b is the mid coordinate, a is the lower coordinate
    bool operator<(const PartTag& other) const {
      if (i < other.i) return true;
      else if (i == other.i) {
	if (b < other.b) return true;
	else if (b == other.b) {
	  if (a < other.a) return true;
	  else return false;
	}
      }
      return false;
    }
  
    // increments the part by one step
    // i is the major coordinate, b is the mid coordinate, a is the lower coordinate
    void increment(int N, int L) {
      ++a;
      if (a>=L) {
	a=0;
	++b;
	if (b>=L) {
	  b=0;
	  ++i;
	  if (i>=N) { i=-1; a=-1; b=-1; }
	}
      }	
    }

    //! An iterator of parts
    class iterator: public std::iterator<std::input_iterator_tag, PartTag> {
      Configuration _c;
      int _N;
      PartTag* _r; 
    public:
      iterator(int N, int L, int i, int a, int b) 
	: _c(L), _N(N), _r(new PartTag(i,a,b)) {}

      ~iterator() { delete _r; }	
      
      iterator& operator++() { 
	(*_r).increment(_N, _c.L);
	return *this;
      }

      bool operator==(const iterator& rhs) const {return *_r==*(rhs._r);}
      bool operator!=(const iterator& rhs) const { return not (*this==rhs);}
      PartTag& operator*() {return *_r;}     
    };


    template <typename X> 
    static iterator begin(const X& x, int L) { return iterator(x.size(), L, 0, 0, 0); } 

    template <typename X> 
    static iterator begin(const Configuration& c, const X& x) { return iterator(x.size(), c.L, 0, 0, 0); } 

    static iterator end() { return iterator(0, 0, -1, -1, -1); }


    static void decompose(const TagSeq& s, Label<PartTag>& y) {
      int last = -1; 
      for (size_t i=0; i<s.size(); ++i) {
	PartTag r(i, last, s[i]);
	y.push_back(r);
	last = s[i];
      }
    }

    template <typename X>
    static void decompose(const TagSymbols& sym, const X& x, const TagSeq& y, Label<PartTag>& parts) {
      decompose(y, parts); 
    }
        
    //! A map indexed by parts
    template <class T>
    class map {
    public:
      template <typename X>
      map<T>(const Configuration& c, const X& x) : _c(c) {
	_array = new T[num_parts(c,x)];
      }       
      ~map<T>() {
	delete [] _array;
      }
      const T& operator[](const PartTag& r) {
	return _array[index(_c,r)];
      }      
      const T& operator[](int i) {
	return _array[i];
      }      
    private:    
      Configuration _c;
      T* _array;
    };

  };

  std::ostream& operator<<(std::ostream & o, const PartTag& r);

}



#endif
