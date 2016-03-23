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
 * \file   script-decode.h
 * \brief  A script for decoding new inputs using a trained model
 * \author Xavier Carreras
 */

#ifndef TREELER_SCRIPT_DECODE
#define TREELER_SCRIPT_DECODE

#include <string>

#include "scripts/script.h"

#include "treeler/control/control.h"
#include "treeler/algo/decoder.h"

namespace treeler {

  using namespace control;

  /** 
   *  \brief A script for making predictions on new data with a structured model
   *  \ingroup control
   */  
  class ScriptDecode: public Script {
  public:
  ScriptDecode();
  ~ScriptDecode();

    struct Configuration {
      /* whether to read input patterns from cin or from a file */
      int from_cin;

      /* the file containing the input patterns to decode */
      std::string ifile;

      /* whether to write the output to cout or to a file */
      int to_cout;

      /* the file where to write the output to  */
      std::string ofile;

      /* what to display for each example: yhat, plus x and or ygold */
      bool display_x; 
      bool display_ygold;

      /* evaluate along the way */
      bool eval;

      /* verbosity */
      int verbose;
    };

    static std::string const name() { return "decode"; }

    void usage(Options& options);

    void run(Options& options);

    static void configure(Options& options, Configuration& config);

    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    static void usage(Options& options, const string& model_name, ostream& log, const string& prefix);

    template <typename Model>
    static void usage(Options& options, ostream& log, const string& prefix);

    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    static void run(Options& options, const string& model_name);

    template <typename Model>
    static void run(Options& options, ostream& log);

    template <typename Model>
    static void run(Options& options, const Configuration& config, ostream& log);

  };


  template <typename Model>
  void ScriptDecode::usage(Options& options, ostream& log, const string& prefix) 
  {
    log << prefix << " : components of \"" << Model::name() << "\":" << endl; 
    Factory<typename Model::Symbols>::usage(log);
    Factory<typename Model::R>::usage(log);
    Factory<typename Model::I>::usage(log);
    Factory<typename Model::S>::usage(log);
    Factory<typename Model::F>::usage(log);
    Factory<typename Model::IO>::usage(log);
  }

  template <typename Model>
  void ScriptDecode::run(Options& options, ostream& log) {
    int oracle = 0; 
    options.get("oracle", oracle); 
    Configuration config;
    configure(options, config); 
    
    if (oracle) {
      //typedef OracleScorer<typename Model::Symbols, typename Model::X, typename Model::R> OracleS; 
      run<Model>(options, config, log); 
    }
    else {
      run<Model>(options, config, log); 
    }    

  }
  

  template <typename Model>
  void ScriptDecode::run(Options& options, const Configuration& config, ostream& log) {
    
    typedef typename Model::Symbols Symbols; 
    typedef typename Model::X X; 
    typedef typename Model::Y Y; 
    typedef typename Model::R R; 
    typedef typename Model::S S; 
    typedef typename Model::I I; 
    typedef typename Model::IO IO; 

    if (config.verbose) {
      cerr << name() << " : running \"" << Model::name() << "\" with ..." << endl;
      cerr << " Sym=" << Factory<Symbols>::name() << endl
	   << " X=" << Factory<X>::name() << endl
	   << " Y=" << Factory<Y>::name() << endl
	   << " R=" << Factory<R>::name() << endl
	   << " I=" << Factory<I>::name() << endl
	   << " S=" << Factory<S>::name() << endl
	   << " IO=" << Factory<IO>::name() << endl << endl;
    }

    try {      

      if (config.verbose)
	log << name() << " : creating components ..." << endl;

      // symbols
      Symbols symbols;
      Factory<Symbols>::configure(symbols, options, config.verbose, log);

      // Input Output options
      IO io(symbols);
      Factory<IO>::configure(io, options, config.verbose, log);
      
      // inference
      I parser(symbols); 
      Factory<I>::configure(parser, options, config.verbose, log);
      
      S scorer(symbols); 
      Factory<S>::configure(symbols, scorer, options, config.verbose, log);

      DataStream<X,Y,IO> ds(io, std::cin);
      if (!config.from_cin) {
	if (config.verbose) log << name() << " : input from \"" << config.ifile << "\"" << endl;
	ds.set_file(config.ifile);
      }
      else {
	if (config.verbose) log << name() << " : input from cin" << endl;
      }

      Decoder decoder;
      Decoder::functor_ostream<X,Y,IO>* f;
      if (config.to_cout) {
	if (config.verbose) log << name() << " : output to cout" << endl;
	f = new Decoder::functor_ostream<X,Y,IO>(io, cout);
      }
      else {
	if (config.verbose) log << name() << " : output to \"" << config.ofile << "\"" << endl;
	f = new Decoder::functor_ostream<X,Y,IO>(io, config.ofile);
      }
      f->display_x(config.display_x); 
      f->display_ygold(config.display_ygold); 

      if (not config.eval) {
	decoder.decode<X,Y,I,S>(parser, scorer, ds.begin(), ds.end(), *f);
      }
      else {
	typedef Decoder::functor_ostream<X,Y,IO> FStream;
	typedef Decoder::functor_eval<X,Y,typename Model::Eval>  FEval; 
	typedef Decoder::functor_pair<X,Y,FStream,FEval> FPair;	
	FEval  feval;
	FPair ff(*f, feval);
	Decoder::decode<X,Y,I,S>(parser, scorer, ds.begin(), ds.end(), ff);
	log << name() << " : evaluation results : " << feval.to_string() << endl; 
      }

      delete f;

    } catch(exception& e) {
      cerr << endl 
	   << endl << "treeler : exception running " << name()
	   << endl 
	   << "\"" << e.what() << "\"" << endl;
      exit(0);
    }
  }


}
#endif
