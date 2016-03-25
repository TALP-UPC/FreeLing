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
 * \file   script-fidx.cc
 * \author Xavier Carreras
 */

#include "treeler/util/register.h"
#include "treeler/control/script-fidx.h"
REGISTER_SCRIPT(fidx, ScriptFIdx);

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "treeler/control/models.h"
#include "treeler/control/script-dump.h"
#include "treeler/control/script-train.h"

#include "treeler/control/factory-base.h"
#include "treeler/control/factory-scores.h"
#include "treeler/control/factory-dep.h"
#include "treeler/control/factory-ioconll.h"

#include "treeler/util/options.h"

#include "treeler/io/io-basic.h"
#include "treeler/io/io-fvec.h"

#include "treeler/dep/fgen-dep-toy.h"
#include "treeler/dep/fgen-dep-McDonald.h"
#include "treeler/dep/fgen-dep-v1.h"

using namespace std;

using namespace treeler::control;

namespace treeler {

  //! 
  //
  void ScriptFIdx::usage(const string& msg) {
    if (msg!="") {
      cerr << name() << " : " << msg << endl;
      return;
    }
    cerr << name() << " : dumps feature-vectors with different feature index types" << endl;
    cerr << endl;
    cerr << "FIdx options:" << endl;
    cerr << " --mod=<name>    : model name" << endl;
    cerr << " --parts=[list]  : dump parts" << endl;
    cerr << "                   * y   : gold parts" << endl;
    cerr << "                   * all : all parts" << endl; 
    cerr << " --idx           : type of feature index " << endl;
    cerr << "                   * v0    : non-templated version" << endl;
    cerr << "                   * bits  : bit strings" << endl;
    cerr << "                   * chars : char strings" << endl;
    cerr << "                   * pairs  : a pair of bit and char strings" << endl;
    cerr << " --fvec          : dump part feature vectors" << endl;
    cerr << " --score         : dump part scores from a model" << endl;
    cerr << " --oracle        : dump part scores from an oracle" << endl;
    cerr << " --max           : dump max solution" << endl;
    cerr << endl;

    string model_name;
    if (!Options::get("mod", model_name)) {
      cerr << "Available models:" << endl;
      cerr << "  * dep1 : dependency parsing, arc factorization" << endl;
      cerr << "    --L : number of dependency labels" << endl;
      cerr << endl;
    }
    else {
      run_script(true); 
    }
    cerr << endl;
  }


  namespace control {
    // define a factory for FGenDepToy
    template <typename X, typename R, typename FIdx> 
    class Factory<FGenDepToy<X,R,FIdx>> {
    public:
      static void usage(std::ostream& o) {
	o << "FGenDepToy options: " << endl;
	o << "  --L=<int> : number of dependency labels" << endl;
	o << endl;
      };
      
      static void create(FGenDepToy<X,R,FIdx>*& f, bool verbose = true) {
	if (verbose) {
	  cerr << "Factory: creating FGenDepToy of type " << typeid(FGenDepToy<X,R,FIdx>).name() << endl;
	}
	int l = 1;
	if (!Options::get("L", l)) {
	  cerr << "Factory<FGenDepToy> : please specify number of dependency labels with --L=<int>" << endl;
	  exit(1);
	}
	f = new FGenDepToy<X,R,FIdx>(l);
      }
    };
    
    // define a factory for FGenDepMcDonald
    
    template <typename X, typename R, typename FIdx> 
    class Factory<FGenDepMcDonald<X,R,FIdx>> {
    public:
      static void usage(std::ostream& o) {
	o << "FGenDepMcDonald options: " << endl;
	o << "  --L=<int> : number of dependency labels" << endl;
	o << endl;
      };
      
