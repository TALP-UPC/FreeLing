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
 * \file   script-srl.cc
 * \author Xavier Lluis, Manu, Xavier Carreras
 */

#include "scripts/srl.h"
#include "scripts/register.h"
REGISTER_SCRIPT(srl, control::ScriptSRL);

#include "scripts/dump.h"
#include "scripts/train.h"
#include "scripts/decode.h"




#include <cstdio>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "treeler/base/score-dumper.h"
#include "treeler/control/factory-base.h"
#include "treeler/control/factory-scores.h"
#include "treeler/control/factory-dep.h"
#include "treeler/control/factory-ioconll.h"

#include "treeler/util/options.h"

// classes srl
#include "treeler/srl/fgen-srl-v1.h"
#include "treeler/srl/srl.h"
#include "treeler/srl/simple-parser.h"
#include "treeler/srl/part-srl0.h"
#include "treeler/srl/io.h"
#include "treeler/srl/scorers.h"
#include "treeler/srl/factory-srl.h"
#include "treeler/srl/srl-eval.h"


using namespace std;

namespace treeler {

  namespace control {

    //!
    //
    void ScriptSRL::usage(Options& options) {
      cerr << name() << " : runs a SRL parser" << endl;
      cerr << "usage : srl --mod=<model_name> --cmd=<command_name> [options]" << endl;
      cerr << endl;

      SRLModel::usage(options, "mod", cerr);
      cerr << endl;
      
      string cmd = ""; 
      options.get("cmd", cmd); 
      if (cmd.empty()) {
	cerr << "Commands available:" << endl; 
	cerr << "   train   : srl train over input file " << endl;
	cerr << "   dump    : reads an input file and dumps it to cout " << endl;
	cerr << "   parse   : srl parse" << endl;
	cerr << "   decode  : srl parse" << endl;
	cerr << "   test-io : io test " << endl;
	cerr << "   mkdicts : generate symbol dictionaries" << endl; 
	cerr << endl;
      }
      else {
	cerr << "Command " << cmd << " help:" << endl; 
	if (cmd == "train") {

	}
	else if (cmd == "dump") {

	}
	else if (cmd == "parse") {

	}
	else if (cmd == "decode") {

	}
	else if (cmd == "mkdicts") {

	}
	else if (cmd == "test-io") {

	}
      }
    }




    // This is the main non-templated script, instantiates the templated script via a generic model selector
    void ScriptSRL::run(Options& options) {
      SRLModel::run(options, "mod", cerr);
    }


    template<typename Model>
    void ScriptSRL::usage(Options& options, ostream& log, const string& prefix) {
      Factory<typename Model::Symbols>::usage(log); 
      Factory<typename Model::R>::usage(log); 
      Factory<typename Model::I>::usage(log); 
      Factory<typename Model::F>::usage(log); 
      Factory<typename Model::IO>::usage(log); 
    }
    

    // This is the main templated script with all types resolved.
    template<typename Model>
    void ScriptSRL::run(Options& options, ostream& log) {

      //typedef typename Model::Symbols Symbols; 
      //typedef typename Model::X X; 
      //typedef typename Model::Y Y; 
      //typedef typename Model::R R; 
      //typedef typename Model::I I; 
      //typedef typename Model::S S; 
      //typedef typename Model::F F; 
      //typedef typename Model::IO IO; 

      string cmd = "parse";
      options.get("cmd", cmd);
      // Choose between parsing sentences or training a new model
      if (cmd == "parse") {
	// Choose between oracle scoring or standard scoring
	int oracle = 0;
	options.get("oracle", oracle);
	if (oracle) {
	  //            typedef OracleScorer<Symbols,X,R> O;
	  //	    parse<Symbols,X,Y,R,I,O,F,IO>(options, model_name);
	} 
	else {
	  parse<Model>(options, log);
	}
      } 
      else if (cmd == "paths") {
	paths<Model>(options, log);
      } 
      else if (cmd == "decode"){
	ScriptDecode::Configuration config;
	ScriptDecode::configure(options, config);
	ScriptDecode::run<Model>(options, config, log);
      } 
      else if (cmd == "train") {
	ScriptTrain::run<Model>(options, log);
      } 
      else if (cmd == "dump") {
	ScriptDump::run<Model>(options, log); 
      } 
      else {
	cerr << "command " << cmd << " not recognized " << endl;
      }
    }
    

