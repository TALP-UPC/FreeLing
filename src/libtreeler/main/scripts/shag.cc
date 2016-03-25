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
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   script-shag.cc
 * \author Xavier Carreras
 */

#include "treeler/util/register.h"
#include "treeler/control/script-shag.h"
REGISTER_SCRIPT(shag, control::ScriptSHAG);
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "treeler/util/options.h"
#include "treeler/base/score-dumper.h"
#include "treeler/control/script-train.h"

#include "treeler/dep/chart.h"
#include "treeler/dep/semiring.h"
#include "treeler/dep/hyperParser.h"
#include "treeler/dep/shagTT.h"

using namespace std;

namespace treeler {
  
  namespace control {

    void ScriptSHAG::usage(const string& msg) {
      if (msg!="") {
	cerr << name() << " : " << msg << endl;
	return;
      }
      cerr << name() << " : runs a SHAG parser" << endl;
      cerr << endl;
      cerr << name() << " commands:" << endl;
      cerr << " --cmd=parse  : parses data with a model (default)" << endl;     
      cerr << " --cmd=parseo  : old parsing routine" << endl;     
      cerr << " --cmd=scores : dumps part-scores to stdout" << endl;
      cerr << " --cmd=sum    : sums the mass of derivations" << endl;
      cerr << " --cmd=hyper  : builds an hypergraph and runs the semiring specified by --mod" << endl;
      cerr << " --cmd=marg   : computes marginals on dependencies" << endl;
      cerr << " --cmd=all    : lists all derivations" << endl;
      cerr << " --cmd=train  : trains a feature-based model with perceptron" << endl;
      cerr << endl;
      cerr << name() << " options:" << endl;
      cerr << " --mod=<name> : model name" << endl;
      cerr << " --oracle     : use oracle scores for parsing " << endl;
      cerr << " --count=<double> : use scores for counting parse trees" << endl;
      cerr << "     (a negative value stands for scores of 1/n, while a" << endl;
      cerr << "      value of 0 is used for counting in the log-space)" << endl;
      cerr << " --scores     : dump part scores to cout" << endl;
      cerr << endl;
      
      string model_name;
      if (!Options::get("mod", model_name)) {
	GenericModel::help(cerr); 
      }
      else {
	GenericModel::run(true); 
      }
      cerr << endl;
    }
    
    
    // This is the main non-templated script, instantiates the templated script via a generic model selector
    void ScriptSHAG::run(const string& dir, const string& data_file) {
      GenericModel::run(false);
    }

    // This scorer is used to count derivations
    template <typename X, typename R>
    class CountScores; 

    template <typename X, typename R>
    class Factory<CountScores<X,R>>;



    /**
     * This auxiliar function selects a type of scores, using
     * command-line options, and calls the routine
     * FUNC.run<X,Y,R,I,S,F,IO>(model_name, help)
     * 
     */
    template <typename FUNC, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>
    void choose_scorer(const string& model_name, bool help) {
      // Choose between oracle scoring, count scoring, or the defaul model scoring
      int oracle = 0; 
      int count = 0; 
      Options::get("oracle", oracle); 

      FUNC f; 
      if (oracle) {
	typedef OracleScores<X,R> OS;
	f.template run<X,Y,R,I,OS,F,IO>(model_name, help);
      }
      else if (Options::get("count", count)) {
	typedef CountScores<X,R> CS;       
	f.template run<X,Y,R,I,CS,F,IO>(model_name, help);
      }
      else {
	f.template run<X,Y,R,I,S,F,IO>(model_name, help);
      }      
    }
       
    
    // This is the main templated script with all types resolved. 
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::run(const string& model_name, bool help) {

      // choose command and then type of scoring

      string cmd = "parse"; 
      Options::get("cmd", cmd);      
      if (cmd=="parseo") {
	// Choose between oracle scoring, count scoring, or the defaul model scoring
	int oracle = 0; 
	int count = 0; 
	Options::get("oracle", oracle); 
	if (oracle) {
	  typedef OracleScores<X,R> O;       
	  parse_old<X,Y,R,I,O,F,IO>(model_name, help);
	}
	else if (Options::get("count", count)) {
	  typedef CountScores<X,R> SCount;       
	  parse_old<X,Y,R,I,SCount,F,IO>(model_name, help);
	}
	else {
	  parse_old<X,Y,R,I,S,F,IO>(model_name, help);
	}

      }
      else if (cmd=="sco" or cmd=="scores") {
	choose_scorer<DumpMain,X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="parse") {
	choose_scorer<ParseMain,X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="hyperparse") {
	choose_scorer<HyperParseMain,X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="hyper") {
	choose_scorer<HyperMain,X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="sum") {
	choose_scorer<SumMain,X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="hypersum") {
	choose_scorer<HyperSumMain,X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="all") {
	typedef OracleScores<X,R> OS;
	print_all_derivations<X,Y,R,I,OS,F,IO>(model_name, help); 
      }
      else if (cmd=="marg" or cmd=="mu") {
	choose_scorer<MarginalsMain,X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="train") {
	//	train<X,Y,R,I,S,F,IO>(model_name, help);
      }
      else {
	cerr << name() << " : unknown command \"" << cmd << "\"" << endl;
	exit(0);
      }
    }    






