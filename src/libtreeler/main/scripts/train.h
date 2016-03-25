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
 * \file   script-train.h
 * \brief  A script for training the parameters of a model
 * \author Xavier Carreras
 */

#ifndef TREELER_SCRIPT_TRAIN
#define TREELER_SCRIPT_TRAIN

#include <string>
#include "scripts/script.h"

#include "treeler/control/control.h"
#include "treeler/control/factory-perceptron.h"
#include "treeler/algo/perceptron.h"


namespace treeler {

  namespace control {

    /** 
     *  \brief A script for training models
     *  \ingroup control
     */
    class ScriptTrain: public Script {
    public:
      ScriptTrain()
	: Script("Trainer")
      {} 

      ~ScriptTrain()
      {} 
      
      static const std::string name() { return "train"; }
      
      void usage(Options& options);       
      void run(Options& options);
      
      template <typename Model>
      static void usage(Options& options, ostream& log, const string& prefix); 
      
      template <typename Model>
      static void run(Options& options, ostream& log);


      template <typename X, typename Y, typename IO>
      static void read_dataset(const string& file, const IO& io, DataSet<X,Y>& d, bool quiet=false);

      template <typename Model>
      static void read_dataset(const string& input_file, 
			       const typename Model::Symbols& sym, const typename Model::I& parser, 
			       const typename Model::IO& io, 
			       DataSet<typename Model::X,Label<typename Model::R>>& d, bool quiet);
      

      template <typename Model>
      static void read_dataset(istream& input, 
			       const typename Model::Symbols& sym, const typename Model::I& parser, 
			       const typename Model::IO& io, 
			       DataSet<typename Model::X,Label<typename Model::R>>& d, bool quiet);

    };
    
     
    template <typename Model>
    void ScriptTrain::usage(Options& options, ostream& log, const string& prefix) {
      typedef Perceptron<Model> Learner;
      Factory<Learner>::usage(log);
      Factory<typename Model::R>::usage(log);
      Factory<typename Model::I>::usage(log);
      Factory<typename Model::F>::usage(log);
      Factory<typename Model::S>::usage(log);
      Factory<typename Model::IO>::usage(log);
      exit(0);
    }
    

    /**
     * Implementation of the training script
     */ 
    template <typename Model>
    void ScriptTrain::run(Options& options, ostream& log) {
      
      typedef Perceptron<Model> Learner;
            
      // select type of learner, only one available at this point!
      string opt = "perc";
      options.get("opt", opt); 
      if (opt!="perc") {
	log << name() << " : sorry, optimizer " << opt << " not supported." << endl;
	exit(0); 
      }

      typename Model::Symbols symbols;
      Factory<typename Model::Symbols>::configure(symbols, options);      

      // create the configuration for the learner
      typename Learner::Configuration config;
      Factory<Learner>::configure(config, options);
      
      // input output options
      typename Model::IO io(symbols);
      Factory<typename Model::IO>::configure(io, options);
      
      typename Model::I parser(symbols);
      Factory<typename Model::I>::configure(parser, options);
            
      typename Model::F fgen(symbols);
      Factory<typename Model::F>::configure(fgen, options);
      
      DataSet<typename Model::X,Label<typename Model::R>> train_ds;
      string train_file, val_file; 
      if (!options.get("train", train_file)) {
	if (!options.get("data", train_file)) {
	  log << name() << " :  please supply training data file with --train=<path>" << endl; 
	  exit(0); 
	}
      } 
      
      int verbose = 0; 
      options.get("verbose", verbose); 
      
      log << name() << " : loading training data from \"" << train_file << "\"" << flush; 
      read_dataset<Model>(train_file, symbols, parser, io, train_ds, verbose==0);
      cerr << " " << train_ds.size() << " examples read" << endl; 
      
      DataSet<typename Model::X,typename Model::Y> val_ds;
      if (options.get("val", val_file)) {
	log << name() << " : loading validation data from \"" << val_file << "\"" << flush; 
	read_dataset(val_file, io, val_ds, verbose==0); 
	log << " " << val_ds.size() << " examples read" << endl; 
      }
      
      Learner::train(config, symbols, parser, fgen, io, train_ds, val_ds);     
    }


    template <typename X, typename Y, typename IO>
    void ScriptTrain::read_dataset(const string& file, const IO& io, DataSet<X,Y>& d, bool quiet) {
      if (!quiet) {
	cerr << "Trainer : reading "  << (file=="-" ? "STDIN" : file) << " " << flush;
      }
      istream* in = &std::cin; 
      ifstream fin; 
      if (file != "-") {
	fin.open(file.c_str(), ifstream::in);
	if (!fin.good()) {
	  cerr << endl << "Trainer : error opening file " << file << endl; 
	  exit(1);
	}
	in = &fin;
      }     
      io.read(*in, d, quiet);
      if (file!="-") {
	fin.close();
      }
      if (!quiet) {
	cerr << " " << d.size() << " examples" << endl;
      }
    }

     
    template <typename Model>
    void ScriptTrain::read_dataset(const string& file, 
				   const typename Model::Symbols& sym, const typename Model::I& parser, 
				   const typename Model::IO& io, 
				   DataSet<typename Model::X,Label<typename Model::R>>& d, 
				   bool quiet) {
      if (!quiet) {
	cerr << "Trainer : reading "  << (file=="-" ? "STDIN" : file) << " " << flush;
      }
      istream* in = &std::cin; 
      ifstream fin; 
      if (file != "-") {
	fin.open(file.c_str(), ifstream::in);
	if (!fin.good()) {
	  cerr << endl << "Trainer : error opening file " << file << endl; 
	  exit(1);
	}
	in = &fin;
      }     
      read_dataset<Model>(*in, sym, parser, io, d, quiet);
      if (file!="-") {
	fin.close();
      }
      if (!quiet) {
	cerr << " " << d.size() << " examples" << endl;
      }
    }
    
    
    template <typename Model>
    void ScriptTrain::read_dataset(istream& input, 
				   const typename Model::Symbols& sym, const typename Model::I& parser, 
				   const typename Model::IO& io, 
				   DataSet<typename Model::X,Label<typename Model::R>>& d, bool quiet) {
      int id = 0;
      assert(d.size() == 0);    
      typename Model::X* x = NULL;
      typename Model::Y* y = NULL;
      while(io.read(input, x, y)) {
	x->set_id(id++); // incrementing id
	Label<typename Model::R>* yr = new Label<typename Model::R>; 
	parser.decompose(*x,*y,*yr);
	d.emplace_back(new Example<typename Model::X,Label<typename Model::R>>(x,yr));	
	delete y;
	x = NULL;
	y = NULL;
	if(!quiet and (id & 0x7ff) == 0) {
	  cerr << "(" << (double)id/1000.0 << "k)" << flush;
	} else if((id & 0xff) == 0) {
	  cerr << "." << flush;
	}
      }    
    }
  }
}

#endif
