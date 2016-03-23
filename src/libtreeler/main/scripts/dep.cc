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
 * \file   script-dep.cc
 * \author Xavier Carreras
 */

#include "scripts/dep.h"
#include "scripts/register.h"
REGISTER_SCRIPT(dep, control::ScriptDep);

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "scripts/dump.h" 

#include "treeler/util/options.h"
#include "treeler/control/forall.h"
#include "treeler/control/trainer.h"
#include "treeler/base/score-dumper.h"
#include "treeler/dep/score-dumper.h"
#include "treeler/dep/shag/shagTT.h"


using namespace std;

namespace treeler {
  
  namespace control {

    void ScriptDep::usage(Options& options) {
      
      string cmd; 
      options.get("cmd", cmd); 

      if (cmd.empty()) {
	cerr << name() << " : runs a dependency parser" << endl;
	cerr << endl;

	cerr << name() << " commands:" << endl;
	cerr << " --cmd=parse  : parses data with a model (default)" << endl;     
	cerr << " --cmd=scores : dumps part-scores to stdout" << endl;
	cerr << " --cmd=sum    : sums the mass of derivations" << endl;
	cerr << " --cmd=marg   : computes marginals on dependencies" << endl;
	cerr << " --cmd=all    : lists all derivations" << endl;

	cerr << endl;
	cerr << name() << "Obsolete commands:" << endl;
	cerr << " --cmd=parseo : old parsing routine" << endl;     
	cerr << " --cmd=train  : trains a feature-based model with perceptron" << endl;
	cerr << " --cmd=hyper  : builds an hypergraph and runs the semiring specified by --mod" << endl;
	cerr << endl;
	cerr << name() << " options:" << endl;
	cerr << " --mod=<name> : model name" << endl;
	cerr << " --oracle     : use oracle scores for parsing " << endl;
	cerr << " --count=<double> : use scores for counting parse trees" << endl;
	cerr << "     (a negative value stands for scores of 1/n, while a" << endl;
	cerr << "      value of 0 is used for counting in the log-space)" << endl;
	cerr << " --scores     : dump part scores to cout" << endl;
	cerr << endl;
	DepModel::usage(options, "mod", cerr); 
	cerr << endl;
      }
      else if (cmd == "parse") {
	cerr << name() << " --cmd=parse : parses input sentences" << endl; 
	cerr << endl;
	cerr << "Parse options:" << endl; 
	cerr << " --oracle         : use oracle scoring" << endl; 
	cerr << " --sdump=<file>   : read scores from a dump file" << endl; 
	cerr << " --scores         : dump scores in addition to parse trees" << endl; 
	cerr << " --errors         : display additional columns marking parsing mistakes" << endl; 
	cerr << endl;
	ModelSelector<ScriptDep, DEP1, DEP2>::usage(options, "mod", cerr);	
      }
      else if (cmd == "score" or cmd == "sco") {
	cerr << name() << " --cmd=score : scores parts and dumps them to output" << endl; 
	cerr << endl;
	cerr << "Score options:" << endl; 
	cerr << " --out=<path>         : dump into this file" << endl; 
	cerr << " --oracle             : use oracle scoring" << endl; 
	cerr << " --sdump=<file>       : read scores from a dump file" << endl; 
	cerr << " --sort=<0|1>         : sort values (high to low)" << endl; 
	cerr << " --threshold=<double> : do not dump scores below this threshold in absolute value" << endl; 	
	cerr << " --precision=<int>    : print scores with this level of precision" << endl; 
	cerr << endl;
	ModelSelector<ScriptDep, DEP1, DEP2>::usage(options, "mod", cerr);	
      }
      else if (cmd == "marg") {
	cerr << name() << " --cmd=marg : computes dependency marginal scores" << endl; 
	cerr << endl;
	cerr << "Marginals options:" << endl; 
	cerr << " --sort=<0|1>         : sort values (high to low)" << endl; 
	cerr << " --threshold=<double> : do not dump scores below this threshold in absolute value" << endl; 	
	cerr << " --precision=<int>    : print scores with this level of precision" << endl; 
	cerr << endl;
	ModelSelector<ScriptDep, DEP1, DEP2>::usage(options, "mod", cerr);	
      }
      else if (cmd == "dump") {
	cerr << name() << " --cmd=dump : dumps part-based information to output" << endl; 
	cerr << endl;
	ModelSelector<ScriptDump, DEP1, DEP2>::usage(options, "mod", cerr);
      }
      else if (cmd == "train") {
	cerr << name() << " --cmd=train : trains a dependency model" << endl; 
	cerr << endl;
	ModelSelector<Trainer, DEP1, DEP2>::usage(options, "mod", cerr);
      }
      else {
	cerr << "No usage information for command \"" << cmd << "\", sorry." << endl; 
      }           

    }
        