    // a generic routine that initializes components, and runs the functor on all sentences
    template <typename Functor, typename X, typename Y, typename R, typename I, typename S, typename FGEN, typename IO>  
    void ScriptSHAG::forall(Functor& f, bool help) {
      cerr << name() << " : running f.name() " << " with ..." << endl;
      cerr << " X=" << typeid(X).name() << endl
	   << " Y=" << typeid(Y).name() << endl
	   << " R=" << typeid(R).name() << endl
	   << " I=" << typeid(I).name() << endl
	   << " S=" << typeid(S).name() << endl
	   << " F=" << typeid(FGEN).name() << endl
	   << " IO=" << typeid(IO).name() << endl << endl;
      
      if (help) {
	Factory<R>::usage(cerr);
	Factory<I>::usage(cerr);
	Factory<S>::usage(cerr);
	Factory<FGEN>::usage(cerr);
	Factory<IO>::usage(cerr);
	exit(0);
      }

      Configuration config;

      typename R::Configuration Rconfig; 
      Factory<R>::configure(Rconfig);      
      // input output options
      IO io;
      Factory<IO>::configure(io);
      
      typename I::Configuration Iconfig;
      Factory<I>::configure(Iconfig);
      Options::get("L", Iconfig.L);
      
      typename S::Scorer scorer; 
      Factory<S>::configure(scorer);
      
      DataStream<X,Y,IO> ds(io, std::cin);
      config.ifile = ""; 
      Options::get("data", config.ifile); 
      config.from_cin = config.ifile.empty() or config.ifile=="-";   
      if (!config.from_cin) {
	ds.set_file(config.ifile);
      }

      /* visit examples and parse them */
      typename DataStream<X,Y,IO>::const_iterator it = ds.begin(), it_end = ds.end();
      for (; it!=it_end; ++it) {
	const X& x  = (*it).x();
	const Y& y  = (*it).y();

	// cout << f << endl;
	f(x,y, Rconfig, Iconfig, scorer, io);
      } // for each example          
    } // generic routine



    //////////////////////////////////////
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::DumpMain::run(const string& model_name, bool help) {
      typedef DumpFunctor<X,Y,R,I,S,F,IO> Functor; 
      Functor f; 
      ScriptSHAG::forall<Functor,X,Y,R,I,S,F,IO>(f,help); 
    } 


    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::DumpFunctor<X,Y,R,I,S,F,IO>::operator()(const X& x, const Y& y, 
							     typename R::Configuration& Rconfig, 
							     typename I::Configuration& Iconfig, 
							     typename S::Scorer& scorer, IO& io) {
      // compute the scores for the current example
      S scores; 
      scorer.scores(x,y,scores);
      ostringstream oss; 
      oss << "SCO " << x.id() << " "; 
      ScoreDumper<X,R,S>::dump(cout, Rconfig, x, scores, oss.str()); 
      cout << endl;
    }


