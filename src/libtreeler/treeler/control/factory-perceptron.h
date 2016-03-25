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
 * \file   factory-perceptron.h
 * \brief  Static methods for creating a Perceptron learner
 * \author Xavier Carreras
 */

#ifndef TREELER_FACTORY_PERCEPTRON_H
#define TREELER_FACTORY_PERCEPTRON_H

#include <string>
#include <list>
#include <iostream>
#include <typeinfo>

#include "treeler/algo/perceptron.h"
#include "treeler/algo/sgd-trainer.h"

using namespace std;

namespace treeler {


  namespace control {
    
    /** 
     * \brief A factory for Perceptron
     * \ingroup control 
     */    
    template <typename Model>
    class Factory<Perceptron<Model>> {
    public:
      static void usage(std::ostream& o) {
	o << "Perceptron options:" << endl;
	dump_options(o, " ");
	o << endl;	
      }
            
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--T=<int>       : number of training epochs" << endl;
	o << prefix << "--wdir=<str>    : directory where to write parameter files" << endl;
	o << prefix << "--rand=<string> : randomization type (det,unif,perm)" << endl;
	o << prefix << "--seed=<int>    : random seed" << endl;
	//	o << prefix << "--averaged      : average the parameter vectors" << endl;
	//	o << prefix << "--sparse        : use sparse parameter vectors" << endl;
	//	o << prefix << "--params=<str>  : data type of parameter vectors" << endl;
	if (0) { /* print a list of available data type names */
	  o << prefix << "                  (";
	  vector<string> typenames; // = parameters_util::get_data_names();
	  for(int i = 0; i < (int)typenames.size(); ++i) {
	    o << "\"" << typenames[i] << "\"";
	    if(i < (int)typenames.size() - 1) { o << ", "; }
	  }
	  o << ")" << endl;
	}
	//	o << prefix << "--margin=<real> : margin-sensitivity (default 0.0)" << endl;
	//	o << prefix << "--noparams      : do not write parameter vectors to disk" << endl;
	o << prefix << "--verbose=<int> : verbosity level (through stderr)" << endl;
      }

      static void configure(typename Perceptron<Model>::Configuration& config, Options& options, std::ostream& o = cerr) {
	if (!options.get("T", config.T)) {
	  o << "Factory<Perceptron>: please supply number of epochs with --T=<int>" << endl;
	  exit(0);
	}

	if (!options.get("wdir", config.wdir)) {
	  if (!options.get("dir", config.wdir)) {
	    o << "Factory<Perceptron>: please supply output directory with --wdir=<path> or --dir=<path>" << endl;
	    exit(0);
	  }
	}
	
	config.rand_type = LearnUtils::RAND_UNIFORM;
	string rtype;
	if(options.get("rand", rtype)) {
	  if(strcmp(rtype.c_str(), "det") == 0) {
	    config.rand_type = LearnUtils::DETERMINISTIC;
	  } else if(strcmp(rtype.c_str(), "unif") == 0) {
	    config.rand_type = LearnUtils::RAND_UNIFORM;
	  } else if(strcmp(rtype.c_str(), "perm") == 0) {
	    config.rand_type = LearnUtils::RAND_PERMUTATION;
	  } else {
	    o << "Perceptron: unrecognized randomization type (--rand=\"" << rtype << "\")" << endl;
	    exit(0);
	  }
	}
	
	int tmp;
	config.seed = 1;
	if(options.get("seed", tmp)) {
	  config.seed = tmp;
	}
	
	config.averaged = false;
	if(options.get("averaged", tmp)) {
	  o << "Factory<Perceptron>: --averaged is deprecated, use --wavg instead" << endl; 
	  config.averaged = tmp;
	}
	if(options.get("wavg", tmp)) {
	  config.averaged = tmp;
	}
	
	config.sparse = false;
	if(options.get("sparse", tmp)) {
	  config.sparse = true;
	}
	
	config.paramtype = "double";
	string ptype;
	if(options.get("params", ptype)) {
	  //parameters_util::check_data_name(ptype);
	  config.paramtype = ptype;
	}
	
	if(!options.get("verbose", config.verbose)) {
	  config.verbose = 0;
	}
	
	config.writeparams = true;
	if(options.get("noparams", tmp)) {
	  config.writeparams = false;
	}
	config.writeparams_every = -1;
	options.get("savep", config.writeparams_every);
	
	config.resume_training = false;
	if (options.get("resume", config.resume_initial_parameters)) {
	  config.resume_training = true;
	}
      }
    };