    // count derivations at marginal level
    void mu_count(Options& options); 


    // main script
    void ScriptDep::run(Options& options) {
      // direct commands
      string cmd=""; 
      options.get("cmd", cmd); 
      if (cmd == "mu-count") {
	mu_count(options); 
      }
      else {
	DepModel::run(options, "mod");
      }
    }

    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void 
    ScriptDep::usage(Options& options, const string& model_name, ostream& log, const string& prefix) {
      cerr << "Using \"" << model_name << "\":" << endl; 
      Factory<R>::usage(log); 
      Factory<I>::usage(log); 
      Factory<S>::usage(log); 
      Factory<F>::usage(log); 
      Factory<IO>::usage(log); 
    }
    

    /////////////////////////////////////////////////////////////////////////////////////
    // INTERMEDIATE COMMANDS AND TYPES
   
    // cached scores
    template<typename R>
    class ScoresMap; 

    // A scorer that loads and dumps scores for sentences
    template<typename R>
    class DumpScorer; 

    template<typename R>
    class Factory<DumpScorer<R>>; 

    // special scorer to count derivations
    template <typename Symbols, typename X, typename R>
    class CountScores; 

    template <typename Symbols, typename X, typename R>
    class Factory<CountScores<Symbols,X,R>>;

    // functors

    // generic forall script: instantiates a functor and runs it for all input sentences
    template <template <typename, typename, typename, typename, typename, typename, typename, typename> class Functor>
    struct ForAllScript;

    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
    struct ParseFunctor;

    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    struct ScoresFunctor;

    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename Feat, typename IO> 
    struct AllDerivationsFunctor;

    /**
     * This auxiliar function selects a type of scores, using
     * command-line options, and calls the routine
     * FUNC.run<Symbols,X,Y,R,I,S,F,IO>(options, model_name)
     * 
     */
    template <typename FUNC, typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>
    void choose_scorer(Options& options, const string& model_name);

    /////////////////////////////////////////////////////////////////////////////////////
    

    // This is the main templated script with all types resolved. 
    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptDep::run(Options& options, const string& model_name) {
      // choose command and then type of scoring
      string cmd = "parse"; 
      options.get("cmd", cmd);      
      if (cmd=="parse") {
	typedef ForAllScript<ParseFunctor> Script; 
	choose_scorer<Script,Symbols,X,Y,R,I,S,F,IO>(options, model_name);
      }
      else if (cmd=="sco" or cmd=="scores") {
	typedef ForAllScript<ScoresFunctor> Script; 
	choose_scorer<Script,Symbols,X,Y,R,I,S,F,IO>(options, model_name);
      }
      else if (cmd=="dump") {
	ScriptDump::run<Symbols,X,Y,R,I,S,F,IO>(options, model_name); 
      }
      else if (cmd=="all") {
	typedef PartDep1 Rprime;
	typedef ProjDep1 Iprime;
	typedef OracleScorer<Symbols,X,Rprime> Sprime;

	print_all_derivations<Symbols,X,Y,Rprime,Iprime,Sprime,F,IO>(options, model_name); 
	//	ForAllScript<AllDerivationsFunctor> script; 
	//	script.template run<Symbols,X,Y,Rprime,Iprime,Sprime,F,IO>(options, model_name);

	//	print_all_derivations<Symbols,X,Y,Rprime,Iprime,OS,F,IO>(options, model_name); 
      }
      else if (cmd=="marg" or cmd=="mu") {
	//	choose_scorer<MarginalsMain,Symbols,X,Y,R,I,S,F,IO>(options, model_name);
      }
      else if (cmd=="train") {
	Trainer::run<Symbols,X,Y,R,I,S,F,IO>(options, model_name); 
      }
      else {
	cerr << name() << " : unknown command \"" << cmd << "\"" << endl;
	exit(0);
      }
    }    


