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
 * \file   viterbi.h
 * \brief  Inference methods for tagging, including Viterbi and Forward-Backward
 * \author Xavier Carreras
 */

#ifndef TAG_VITERBI_H
#define TAG_VITERBI_H

#include <iostream>
#include <iomanip>
#include <cassert>
#include <vector>
#include <tuple>
#include <cmath>

#include "treeler/tag/tag-symbols.h"

using namespace std;

namespace treeler {

  template <typename R>
  class TabularScores {
  private:
    int _n;
    int _L;
    int _L1;
    int _LL1;
    vector<double> _sco;
    
    inline size_t index(const R& r) const {
      return r.i*_LL1 + r.b*_L1 + r.a+1; 
    }
    
  public:
    TabularScores(int n, int L) : _n(n), _L(L), _L1(_L+1), _LL1(_L*_L1), _sco(_LL1*_n, 0)
      {}
    
    TabularScores(int n, int L, double v) : _n(n), _L(L), _L1(_L+1), _LL1(_L*_L1), _sco(_LL1*_n, v)
      {}
    
    void set_score(const R& r, double s) {
      _sco[index(r)]=s;
    }
    
    double operator()(const R& r) const {
      return _sco[index(r)]; 
    }          
  };
  

  /** 
   * \brief  Inference methods for tagging, including Viterbi and Forward-Backward
   * \ingroup tag 
   * \author Xavier Carreras   
   */
  class Viterbi {
  public:
    /** 
     * \brief Configuration of Viterbi inference
     * 
     */ 
    struct Configuration {
      int L;          //!< Number of output tags 
      string inf;     //!< Type of \c argmax inference (\c plain, \c mbr or \c mbr-count)
      int verbose;    //!< Verbosity level during inference
      double gamma;   //!< Scoring factor
    };

    typedef tuple<double,int> Value_t;
    typedef vector<Value_t> Alpha_t; 
    typedef vector<Alpha_t> Chart_t;
    typedef PartTag R;

  private:
    const TagSymbols& _symbols;
    Configuration _config;
  public:

    Viterbi(const TagSymbols& sym) 
      : _symbols(sym)
    {}

    Configuration& config() { return _config; }   

    template <typename X>
    void decompose(const X& x, const TagSeq& y, Label<R>& parts) const;

    template <typename X>
    void compose(const X& x, const Label<R>& parts, TagSeq& y) const;

    template <typename X, typename S>
    double argmax(const X& x, 
		  S& scores, 
		  TagSeq& y);

    template <typename X, typename S>
    double argmax(const X& x, 
		  S& scores, 
		  Label<PartTag>& y);

    /** 
     * \brief Computes the top-scored sequence of tags and returns its score.
     * 
     * \tparam X The type of input patterns
     * \tparam S The type of scores
     * \param config Configuration     
     * \param x The input pattern
     * \param scores The scores of each part for \c x 
     * \param y The output structure
     */ 
    template <typename X, typename S>
    static double argmax(const Configuration& config, 
			 const X& x, 
			 S& scores, 
			 Label<PartTag>& y);
    
    template <typename X, typename S>
    static double viterbi(const Configuration& config, 
			  const X& x, 
			  S& scores, 
			  Label<PartTag>& y);
    
    template <typename X, typename S>
    static double mbr(const Configuration& config, 
		      const X& x, 
		      S& scores, 
		      Label<PartTag>& y);
    
    template <typename X, typename S>
    static double fb(const Configuration& config, 
		     const X& x, 
		     S& scores, 
		     TabularScores<R>& mu);
    
    template <typename R>    
    static void write(ostream& o, const Chart_t& T, string label="");
    
    template <typename X, typename S>    
    static void dump(ostream& o, const Configuration& config, 
		     const X& x, 
		     S& scores, 
		     string label="");
    
    
    static inline double log_add(const double x, const double y) {
      if(x > y) {
	return x + log(1.0 + exp(y - x));
      } else {
	return y + log(1.0 + exp(x - y));
      }
    }
    
  };
  

  template <typename X>
  void Viterbi::decompose(const X& x, const TagSeq& y, Label<R>& parts) const {
    int last = -1; 
    for (int i=0; i<x.size(); ++i) {
      int cur = _symbols.template map_field<TAG,int>(y[i]);
      PartTag r(i, last, cur);
      parts.push_back(r);
      last = cur;
    }
  }

  template <typename X>
  void Viterbi::compose(const X& x, const Label<R>& parts, TagSeq& y) const {
    y.resize(x.size(),-1);
    for (auto r=parts.begin(); r!=parts.end(); ++r) {
      y[r->i] = r->b;
    }
  }