      static void create(FGenDepMcDonald<X,R,FIdx>*& f, bool verbose = true) {
	if (verbose) {
	  cerr << "Factory: creating FGenDepMcDonald of type " << typeid(FGenDepMcDonald<X,R,FIdx>).name() << endl;
	}
	int l = 1;
	if (!Options::get("L", l)) {
	  cerr << "Factory<FGenDepMcDonald> : please specify number of dependency labels with --L=<int>" << endl;
	  exit(1);
	}
	f = new FGenDepMcDonald<X,R,FIdx>(l);
      }
    };

    // define a factory for FGenDepV1
    template <typename X, typename R, typename FIdx> 
    class Factory<FGenDepV1<X,R,FIdx>> {
    public:
      static void usage(std::ostream& o) {
	o << "FGenDepV1 options: " << endl;
	o << "  --L=<int> : number of dependency labels" << endl;
	o << endl;
      };
      
      static void create(FGenDepV1<X,R,FIdx>*& f) {
	cerr << "Factory: creating FGenDepV1 of type " << typeid(FGenDepV1<X,R,FIdx>).name() << endl;
	int l = 1;
	if (!Options::get("L", l)) {
	  cerr << "Factory<FGenDepV1> : please specify number of dependency labels with --L=<int>" << endl;
	  exit(-1);
	}
	f = new FGenDepV1<X,R,FIdx>(l);
      }
    };
  }
  

  void ScriptFIdx::run(const string& dir, const string& data_file) {
    run_script(false);
  }

  //! dispatch according to model name
  void ScriptFIdx::run_script(bool help) { 
    string model_name;
    if (!Options::get("mod", model_name)) {
      model_name = "dep1";
    }

    if (model_name=="dep1") {
      run_script<control::DEP1>(model_name, help);
    }
    else {
      cerr << name() << " : sorry, model " << model_name << " not supported." << endl;
    }
  }


  template <int M>
  void ScriptFIdx::run_script(const string& model_name, bool help) {
    string idx_type = ""; 
    if (!Options::get("idx", idx_type)) {
      usage("please name a type for feature indices (--idx)"); 
      exit(0); 
    }
    
    if (idx_type=="v0") {
      // run the standard feature generator of model M
      dump_fvec<typename control::Model<M>::X, 
	        typename control::Model<M>::Y, 
	        typename control::Model<M>::R, 
	        typename control::Model<M>::I,
                typename control::Model<M>::S, 
                typename control::Model<M>::F,
                OracleScores<typename control::Model<M>::X,typename control::Model<M>::R>,  
	        typename control::Model<M>::IO>(model_name, help);
    }
    // otherwise, select the fidx type
    else if (idx_type=="bits") {
      run_script<M, FIdxBits>(model_name, help);
    }
    else if (idx_type=="chars") {
      run_script<M, FIdxChars>(model_name, help);
    }
    else if (idx_type=="pairs") {
      run_script<M, FIdxPair>(model_name, help);
      //cerr << Script::name() << " : sorry, pair feature indices are not supported yet." << endl;
    }
    else {
      cerr << name << " : sorry, index type " << idx_type << " not supported." << endl;
    }
  }