    ////////////////////////////////////////////////////////////////////////////
    //  M A P   O F   S C O R E S
    //////////////////////////////////////


    template<>
    class ScoresMap<PartDep1> {
      typedef PartDep1 R; 
      typedef R::map<double> ScoresType;
    private:
      ScoresType* _scores;
    public:
      ScoresMap() : _scores(NULL) {}; 
      ~ScoresMap() {
	delete _scores; 
      }

      void initialize(const R::Configuration& config, int n, double def) {
	if (_scores != NULL) delete _scores;
	_scores = new ScoresType(config, n, def);
      }

      double operator()(const PartDep1& r) const {
	
	if (_scores==NULL) return 0; 
	return (*_scores)(r);
      }

      double& operator()(const PartDep1& r) {
	return (*_scores)(r);
      }
      
      template <typename ...Args>
      double operator()(Args... a) const {
	if (_scores==NULL) return 0; 
	return (*_scores)(a...);
      }

      template <typename ...Args>
      double& operator()(Args... a)  {
	return (*_scores)(a...);
      }

    };



    ////////////////////////////////////////////////////////////////////////////
    //  D U M P   S C O R E R 
    //////////////////////////////////////

    template<typename R>
    class DumpScorer {
    public:
      typedef ScoresMap<R> Scores; 
    private:
      typename R::Configuration _rconfig; 
      istream* _input; 
      ifstream* _finput; 
      int _linen; 
    public:

      DumpScorer(DepSymbols& sym) {	
	_rconfig.L = sym.d_syntactic_labels.size(); 
	_input = NULL; 
	_finput = NULL; 
	_linen = 0;
      }

      ~DumpScorer() {
	if (_finput!=NULL) {
	  _finput->close(); 
	  delete _finput;
	}
      }

      void set_input(istream& i) {
	_input = &i; 
      }

      void set_num_labels(int l) {
	_rconfig.L = l; 
      }

      bool set_file(const string& filename) {
	if (_finput!=NULL) {
	  delete _finput; 
	}
	_finput = new ifstream(filename); 
	_input = _finput; 
	return _finput->good(); 
      }

      template <typename X>
      void scores(const X& x, Scores& s) {
	ScoreDumper<PartDep1> dumper;
	dumper.load(*_input, _rconfig, x, s);
      }

      template <typename X, typename Y>
      void scores(const X& x, const Y& y, Scores& s) {
	scores(x, s); 
      }
      
    }; 


    template<typename R>
    class Factory<DumpScorer<R>> {
    public:
      static void configure(DepSymbols& sym, DumpScorer<R>& scorer, Options& options, ostream& out=cerr) {
	string filename; 
	if (!options.get("sdump", filename)) {
	  cerr << "Factory<DumpScorer> : please specify file of scores" << endl; 
	  exit(-1); 
	}
	if (!scorer.set_file(filename)) {
	  cerr << "Factory<DumpScorer> : error opening " << filename << endl; 
	  exit(-1); 
	}
	int l; 
	if (options.get("L", l)) {
	  scorer.set_num_labels(l); 
	}
      }

    };




    /////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    //  P A R S I N G
    //////////////////////////////////////

        
    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
    struct ParseFunctor {
    public:
      ostream& output; 
      DepEval eval;
      int  ntrees;
      int verbose; 
      bool dump_scores;
      bool dump_parts;
      bool display_errors; 
      
