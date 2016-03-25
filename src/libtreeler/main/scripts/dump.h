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
 * \file   script-dump.h
 * \brief  A script for dumping part-factored structures
 * \author Xavier Carreras
 */

#ifndef TREELER_SCRIPT_DUMP
#define TREELER_SCRIPT_DUMP

#include <string>

#include "scripts/script.h"
#include "treeler/control/control.h"

namespace treeler {

  // template <typename FIdxA, typename FIdxB, 
  // 	    template<typename F,typename... A> class FGen, 
  // 	    typename ...Args>
  // struct SwitchFIdx<FGen<FIdxA,Args...>, FIdxB> {
  //   typedef FGen<FIdxB,Args...> F; 
  // };


  namespace control {

    /** 
     *  \brief A script for dumping part-factored objects of various kinds
     *  \ingroup control
     */  
    class ScriptDump: public Script {
    private:
      typedef ModelSelector<ScriptDump, TAG1, DEP1, DEP1F, DEP2, ISHAG1> GenericModel;
      
    public:
    ScriptDump()
      : Script("dump")
	{} 
      ~ScriptDump()
	{} 

      /** 
       *  \brief Configuration struct of the script 
       */ 
      struct Configuration {
	string ifile;

	// whether to enumerate parts, and which selection
	int run_parts_y; 
	int run_parts_all; 
	int run_parts_max;
	int run_parts_cin;
	
	// display options, general
	int display_x; 
	int display_y; 
	int display_xy; 
	
	// display options for parts
	int display_fvec;
	bool fvec_pretty;
	int display_oracle;
	int display_score;
	int display_marginal;
	
	// whether to calculate max solution
	int run_max; 
	int display_max;       
	
	// type of fidx
	string fidx_type;	       
      };

      /** 
       *  Provides methods to configure the script from command-line options
       */ 
      struct Configurer {
	static void dump_options(const string& name, ostream& log = cerr, const string& prefix = ""); 
  	static void configure(Configuration& config, Options& options, const string& name = "dump", ostream& log = cerr);
      };
      
      /** 
       *  \brief The name of the script
       */ 
      static std::string name() { return "dump"; }
      
      /** 
       *  \brief Usage information; the actual type of model is decided via command-line options
       */       
      void usage(Options& options);

      /** 
       *  \brief Runs the script, the actual model and the configuration is set from command-line options
       */       
      void run(Options& options);


      /** 
       *  \brief Generic version of the usage
       */       
      template <typename Model>
      static void usage(Options& options, ostream& log, const string& prefix);
      

      /** 
       *  \brief Generic version of the script (it first sets the configuration from command-line options)
       */       
      template <typename Model, 
		typename Conf = ScriptDump::Configurer>  
      static void run(Options& options, ostream& log); 

      
      /** 
       *  \brief Generic version of the script, does the real work
       */       
      template <typename Model>
      static void run(Options& options, const Configuration& config, ostream& output = cout, ostream& log = cerr);
      
      
    private:
      
      /** 
       *  \brief Generic part-based subroutine
       */       
      template <typename Model, typename O, typename RIT>
      static void dump_parts(const Configuration& config, ostream& output, typename Model::IO& io, const string& label, 
			     const typename Model::X& x, typename Model::S::Scores* scores, typename Model::F::Features* features, typename O::Scores* oracle_scores, 
			     RIT& r, RIT& r_end);      
    };
    

    //---------- IMPLEMENTATION OF TEMPLATE GENERIC FUNCTIONS

    template <typename Model, typename FIdx> 
    struct ModelSwitchFIdx {
      typedef typename Model::Symbols Symbols; 
      typedef typename Model::X X; 
      typedef typename Model::Y Y; 
      typedef typename Model::R R; 
      typedef typename Model::S S; 
      typedef typename SwitchFIdx<typename Model::F,FIdx>::F F; 
      typedef typename Model::I I; 
      typedef typename Model::IO IO;       
    };