    //////////////////////////////////////
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::ParseMain::run(const string& model_name, bool help) {
      typedef ParseFunctor<X,Y,R,ShagTT<X,Y,R,MAX>,S,F,IO> Functor; 
      Functor f; 
      f.dump_scores = false; 
      ScriptSHAG::forall<Functor,X,Y,R,ShagTT<X,Y,R,MAX>,S,F,IO>(f,help); 
      cout << "Dependences-accuracy: " << 100.0*f.correct_dep/f.total_dep << endl;
      cout << "Trees-accuracy: " << 100.0*f.correct_tree/f.total_tree << endl;
    } 


    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::ParseFunctor<X,Y,R,I,S,F,IO>::operator()(const X& x, const Y& y, 
							      typename R::Configuration& Rconfig, 
							      typename I::Configuration& Iconfig, 
							      typename S::Scorer& scorer, IO& io) {
      // compute the scores for the current example
      S scores; 
      scorer.scores(x,y,scores);
      
      if (dump_scores) {	  
	ostringstream oss; 
	oss << "SCO " << x.id() << " "; 
	ScoreDumper<X,R,S>::dump(cout, Rconfig, x, scores, oss.str()); 
      }
      
      // parse
	int n = (int)x.size();
	Chart<typename I::Item> inside(n+1);
	I::inside(Iconfig, x, scores, inside);
#define escriu2(side, kind) \
	for (int i = 0; i <= n; i++) { \
	  for (int j = 0; j <= n; j++) { \
	    if (j < i) cout << setw(4) << ' ' << ' '; \
	    else cout << setw(4) << side(i, j, kind).get_w() << ' '; \
	  } \
	  cout << endl; \
	} \
	cout << endl;
	
#define escriu(side) \
	cout << " - RTRI" << endl; escriu2(side, RTRI); \
	cout << " - LTRI" << endl; escriu2(side, LTRI); \
	cout << " - RTRA" << endl; escriu2(side, RTRA); \
	cout << " - LTRA" << endl; escriu2(side, LTRA); \
	cout << endl;
// 	escriu(inside);
#undef escriu
#undef escriu2
	double w = inside(0, n, RTRI).get_w();
	Y yhat;
	inside(Key(0, n, RTRI)).get_tree(yhat);
	
	cout << "Total weight: " << w << endl;
      
      // write it 
      io.write(cout, x, y, yhat); 
      
      int differ = total_dep - correct_dep;
      typename Y::const_iterator i=yhat.begin(), iend=yhat.end();
      while (i!=iend) {
	if (y.count(*i) > 0) correct_dep++;
	total_dep++;
	++i;
      }
      if (total_dep - correct_dep == differ) correct_tree++;
      total_tree++;
    }
    
    
    //////////////////////////////////////
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::HyperMain::run(const string& model_name, bool help) {
      typedef HyperFunctor<X,Y,R,I,S,F,IO> Functor; 
      Functor f; 
      f.dump_scores = false; 

      Factory<typename Functor::HyperParser>::configure(f.HPConfig);
      ScriptSHAG::forall<Functor,X,Y,R,I,S,F,IO>(f,help); 
    } 

    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::HyperFunctor<X,Y,R,I,S,F,IO>::operator()(const X& x, const Y& y, 
							      typename R::Configuration& Rconfig, 
							      typename I::Configuration& Iconfig, 
							      typename S::Scorer& scorer, IO& io) {
      typedef ShagTT<X,Y,R,HYPER> HyperParser;
      typedef Chart<Semiring<HYPER>::Item> HyperGraph; 
      typedef Semiring<HYPER>::Item Item; 
      typedef Semiring<HYPER>::Edge Edge; 
      
      // compute the scores for the current example
      S scores; 
      scorer.scores(x,y,scores);
      
      if (dump_scores) {	  
	ostringstream oss; 
	oss << "SCO " << x.id() << " "; 
	ScoreDumper<X,R,S>::dump(cout, Rconfig, x, scores, oss.str()); 
      }
      
      // parse
      int n = (int)x.size();
      HyperGraph hyper(n+1);
      HyperParser::inside(HPConfig, x, scores, hyper);
      
      Chart<typename I::Item> inside(n+1), outside(n+1), marginals(n+1);
      insideHyper<I::_SR_t>(hyper, inside);
      outsideHyper<I::_SR_t>(hyper, inside, outside);
      I::marginals(Iconfig, x, scores, inside, outside, marginals);
#define escriu2(side, kind) \
      for (int i = 0; i <= n; i++) { \
	for (int j = 0; j <= n; j++) { \
	  if (j < i) cout << setw(5) << ' ' << ' '; \
	  else cout << setw(5) << side(i, j, kind).get_w() << ' '; \
	} \
	cout << endl; \
      } \
      cout << endl;
      
#define escriu(side) \
      cout << " - RTRI" << endl; escriu2(side, RTRI); \
      cout << " - LTRI" << endl; escriu2(side, LTRI); \
      cout << " - RTRA" << endl; escriu2(side, RTRA); \
      cout << " - LTRA" << endl; escriu2(side, LTRA); \
      cout << endl;
// 	cout << "INSIDE" << endl; escriu(inside);
// 	cout << "OUTSIDE" << endl; escriu(outside);
     cout << "MARGINALS" << endl; escriu(marginals);
#undef escriu
#undef escriu2
    }
    
    
    //////////////////////////////////////
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::HyperParseMain::run(const string& model_name, bool help) {
      typedef HyperParseFunctor<X,Y,R,ShagTT<X,Y,R,HYPER>,S,F,IO> Functor; 
      Functor f; 
      f.dump_scores = false; 

      ScriptSHAG::forall<Functor,X,Y,R,ShagTT<X,Y,R,HYPER>,S,F,IO>(f,help); 
      
      cout << "Dependences-accuracy: " << 100.0*f.correct_dep/f.total_dep << endl;
      cout << "Trees-accuracy: " << 100.0*f.correct_tree/f.total_tree << endl;
    } 

    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::HyperParseFunctor<X,Y,R,I,S,F,IO>::operator()(const X& x, const Y& y, 
							      typename R::Configuration& Rconfig, 
							      typename I::Configuration& Iconfig, 
							      typename S::Scorer& scorer, IO& io) {
      typedef ShagTT<X,Y,R,HYPER> HyperParser;
      typedef Chart<Semiring<HYPER>::Item> HyperGraph;
      typedef Semiring<HYPER>::Item Item;
      typedef Semiring<HYPER>::Edge Edge;
      
      // compute the scores for the current example
      S scores; 
      scorer.scores(x,y,scores);
      
      if (dump_scores) {	  
	ostringstream oss; 
	oss << "SCO " << x.id() << " "; 
	ScoreDumper<X,R,S>::dump(cout, Rconfig, x, scores, oss.str()); 
      }
      
      // parse
      int n = (int)x.size();
      HyperGraph hyper(n+1);
      I::inside(Iconfig, x, scores, hyper);
      
      Chart<typename Semiring<MAX>::Item> inside(n+1);
      insideHyper<MAX>(hyper, inside);
      double w = inside(0, n, RTRI).get_w();
      Y yhat;
      inside(Key(0, n, RTRI)).get_tree(yhat);
      
      cout << "Total weight: " << w << endl;
      
      // write it 
      io.write(cout, x, y, yhat); 
      
      int differ = total_dep - correct_dep;
      typename Y::const_iterator i=yhat.begin(), iend=yhat.end();
      while (i!=iend) {
	if (y.count(*i) > 0) correct_dep++;
	total_dep++;
	++i;
      }
      if (total_dep - correct_dep == differ) correct_tree++;
      total_tree++;
    }
    
    
    //////////////////////////////////////
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::HyperSumMain::run(const string& model_name, bool help) {
      typedef HyperSumFunctor<X,Y,R,ShagTT<X,Y,R,HYPER>,S,F,IO> Functor; 
      Functor f; 
      f.dump_scores = false; 

      ScriptSHAG::forall<Functor,X,Y,R,ShagTT<X,Y,R,HYPER>,S,F,IO>(f,help);
    } 

    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::HyperSumFunctor<X,Y,R,I,S,F,IO>::operator()(const X& x, const Y& y, 
							      typename R::Configuration& Rconfig, 
							      typename I::Configuration& Iconfig, 
							      typename S::Scorer& scorer, IO& io) {
      typedef ShagTT<X,Y,R,HYPER> HyperParser;
      typedef Chart<Semiring<HYPER>::Item> HyperGraph;
      typedef Semiring<HYPER>::Item Item;
      typedef Semiring<HYPER>::Edge Edge;
      
      // compute the scores for the current example
      S scores; 
      scorer.scores(x,y,scores);
      
      if (dump_scores) {	  
	ostringstream oss; 
	oss << "SCO " << x.id() << " "; 
	ScoreDumper<X,R,S>::dump(cout, Rconfig, x, scores, oss.str()); 
      }
      
      // parse
      int n = (int)x.size();
      HyperGraph hyper(n+1);
      I::inside(Iconfig, x, scores, hyper);
      
      Chart<typename Semiring<ADD>::Item> inside(n+1);
      insideHyper<ADD>(hyper, inside);
      double w = inside(0, n, RTRI).get_w();
      
      cout << "Total weight: " << w << endl;
    }
    
    
    //////////////////////////////////////
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::SumMain::run(const string& model_name, bool help) {
      cerr << name() << " : summing with " << model_name << endl;
      cerr << " X=" << typeid(X).name() << endl
	   << " Y=" << typeid(Y).name() << endl
	   << " R=" << typeid(R).name() << endl
	   << " I=" << typeid(I).name() << endl
	   << " S=" << typeid(S).name() << endl
	   << " F=" << typeid(F).name() << endl
	   << " IO=" << typeid(IO).name() << endl << endl;
      typedef SumFunctor<X,Y,R,I,S,F,IO> Functor; 
      Functor f; 
      ScriptSHAG::forall<Functor,X,Y,R,I,S,F,IO>(f,help);            
    } 

    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::SumFunctor<X,Y,R,I,S,F,IO>::operator()(const X& x, const Y& y, 
							    typename R::Configuration& Rconfig, 
							    typename I::Configuration& Iconfig, 
							    typename S::Scorer& scorer, IO& io) {
      cout << "(summing derivations on " << x.id() << ")" << endl; // compute the scores for the current example
      S scores; 
      scorer.scores(x,y,scores);
      
      // parse
	int n = (int)x.size();
	Chart<typename I::Item> inside(n+1);
	I::inside(Iconfig, x, scores, inside);
#define escriu2(side, kind) \
	for (int i = 0; i <= n; i++) { \
	  for (int j = 0; j <= n; j++) { \
	    if (j < i) cout << setw(4) << ' ' << ' '; \
	    else cout << setw(4) << side(i, j, kind).get_w() << ' '; \
	  } \
	  cout << endl; \
	} \
	cout << endl;
	
#define escriu(side) \
	cout << " - RTRI" << endl; escriu2(side, RTRI); \
	cout << " - LTRI" << endl; escriu2(side, LTRI); \
	cout << " - RTRA" << endl; escriu2(side, RTRA); \
	cout << " - LTRA" << endl; escriu2(side, LTRA); \
	cout << endl;
// 	escriu(inside);
#undef escriu
#undef escriu2
	double w = inside(0, n, RTRI).get_w();
	
	cout << "Total weight: " << w << endl;
    }