    public:

      ParseFunctor(Options& options, const string& model_name) 
	: output(cout), ntrees(0), verbose(0), dump_scores(false), dump_parts(false)
      {
	options.get("scores", dump_scores); 
	options.get("errors", display_errors); 
      }


      void start(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io) {}

      void end(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io) {
	std::cerr << "Evaluation: " << eval.to_string() << endl;
      }

      void operator()(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io, const X& x, const Y& y) {
	typename S::Scores scores; 
	scorer.scores(x,y,scores);
	if (dump_scores) {	  
	  ostringstream oss; 
	  oss << "SCO " << x.id() << " "; 
	  ScoreDumper<R> dumper;
	  dumper.dump(cout, Rconfig, x, scores, oss.str()); 
	}
	
	// parse
	Y yhat; 
	double s = parser.argmax(x, scores, yhat);       

	if (verbose) {
	  output << "Example " << x.id() << " " << s << endl; 
	}

	if (dump_parts) {
	  Label<R> parts; 
	  parser.decompose(x,yhat,parts); 
	  for (auto r = parts.begin(); r!=parts.end(); ++r) {
	    cout << "   " << *r << " " << scores(*r) << endl; 
	  }
	}
	
	if (display_errors) {
	  vector<string> err(x.size()); 
	  for (int i=0; i<x.size(); ++i) {
	    if (y[i].h!=yhat[i].h and y[i].l!=yhat[i].l) {
	      err[i] = "HL"; 
	    }
	    else if (y[i].h!=yhat[i].h) {
	      err[i] = "HL"; 
	    }
	    else if (y[i].l!=yhat[i].l) {
	      err[i] = "L"; 
	    }	  
	  }
	  io.write(output, x, y, yhat, err); 
	}
	else {
	  io.write(output, x, y, yhat); 
	}
	
	eval(x,y,yhat); 
	DepTree<string> t = DepTree<string>::convert(y); 
	DepTree<string> that = DepTree<string>::convert(yhat); 
		
	ntrees++;
      }      
    };




    ////////////////////////////////////////////////////////////////////////////
    //  C H O O S E   S C O R E R 
    //////////////////////////////////////

    /**
     * This auxiliar function selects a type of scores, using
     * command-line options, and calls the routine
     * FUNC.run<Symbols,X,Y,R,I,S,F,IO>(options, model_name)
     * 
     */
    template <typename FUNC, typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>
    void choose_scorer(Options& options, const string& model_name) {
      // Choose between oracle scoring, count scoring, or the default model scoring
      int oracle = 0; 
      int count = 0; 
      string sdump = "";
      options.get("oracle", oracle); 
      options.get("sdump", sdump); 
      FUNC f; 
      if (oracle) {
	// typedef OracleScorer<Symbols,X,R> OS;
	//	f.template run<Symbols,X,Y,R,I,OS,F,IO>(options, model_name);
      }
      else if (sdump!="") {
	typedef DumpScorer<R> DS; 
	f.template run<Symbols,X,Y,R,I,DS,F,IO>(options, model_name);
      }
      else if (options.get("count", count) and count) {
	//	typedef CountScores<Symbols,X,R> CS;       
	//	f.template run<Symbols,X,Y,R,I,CS,F,IO>(options, model_name);
      }
      else {
	f.template run<Symbols,X,Y,R,I,S,F,IO>(options, model_name);
      }      
    }


    ////////////////////////////////////////////////////////////////////////////
    //  S C O R E   D U M P I N G 
    //////////////////////////////////////

    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    struct ScoresFunctor {
    public:
      ScoreDumper<R> dumper;

      ScoresFunctor(Options& options, string model_name) {
	dumper.sort_values = true; 
	dumper.skip_zero = true; 
	dumper.print_xid = true; 
	dumper.precision = -1; 
	options.get("sort", dumper.sort_values); 
	double th = -1; 
	options.get("threshold", th); 
	dumper.set_threshold(th);
	options.get("precision", dumper.precision); 
      } 
      
      void start(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io) {}
      void end(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io) {}

      void operator()(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io, const X& x, const Y& y) {
	typename S::Scores s; 
	scorer.scores(x, s); 
	dumper.dump(cout, Rconfig, x, s); 
      }
    };


