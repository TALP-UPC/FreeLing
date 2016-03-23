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
 * \file   factory-scores.h
 * \brief  A factory for creating score components
 * \author Xavier Carreras
 */

#ifndef TREELER_FACTORY_SCORES_H
#define TREELER_FACTORY_SCORES_H

#include "treeler/util/options.h"
#include "treeler/control/factory-base.h"
#include "treeler/base/scores.h"
#include "treeler/base/parameters.h"
#include "treeler/base/label.h"
#include <string>
#include <list>
#include <iostream>
#include <typeinfo>

namespace treeler {

  namespace control {
    
    template <typename Symbols, typename X, typename R>
    class Factory<OracleScorer<Symbols,X,R>> {
    public:
      static string name() {
	string SYMname = Factory<Symbols>::name(); 
	string Xname = Factory<X>::name(); 
	string Rname = Factory<R>::name();
	return "Oracle<" + SYMname + "," + Xname + "," + Rname + ">"; 
      }

      static void usage(std::ostream& o) {
	o << "Oracle scorer options: (no options)" << endl;
	o << endl;
      }
      
      static void configure(const Symbols& symbols, OracleScorer<Symbols,X,R>& t, Options& options, bool verbose=true, ostream& log=cerr) {
	if (verbose)
	  log << "Factory Scores: configuring oracle scores of type " << typeid(OracleScores<Symbols,X,R>).name() 
	       << ", no options required" << endl;
      }
    };
    

    template <typename FIdx>
    class Factory<Parameters<FIdx> > {
    public:
      static void usage(std::ostream& o) {
	o << "Parameters options:" << endl;
	dump_options(o, " "); 
	o << endl;
	}
      
      static void dump_options(std::ostream& o, string prefix) {
	o << prefix << "--wdir <path>    : directory of parameter files" << endl; 
	o << prefix << "      (if not present --dir=<path> is checked instead)" << endl;
	o << prefix << "--wstem=<string> : stem for parameter filenames (default: \"parameters\"" << endl;
	o << prefix << "--wt=<int>       : version number of parameter file" << endl;
	o << prefix << "--wavg=[0,1]     : use averaged parameters." << endl;
	o << prefix << "--wzero          : create a zero-ed parameter vector instead of loading one" << endl;
	}
      
      // create a Parameter vector
      template <typename F> 
      static void create(const F& f, Parameters<FIdx>*& w, Options& options, bool verbose=true, ostream& log=cerr) {
	assert(w==NULL);
	int tmp = 0;
	if (options.get("wzero", tmp) and tmp) {
	  w = new Parameters<FIdx>(f.spaces()); 
	  return;
	  }
	std::string wdir, wstem="parameters";
	int wt;
	if (!options.get("wdir", wdir)) { 
	  if (!options.get("dir", wdir)){ 
	    std::cerr << "Factory Scores" << " : please specify directory of parameter files via --wdir or --dir" << endl;
	    exit(1);
	    }
	}
	if (!options.get("wt", wt)) {
	  std::cerr << "Factory Scores" << " : please specify --wt to load parameter file "<< std::endl;
	  exit(1);
	}
	options.get("wstem", wdir); 
	int averaged =1 ; 
	options.get("wavg", averaged); 
	if (verbose) {
	  log << "Factory Scores" << " : loading " << (averaged ? "" : "non-") << "averaged " 
	      << "parameters from (" << wdir << "," << wt << ")" << std::endl;
	}
	w = new Parameters<FIdx>(0);
	w->load(wdir, wt, wstem, verbose);
	w->avg_set(averaged!=0);
      }
      
    };
    
    /**
     * \brief A factory for WFScores
     */
    template <typename Symbols, typename X, typename R, typename F>
    class Factory<WFScorer<Symbols,X,R,F>> {
    public:
      static string name() {
	string SYMname = Factory<Symbols>::name(); 
	string Xname = Factory<X>::name(); 
	string Rname = Factory<R>::name();
	string Fname = Factory<F>::name();
	return "WFscorer<" + SYMname + "," + Xname + "," + Rname + "," + Fname + ">"; 
      }
      
      static void usage(std::ostream& o) {
	o << "Scores options:" << endl;
	dump_options(o, " ");
	o << endl;
      };
           
      static void dump_options(std::ostream& o, string prefix) {
	Factory<typename WFScorer<Symbols,X,R,F>::W>::dump_options(o, prefix); 
      }
      
      static void configure(Symbols& sym, WFScorer<Symbols,X,R,F>& scorer, Options& options, bool verbose = true, ostream& log=cerr) {
	if (verbose) 
	  log << "Factory Scores: creating scorer of type " << name() << endl;
	F* f =  new F(sym);
	Factory<F>::configure(*f, options, verbose);
	scorer.set_f(f);
	
	// create parameters
	typename WFScorer<Symbols,X,R,F>::W* w = NULL;
	Factory<typename WFScorer<Symbols,X,R,F>::W>::create(*f, w, options, verbose, log);
	scorer.set_w(w);      
	if (verbose) 
	  log << "Factory Scores: done" << endl;
      }	
    };

    /**
     * \brief A factory for NullFScores
     */
    template <typename X, typename R, typename F>
    class Factory<NullFScorer<X,R,F>> {
    public:
      static void usage(std::ostream& o) {
	o << "NullFScores options:" << endl;
	dump_options(o, " ");
	o << endl;
      };
      
      static void dump_options(std::ostream& o, string prefix) {
	Factory<F>::dump_options(o, prefix); 
      }
      
      template <typename Symbols>
	static void configure(const Symbols& symbols, NullFScorer<X,R,F>& scorer, Options& options, bool verbose=true, ostream& log=cerr) {
	if (verbose) 
	  log << "Factory Scores: creating NullFScorer for" 
	       << " X=" << typeid(X).name()
	       << " R=" << typeid(R).name() 
	       << " F=" << typeid(F).name() << endl;
	F* f = NULL;
	Factory<F>::create(f, verbose, log);
	scorer.set_f(f);
	
	if (verbose)
	  log << "Factory Scores: done" << endl;
      }	
    };
    
    /**
     * \brief A factory for NullFeatures
     */
    template <typename X, typename R, typename FIdx>
    class Factory<NullFeatures<X,R,FIdx>> {
    public:
      static void usage(std::ostream& o) {
	o << "-- no options for NullFeatures -- " << endl;
      }
      
      static void dump_options(std::ostream& o, const string& prefix="") {
      }
      
      static void create(NullFeatures<X,R,FIdx>*& f, bool verbose=true, std::ostream& o = cerr) {
      	if (verbose) {
      	  o << "Empty configuration for NullFeatures: (no options)" << endl;
      	}
      	f = new NullFeatures<X,R,FIdx>();
      }
      
    };    
    
  }

}

#endif /* TREELER_FACTORY_SCORES_H */
