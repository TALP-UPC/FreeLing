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
 * \file   part-dep2.cc
 * \brief  Declaration of PartDep2
 * \author Xavier Carreras
 */

#ifndef DEP_PARTDEP2_H
#define DEP_PARTDEP2_H

#include <iostream>
#include <queue>

#include "treeler/base/windll.h"

using namespace std; 

#include "treeler/dep/dep-tree.h" 
#include "treeler/dep/dep-symbols.h"
#include "treeler/base/label.h"
#include "treeler/io/conllstream.h"


namespace treeler {

  class PartDep2 {
  public:
    /* this enumeration defines the types of 2nd order parts : 
       FO  : first-order part
       SIB : sibling relation, 
       CMI : child of the modifier inside the main dependency
       CMO : child of the modifier outside the main dependency
    */
    enum type_t { FO, SIB, CMI, CMO };

  public:
    type_t type;
    int  h, m, c, l;

  public:
    //! a configuration struct to generate parts
    struct Configuration {
      int L;
      bool do_sib, do_gc; 
      Configuration() : L(-1), do_sib(true), do_gc(true) {}
      Configuration(int l) : L(l), do_sib(true), do_gc(true) {}
      Configuration(int l, bool dosib, bool dogc) : L(l), do_sib(dosib), do_gc(dogc) {}
    };
    
    //! construct a null dependency 
    PartDep2()
      : type(FO), h(-1), m(-1), c(-1), l(-1)
    {}

    //! construct an unlabeled dependency 
    PartDep2(type_t t0, int h0, int m0, int c0)
      : type(t0), h(h0), m(m0), c(c0), l(0)
    {}

    //! construct a labeled dependency 
    PartDep2(type_t t0, int h0, int m0, int c0, int l0)
      : type(t0), h(h0), m(m0), c(c0), l(l0)
    {}

    int head() const { return h; }
    int mod() const { return m; }
    int child() const { return c; }
    int label() const { return l; }
    
    bool operator==(const PartDep2& other) const {
      if (type != other.type) return false; 
      if (m != other.m) return false;
      if (h != other.h) return false;
      if (c != other.c) return false;
      if (l != other.l) return false;
      return true;      
    }
    
    bool operator<(const PartDep2& other) const {
      if (m < other.m) return true;
      if (m == other.m) {	  
	if (type < other.type) return true;
	if (type == other.type) {
	  if (h < other.h) return true;
	  if (h == other.h) {
	    if (c < other.c) return true;
	    if (c == other.c) {
	      if (l < other.l) return true;
	    }
	  }
	}
      }	  
      return false;
    }


    // increments the part tuple by one, independendly of whether the assignment is valid
    // returns true if the increment can be done, otherwise false
    bool increment_blind(int N, int L, bool do_sib, bool do_gc) {
      ++l;
      if (l==L) {
	l = 0;
	++c;
	if (c==N) {
	  c = -1;
	  ++h;
	  if (h==N) {
	    h = -1; 
	    if (type == FO) type = SIB; 
	    else if (type == SIB) type = CMI; 
	    else if (type == CMI) type = CMO; 
	    else {	     
	      type = FO; 
	      ++m;
	      if (m == N) {
		return false;
	      }
	    }
	  }
	}
      }
      return true;
    }

    // returns true if the values of the part tuple are semantically valid
    bool is_valid() {
      if (m==h) return false;
      if (type == FO) return (c==-1);
      if (type == SIB or type == CMI) {
	if (h < m) return (c==-1 or (h<c and c<m)); 
	else         return (c==-1 or (m<c and c<h)); 
      }
      if (type == CMO) {
	if (h < m) return (c==-1 or m<c); 
	else         return (c==-1 or c<m); 
      }
      return false;
    }
      