    ////////////////////////////////////////////////////////////////////////////
    //  M U   C O U N T
    //////////////////////////////////////

    void mu_count(Options& options) {      
      typedef DepSymbols Symbols;
      typedef BasicSentence<string,string> X; 
      typedef DepVector<string> Y; 
      typedef PartDep1 R; 
      typedef ProjDep1 Parser; 
    
      Symbols symbols; 
      Factory<Symbols>::configure(symbols, options);       
      Parser parser(symbols);       
      R::Configuration rconfig(symbols.d_syntactic_labels.size()); 
      ScoreDumper<R> dumper; 

      int iarg=0; 
      options.get("random", iarg); 
      bool random = (iarg!=0);

      CoNLLStream istrm, ostrm;
      ostrm.add_padding = true; 
      while (cin >> istrm) {
	vector<int> ids; 
	X x; 
	Y ygold, yhat; 
	istrm >> ids >> x >> ygold; 
	
	ConstantScores<X,R> scores(x, 0);
	if (random) scores.set_random(random);
	parser.argmax(x, scores, yhat); 
		
	R::map<double> mu; 
	double log_z = parser.marginals(x, scores, mu); 

	cout << "SENTENCE  " << x.id() << endl; 
	cout << "SIZE is " << x.size() << ", log sum is " << log_z << " and sum is " << exp(log_z) << endl; 
	cout << "SCO1" << endl; 
	dumper.dump(cout, rconfig, x, scores); 	
	cout << "SCO2" << endl; 
	dumper.dump(cout, rconfig, x, scores); 	

	cout << "MU" << endl; 
	dumper.dump(cout, rconfig, x, mu); 	
	R::map<double> counts(rconfig, x, 0); 
	for (auto r = PartDep1::begin(rconfig, x); r!=PartDep1::end(); ++r) {
	  counts(*r) = mu(*r) * exp(log_z);
	}
	cout << "COUNTS" << endl; 
	dumper.dump(cout, rconfig, x, counts);
	
	ostrm << x << "G>" << ygold; 
	ostrm << "P>" << yhat; 

	yhat.clear();
	parser.argmax(x, mu, yhat); 
	ostrm << "M>" << yhat; 

	cout << ostrm; 
	ostrm.clear(); 
	cout << endl; 
      }
    }


    ////////////////////////////////////////////////////////////////////////////
    //  C O U N T   S C O R E S
    //////////////////////////////////////

    /** 
     * \brief A type of scores used for counting parse trees 
     * \author Xavier Carreras
     * \ingroup base
     */
    template <typename Symbols, typename X, typename R>
    class CountScores {
    private:
      double _s; 
    public:
      //! \brief Constructor with default score set to 1
      CountScores() : _s(1.0) {}
      //! \brief Constructor with default score set to 1
      CountScores(const X& x) : _s(1.0) {}
      //! \brief Constructor specifying the score to be returned
      CountScores(const X& x, double s) : _s(s) {}
      //! \brief Scores given part
      void set_score(double s) { _s = s; }
      //! \brief Scores given part
      double operator()(const R& r) {
	return _s;
      }
      
      class Scorer {
      private:
	double _s; 
      public:
	Scorer() : _s(0) {};
	Scorer(double s) : _s(s) {};
	
	void set_score(double s) { _s = s; }
	
	void scores(const X& x, CountScores<Symbols,X,R>& scores) const {
	  if (_s<0) {
	    scores.set_score(1.0/x.size()); 
	  }
	  else {
	    scores.set_score(_s); 
	  }
	}
	
	template <typename Y>
	void scores(const X& x, const Y& y, CountScores<Symbols,X,R>& s) const {
	  scores(x, s); 
	}
	