    //////////////////////////////////////
    // computation of first-order marginals
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::MarginalsMain::run(const string& model_name, bool help) {
      cerr << name() << " : marginals with " << model_name << endl;
      typedef MarginalsFunctor<X,Y,R,I,S,F,IO> Functor; 
      Functor f; 
      ScriptSHAG::forall<Functor,X,Y,R,I,S,F,IO>(f,help);            
    } 

    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::MarginalsFunctor<X,Y,R,I,S,F,IO>::operator()(const X& x, const Y& y, 
								  typename R::Configuration& Rconfig, 
								  typename I::Configuration& Iconfig, 
								  typename S::Scorer& scorer, IO& io) {
      // compute the scores for the current example
      S scores; 
      scorer.scores(x,y,scores);
      
      // parse
	int n = (int)x.size();
	Chart<typename I::Item> inside(n+1), outside(n+1), marginals(n+1);
	I::inside(Iconfig, x, scores, inside);
	I::outside(Iconfig, x, scores, inside, outside);
	I::marginals(Iconfig, x, scores, inside, outside, marginals);
#define escriu2(side, kind) \
	for (int i = 0; i <= n; i++) { \
	  for (int j = 0; j <= n; j++) { \
	    if (j < i) cout << setw(5) << ' ' << ' '; \
	    else cout << setw(5) << side(i, j, kind).get_w() << ' '; \
	  } \
	  cout << endl; \
	} \
	cout << endl;
	
#define escriu(side) \
	cout << " - RTRI" << endl; escriu2(side, RTRI); \
	cout << " - LTRI" << endl; escriu2(side, LTRI); \
	cout << " - RTRA" << endl; escriu2(side, RTRA); \
	cout << " - LTRA" << endl; escriu2(side, LTRA); \
	cout << endl;
// 	cout << "INSIDE" << endl; escriu(inside);
// 	cout << "OUTSIDE" << endl; escriu(outside);
     cout << "MARGINALS" << endl; escriu(marginals);
#undef escriu
#undef escriu2
    }