    // increments the part by 1 step
    // coordinate precedence from major to minor: type, mod, head, child, label is the lower coordinate
    void increment(int N, int L, bool do_sib, bool do_gc) {
      bool v = increment_blind(N, L, do_sib, do_gc); 
      while (v and !is_valid()) {
	v = increment_blind(N, L, do_sib, do_gc);
      }
      if (!v) {
	// set it to NULL 
	type = FO; 
	m = h = c = l = -1;	
      }
    }

    //! An iterator of parts
    class iterator: public std::iterator<std::input_iterator_tag, PartDep2> {
      Configuration _c;
      int _N;
      PartDep2* _r; 
    public:
      iterator(int N, int L, bool do_sib, bool do_gc, type_t t, int h, int m, int c, int l) 
	: _c(L, do_sib, do_gc), _N(N), _r(new PartDep2(t, h, m, c, l)) 
      {}
      
      ~iterator() { delete _r; }	
      
      iterator& operator++() { 
	(*_r).increment(_N, _c.L, _c.do_sib, _c.do_gc);
	return *this;
      }

      bool operator==(const iterator& rhs) const {return *_r==*(rhs._r);}
      bool operator!=(const iterator& rhs) const { return not (*this==rhs);}
      PartDep2& operator*() {return *_r;}     
    };

    template <typename X>
    static iterator begin(const X& x, int L) { return iterator(x.size(), L, true, true, PartDep2::FO, -1, 0, -1, 0); } 
    
    template <typename X>
    static iterator begin(const Configuration& c, const X& x) { 
      return iterator(x.size(), c.L, c.do_sib, c.do_gc, PartDep2::FO, -1, 0, -1, 0); 
    } 

    static iterator end() { return iterator(0, 0, true, true, PartDep2::FO, -1, -1, -1, -1); }
    
    
    /**
       \brief Extracts the parts of dependency tree t into y
    */
    template <typename X, typename LabelT>
    static void decompose(const DepSymbols& sym, const X& x, const DepTree<LabelT>& y, Label<PartDep2>& parts);

    template <typename X, typename LabelT>
    static void decompose(const DepSymbols& sym, const X& x, const DepVector<LabelT>& y, Label<PartDep2>& parts);
    
    template <typename X, typename LabelT> 
    static void compose(const DepSymbols& sym, const X& x, const Label<PartDep2>& yin, DepVector<LabelT>& yout);

    template <typename X> 
    static void compose(const X& x, const Label<PartDep2>& yin, DepVector<int>& yout);
      
    template<class PartScorer>
    double score(PartScorer& S) const {
      /* dispatch according to type */
      switch(type) {
      case FO: return S.fo();
      case SIB: return S.ch();
      case CMI:return S.cmi();
      case CMO:return S.cmo();
      default: return 0;
      }
    }

  };    

  std::ostream& operator<<(std::ostream & o, const PartDep2& p);
  std::istream& operator>>(std::istream & i, PartDep2& p);


  template <typename Mapping, typename Format, typename CharT>
  CoNLLBasicStream<Mapping,Format,CharT>& operator<<(CoNLLBasicStream<Mapping,Format,CharT>& strm, const Label<PartDep2>& y) {
    DepVector<int> tmp;
    auto r = y.begin(); 
    auto r_end = y.end(); 
    for (; r!=r_end; ++r) {
      if (r->type == PartDep2::FO) {
	int n = r->m > r->h ? r->m : r->h; 
	if (n>=(int)tmp.size()) tmp.resize(n+1, HeadLabelPair<int>());
	HeadLabelPair<int>& hl = tmp[r->m];
	hl.h = r->h; 
	hl.l = r->l; 
      }
    }    
    strm << tmp; 
    return strm;     
  }

    

  /* Implementation of template functions */


  template <typename X, typename LabelT> 
  void PartDep2::compose(const DepSymbols& sym, const X& x, const Label<PartDep2>& parts, DepVector<LabelT>& y) {
    y.resize(x.size(), HeadLabelPair<LabelT>()); 
    auto r = parts.begin(); 
    auto r_end = parts.end(); 
    for (; r!=r_end; ++r) {
      if (r->type == PartDep2::FO) {
	HeadLabelPair<LabelT>& hl = y[r->m];
	hl.h = r->h; 
	hl.l = sym.template map_field<SYNTACTIC_LABEL,LabelT>(r->l); 
      }
    }    
  }

