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
 * \file   part-srl.h
 * \brief  Declaration of PartSRL
 * \author Xavier Carreras
 */

#ifndef DEP_PARTSRL_H
#define DEP_PARTSRL_H

#include <iostream>
#include <cassert>
#include <iterator>

#include "treeler/base/windll.h"

#include "treeler/base/score-dumper.h"

namespace treeler {
  namespace srl {
    
    /**
     * \brief A part for SRL
     * \author Xavier Carreras
     * 
     *  A SRL part with <p, a, r> where
     *      p is the pred token id
     *      a is the arg token id
     *      r is the role label (for unlabeled, constant to 0)
     *  
     */
    class PartSRL {
    public:
      int p_, a_, rl_;
      
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
	const int N =  x.size(); 
	return N*N*c.L*2; 
      } 
      
      //! construct a null part
      PartSRL() : p_(-1), a_(-1), rl_(-1)
      {}
      
      //! construct an unlabeled srl part
      PartSRL(int p, int a) : p_(p), a_(a), rl_(0)
      {}
      
      //! construct a labeled srl part
      PartSRL(int p, int a, int rl) : p_(p), a_(a), rl_(rl)
      {}
      
      // PartSRL(int p, int a, int rl, int head, int mod, int syn_label) : 
      // 	p_(p), a_(a), rl_(rl), head_(head), mod_(mod), syn_label_(syn_label) {
      // }
      
	          
      int pred() const {return p_; }
      int arg() const { return a_; }
      int rolelabel() const { return rl_; }
      
      int label() const {assert(false); return rl_;}
      
      // int head() const { assert(false); return head_; }
      // int mod() const { assert(false); return mod_; }
      // int synlabel() const { assert(false); return syn_label_; }
      
      bool operator==(const PartSRL& other) const {
	if (rl_ == -1 and other.rl_ == -1) return true;
	if (p_ != other.p_) return false;
	if (a_ != other.a_) return false;
	if (rl_ != other.rl_) return false;
	return true;      
      }
      
      bool operator<(const PartSRL& other) const {
	if (p_ < other.p_) return true;
	else if (p_ == other.p_) {
	  if (a_ < other.a_) return true;
	  else if (a_ == other.a_) {
	    if (rl_ < other.rl_) return true;
	    else return false;
	  }
	}
	return false;
      }
    
      
      void increment(int N, int L) {
	p_ = a_ = rl_ = -1;
      }
      


      // ITERATOR

      //! An iterator of parts
      class iterator: public std::iterator<std::input_iterator_tag, PartSRL> {
	Configuration _c;
	int _N;
	PartSRL* _r;       
      public:
	iterator() 
	  : _c(0), _N(0), _r(new PartSRL(-1,-1,-1)) {}
	
	iterator(int N, int L, int h, int m, int l) 
	  : _c(L), _N(N), _r(new PartSRL(h, m, l)) {}
	
	iterator(const iterator& other) = delete;
	
	// move 
	iterator(iterator&& other)
	  : _c(other._c.L), _N(other._N), _r(other._r) 
	{
	  other._r = NULL;
	}
	
	~iterator() { delete _r; }	
	
	
      private : 
	iterator& operator=(const iterator& other) 
	{
	  _N = other._N; 
	  _c.L = other._c.L; 
	  if (_r==NULL) _r = new PartSRL(other._r->p_, other._r->a_, other._r->rl_);
	  else {
	    _r->p_ = other._r->p_; 
	    _r->a_ = other._r->a_; 
	    _r->rl_ = other._r->rl_; 
	  }	
	  return *this;
	};
	
      public: 
	
	iterator& operator++() { 
	  (*_r).increment(_N, _c.L);
	  return *this;
	}
	
	bool operator==(const iterator& rhs) const {return *_r==*(rhs._r);}
	bool operator!=(const iterator& rhs) const { return not (*this==rhs);}
	PartSRL& operator*() {return *_r;}     
	const PartSRL& operator*() const {return *_r;}
      };
      
      
      template <typename X> 
      static iterator begin(const Configuration& c, const X& x) { return (iterator(x.size(), c.L, -1, -1, -1)); } 
      
      static iterator end() { return iterator(0, 0, -1, -1, -1); }
      
    
    };
    
  }
}
      
namespace std {  
  std::ostream& operator<<(std::ostream & o, const treeler::srl::PartSRL& p);
}


#endif