  template <int M, typename FIdx>
  void ScriptFIdx::run_script(const string& model_name, bool help) {
    typedef typename control::Model<M>::X X;
    typedef typename control::Model<M>::Y Y;
    typedef typename control::Model<M>::R R;
    typedef typename control::Model<M>::I I;
    typedef typename control::Model<M>::IO IO;
    
    string ftype = "toy"; 
    Options::get("f", ftype); 
    if (ftype == "toy") {
      typedef typename treeler::FGenDepToy<X,R,FIdx> F;
      // typedef typename treeler::NullFScores<X,R,F> S;      
      typedef typename treeler::WFScores<X,R,F> S;

      string cmd="dump";
      Options::get("cmd", cmd);
      if (cmd=="dump") {
	ScriptDump::run<X,Y,R,I,S,F,IO>(model_name);
      }
      else if (cmd=="train") {
	ScriptTrain::run<X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="tpar") {
	test_params<X,Y,R,F,IO>();
      }
      else {
	cerr << name << " : unavailable command for " << ftype << endl;
	exit(0);
      }

    }
    else if (ftype == "mcd") {
      typedef typename treeler::FGenDepMcDonald<X,R,FIdx> F;
      typedef typename treeler::WFScores<X,R,F> S;      
      string cmd="dump";
      Options::get("cmd", cmd);
      if (cmd=="dump") {
	ScriptDump::run<X,Y,R,I,S,F,IO>(model_name);
      }
      else if (cmd=="train") {
	ScriptTrain::run<X,Y,R,I,S,F,IO>(model_name, help);
      }
      else if (cmd=="tpar") {
	test_params<X,Y,R,F,IO>();
      }
      else {
	cerr << name << " : unavailable command for " << ftype << endl;
	exit(0);
      }

    }
    else if (ftype=="v1") {
      typedef typename treeler::FGenDepV1<X,R,FIdx> F;
      typedef typename treeler::NullFScores<X,R,F> S;      
      ScriptDump::run<X,Y,R,I,S,F,IO>(model_name);

    }
    else {
      cerr << name << " : unsupported feature generator \"" << ftype << "\"" << endl;
    }
  }


  template <typename X, typename Y, typename R, typename F, typename IO>
  void ScriptFIdx::test_params() {

    typedef Parameters<typename F::FIdx> WType;     

    typename R::Configuration Rconfig; 
    Factory<R>::configure(Rconfig);

    // input output options
    IO io;
    Factory<IO>::configure(io);
        
    F* fgen = NULL; 
    Factory<F>::create(fgen);

    WType W(fgen->spaces()); 
    W.zero();
        
    DataStream<X,Y,IO> ds(io, std::cin);
    string ifile;
    Options::get("data", ifile); 
    if (!(ifile.empty() or ifile=="-")) {
      ds.set_file(ifile);
    }

    typename DataStream<X,Y,IO>::const_iterator it = ds.begin(), it_end = ds.end();
    for (; it!=it_end; ++it) {
      const X& x  = (*it).x(); 
      const Y& y  = (*it).y(); 
      cout << "+-- EXAMPLE " << x.id() << " -------------------" << endl;
      io.write(cout, x, y); 
      
      typename F::Features* features = NULL; 
      if (fgen!=NULL) {
	features = new typename F::Features(); 
	fgen->features(x, *features); 
      }
      
      auto r = y.begin();
      auto r_end = y.end(); 
      for ( ; r!=r_end; ++r) {
	const auto* f = features->phi(x, *r);
	cout << "ADDING FVEC " << " " << x.id() << " " << *r << " ";
	IOFVec::write(cout, *f);	  
	W.add(f);	
	cout << endl;
	features->discard(f, x, *r);	  
      }      
      delete features; 
    } // for each example            
    // save params
    string wdir; 
    if (Options::get("wdir", wdir)) {
      string wstem = "parameters";
      int wt = 1; 
      Options::get("wstem", wstem); 
      Options::get("wt", wt); 
      W.save(wdir, wt, wstem); 
    }
  } 
  