    template<typename Model>
    void ScriptSRL::parse(Options& options, ostream& log) {

      typedef typename Model::Symbols Symbols; 
      typedef typename Model::X X; 
      typedef typename Model::Y Y; 
      typedef typename Model::R R; 
      typedef typename Model::I I; 
      typedef typename Model::S S; 
      //typedef typename Model::F F; 
      typedef typename Model::IO IO; 


      Symbols symbols; 
      Factory<Symbols>::configure(symbols, options);
      
      S scorer(symbols); 
      Factory<S>::configure(symbols, scorer, options);

      I parser(symbols);
      Factory<I>::configure(parser, options);

      IO io(symbols);
      Factory<IO>::configure(io, options);

      bool eval_flag = false; 
      options.get("eval", eval_flag); 
      bool display_gold = true; 
      options.get("ygold", display_gold); 

      srl::SrlEval eval; 

      DataStream<X,Y,IO> ds(io, std::cin);      
      for (auto const & xy : ds) {
	const X& x = xy.x();
 	const Y& y = xy.y(); 
	typename S::Scores scores; 
	scorer.scores(x, scores); 
	Y predicted;
	Label<R> pparts; 
	parser.argmax(x, scores, predicted, pparts);
	if (display_gold) {
	  //	  io.write(cout, x, "GOLD>", y, "HAT>", predicted);
	  BasicSentence<int,int> mapped_x;
	  symbols.map(x, mapped_x);
	  io.write(cout, x, "MAPPED>", mapped_x);
	}
	else {
	  io.write(cout, x, predicted);
	}
	if (eval_flag) {
	  Label<R> gparts;
	  parser.decompose(x, y, gparts);

	  DepTree<string> t = DepTree<string>::convert(x.dependency_vector()); 
	  t.print(cerr, "DTREE  ");

	  srl::SrlEval esent(symbols.d_semantic_labels.map("_")); 
	  cerr << "GPARTS ";
	  for (auto & r : gparts) if (r.rolelabel()!=0) cerr << " " << r; 
	  cerr << endl;
	  cerr << "PPARTS ";
	  for (auto & r : pparts) if (r.rolelabel()!=0) cerr << " " << r; 
	  cerr << endl;
	  esent(x, gparts, pparts); 
	  cerr << "EVAL_SENT " << x.id() << " " << esent.to_string() << endl;
	  srl::SrlEval e2(symbols.d_semantic_labels.map("_")); 
	  e2(x, y, predicted); 
	  cerr << "EVAL_SEN2 " << e2.to_string() << endl;
	  eval += esent; 
	  cerr << "EVAL_DATA " << eval.to_string() << endl; 

	}
      }
      cerr << "EVAL_DATA " << eval.to_string() << endl;    
    }


    template<typename Model>
    void ScriptSRL::paths(Options& options, ostream& log) {

      typedef typename Model::Symbols Symbols; 
      typedef typename Model::X X; 
      typedef typename Model::Y Y; 
      //typedef typename Model::R R; 
      typedef typename Model::I I; 
      typedef typename Model::IO IO; 

      typedef FIdxPair FIdx;
      typedef typename SwitchFIdx<typename Model::F, FIdx>::F F;

      string disp_fvec = "0";
      options.get("fvec", disp_fvec);

      Symbols symbols; 
      Factory<Symbols>::configure(symbols, options);
      
      I parser(symbols);
      Factory<I>::configure(parser, options);

      F fgen(symbols);
      Factory<F>::configure(fgen, options);

      IO io(symbols);
      Factory<IO>::configure(io, options);

      X *sent;
      Y *paset;

      while (io.read(cin, sent, paset)) {
	typename F::Features features; 

	fgen.features(*sent, features); 

	cout << "SENTENCE " << sent->id() << endl;
	//	io.config.offset = 0;
	io.write(cout, *sent, "GOLD>", *paset);
	//	io.config.offset = 1;

	const list<int>& pr = sent->get_predicates(); 
	for (auto p : pr) {
	  cout << "PREDICATE " << p << " " << (*paset)[p].sense << " token " << (*sent)[p].word() << endl; 
          cerr << "PATHS pred " << p << endl;
          for (int a = 0; a < sent->size(); ++a){
	    cerr << "  " << p << " " << a << " : " ;
            cerr << sent->paths_.PathToString(p, a) << endl;
	    int role = 0;
	    srl::PartSRL r(p,a,role);
	    if (disp_fvec != "0") {
	      FeatureVector<FIdx>* f = features(r);
	      cerr << "  f ";
	      if (disp_fvec == "p") {
		IOFVec::pretty_print(cerr, *f, "    ");
	      }	
	      else {
		IOFVec::write(cerr, *f);
	      }
	      features.discard(f);
	      cerr << endl;
	    }
          }


	}
	cout << endl;
      }      
    }
    


  } // control
} // treeler

