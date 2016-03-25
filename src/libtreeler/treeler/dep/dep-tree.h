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
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   dep-tree.h
 * \brief  Declaration of class DepTree
 * \author Xavier Carreras and Terry Koo
 */
#ifndef DEP_DEPTREE_H
#define DEP_DEPTREE_H

/* DepTree : a child-list representation of a dependency tree. */

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cstddef>
#include <cstdlib>

#include "treeler/base/basic-sentence.h"

namespace treeler {

  enum DepFields {
    SYNTACTIC_LABEL = SentenceFields::__EXTENSION
  };

  template <typename LabelT>
  struct DepLabelTraits {
    static LabelT empty() { return LabelT(); } 
    static LabelT root() { return  LabelT(); } 
  };

  template <>
  struct DepLabelTraits<int> {
    static int null() { return 0; } 
    static int root() { return 0; } 
  };

  template <>
  struct DepLabelTraits<std::string> {
    static std::string null() { return ""; } 
    static std::string root() { return "*"; }
  };


  // a pair of head index and label 
  template <typename LabelT>
    struct HeadLabelPair {
    public: 
      int h; 
      LabelT l; 
      
    HeadLabelPair() : h(-1), l(DepLabelTraits<LabelT>::null()) {};
    HeadLabelPair(int h0) : h(h0), l(DepLabelTraits<LabelT>::null()) {};
    HeadLabelPair(int h0, const LabelT& l0) : h(h0), l(l0) {};        
    };
  
  template <typename LabelT>
  class DepVector : public std::vector<HeadLabelPair<LabelT>> {
  public:
    DepVector() 
      : std::vector<HeadLabelPair<LabelT>>() {}
    DepVector(size_t t) 
      : std::vector<HeadLabelPair<LabelT>>(t) {}    
  //  DepVector(const DepVector& v) = delete;
  };

  template <typename LabelT>
  class DepTree {
  private:
    int _idx;
    LabelT _label; /* label of dependency between current node and its parent */
    int _nlc;
    int _nrc;
    DepTree* _p;
    DepTree** _lc;
    DepTree** _rc;
    DepTree** _t; /* pointer to each subtree terminal, only allocated for the root deptree */

  public:
    enum dir_t {LEFT, RIGHT};

    DepTree(const int idx, const int nl, const int nr)
      : _idx(idx), _label(DepLabelTraits<LabelT>::null()), _nlc(nl), _nrc(nr), _p(NULL),
	_lc(new DepTree*[nl]), _rc(new DepTree*[nr]), _t(NULL) {}

    DepTree(const int idx, const LabelT& lab, const int nl, const int nr)
      : _idx(idx), _label(lab), _nlc(nl), _nrc(nr), _p(NULL),
	_lc(new DepTree*[nl]), _rc(new DepTree*[nr]), _t(NULL) {}

    // disabling the copy constructor
    DepTree(const DepTree<LabelT>& d) = delete; 

    // move 
    DepTree(DepTree<LabelT>&& d) 
    : _idx(d._idx), _label(d._label), _nlc(d._nlc), _nrc(d._nrc), 
      _p(d._p), _lc(d._lc), _rc(d._rc), _t(d._t) 
      { 
	// d._nlc = 0; 
	// d._nrc = 0; 
	// _p = NULL; 
	// _lc = NULL; 
	// _rc = NULL; 
	//	std::cerr << "(moving a dep-tree)" << std::endl;
      }

    ~DepTree();

    int idx() const { return _idx; }
    LabelT label() const { return _label; }

    /** 
     * Returns the number of children in direction d
     */
    int num_children(dir_t const& d) const { if (d==LEFT) return _nlc; return _nrc; }
    /** 
     * Returns a reference to the i'th child in direction d, in head-outwards order
     */
    DepTree const& child(dir_t const& d, int const i) const { if (d==LEFT) return *_lc[i]; return *_rc[i]; }

    /** 
     * Returns a reference to the parent DepTree, assuming it exists
     */
    const DepTree* parent() const { return _p; }
    
    /** 
     * Indicates if the current tree has a parent or is the root tree
     */
    bool has_parent() const { return _p != NULL; }

    void print(std::ostream& o, const std::string& prefix="", const int indent = 0) const;

    static DepTree<LabelT> && convert(const DepVector<LabelT>& v); 