    template <typename Model>
    void ScriptDump::usage(Options& options, ostream& log, const string& prefix) 
    {
      Factory<typename Model::Symbols>::usage(log);
      Factory<typename Model::R>::usage(log);
      Factory<typename Model::I>::usage(log);
      Factory<typename Model::S>::usage(log);
      Factory<typename Model::F>::usage(log);
      Factory<typename Model::IO>::usage(log);
    }
    

    template <typename Model, typename Conf>  
    void ScriptDump::run(Options& options, ostream& log) 
    {
      typedef typename Model::Symbols Symbols; 
      typedef typename Model::X X; 
      //typedef typename Model::Y Y; 
      typedef typename Model::R R; 
      typedef typename Model::S S; 
      typedef typename Model::I I; 
      typedef typename Model::IO IO;       
      typedef OracleScorer<Symbols,X,R> O;
      ostream& output = cout; 
      
      log << name() << " : running " << Model::name() << " with ..." << endl;
      log << " Sym=" << Factory<Symbols>::name() << endl
	  << " X=" << Factory<X>::name() << endl
	  << " R=" << Factory<R>::name() << endl
	  << " I=" << Factory<I>::name() << endl
	  << " S=" << Factory<S>::name() << endl
	  << " O=" << Factory<O>::name() << endl
	  << " IO=" << Factory<IO>::name() << endl << endl;
      
         
      Configuration config; 
      Conf::configure(config, options, name(), log);

      if (config.fidx_type=="") {
	ScriptDump::run<Model>(options, config, output, log);
      }
      else if (config.fidx_type=="bits") {
	typedef ModelSwitchFIdx<Model,FIdxBits> Mprime;
	ScriptDump::run<Mprime>(options, config, output, log);
      }
      else if (config.fidx_type=="chars") {
	typedef ModelSwitchFIdx<Model,FIdxChars> Mprime;
	ScriptDump::run<Mprime>(options, config, output, log);
      }	  
      else if (config.fidx_type=="pair") {
	typedef ModelSwitchFIdx<Model,FIdxPair> Mprime;
	ScriptDump::run<Mprime>(options, config, output, log);
      }
      else {
	cerr << name() << " : fidx type \"" << config.fidx_type << "\" not valid" << endl;
      }
    }