    //////////////////////////////////////
    // the old routine for input parsing sentences
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::parse_old(const string& model_name, bool help) {
      cerr << name() << " : running " << model_name << " parser with ..." << endl;
      cerr << " X=" << typeid(X).name() << endl
	   << " Y=" << typeid(Y).name() << endl
	   << " R=" << typeid(R).name() << endl
	   << " I=" << typeid(I).name() << endl
	   << " S=" << typeid(S).name() << endl
	   << " F=" << typeid(F).name() << endl
	   << " IO=" << typeid(IO).name() << endl << endl;
      
      if (help) {
	Factory<R>::usage(cerr);
	Factory<I>::usage(cerr);
	Factory<S>::usage(cerr);
	Factory<F>::usage(cerr);
	Factory<IO>::usage(cerr);
	exit(0);
      }

      Configuration config; 
      
      typename R::Configuration Rconfig; 
      Factory<R>::configure(Rconfig);
      
      // input output options
      IO io;
      Factory<IO>::configure(io);
      
      typename I::Configuration Iconfig;
      Factory<I>::configure(Iconfig);
      Options::get("L", Iconfig.L);
      
      typename S::Scorer scorer; 
      Factory<S>::configure(scorer);
      
      DataStream<X,Y,IO> ds(io, std::cin);
      config.ifile = ""; 
      Options::get("data", config.ifile); 
      config.from_cin = config.ifile.empty() or config.ifile=="-";   
      if (!config.from_cin) {
	ds.set_file(config.ifile);
      }

      config.dump_scores = 0; 
      Options::get("scores", config.dump_scores); 
      /* visit examples and parse them */
      typename DataStream<X,Y,IO>::const_iterator it = ds.begin(), it_end = ds.end();
      int correct_dep = 0;
      int total_dep = 0;
      int correct_tree = 0;
      int total_tree = 0;
      for (; it!=it_end; ++it) {
	const X& x  = (*it).x();
	const Y& y  = (*it).y();

	// compute the scores for the current example
	S scores; 
	scorer.scores(*it, scores);
	
	if (config.dump_scores) {	  
	  ostringstream oss; 
	  oss << "SCO " << x.id() << " "; 
	  ScoreDumper<X,R,S>::dump(cout, Rconfig, x, scores, oss.str()); 
	}

	// parse
	int n = (int)x.size();
	Chart<typename I::Item> inside(n+1), outside(n+1), totalside(n+1);
	I::inside(Iconfig, x, scores, inside);
// 	I::outside(Iconfig, x, scores, inside, outside);
// 	I::totalside(Iconfig, x, scores, inside, outside, totalside);
#define escriu(side) \
	cout << " - tri" << endl; \
	for (int i = 0; i <= n; i++) { \
	  for (int j = i; j <= n; j++) cout << setw(4) << side(i, j, RTRI).get_w() << ' '; \
	  cout << endl; \
	} \
	cout << endl; \
	for (int i = 0; i <= n; i++) { \
	  cout << "                       "; \
	  for (int j = i; j <= n; j++) cout << setw(4) << side(i, j, LTRI).get_w() << ' '; \
	  cout << endl; \
	} \
	cout << endl; \
	cout << " - tra" << endl; \
	for (int i = 0; i <= n; i++) { \
	  for (int j = i; j <= n; j++) cout << setw(4) << side(i, j, RTRA).get_w() << ' '; \
	  cout << endl; \
	} \
	cout << endl; \
	for (int i = 0; i <= n; i++) { \
	  cout << "                       "; \
	  for (int j = i; j <= n; j++) cout << setw(4) << side(i, j, LTRA).get_w() << ' '; \
	  cout << endl; \
	} \
	cout << endl << endl;
// 	cout << "INSIDE VALUES" << endl; escriu(inside);
// 	cout << "OUTSIDE VALUES" << endl; escriu(outside);
// 	cout << "TOTAL VALUES" << endl; escriu(totalside);
#undef escriu
	double w = inside(0, n, RTRI).get_w();
	Y yhat;
	inside(Key(0, n, RTRI)).get_tree(yhat);
	
	cout << "Total weight: " << w << endl;

	// write it 
	io.write(cout, x, y, yhat); 
	
	int differ = total_dep - correct_dep;
	typename Y::const_iterator i=yhat.begin(), iend=yhat.end();
	while (i!=iend) {
	  if (y.count(*i) > 0) correct_dep++;
	  total_dep++;
	  ++i;
	}
	if (total_dep - correct_dep == differ) correct_tree++;
	total_tree++;
      
      } // for each example    
      cout << "Dependences-accuracy: " << 100.0*correct_dep/total_dep << endl;
      cout << "Trees-accuracy: " << 100.0*correct_tree/total_tree << endl;
    } // parse