    /** 
     * \brief A factory for SGDTrainer
     * \ingroup control 
     */    
    template <typename Objective, typename Symbols, typename X, typename R, typename I, typename F, typename IO>
    class Factory<SGDTrainer<Objective,Symbols,X,R,I,F,IO>> {
    public:
      typedef SGDTrainer<Objective,Symbols,X,R,I,F,IO> Learner;

      static string name() { return "Factory<SGDTrainer>"; }

      static void usage(std::ostream& o) {
	o << "SGD options:" << endl;
	dump_options(o, " ");
	o << endl;	
      }
            
      static void dump_options(std::ostream& o, const string& prefix="") {
	o << prefix << "--T=<int>         : number of training epochs" << endl;
	o << prefix << "--wdir=<str>      : directory where to write parameter files" << endl;
	o << prefix << "--rand=<string>   : randomization type (det,unif,perm)" << endl;
	o << prefix << "--seed=<int>      : random seed" << endl;
	o << prefix << "--C=<double>      : regularization constant" << endl;
	o << prefix << "--eta=<double>    : initial learning rate" << endl;
	o << prefix << "                    ..." << endl;
	o << prefix << "--anneal=<double> : learning rate annealing" << endl;
	o << prefix << "                    ..." << endl;
	o << prefix << "--decay=<double>  : ???" << endl;
	o << prefix << "--primal=<bool>   : compute primal objective while training" << endl;
	o << prefix << "--verbose=<int> : verbosity level (through stderr)" << endl;
      }

      static void configure(typename Learner::Configuration& config, Options& options, std::ostream& o = cerr) {
	if (!options.get("T", config.T)) {
	  o << name() << ": please supply number of epochs with --T=<int>" << endl;
	  exit(0);
	}

	if (!options.get("wdir", config.wdir)) {
	  if (!options.get("dir", config.wdir)) {
	    o << name() << ": please supply output directory with --wdir=<path> or --dir=<path>" << endl;
	    exit(0);
	  }
	}
	
	config.rand_type = LearnUtils::RAND_UNIFORM;
	string rtype;
	if(options.get("rand", rtype)) {
	  if(strcmp(rtype.c_str(), "det") == 0) {
	    config.rand_type = LearnUtils::DETERMINISTIC;
	  } else if(strcmp(rtype.c_str(), "unif") == 0) {
	    config.rand_type = LearnUtils::RAND_UNIFORM;
	  } else if(strcmp(rtype.c_str(), "perm") == 0) {
	    config.rand_type = LearnUtils::RAND_PERMUTATION;
	  } else {
	    o << name() << ": unrecognized randomization type (--rand=\"" << rtype << "\")" << endl;
	    exit(0);
	  }
	}
	
	int tmp;
	config.seed = 1;
	if(options.get("seed", tmp)) {
	  config.seed = tmp;
	}

	config.sparse = true;
	if(options.get("sparse", tmp)) {
	  config.sparse = tmp;
	}
	
	if (!options.get("C", config.C)) {
	  o << name() << ": please provide --C=<double>" << endl;
	  exit(0);
	}
	if (!options.get("eta", config.lrate)) {
	  o << name() << ": please provide --eta=<double>" << endl;
	  exit(0);
	}
	config.anneal = 0; 
	options.get("anneal", config.anneal); 
	
	config.decay = 0; 
	options.get("decay", config.decay); 
	
	config.doprimal = false; 
	options.get("primal", config.doprimal); 
		
	if(!options.get("verbose", config.verbose)) {
	  config.verbose = 0;
	}
	
	config.writeparams = true;
	if(options.get("noparams", tmp)) {
	  config.writeparams = false;
	}
	config.writeparams_every = -1;
	options.get("savep", config.writeparams_every);
	
	config.resume_training = false;
	if (options.get("resume", config.resume_initial_parameters)) {
	  config.resume_training = true;
	}
      }
    };

        
  } // namespace control
} // namespace treeler


#endif /* TREELER_FACTORY_PERCEPTRON_H */