    /** 
     * Returns the number of left children
     */
    int nlc() const { return _nlc; }
    /** 
     * Returns the number of right children
     */
    int nrc() const { return _nrc; }

    /** 
     * Returns a pointer to i-th left child, head-outwards
     */
    const DepTree* lc(const int i) const { return _lc[i]; }

    /** 
     * Returns a pointer to i-th right child, head-outwards
     */
    const DepTree* rc(const int i) const { return _rc[i]; }

    /** 
     * Returns a pointer to the dependency tree headed by the i-th token
     * It can onlly be called on the root tree
     */
    const DepTree* terminal(const int i) const { assert(_t!=NULL); return _t[i]; }

  };

  
  /** IMPLEMENTATION OF TEMPLATE METHODS **/ 

  
  template<typename LabelT>
  DepTree<LabelT>::~DepTree() {
    for(int i = 0; i < _nlc; ++i) { delete _lc[i]; }
    for(int i = 0; i < _nrc; ++i) { delete _rc[i]; }
    delete [] _lc;
    delete [] _rc;
    delete [] _t;
  }

  template<typename LabelT>
  void DepTree<LabelT>::print(std::ostream& o, const std::string& prefix, const int indent) const {
    std::stringstream root; 
    root << std::string(indent, ' '); 
    if (_p != NULL) {
      if (_p->_idx > _idx) 
	root << _p->_idx << " L ";
      else 
	root << _p->_idx << " R ";
    }

    for(int i = _nlc-1; i>=0; --i) {
      _lc[i]->print(o, prefix, root.str().length());
    }
    if (_p!=NULL) {
      o << prefix << root.str() << _idx << " (" << _label << ")" << std::endl;
    }
    for(int i = 0; i<_nrc; ++i) {
      _rc[i]->print(o, prefix, root.str().length());
    }        
    return; 
  }


  template <typename LabelT>
  DepTree<LabelT> && DepTree<LabelT>::convert(const DepVector<LabelT>& v) {
    const int n = v.size();
    const int np1 = n + 1;
    const int nn = np1*np1;
    int* const lc = np1 + (int*)malloc(nn*sizeof(int));
    int* const rc = np1 + (int*)malloc(nn*sizeof(int));
    int* const nl = 1 + (int*)calloc(np1, sizeof(int));
    int* const nr = 1 + (int*)calloc(np1, sizeof(int));

    /* accumulate left and right children */
    for(int i = 0; i < n; ++i) {
      const int h = v[i].h;
      if(h > i) {
	int* const lch = lc + h*np1;
	const int nlc = nl[h];
	lch[nlc] = i;
	nl[h] = nlc + 1;
      } else {
	int* const rch = rc + h*np1;
	const int nrc = nr[h];
	rch[nrc] = i;
	nr[h] = nrc + 1;
      }
    }


    /* create the root tree */
    DepTree<LabelT>*  rtree = new DepTree<LabelT>(-1, DepLabelTraits<LabelT>::root(), nl[-1], nr[-1]);
    
    /* create dep tree nodes for each token */
    DepTree<LabelT>** ttree = new DepTree<LabelT>*[n];
    for(int i = 0; i < n; ++i) {
      ttree[i] = new DepTree<LabelT>(i, v[i].l, nl[i], nr[i]);
    }
    
    /* assign children in head-outward order */
    for(int i = -1; i < n; ++i) {
      DepTree<LabelT>* t = (i<0) ? rtree : ttree[i];
      /* the right children are already in head-outward order */
      const int nrc = nr[i];
      int* const rch = rc + i*np1;
      DepTree<LabelT>** trc = t->_rc;
      for(int j = 0; j < nrc; ++j) {
	DepTree* c = ttree[rch[j]];
	c->_p = t;
	trc[j] = c;
      }
      /* the left children must be reversed */
      const int last = nl[i] - 1;
      int* const lch = lc + i*np1;
      DepTree<LabelT>** tlc = t->_lc;
      for(int j = last; j >= 0; --j) {
	DepTree* c = ttree[lch[j]];
	c->_p = t;
	tlc[last - j] = c;
      }
    }

    rtree->_t = ttree;

    free(lc - np1);
    free(rc - np1);
    free(nl - 1);
    free(nr - 1);

    return std::move(*rtree);
  }



}



#endif /* DEP_DEPTREE_H */