    template <typename Model>
    void ScriptDump::run(Options& options, const Configuration& config, ostream& output, ostream& log) 
    {
      typedef typename Model::Symbols Symbols; 
      typedef typename Model::X X; 
      typedef typename Model::Y Y; 
      typedef typename Model::R R; 
      typedef typename Model::S S; 
      typedef typename Model::F F; 
      typedef typename Model::I I; 
      typedef typename Model::IO IO;       
      typedef OracleScorer<Symbols,X,R> O;
      
      log << name() << " : creating components ..." << endl;
      // symbols
      Symbols symbols;
      Factory<Symbols>::configure(symbols, options);

      // factors
      typename R::Configuration Rconfig; 
      Factory<R>::configure(Rconfig, options);
      
      // input output options
      IO io(symbols);
      Factory<IO>::configure(io, options);
      
      // inference
      I parser(symbols);
      Factory<I>::configure(parser, options);
      
      // scoring
      S* scorer = NULL; 
      if (config.display_score or config.run_max) {
	scorer = new S(symbols); 
	Factory<S>::configure(symbols, *scorer, options);
      }
      
      // feature generator
      F fgen(symbols); 
      if (config.display_fvec) {
	Factory<F>::configure(fgen, options);
      }

      // oracle scorer
      O* oracle = NULL; 
      if (config.display_oracle) {
	oracle = new O(symbols); 
	Factory<O>::configure(symbols, *oracle, options);
      }
      
      // data
      DataStream<X,Y,IO> ds(io, std::cin);
      if (!config.ifile.empty() and config.ifile=="-") {
	ds.set_file(config.ifile);
      }
      
      log << name() << " : dumping ..." << endl;
      /* PROCESS EACH EXAMPLE AND DO THE DUMPING */
      typename DataStream<X,Y,IO>::const_iterator it = ds.begin(), it_end = ds.end();
      for (; it!=it_end; ++it) {
	const X& x  = (*it).x();  // static_cast<const X&>(*((*it)->x()));
	const Y& y  = (*it).y();  // static_cast<const Y&>(*(*it)->y());
	if (config.display_xy or config.display_x or config.display_y) {
	  output << "+-- EXAMPLE " << x.id() << " -------------------" << endl; 
	}
	if (config.display_xy) { io.write(output, x, y); } 
	if (config.display_x) { io.write(output, x);  }
	if (config.display_y) { io.write(output, y);  }
	
	Label<R> parts; 
	parser.decompose(x, y, parts); 

	typename O::Scores* oracle_scores = NULL; 
	if (oracle!=NULL) {
	  oracle_scores = new typename O::Scores(); 
	  oracle->scores(x,parts,*oracle_scores); 
	}
	
	typename S::Scores* scores = NULL; 
	if (scorer!=NULL) {
	  scores = new typename S::Scores(); 
	  scorer->scores(x, *scores);
	}
	
	typename F::Features* features = NULL; 
	if (config.display_fvec) {
	  features = new typename F::Features(); 
	  fgen.features(x, *features); 
	}
	
	if (config.run_parts_y) {
	  auto r = parts.begin(); 
	  auto rend = parts.end(); 
	  dump_parts<Model,O>(config, output, io, "Y", 
			      x, scores, features, oracle_scores, 
			      r, rend); 
	}
	
	if (config.run_parts_all) {
	  auto r = R::begin(Rconfig, x); 
	  auto rend =  R::end();
	  dump_parts<Model,O>(config, output, io, "ALL", 
			      x, scores, features, oracle_scores, 
			      r, rend); 
	}
	
	if (config.run_max) {
	  Label<R> yhat;
	  double score_max = parser.argmax(x, *scores, yhat); 
	  output << "MAX Y " << x.id() << " " << score_max << endl; 
	  io.write(output, x, y, yhat); 
	  output << endl;	
	  auto r = yhat.begin(); 
	  auto rend = yhat.end(); 
	  dump_parts<Model,O>(config, output, io, "MAX", 
			      x, scores, features, oracle_scores, 
			      r, rend); 
	}
	
	if (oracle_scores!=NULL) {
	  delete oracle_scores; 
	}
	if (features!=NULL and scores==NULL) {
	  delete features; 
	}
	if (scores!=NULL) {
	  delete scores;
	}
      } // for each example    
    } 

    
    template <typename Model, typename O, typename RIT>  
    void ScriptDump::dump_parts(const Configuration& config, 
				ostream& output, 
				typename Model::IO& io, 
				const string& label,
				const typename Model::X& x, 
				typename Model::S::Scores* scores, 
				typename Model::F::Features* features, 
				typename O::Scores* oracle_scores, 
				RIT& r, RIT& r_end) 
    {
      typedef typename Model::F F; 
      typedef typename F::FIdx FIdx;
      
      double sum_scores = 0; 
      while (r != r_end) {      
	output << "PART " << label << " " << x.id() << " " << *r << endl;
	
	if (config.display_fvec) {
	  if (features!=NULL) {
	    ostringstream oss;
	    const FeatureVector<FIdx>* f = features->phi(*r);
	    oss << "FVEC " << x.id() << " " << *r << " ";	    
	    output << oss.str(); 
	    if (config.fvec_pretty) {
	      IOFVec::pretty_print(output, *f, string(oss.str().length(), ' '));
	    }
	    else {
	      IOFVec::write(output, *f);
	    }
	    output << endl;
	    features->discard(f);
	  }
	}
	
	if (config.display_score or config.display_oracle) {
	  output << "SCO " << x.id() << " " << *r ;
	  if (oracle_scores!=NULL) { output << " " << (*oracle_scores)(*r); }
	  if (scores!=NULL) { 
	    double s = (*scores)(*r); 
	    sum_scores += s; 
	    output << " " << s; 
	  }		
	  output << endl;
	}

	++r; 
      }
      if (scores!=NULL) output << "SUM  " << label << " " << x.id() << " " << sum_scores << endl << endl;
    }
    
  } // control
} // treeler


#endif