  template <typename X> 
  void PartDep2::compose(const X& x, const Label<PartDep2>& parts, DepVector<int>& y) {
    y.resize(x.size(), HeadLabelPair<int>()); 
    auto r = parts.begin(); 
    auto r_end = parts.end(); 
    for (; r!=r_end; ++r) {
      if (r->type == PartDep2::FO) {
	HeadLabelPair<int>& hl = y[r->m];
	hl.h = r->h; 
	hl.l = r->l; 
      }
    }    
  }

  template <typename X, typename LabelT> 
  void PartDep2::decompose(const DepSymbols& sym, const X& x, const DepVector<LabelT>& y, Label<PartDep2>& parts) {
    DepTree<LabelT> t = DepTree<LabelT>::convert(y);
    decompose(sym, x, t, parts); 
  }


  template <typename X, typename LabelT>
  void PartDep2::decompose(const DepSymbols& sym, const X& x, const DepTree<LabelT>& y, Label<PartDep2>& parts) {
    const bool verbose = false;
    const string prefix = "EXTRACT_PARTS :";
    //    y.print(cout, prefix);
    queue<DepTree<LabelT> const*> Q; 
    Q.push(&y);     
    while (!Q.empty()) {
      const DepTree<LabelT>* n = Q.front();
      Q.pop();      
      int head = n->idx();
      /* for each direction, left and right */
      for (int d=0; d<2; ++d) {
	typename DepTree<LabelT>::dir_t dir = d==0 ? DepTree<LabelT>::LEFT : DepTree<LabelT>::RIGHT;
	
	bool first = true;
	int sib = -1; 
	for (int i=0; i<n->num_children(dir); ++i) {
	  DepTree<LabelT> const* nn = &(n->child(dir, i));
	  int mod = nn->idx();
	  int label = sym.map_field<SYNTACTIC_LABEL,int>(nn->label());
	  if (verbose) cerr << prefix << " " << " [" << (d ? "R," : "L,") << head << "," << mod << "," << (first ? "+F" : "-F") << "] " << flush;	 

	  /* first-order part */	    
	  parts.insert(PartDep2(FO, head, mod, -1, label));
	  /* sibling part */
	  parts.insert(PartDep2(SIB, head, mod, sib, label));
	  
	  /* grandparent parts (for outermost grandchildren only) */
	  int cmo = -1, cmi = -1;
	  if (dir==DepTree<LabelT>::LEFT) {
	    /* left-left is cmo, left-right is cmi */	    
	    if(nn->nlc() > 0) { cmo = nn->lc(nn->nlc() - 1)->idx(); }
	    if(nn->nrc() > 0) { cmi = nn->rc(nn->nrc() - 1)->idx(); }
	  }
	  else {
	    /* right-left is cmi, right-right is cmo */	    
	    if(nn->nlc() > 0) { cmi = nn->lc(nn->nlc() - 1)->idx(); }
	    if(nn->nrc() > 0) { cmo = nn->rc(nn->nrc() - 1)->idx(); }
	  }
	  parts.insert(PartDep2(PartDep2::CMO, head, mod, cmo, label));
	  parts.insert(PartDep2(PartDep2::CMI, head, mod, cmi, label));
	  first = false;
	  sib = mod; 
	  Q.push(nn);
	}
	if (head==-1 and d==0) {
	  assert(first);
	}
	else { // stop
	  if (verbose) cerr << prefix << " " << " [" << (d ? "R," : "L,") << head << ",S," << (first ? "+F" : "-F") << "] " << flush;	  	  
	  int STOP = -1;
	  PartDep2 p(FO, head, STOP, sib);
	  //y.insert(p);
	  if (verbose) cerr << p << endl;	 
	}	
      }           
    }
  }

}

#endif
