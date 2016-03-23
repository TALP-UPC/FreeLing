//////////////////////////////////////////////////////////////////
//
//    Treeler - Open-source Structured Prediction for NLP
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//                          02111-1307 USA
//
//    contact: Xavier Carreras (carreras@lsi.upc.es)
//             TALP Research Center
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef PARSER_PROJDEP1_H
#define PARSER_PROJDEP1_H

#include <iostream>

#include "treeler/dep/dep-symbols.h"
#include "treeler/dep/part-dep1.h"
#include "treeler/dep/dep-tree.h"
#include "treeler/base/label.h"

#include "treeler/base/windll.h"

namespace treeler {

  /**
   *  \brief An edge-factorized projective dependency parser
   *  \ingroup dep
   *  \author  Xavier Carreras, Terry Koo
   *  
   *  The argmax, partition, and marginals routines were written by
   *  Xavier Carreras.  The kbest routines were written by Terry Koo.
   *
   */
  class WINDLL ProjDep1 {

  public:

    typedef PartDep1 R; 

    struct Configuration {
      int  L;          /* number of edge labels */
      bool multiroot;  /* whether parsing is multiroot or not */
    };

    /* STATIC METHODS */ 

    template <typename X, typename S>
    static double argmax(const Configuration& c,
			 const X& x,
			 S& scores,
			 Label<PartDep1>& y);
    
    template <typename X, typename S>
    static double argmax(const Configuration& c,
			 const X& x,
			 S& scores,
			 DepVector<int>& y) {
      // a factored label
      Label<PartDep1> fy;
      double s = argmax(c,x,scores,fy); 
      R::compose(x,fy,y); 
      return s; 
    }

    template <typename X, typename S>
    static double partition(const Configuration& c, const X& x, S& scores);

    template <typename X, typename S, typename M>
    static double marginals(const Configuration& c, const X& x, S& scores, M& mu);


    /* NON-STATIC METHODS */ 
    
    template <typename X, typename Y, typename S>
    double argmax(const X& x,
		  S& scores,
		  Y& y) {
      Configuration& c = _config; 
      c.L = _symbols.d_syntactic_labels.size(); 
      // a factored label
      Label<PartDep1> fy;
      double s = argmax(c,x,scores,fy); 
      R::compose(_symbols,x,fy,y); 
      return s; 
    }
    
    // non-static method (uses symbols and configuration)
    // specialization for Y=Label<R>
    template <typename X, typename S>
    double argmax(const X& x,
		  S& scores,
		  Label<R>& y) {
      Configuration& c = _config; 
      c.L = _symbols.d_syntactic_labels.size(); 
      double s = argmax(c,x,scores,y); 
      return s; 
    }

    template <typename X, typename S>
    double partition(const X& x, S& scores) {
      Configuration& c = _config; 
      c.L = _symbols.d_syntactic_labels.size(); 
      return partition(c, x, scores); 
    }

    template <typename X, typename S, typename M>
    double marginals(const X& x, S& scores, M& mu) {
      Configuration& c = _config; 
      c.L = _symbols.d_syntactic_labels.size(); 
      return marginals(c, x, scores, mu); 
    }

    template <typename X, typename Y>
    void decompose(const X& x, Y& y, Label<R>& parts) const {
      R::decompose(_symbols, x, y, parts); 
    }

    template <typename X>
    R::iterator rbegin(const X& x) const { 
      R::Configuration c(_config.L); 
      return R::begin(c, x); 
    }

    R::iterator rend() const { 
      return R::end(); 
    }


  private:

    Configuration _config; 

  public:
    const DepSymbols& _symbols;

  public:

    ProjDep1(const DepSymbols& s)
      : _symbols(s)
    {}

    Configuration& configuration() { return _config; }

    

  private:
    template <typename X, typename S>
    class MyScores {
    public:
      MyScores(const X& x, S& sco) : _x(x), _sco(sco) {}

      double operator()(int h, int m, int l) const {
	PartDep1 r(h, m, l); 
	double s = _sco(r);
	// cerr << "SCORE " << _x.id() << " [" << h << " " << m << " " << l << "] " << s << endl;
	return s;
      }
    private:
      const X& _x;
      S& _sco; 
    };

    template <typename X, typename S>
    static double argmax_impl(const Configuration& c,
			      const X& x,
			      S& scores,
			      Label<PartDep1>& y);

    /* nested datatypes for inference routines */
    struct backp;
    class ChartIO;

    /* walk the path of backpointers to recover the viterbi tree */
    static void unravel_tree(Label<PartDep1>& y, const struct backp* BP,
			     const int h, const int m,
			     const bool complete, const int N);

    /* compute inside and outside scores for all parts */
    template <typename S>
    static void inside(const Configuration& c, S& scores, ChartIO& chart);

    template <typename S>
    static void outside(const Configuration& c, S& scores, ChartIO& chart);
    
    template <typename X>
    static void write_chart(std::ostream& o, const X& x, 
			    double** SCom, double** SInc, double* SRCom, double* SRInc, const struct backp& bp);
  };
}

#include "treeler/dep/parser-projdep1.tcc"

#endif /* PARSER_PROJDEP1_H */