  template <typename X, typename Y, typename R, typename I, typename S, typename F, typename O, typename IO>  
  void ScriptFIdx::dump_fvec(const string& model_name, bool help) {
    cerr << name() << " : running " << model_name << " with ..." << endl;
    cerr << " X=" << typeid(X).name() << endl
	 << " Y=" << typeid(Y).name() << endl
	 << " R=" << typeid(R).name() << endl
	 << " I=" << typeid(I).name() << endl
      //	 << " S=" << typeid(S).name() << endl
	 << " F=" << typeid(F).name() << endl
      //	 << " O=" << typeid(O).name() << endl
	 << " IO=" << typeid(IO).name() << endl << endl;
    
    if (help) {
      Factory<R>::usage(cerr);
      Factory<I>::usage(cerr);
      // Factory<S>::usage(cerr);
      Factory<F>::usage(cerr);
      // Factory<O>::usage(cerr);
      Factory<IO>::usage(cerr);
      exit(0);
    }

    Configuration config; 
    config.display_x = 0; 
    config.display_y = 0; 
    config.display_xy = 1; 
    Options::get("xy", config.display_xy);
    config.run_parts_y = 0;
    config.run_parts_all = 0;
    config.run_parts_max = 0;
    config.run_parts_cin = 0;
    string tmp;
    if (Options::get("parts", tmp)) {
      if (tmp=="1" or tmp=="y") {
	config.run_parts_y = 1;
      }
      else if (tmp=="all") {
	config.run_parts_all = 1; 
      }
      else if (0 and tmp=="max") {
	config.run_parts_max = 1; 
      }
      else {
	cerr << name() << " : bad parts spec! (--parts=\"" << tmp <<"\")" << endl;
	exit(1);	
      }
    }
    else {
      config.run_parts_y = 1; 
      // cerr << "no parts spec " << endl; 
    }

    config.display_fvec = 1; 
    Options::get("fvec", config.display_fvec); 
    config.display_score = 0;
    Options::get("score", config.display_score); 
    config.display_oracle = 0;
    Options::get("oracle", config.display_oracle); 
    config.display_marginal = 0;
    Options::get("marg", config.display_marginal); 

    config.run_max = 0; 
    Options::get("max", config.run_max); 
    config.display_max = config.run_max!=0;

    typename R::Configuration Rconfig; 
    Factory<R>::configure(Rconfig);

    // input output options
    IO io;
    Factory<IO>::configure(io);
    
    typename I::Configuration Iconfig;
    if (config.run_max) {
      Factory<I>::configure(Iconfig);
    }
    
    F* fgen = NULL; 
    if (config.display_fvec) {
      Factory<F>::create(fgen);
    }
        
    DataStream<X,Y,IO> ds(io, std::cin);
    config.ifile = ""; 
    Options::get("data", config.ifile); 
    config.from_cin = config.ifile.empty() or config.ifile=="-";   
    if (!config.from_cin) {
      ds.set_file(config.ifile);
    }
    

    /* PROCESS EACH EXAMPLE AND DO THE DUMPING */
    typename DataStream<X,Y,IO>::const_iterator it = ds.begin(), it_end = ds.end();
    for (; it!=it_end; ++it) {
      const X& x  = (*it).x(); 
      const Y& y  = (*it).y(); 
      if (config.display_xy) { 
	cout << "+-- EXAMPLE " << x.id() << " -------------------" << endl;
	io.write(cout, x, y); 
      }

      typename F::Features* features = NULL; 
      if (fgen!=NULL) {
	features = new typename F::Features(); 
	fgen->features(x, *features); 
      }

      if (config.run_parts_y) {
	dump_fvec_body<X,Y,R,I,S,F,O,IO>(config, "Y", 
					 x, features, 
					 y.begin(), y.end()); 
      }

      if (config.run_parts_all) {
	dump_fvec_body<X,Y,R,I,S,F,O,IO>(config, "ALL", 
					 x, features, 
					 R::begin(Rconfig, x), R::end()); 	
      }
      
      if (features!=NULL) {
	delete features; 
      }
    } // for each example    
  }

  template <typename X, typename Y, typename R, typename I, typename S, typename F, typename O, typename IO, typename RIT>  
  void ScriptFIdx::dump_fvec_body(const Configuration& config, 
				   const string& label,
				   const X& x, 
				   typename F::Features* features, 
				   RIT r, RIT r_end) {
    while (r != r_end) {      
      cout << "PART " << label << " " << x.id() << " " << *r << endl;
      
      if (config.display_fvec) {
	if (features!=NULL) {
	  const auto* f = features->phi(x, *r);
	  cout << "FVEC " << label << " " << x.id() << " " << *r << " ";
	  IOFVec::write(cout, *f);
	  cout << endl;
	  features->discard(f, x, *r);
	}
      }
      ++r; 
    }
  }

}

