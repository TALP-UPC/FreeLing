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
 * \file   parser-projdep2.h
 * \brief  Declaration of ProjDep2
 * \author Xavier Carreras
 */


#ifndef TREELER_PROJDEP2_H
#define TREELER_PROJDEP2_H

#include "treeler/dep/dep-tree.h"
#include "treeler/dep/dep-symbols.h"
#include "treeler/base/label.h"
#include "treeler/dep/part-dep2.h"
#include "treeler/io/io-basic.h"

#include<map>

using namespace std;

namespace treeler {



  /**
   *  \brief An second-order projective dependency parser
   *  \ingroup dep
   *  \author  Xavier Carreras, Terry Koo
   *  
   */
  class ProjDep2 {
  public:
    
    typedef PartDep2 R;
    
    struct Configuration {
      int  L;          /* number of edge labels */
      bool multiroot;  /* whether parsing is multiroot or not */
    };
    
    template <typename X, typename S>
    static double argmax(const Configuration& c,
			 const X& x,
			 S& scores,
			 Label<PartDep2>& y);    
    
    template <typename X, typename S>
    static double argmax(const Configuration& c,
			 const X& x,
			 S& scores,
			 DepVector<int>& y) {
      // a factored label
      Label<PartDep2> fy;
      double s = argmax(c,x,scores,fy); 
      convert(x,fy, y);
      return s; 
    }
    
  protected:
    Configuration _config;
    const DepSymbols& _symbols;

  public:

    ProjDep2(const DepSymbols& sym0) 
      : _symbols(sym0)
    {
      _config.L = _symbols.d_syntactic_labels.size();
    }
    
    Configuration& configuration() { return _config; }
    
    template <typename X, typename Y>
    void decompose(const X& x, const Y& y, Label<R>& parts) const {
      R::decompose(_symbols, x, y, parts); 
    }

    template <typename X, typename Y>
    void compose(const X& x, const Label<R>& parts, Y& y) const {
      R::compose(_symbols, x, parts, y); 
    }

    template <typename X, typename S>
    double argmax(const X& x,
		  S& scores,
		  Label<PartDep2>& y) {
      return argmax(_config, x, scores, y); 
    }

    template <typename X, typename S, typename LabelT>
    double argmax(const X& x,
		  S& scores,
		  DepVector<LabelT>& y) {
      Label<PartDep2> parts; 
      double s = argmax(_config, x, scores, parts); 
      compose(x, parts, y); 
      return s; 
    }

  protected:
    

    /* nested datatypes for inference routines */
    /*     struct U_signature; */
    /*     struct C_signature; */
    /*     struct U_backpointer; */
    /*     struct C_backpointer; */
    class outside_scores;

    /* chart entry for uncomplete dependency structures */
    /* defined by: head, modifier and label of dominating depencency */
    struct U_signature {
        int _h, _m, _l;
        
        bool operator< (const U_signature & s2) const {
            if (_h<s2._h) return true;
            else if (_h==s2._h) {
                if (_m<s2._m) return true;
                else if (_m==s2._m) {
                    return (_l<s2._l);
                }
            }
            return false;
        }
        
        U_signature(int h0, int m0, int l0)
            : _h(h0), _m(m0), _l(l0)
        {}
        
        int head() const { return _h; }
        int mod() const { return _m; }
        int label() const { return _l; }
    };

    /* the value of a U_signature chart entry */
    /* contains: splitting point, and positions of head's child and inner modifier's child */
    struct U_backpointer {
      int  _r,_ch,_cm;
      
      U_backpointer(int r0, int ch0, int cm0)
	: _r(r0), _ch(ch0), _cm(cm0)
      {}
    };
    
    /* chart entry for uncomplete dependency structures */
      /* defined by: head, modifier of dominating dependency, and end point of the sentence span */
    struct C_signature {
      int _h, _e, _m;
      
      bool operator< (const C_signature & s2) const {
	if (_h<s2._h) return true;
	else if (_h==s2._h) {
	  if (_e<s2._e) return true;
	  else if (_e==s2._e) {
	    return (_m<s2._m);
	  }
	}
	return false;
      }
      
      C_signature(int h0, int e0, int m0)
	: _h(h0), _e(e0), _m(m0)
          {}
      
      int head() const { return _h; }
      int end() const { return _e; }
          int mod() const { return _m; }
    };
      
    /* the value of a C_signature chart entry */
    /* contains: label of dominating dependency and outer child of modifier */
    struct C_backpointer {
      int _l, _cm;
      
      C_backpointer(int l0, int cm0)
	: _l(l0), _cm(cm0)
      {}
    };
    
    
    /* Structure of  back pointers in the chart
     * The pointers are indexed by head and modifier
     * Internally, pointers for head=-1 (root) are stored in the [m,m] position
     */
      struct chart_values {
	map<C_signature,C_backpointer> CBP;
	map<U_signature,U_backpointer> UBP;
        
	chart_values() {}
	~chart_values() {}
        
	void set_cbp(int h, int e, int m,  int l, int cm);
	void set_ubp(int h, int m, int l, int r, int ch, int cm);
	bool get_cbp(int h, int e, int m, int *l, int *cm) const;
	bool get_ubp(int h, int m, int l, int *r, int *ch, int *cm) const;
        
      };
    
    struct chart_scores {
      double *_CS; // scores for complete structures
      double *_US; // scores for uncomplete structures
      const int _N, _L, _NL, _N2, _N2L;
      
      chart_scores (int N0, int L0);
      ~chart_scores();
      
      double cscore (int h, int e, int m) const;
      void cscore_set (int h, int e, int m, double sco);
      double uscore (int h, int m, int l) const;
      void uscore_set (int h, int m, int l, double sco);
    };
      
    /* walk the path of backpointers to recover the viterbi tree */
    static void unravel_tree(const int N, Label<PartDep2>& y, const struct chart_values& CV,
			     const int h, const int e, const int m);
    

      /* TERRY: I changed this to use templated functors instead of a
         function; I think the use of a function with conditional if
         statements might be marginally slower than templated functors,
         which can be inlined */
      /* argmax parsing */
      
      /* template<class PartScorer> */
      /* double argmax_impl(const Pattern* const x, */
      /*                    PartScorer& part_score, */
      /*                    Label<PartDep2>& y); */
      /* /\* inside pass: fills IS with inside scores and returns logZ *\/ */
      
      /* template<class PartScorer> */
      /* double inside(const Pattern* const x, */
      /*               chart_scores& IS, */
      /*               PartScorer& part_scorer); */
      
      /* /\* outside pass: takes IS as input and fills ret_M with */
      /*    marginals *\/ */
      /* template<class PartScorer> */
      /* void outside(const Pattern* const x, */
      /*              const chart_scores& IS, */
      /*              const double logZ, */
      /*              PartScorer& part_scorer, */
      /*              double* const ret_M); */
      
      /* test the parser's inference routines against a brute-force
         method */
      template<class UseTree, class PartScorer>
      void bruteforce(const int N, UseTree& use, PartScorer& part_score);
      void testinf();
      
  protected:
      int _L;          /* number of edge labels */
      bool _multiroot; /* whether parsing is multiroot or not */
      
      /* whether to use a deptree-based method to construct labels */
      bool _use_deptree_labels;
      
  public:


      
      /* helper function that converts a first-order deptree
         representation to a second-order label */
      static void deptree2label(const bool multiroot, const int N,
                                const DepTree<int>* const root, Label<PartDep2>& y);
  };
}

#include "treeler/dep/parser-projdep2.tcc"

#endif /* TREELER_PROJDEP2_H */