    //////////////////////////////////////
    // a Perceptron training routine
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::train(const string& model_name, bool help) {
      cerr << name() << " : training " << model_name << " with ..." << endl;
      cerr << " X=" << typeid(X).name() << endl
	   << " Y=" << typeid(Y).name() << endl
	   << " R=" << typeid(R).name() << endl
	   << " I=" << typeid(I).name() << endl
	   << " S=" << typeid(S).name() << endl
	   << " F=" << typeid(F).name() << endl
	   << " IO=" << typeid(IO).name() << endl << endl;
      
      ScriptTrain::run<X,Y,R,I,S,F,IO>(model_name, help);
    } // train



    /** 
     * \brief A type of scores used for counting parse trees 
     * \author Xavier Carreras
     * \ingroup base
     */
    template <typename X, typename R>
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
	
	void scores(const X& x, CountScores<X,R>& scores) const {
	  if (_s<0) {
	    scores.set_score(1.0/x.size()); 
	  }
	  else {
	    scores.set_score(_s); 
	  }
	}
	
	template <typename Y>
	void scores(const X& x, const Y& y, CountScores<X,R>& s) const {
	  scores(x, s); 
	}
	
	void scores(const Example<X,Label<R>>& xy, CountScores<X,R>& scores) const {
	  if (_s<0) {
	    scores.set_score(1.0/xy.x().size()); 
	  }
	  else {
	    scores.set_score(_s); 
	  }
	}

      };
    };
    
    template <typename X, typename R>
    class Factory<CountScores<X,R>> {
    public:
      static void usage(std::ostream& o) {
	o << "Oracle scores options: (no options)" << endl;
	o << endl;
      }      
      static void configure(CountScores<X,R>& t) {
	cerr << "Factory: configuring scores of type " << typeid(CountScores<X,R>).name() << endl; 
      }      
      static void configure(typename CountScores<X,R>::Scorer& t) {
	cerr << "Factory: configuring scorer of type " << typeid(CountScores<X,R>).name() << endl;
	double count; 
	Options::get("count", count); 
	t.set_score(count);
      }
    };



    //////////////////////////////////////
    //  A L L   D E R I V A T I O N S
    //  
    //  (experimental!!!)
    //////////////////////////////////////

    // this functor operates on each sentence
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO> 
    struct PrintAllDerivationsFunctor;
    
    // this is the main code for this command
    template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void ScriptSHAG::print_all_derivations(const string& model_name, bool help) {
      cerr << name() << " : all derivations " << endl; 

      typedef PrintAllDerivationsFunctor<X,Y,R,I,S,F,IO> Functor; 
      Functor f; 
      Factory<typename Functor::HyperParser>::configure(f.HPConfig);
      ScriptSHAG::forall<Functor,X,Y,R,I,S,F,IO>(f,help);
    } 


    template <typename X, typename Y, typename R, typename I, typename S, typename Feat, typename IO> 
    struct PrintAllDerivationsFunctor {
    public:
      typedef ShagTT<X,Y,R,HYPER> HyperParser;
      typedef Chart<Semiring<HYPER>::Item> HyperGraph; 
      typedef Semiring<HYPER>::Item Item; 
      typedef Semiring<HYPER>::Edge Edge; 

      typename HyperParser::Configuration HPConfig; 
      
      void operator()(const X& x, const Y& y,
		      typename R::Configuration& RConfig, 
		      typename I::Configuration& IConfig,
		      typename S::Scorer& scorer,
		      IO& io) {
	int n = x.size();
	cout << "All derivations for sequence " << x.id() << " (" << n << " tokens)" << endl; 

	S scores; 
	scorer.scores(x,y,scores); 

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

