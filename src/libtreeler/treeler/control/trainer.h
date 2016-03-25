/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2012   TALP Research Center
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
 * \file   script-train.h
 * \brief  A script for training the parameters of a model
 * \author Xavier Carreras
 */

#ifndef TREELER_TRAINER
#define TREELER_TRAINER

#include <string>
#include "treeler/control/control.h"
#include "treeler/control/factory-perceptron.h"
#include "treeler/algo/perceptron.h"
#include "treeler/algo/sgd-trainer.h"
#include "treeler/algo/objectives.h"


namespace treeler {

  namespace control {

    /** 
     *  \brief A script for training models
     *  \ingroup control
     */
    class Trainer {
    private:
      typedef ModelSelector<Trainer, TAG1, DEP1, DEP2> GenericModel; 

    public:
      Trainer()
      {} 

      ~Trainer()
      {} 
      
      static const std::string name() { return "Trainer"; }
      
      void usage(Options& options);       
      void run(Options& options);
      
      template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
      static void usage(Options& options, const string& model_name, ostream& log, const string& prefix); 
      
      template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>
      static void run(Options& options, const std::string& model_name);

      template <typename Learner, typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>
      static void run_learner(Options& options, const std::string& model_name);

      template <typename X, typename Y, typename IO>
      static void read_dataset(const string& file, const IO& io, DataSet<X,Y>& d, ostream& log, bool quiet);

      template <typename Symbols, typename X, typename Y, typename R, typename I, typename IO>
      static void read_dataset(istream& input, const Symbols& sym, const I& parser, const IO& io, DataSet<X,Label<R>>& d, bool quiet=false);

      template <typename Symbols, typename X, typename Y, typename R, typename I, typename IO>
      static void read_dataset(const string& file, const Symbols& sym, const I& parser, const IO& io, DataSet<X,Label<R>>& d, bool quiet=false);

    };
    
     
    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void Trainer::usage(Options& options, const string& model_name, ostream& log, const string& prefix) {
      string opt = "perc";
      options.get("opt", opt); 
      if (opt=="perc") {
	typedef Perceptron<Symbols,X,Y,R,I,S,F,IO> Learner;
	Factory<Learner>::usage(log);
      }      
      else if (opt=="sgd") {
	typedef SGDTrainer<LogLinearObjective,Symbols,X,R,I,F,IO> Learner;
	Factory<Learner>::usage(log);
      }      
      else {
	cerr << name() << " : sorry, optimizer \"" << opt << "\" not supported." << endl;
	exit(0); 
      }

      Factory<R>::usage(log);
      Factory<I>::usage(log);
      Factory<F>::usage(log);
      Factory<S>::usage(log);
      Factory<IO>::usage(log);
      exit(0);
    }
    

    /**
     * Implementation of the training script
     */ 
    template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void Trainer::run(Options& options, const string& model_name) {
      // select type of learner, only one available at this point!
      string opt = "perc";
      options.get("opt", opt); 
      cerr << name() << " :  training " << model_name << " with " << opt << endl; 
      if (opt=="perc") {
	typedef Perceptron<Symbols,X,Y,R,I,S,F,IO> Learner;
	run_learner<Learner,Symbols,X,Y,R,I,S,F,IO>(options, model_name);
      }      
      else if (opt=="sgd") {
	typedef SGDTrainer<LogLinearObjective,Symbols,X,R,I,F,IO> Learner;
	run_learner<Learner,Symbols,X,Y,R,I,S,F,IO>(options, model_name);
      }      
      else {
	cerr << name() << " : sorry, optimizer \"" << opt << "\" not supported." << endl;
	exit(0); 
      }
    }

    template <typename Learner, typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>
    void Trainer::run_learner(Options& options, const string& model_name) {

      ostream& log = cerr; 

      log << name() << " : initializing ... " << endl; 

      Symbols symbols;
      Factory<Symbols>::configure(symbols, options);      

      // create the configuration for the learner
      typename Learner::Configuration config;
      Factory<Learner>::configure(config, options);
      
      typename R::Configuration Rconfig; 
      Factory<R>::configure(Rconfig, options);

      // input output options
      IO io(symbols);
      Factory<IO>::configure(io, options);
      
      I parser(symbols);
      Factory<I>::configure(parser, options);
            
      F fgen(symbols);
      Factory<F>::configure(fgen, options);
      
      DataSet<X,Label<R>> train_ds;
      string train_file, val_file; 
      if (!options.get("train", train_file)) {
	if (!options.get("data", train_file)) {
	  cerr << name() << " :  please supply training data file with --train=<path>" << endl; 
	  exit(0); 
	}
      } 
      
      int verbose = 0; 
      options.get("verbose", verbose); 
      
      log << name() << " : loading training data ..." << flush; 
      read_dataset<Symbols,X,Y,R,I,IO>(train_file, symbols, parser, io, train_ds, verbose==0);
      log << " " << train_ds.size() << " examples read" << endl; 
      
      
      DataSet<X,Y> val_ds;
      if (options.get("val", val_file)) {
	log << name() << " : loading validation data from \"" << val_file << "\"" << flush; 
	read_dataset(val_file, io, val_ds, log, verbose==0); 
	log << " " << val_ds.size() << " examples read" << endl; 
      }

      log << name() << " : calling learner" << endl; 
      Learner::train(config, symbols, parser, fgen, io, train_ds, val_ds);     
    }



    template <typename X, typename Y, typename IO>
    void Trainer::read_dataset(const string& file, const IO& io, DataSet<X,Y>& d, ostream& log, bool quiet) {
      if (!quiet) {
	log << "Trainer : reading "  << (file=="-" ? "STDIN" : file) << " " << flush;
      }
      istream* in = &std::cin; 
      ifstream fin; 
      if (file != "-") {
	fin.open(file.c_str(), ifstream::in);
	if (!fin.good()) {
	  log << endl << "Trainer : error opening file " << file << endl; 
	  exit(1);
	}
	in = &fin;
      }     
      io.read(*in, d, quiet);
      if (file!="-") {
	fin.close();
      }
      if (!quiet) {
	log << " " << d.size() << " examples" << endl;
      }
    }

     
    template <typename Symbols, typename X, typename Y, typename R, typename I, typename IO>
    void Trainer::read_dataset(const string& file, const Symbols& sym, const I& parser, const IO& io, DataSet<X,Label<R>>& d, bool quiet) {
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
      read_dataset<Symbols,X,Y,R,I,IO>(*in, sym, parser, io, d, quiet);
      if (file!="-") {
	fin.close();
      }
      if (!quiet) {
      cerr << " " << d.size() << " examples" << endl;
      }
    }
    
    
    template <typename Symbols, typename X, typename Y, typename R, typename I, typename IO>
    void Trainer::read_dataset(istream& input, const Symbols& sym, const I& parser, const IO& io, DataSet<X,Label<R>>& d, bool quiet) {
      int id = 0;
      assert(d.size() == 0);    
      X* x = NULL;
      Y* y = NULL;
      while(io.read(input, x, y)) {
	x->set_id(id++); // incrementing id
	Label<R>* yr = new Label<R>; 
	parser.decompose(*x,*y,*yr);
	d.emplace_back(new Example<X,Label<R>>(x,yr));	
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
