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
 * \file   sgd-trainer.h
 * \brief  Declaration of an SGD Trainer
 * \author Terry Koo, adapted by Xavier Carreras for Treeler
 */

#ifndef TREELER_SGDTRAINER_H
#define TREELER_SGDTRAINER_H

// #include "Trainer.h"
// #include "Example.h"
// #include "Parameters.h"


#include "treeler/base/sparse-parameters.h"
#include "treeler/algo/learn-utils.h"

namespace treeler {

  /*
   * \brief Trains a weight vector by stochastic gradient descent in
   *        the primal
   * \author Terry Koo and Xavier Carreras
   *
   *  SGDTrainer depends on an objective function Obj
   *
   *  Obj is expected to provide the following methods
   *
   *   Obj::objective(example ex, inference i, scores s)
   *   Obj::gradient(parameters g, double scale, example ex,
   *                 inference i, scores s, features f)
   *     These return the example-wise contribution to the primal
   *     objective and the primal gradient
   */
  template <typename Objective, typename Symbols, typename X, typename R, typename Inference, typename FGen, typename IO>
  class SGDTrainer {
  public:

    typedef typename FGen::FIdx FIdx;
    typedef SparseParameters<FIdx> WType; 
    typedef WFScorer<Symbols,X,R,FGen,SparseParameters> Scorer; 

    struct Configuration {
      /* max number of training epochs */
      int T;
      /* directory where to save parameter files to */ 
      string wdir; 

      /* style of randomization */
      LearnUtils::randomization_t rand_type;
      /* seed used for srandom() */
      int seed;

      /* SGD parameters */
      /* regularization constant */
      double C; 
      /* global initial learning rate (eta0) */
      double lrate; 
      /* whether the learning rate is reduced after each example */
      double anneal;
      /* squared radius of an L2-ball constraint, or negative if the
	 constraint is inactive */      
      double projl2; 
      /* the decay factor in the adaptive learning rate adjustment */
      double decay;
      
      /* whether to compute the primal objective on each iteration */
      bool doprimal;
      
      /* whether the primal parameters are sparse */
      bool sparse;

      /* whether to write parameter vectors to disk */
      bool writeparams;
      /* whether to write parameter vectors to disk every time this number of examples are processed */      int writeparams_every;
      
      /* whether to resume training, initializing the parameters from some given file */
      bool resume_training;
      string resume_initial_parameters;
      
      /* verbosity level */
      int verbose;

    };

    struct Status {
      int num_processed, mistakes;
      double hamming_loss;
      int cpu_time;

    };


  public:

    static string name(Configuration &config) {
      return "SGDTrainer<" + Objective::name() + ">";
    }
    
    template <typename Y>
    static void train(Configuration& config, 
		      const Symbols& symbols, 
		      Inference& parser,
		      FGen& fgen, 
		      IO& io,
		      DataSet<X,Label<R>>& trdata, 
		      DataSet<X,Y>& valdata);

    /* examine this example */
    static void process_example(const Configuration& config, 
				Inference& parser, 
				Scorer& scorer, 
				const Example<X,Label<R>>& ex,
				Status& status);

  };
  


  /*
   *
   *
   */