  template <typename X, typename S>
  double Viterbi::argmax(const X& x, 
			 S& scores, 
			 TagSeq& y) {
    Label<PartTag> parts; 
    double s = argmax(_config, x, scores, parts); 
    compose(x, parts, y); 
    return s; 
  }


  template <typename X, typename S>
  double Viterbi::argmax(const X& x, 
			 S& scores, 
			 Label<PartTag>& y) {
    return argmax(_config, x, scores, y); 
  }


  template <typename X, typename S>
  double Viterbi::argmax(const Configuration& config, 
			 const X& x, 
			 S& scores, 
			 Label<PartTag>& y) {
    if (config.inf=="mbr") {
      return mbr(config, x, scores, y); 
    }
    else if (config.inf=="mbr-count") {
      // count       
      TabularScores<PartTag> logones(x.size(), config.L, 0);
      return mbr(config, x, logones, y); 
    }
    else {
      return viterbi(config, x, scores, y); 
    }    
  }
  
  
  template <typename X, typename S>
  double Viterbi::viterbi(const Configuration& config, 
			  const X& x, 
			  S& scores, 
			  Label<PartTag>& y) {
    
    int n = x.size();
    Chart_t C(n, Alpha_t(config.L,make_tuple(-HUGE_VAL, -1))); 
    PartTag r(0,-1,0);
    {
      Alpha_t& alpha = C[0];
      for (r.b=0; r.b<config.L; ++r.b) {	
	get<0>(alpha[r.b]) = scores(r);
	get<1>(alpha[r.b]) = r.a;
      }
    }
    
    for (r.i=1; r.i<n; ++r.i) {
      Alpha_t& alphaP = C[r.i-1];
      Alpha_t& alpha = C[r.i];
      for (r.b=0; r.b<config.L; ++r.b) {
	bool first = true;
	double& maxs = get<0>(alpha[r.b]); 
	int& maxa = get<1>(alpha[r.b]); 
	for (r.a=0; r.a<config.L; ++r.a) {
	  if (first) {
	    maxs = scores(r) + get<0>(alphaP[r.a]);
	    maxa = r.a;
	    first = 0;
	  }
	  else {
	    double s = scores(r) + get<0>(alphaP[r.a]);
	    if (s>maxs) {
	      maxs = s;
	      maxa = r.a;	      
	    }
	  }
	}
      }
    }
    if (config.verbose>=2) {
      write<PartTag>(cerr, C, " VITERBI SCORES "); 
    }

    int maxa,maxb;
    double maxs;
    {    
      Alpha_t& alpha = C[n-1];
      maxb = 0;
      maxs = get<0>(alpha[0]);
      maxa = get<1>(alpha[0]);
      for (int l=1; l<config.L; ++l) {
	if (get<0>(alpha[l])>maxs) {
	  maxb = l;
	  maxs = get<0>(alpha[l]);
	  maxa = get<1>(alpha[l]);
	}
      }
    }
    y.insert(PartTag(n-1, maxa, maxb));
    for (int i=n-2; i>=0; --i) {
      maxb = maxa;
      maxa = get<1>(C[i][maxb]);
      y.insert(PartTag(i, maxa, maxb));
    }    

    return maxs; 
  }

  
  template <typename X, typename S>
  double Viterbi::mbr(const Configuration& config, 
		      const X& x, 
		      S& scores, 
		      Label<PartTag>& y) {
    
    TabularScores<PartTag> mu(x.size(), config.L);
    fb(config, x, scores, mu); 
    return viterbi(config, x, mu, y);  
  }
  
  
  template <typename X, typename S>
  double Viterbi::fb(const Configuration& config, 
		     const X& x, 
		     S& scores, 
		     TabularScores<R>& mu) {    
    int n = x.size();
    Chart_t FWD(n, Alpha_t(config.L,make_tuple(-HUGE_VAL, -1))); 
    Chart_t BWD(n, Alpha_t(config.L,make_tuple(-HUGE_VAL, -1))); 
    PartTag r(0,-1,0);
    
    if (config.verbose>=1) {
      dump(cerr, config, x, scores, " PART SCORES ");
    }

    // FWD Pass
    {
      Alpha_t& alpha = FWD[0];
      for (r.b=0; r.b<config.L; ++r.b) {	
	get<0>(alpha[r.b]) = scores(r)*config.gamma;
      }
    }    
    for (r.i=1; r.i<n; ++r.i) {
      Alpha_t& alphaP = FWD[r.i-1];
      Alpha_t& alpha = FWD[r.i];
      for (r.b=0; r.b<config.L; ++r.b) {
	double& sum = get<0>(alpha[r.b]); 
	for (r.a=0; r.a<config.L; ++r.a) {
	  double s = scores(r)*config.gamma + get<0>(alphaP[r.a]);
	  sum = (r.a==0) ? s : log_add(sum, s); 
	}
      }
    }
    if (config.verbose>=2) {    
      write<PartTag>(cerr, FWD, " FORWARD SCORES ");
    }

    // BWD Pass
    {
      Alpha_t& alpha = BWD[n-1];
      for (r.b=0; r.b<config.L; ++r.b) {	
	get<0>(alpha[r.b]) = 0;
      }
    }    
    for (r.i=n-2; r.i>=0; --r.i) {
      Alpha_t& alphaN = BWD[r.i+1];
      Alpha_t& alpha = BWD[r.i];
      for (r.a=0; r.a<config.L; ++r.a) {
	double& sum = get<0>(alpha[r.a]); 
	sum = log(0); 
	for (r.b=0; r.b<config.L; ++r.b) {
	  double s = scores(r)*config.gamma + get<0>(alphaN[r.b]);
	  sum = log_add(sum, s); 
	}
      }
    }
   
    if (config.verbose>=2) {
      write<PartTag>(cerr, BWD, " BACKWARD SCORES ");
    }

    double logZ = 0; 
    {
      Alpha_t& alpha = FWD[n-1];
      logZ = get<0>(alpha[0]);
      for (int i=1; i<config.L; ++i) {
	logZ = log_add(logZ, get<0>(alpha[i]));
      }
    }

    double logZB = log(0); 
    {
      Alpha_t& alpha = BWD[0];
      PartTag r(0,-1,0);
      for (r.b=0; r.b<config.L; ++r.b) {
	logZB = log_add(logZB, config.gamma*scores(r) + get<0>(alpha[r.b]));
      }
    }

    if (config.verbose>=1) {
      cerr << "partition="<< exp(logZ) << "    Bpartition=" << exp(logZB) << endl;
    }

    TabularScores<PartTag> expmu(x.size(), config.L);
    // compute marginals
    {
      r.i=0; 
      r.a=-1;
      Alpha_t& bwd = BWD[r.i];
      for (r.b=0; r.b<config.L; ++r.b) {
	double s =  get<0>(bwd[r.b]) + scores(r)*config.gamma - logZ;
	mu.set_score(r, s); 
	expmu.set_score(r, exp(s)); 
      }
    }
    for (r.i=1; r.i<n; ++r.i) {
      Alpha_t& fwd = FWD[r.i-1];
      Alpha_t& bwd = BWD[r.i];
      for (r.a=0; r.a<config.L; ++r.a) {
	for (r.b=0; r.b<config.L; ++r.b) {
	  double s = get<0>(fwd[r.a]) + get<0>(bwd[r.b]) + scores(r)*config.gamma - logZ;
	  mu.set_score(r, s); 
	  expmu.set_score(r, exp(s));
	}
      }
    }
    if (config.verbose>=2) {
      dump(cerr, config, x, mu, " LOG MARGINALS ");
      dump(cerr, config, x, expmu, " MARGINALS ");
    }

    return logZ;
  }


  template <typename X, typename S>    
  void Viterbi::dump(ostream& o, const Configuration& config, 
		     const X& x, 
		     S& scores, 
		     string label ) {
    o << "#####" << label << "#####" << endl;
    PartTag r; 
    for (r.i=0; r.i<x.size(); ++r.i) {
      o << "t=" << r.i; 
      for (r.a=-1; r.a<config.L; ++r.a) {
	for (r.b=0; r.b<config.L; ++r.b) {
	  o << " <"<<r.a << "," << r.b << "," << setprecision(4) << scores(r) << ">";
	}
      }
      o << endl;
    }
  }
  
  template <typename R>
  void Viterbi::write(ostream& o, const Chart_t& T, 
		      string label) {
    o << "#####" << label << "#####" << endl;
    for (size_t t=0; t<T.size(); ++t) {
      o << "t=" << t; 
      const Alpha_t& alpha = T[t];
      for (size_t l=0; l<alpha.size(); ++l) {
	o << " [l=" << l << ",s=" << get<0>(alpha[l]) << ",b=" << get<1>(alpha[l]) << "]";
      }
      o << endl;
    }
    o << "##########" << endl;
  }

}

#endif