	void scores(const Example<X,Label<R>>& xy, CountScores<Symbols,X,R>& scores) const {
	  if (_s<0) {
	    scores.set_score(1.0/xy.x().size()); 
	  }
	  else {
	    scores.set_score(_s); 
	  }
	}

      };
    };
    
    template <typename Symbols, typename X, typename R>
    class Factory<CountScores<Symbols,X,R>> {
    public:
      static void usage(std::ostream& o) {
	o << "Oracle scores options: (no options)" << endl;
	o << endl;
      }      
      static void configure(CountScores<Symbols,X,R> & t, Options& options) {
	cerr << "Factory: configuring scores of type " << typeid(CountScores<Symbols,X,R>).name() << endl; 
      }      
      static void configure(typename CountScores<Symbols,X,R>::Scorer& t, Options& options) {
	cerr << "Factory: configuring scorer of type " << typeid(CountScores<Symbols,X,R>).name() << endl;
	double count; 
	options.get("count", count); 
	t.set_score(count);
      }
    };




    ////////////////////////////////////////////////////////////////////////////
    //  A L L   D E R I V A T I O N S
    //  
    //  (experimental!!!)
    //////////////////////////////////////

    // // this functor operates on each sentence
    // template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
    // struct AllerivationsFunctor;
    
    // this is the main code for this command
    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptDep::print_all_derivations(Options& options, const string& model_name) {
      cerr << name() << " : all derivations " << endl; 

      typedef AllDerivationsFunctor<Symbols,X,Y,R,I,S,F,IO> Functor; 
      Functor f(options, model_name); 
      forall<Functor,Symbols,X,Y,R,I,S,F,IO>(f,options);
    } 


    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename Feat, typename IO> 
    struct AllDerivationsFunctor {
    public:
      typedef ShagTT<X,Y,R,HYPER> HyperParser;
      typedef Chart<Semiring<HYPER>::Item> HyperGraph; 
      typedef Semiring<HYPER>::Item Item; 
      typedef Semiring<HYPER>::Edge Edge; 

      typename HyperParser::Configuration HPConfig; 

      AllDerivationsFunctor(Options& options, const string& model_name) {
	cerr << model_name << " : all derivations " << endl; 
	//	Factory<HyperParser>::configure(HPConfig);
      }

      void start(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io) {}
      void end(Symbols& symbols, typename R::Configuration& Rconfig, I& parser, S& scorer, IO& io) {}
      
      void operator()(Symbols& symbols,
		      typename R::Configuration& RConfig, 
		      I& parser,
		      S& scorer,
		      IO& io, 
		      const X& x, const Y& y) {
	int n = x.size();

	Label<R> parts;
	parser.decompose(x,y,parts);
	typename S::Scores scores; 
	scorer.scores(x,parts,scores); 

	cout << "///////////////////////////" << endl; 
	cout << "// SENTENCE  " << x.id() << " (" << n << " tokens)" << endl; 

	CoNLLStream conll;
	conll.add_padding = true; 
	conll << x << y << parts; 
	cerr << conll; 

	// compute an hypergraph
	HyperGraph hg(n+1);
	cout << " [computing the hypergraph ... " << flush;  
	HyperParser::inside(HPConfig, x, scores, hg);
	cout << "done!]" << endl; 

	WriteDerivation writer; 
	_hypergraph::HeadsVector d(x.size()); 

	all_derivations<WriteDerivation,_hypergraph::HeadsVector>(hg(0, n, RTRI), writer, d); 
      }

      /**
       *  \brief Writes a vector of dependency heads to cout, while
       *  keeping track of the number of derivations processed so far.
       */ 
      struct WriteDerivation {
	int n; 
	WriteDerivation() : n(0) {}
	inline void operator()(const _hypergraph::HeadsVector& d) {
	  ++n; 
	  cout << "(D" << n << ")"; 
	  for (size_t i=0; i<d.head.size(); ++i) {
	    cout << "\t" << d.head[i]; 
	  }
	  cout << endl;
	}
      };

    };



  } // control 
} // treeler

