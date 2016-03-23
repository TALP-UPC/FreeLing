 /*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2013   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   score-dumper.h
 * \brief  Score Dumpers for pependency parts
 * \author Xavier Carreras
 */

#ifndef DEP_SCOREDUMPER
#define DEP_SCOREDUMPER

#include <tuple>
#include "treeler/dep/part-dep1.h"

namespace treeler {

  namespace _dep_score_dumper {
    template <typename A> 
    struct ScoreTraits {};

    template <> 
    struct ScoreTraits<double> {
      double th;
      double zero_value;

      ScoreTraits() 
	: th(0), zero_value(0)
	{}      

      bool threshold(const double& v) {
	return th<0 or fabs(v)>=th;
      }
    };

    template <> 
    struct ScoreTraits<string> {
      string th;
      string zero_value;

      ScoreTraits() 
	: th(""), zero_value("")
	{}

      bool threshold(const string& s) {
	return s>=th;
      }
    };
  };


  /**
   *  A score dumper specialized for PartDep1
   */
  template<typename T>
  class ScoreDumper<PartDep1,T> {
  public:  
    typedef PartDep1 R;
    
    int offset; 
    bool sort_values;
    bool skip_zero;
    bool print_xid; 
    int  precision; 
    

    _dep_score_dumper::ScoreTraits<T> traits; 

    void set_threshold(T t) { traits.th = t; }




    ScoreDumper() 
      : offset(1), sort_values(true), skip_zero(true), print_xid(true), precision(-1), traits()
    {}

    template <typename X, typename S>
    void dump(std::ostream& o, const typename R::Configuration rconfig, const X& x, S& s, const std::string& prefix = "") {
      
      typedef tuple<int,int,T> my_tuple; 
      
      if (precision>=0) {
	o.setf(ios::fixed); 
	o.precision(precision); 
      }
	      
      int n = x.size();
      R r (-1,0,0);
      for (r.m=0; r.m<n; ++r.m) {
	if (print_xid) {
	  o << prefix << x.id() << " " << r.m+offset << flush; 
	}
	else {
	  o << prefix << r.m+offset << flush; 
	}

    	std::vector<my_tuple> v; 

    	for (r.h=-1; r.h<n; ++r.h) {
    	  if (r.m==r.h) continue;
    	  if (rconfig.L==1) {
    	    r.l = 0;
    	    o <<  " " << r.h+offset << " " << s(r);
    	  }
    	  else {
    	    for (r.l=0; r.l<rconfig.L; ++r.l) {
    	      T sr = s(r);
	      if (traits.threshold(sr)) {
		if (!skip_zero or sr!=traits.zero_value) {
		  if (sort_values) {
		    //		  o << " pushing " << r.h << " " << r.l << " " << sr << endl; 
		    v.push_back(make_tuple(r.h, r.l, sr)); 
		  }
		  else {
		    o <<  " " << r.h+offset << " " << r.l << " " << sr;
		  }
		}
	      }
    	    }
    	  }
    	} 
    	if (sort_values) {
	  std::sort(v.begin(), v.end(), 
		    [](const my_tuple& a, const my_tuple& b) { 
		      return 
			(get<2>(a) > get<2>(b)) or 
			(get<2>(a) == get<2>(b) and (get<0>(a) < get<0>(b) or (get<0>(a) == get<0>(b) and (get<1>(a) <= get<1>(b))))); 
		    }); 
	  for (my_tuple& a : v) {
	    o << " " << get<0>(a)+offset << " " << get<1>(a) << " " << get<2>(a);
	  }
    	}	    
    	o << std::endl;
      }
      o << std::endl;
    }


    template <typename X, typename S>
    void load(std::istream& i, const typename R::Configuration rconfig, const X& x, S& s) {
      // cerr << "score-dumpber: loading scores for input " << x.id() << " L=" << rconfig.L << " ..." << flush ; 
      int linen = 0;
      s.initialize(rconfig, x.size(), 0); 
      //      cerr << "(i)..." << flush;      
      int n=0; 
      string line; 	
      while (getline(i, line)) {
	++linen;
	//	  cerr << "read : " << line << endl; 
	int id, head, mod, lab;
	double score;
	stringstream iss(line); 
	if (!(iss>>id)) {
	    break;
	  }	  
	  ++n;
	  if (n>x.size()) {
	    cerr << "score-dumper: line " << linen << " : too many lines for input " << x.id() << "!\n!"; 
	  }
	  iss >> mod;
	  // substract one 
	  --mod; 
	  while (iss >> head >> lab >> score) {
	    if (lab >= rconfig.L) {
	      cerr << "score-dumper: line " << linen << " : label id of dependency (" << lab << ") is too large (L=" << rconfig.L << ")!\n"; 
	    }
	    // substract one 
	    --head;
	    s(head,mod,lab) = score; 
	  }    
	}
	if (n!=x.size()) {
	  cerr << "score-dumper: line " << linen << " : too few lines for input " << x.id() << "(size " << x.size() << "," << n << "lines read)!\n!"; 
	}
	// cerr << " ok" << endl;        
    }
  };

}

#endif