  template <typename Objective, typename Symbols, typename X, typename R, typename Inference, typename FGen, typename IO>
  template <typename Y>
  void SGDTrainer<Objective,Symbols,X,R,Inference,FGen,IO>::train(Configuration& config, 
								  const Symbols& symbols, 
								  Inference& parser,
								  FGen& fgen, 
								  IO& io, 
								  DataSet<X,Label<R>>& trdata, 
								  DataSet<X,Y>& valdata) {
    ostream& log = cerr;
    log << name(config) << " : training started for max " << config.T
	<< " rounds ... " << endl;
    
    ////////////////////////////////////////
    // Model parameters
    
    WType* w = NULL;
    // create parameter vector (todo: resume learning)
    {
      log << name(config) << " : creating sparse parameter vector with "
	  << fgen.spaces() << " feature spaces..." << flush;
      w = new WType(fgen.spaces());
      w->zero();
      log << " ok" << endl;
    }
    
    // auxiliary scorer
    Scorer scorer(symbols); 
    scorer.set_w(w);
    scorer.set_f(&fgen);
    
    ////////////////////////////////////////
    // Parameters of SGD
    log << name(config) << " : provided eta0 = " << config.lrate;
    /* scale the learning rate by 1/C so that the learning rate has a
       clear interpretation as the downweighting of the current
       primals; i.e., the update w' = w - C*lrate*w + lrate*f'(w)
       results in w' = (1 - lrate)*w + lrate*f'(w) */
    config.lrate = config.lrate/config.C;
    log << " => internal eta0 = " << config.lrate << endl;

    /* save the initial value */
    const double eta0 = config.lrate;

    const int Ntr = trdata.size();

    /* special case for lecun-style learning rate */
    if(config.anneal == 0) { config.anneal = 1.0/(double)Ntr; }

    if(config.anneal < 0) {
      log << name(config) << " : learning rate is constant" << endl;
    } else {
      log << name(config) << " : learning rate is annealed using "
	   << config.anneal << endl;
      assert(config.anneal > 0);
    }

    if(config.projl2 < 0) {
      log << name(config) << " : no L2 projection" << endl;
    } else {
      log << name(config) << " : L2-projection with R = "
	   << sqrt(config.projl2) << endl;
    }


    // training data

    log << name(config) << " : data order is "
	 << RAND_LONG_DESCRIPTION(config.rand_type) << endl;
    
    if (config.rand_type != LearnUtils::DETERMINISTIC) {
      cerr << name(config) << " : seeding the RNG with value " << config.seed << endl;
      srandom(config.seed);
    }
    
    
    /* log file for primal and #pass information */
    string objfilename =  config.wdir + "/obj.out";
    cerr << name(config) << " : logging to "<< objfilename << endl;
    ofstream obj_out(objfilename.c_str(), ios::out | ios::trunc);
    assert(obj_out.good());
        
    Status status;
    status.num_processed = 0;
    status.mistakes = 0;
    status.hamming_loss = 0;
    status.cpu_time = 0;
    for(int t = 1; t <= config.T; ++t) {
      cerr << name(config) << " : iter " << t << " " << flush;
      if (config.verbose) cerr << endl;
      timer_start();
      const clock_t t0 = clock();
      for(int i = 0; i < Ntr; ++i) {
	/* first, pick an example according to the desired scheme.
	   NB: the first round of rand-uniform will be a permutation,
	   to ensure that all examples are visited. */
	const LearnUtils::randomization_t rt = (t == 1 && config.rand_type == LearnUtils::RAND_UNIFORM && !config.resume_training ?
						LearnUtils::RAND_PERMUTATION : config.rand_type);
	const Example<X,Label<R>>& ex = LearnUtils::randselect<X,Label<R>>(rt, i, trdata);

	/* adjust the learning rate if necessary */
	if(config.anneal > 0) {
	  /* formula is eta = eta0/(1 + A*t) */
	  config.lrate = eta0/(1.0 + config.anneal*((double)status.num_processed));
	}

	/* perform a stochastic update */
	process_example(config, parser, scorer, ex, status);

	/* optionally project the primals onto an L2-ball */
	if(config.projl2 > 0) {
	  const double sqL2 = w->sqL2();
	  if(sqL2 > config.projl2) {
	    w->scale(sqrt(config.projl2/sqL2));
	    cerr << "p" << flush;
	  }
	}
	
	++status.num_processed;
	ticker(i + 1, Ntr);
      }
      ticker(Ntr, Ntr);
      timer_stop();
      status.cpu_time += ((double)(clock() - t0))/(double)CLOCKS_PER_SEC;

      /* save the primals in the model directory */
      //w->avg_flush();
      if (config.writeparams) { w->save(config.wdir, t); }
      
      /* #passes is an abstract measure of computation */
      const double num_passes = ((double)status.num_processed)/((double)Ntr);
      /* update the objective log with the current iteration */
      obj_out << scientific << setprecision(16)
	      << t << " " << num_passes << " "
	      << status.cpu_time << " "
	      << w->sqL2() << " "
	      << -1 << " "
	      << status.mistakes << endl;

      /* print a summary */
      cerr << name(config) << " : finished it = " << t
	   << scientific << setprecision(5)
	   << " ; ops = " << num_passes
	   << " ; cpu = " << status.cpu_time
	   << " ; W^2 = " << w->sqL2()
	   << " ; W^2 = " << "n/a"
	   << " ; mistakes = " << status.mistakes
	   << " ; hamming_loss = " << status.hamming_loss <<  endl;
      
      /* parse the validation data if provided */
      if(!valdata.empty()) {
      }
    }

    /* tidy up */
    scorer.unset_w(); 
    scorer.unset_f(); 
    delete w; 
    w = NULL;
    obj_out.close();
  } // train
 


  /* adjust the primals with the gradient on the given sentence */
  template<typename Objective, typename Symbols, typename X, typename R, typename Inference,typename FGen, typename IO>
  void SGDTrainer<Objective,Symbols,X,R,Inference,FGen,IO>::process_example(const Configuration& config, 
									    Inference& parser, 
									    Scorer& scorer, 
									    const Example<X,Label<R>>& ex,
									    Status& status) {
    const X& x = ex.x();
    typename Scorer::Scores s; 
    scorer.scores(x,s);

    // regularizer gradient is -(_C*V)
    scorer.w().scale(1.0 - (config.lrate * config.C));

    // write the gradient update directly onto the primals
    double obj = Objective::gradient(s.w(), config.lrate, ex, parser, s, s.f()); 

    cerr << "  example " << x.id() << " objective " << obj << endl; 

    // const int R = x->nparts();   
    // /* get the inner products */
    // const struct fvec* const F = _fg->phi(x);
    // double* const S = (double*)malloc(R*sizeof(double));
    // _W->dot(S, F, R);


    // _W->scale(1.0 - (_lrate * _C));
    // /* write the gradient update directly onto the primals */
    // Obj::exgrad(*_W, _lrate, ex, S, F, *_M);

    // _fg->discard(F, x);
    // free(S);
  }

 
}

// #include "treeler/algo/sgd-trainer.tcc"

#endif /* TREELER_SGDTRAINER_H */
