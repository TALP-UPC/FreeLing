/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2011   TALP Research Center
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
 * \file   perceptron.tcc
 * \brief  Implementation of template methods of Perceptron
 * \author Xavier Carreras, Terry Koo
 */

#include "treeler/util/timer.h"
#include "treeler/algo/decoder.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* detailed name of the trainer */
#define myname(config)						\
  (config.averaged ? "A" : "")					\
  << RAND_SHORT_DESCRIPTION(config.rand_type)			\
  << "Perc[" <<  Model::name() << "]"

using namespace std;

namespace treeler {

  template <typename Model>
  void Perceptron<Model>::train(Configuration& config, 
				const typename Model::Symbols& symbols,
				typename Model::I& parser,
				typename Model::F& fgen, 
				typename Model::IO& io,
				DataSet<typename Model::X,Label<typename Model::R>>& trdata, 
				DataSet<typename Model::X,typename Model::Y>& valdata) {
  
    cerr << myname(config) << " : training started for max " << config.T << " rounds ... " << endl;

    typedef typename Model::X X; 
    typedef typename Model::Y Y; 
    typedef typename Model::I I; 
    typedef typename Model::R R; 
    typedef typename Model::S S; 
    typedef typename Model::F F; 
    typedef Parameters<typename F::FIdx> WType; 
    WType* W = NULL;

    // create parameter vector (todo: resume learning)
    {
      cerr << myname(config) << " : creating sparse parameter vector with "
	   << fgen.spaces() << " feature spaces..." << flush;
      W = new WType(fgen.spaces());
      W->zero();
      cerr << " ok" << endl;
    }

    // auxiliary scorer
    S scorer(symbols); 
    scorer.set_w(W);
    scorer.set_f(&fgen);
    
    cerr << myname(config) << " : data order is "
	 << RAND_LONG_DESCRIPTION(config.rand_type) << endl;
    
    if(config.rand_type != LearnUtils::DETERMINISTIC) {
      cerr << myname(config) << " : seeding the RNG with value " << config.seed << endl;
      srandom(config.seed);
    }
    
    const int Ntr = trdata.size();
    
    /* log file for primal and #pass information */
    string objfilename =  config.wdir + "/obj.out";
    cerr << myname(config) << " : logging to "<< objfilename << endl;
    ofstream obj_out(objfilename.c_str(), ios::out | ios::trunc);
    assert(obj_out.good());
        
    Status status;
    status.num_processed = 0;
    status.mistakes = 0;
    status.hamming_loss = 0;
    status.cpu_time = 0;
    for(int t = 1; t <= config.T; ++t) {
      cerr << myname(config) << " : iter " << t << " " << flush;
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
	
	/* perform a perceptron update */
	process_example(config, parser, scorer, ex, status);
	++status.num_processed;
	W->avg_tick();
	
	ticker(i + 1, Ntr);
      }
      timer_stop();
      double cpu_time = ((double)(clock() - t0))/(double)CLOCKS_PER_SEC;
      status.cpu_time += cpu_time;
      cerr << " [" << cpu_time << "]" << endl;


      /* save the primals in the model directory */
      W->avg_flush();
      if (config.writeparams) { W->save(config.wdir, t); }
      
      /* #passes is an abstract measure of computation */
      const double num_passes = ((double)status.num_processed)/((double)Ntr);
      /* update the objective log with the current iteration */
      obj_out << scientific << setprecision(16)
	      << t << " " << num_passes << " "
	      << status.cpu_time << " "
	//	      << _W->sqL2() << " "
	      << -1 << " "
	      << status.mistakes << endl;
      /* print a summary */
      cerr << myname(config) << " : finished it = " << t
	   << scientific << setprecision(5)
	   << " ; ops = " << num_passes
	   << " ; cpu = " << status.cpu_time
	//	   << " ; W^2 = " << _W->sqL2()
	   << " ; W^2 = " << "n/a"
	   << " ; mistakes = " << status.mistakes
	   << " ; hamming_loss = " << status.hamming_loss <<  endl;

      /* parse the validation data if provided */
      if (!valdata.empty()) {
	/* NB: the perceptron is not a probabilistic learning
	   approach, so it does not make sense to compute minimum-risk
	   labels */
	if (config.averaged) { W->avg_set(true); }
	char* buf = new char[64];
	sprintf(buf, "%03d", t);	  
	string filename = config.wdir + '/' + "max-labels" + '.' + buf; 
	
	typedef typename Decoder::functor_ostream<X,Y,typename Model::IO> FStream; 
	typedef Decoder::functor_eval<X,Y,typename Model::Eval>  FEval; 
	typedef Decoder::functor_pair<X,Y,FStream,FEval> FFinal;
	
	FStream f1(io, filename);
	FEval  feval;
	FFinal f(f1, feval);
	Decoder::decode<X,Y,I,S>(parser, scorer, valdata.begin(), valdata.end(), f);
	cerr << myname(config) << " : it " << t << " validation results : " << feval.to_string() << endl; 
	if(config.averaged) { W->avg_set(false); }
      }
    }

    /* tidy up */
    scorer.unset_w(); 
    scorer.unset_f(); 
    delete W; W = NULL;
    obj_out.close();
  }


  /* adjust the primals with the gradient on the given example */
  template <typename Model>
  void Perceptron<Model>::process_example(const Configuration& config, 
					  typename Model::I& parser, 
					  typename Model::S& scorer, 
					  const Example<typename Model::X,Label<typename Model::R>>& ex,
					  Status& status) {

    typedef typename Model::X X; 
    typedef typename Model::R R; 
    typedef typename Model::S S; 
    typedef typename Model::F F; 
    
    const X& x  = ex.x();
    const Label<R>& y = ex.y();

    const clock_t t0 = clock();
    if (config.verbose) {
      cerr << myname(config) << " e " << x.id() << " (size " << x.size() << ") " << flush;
    }

    /* argmax solution */
    typename S::Scores scores;
    scorer.scores(ex, scores);          
    if (config.verbose) {
      cerr << " ...s" << flush;
    }

    Label<R> yhat;
    parser.argmax(x, scores, yhat);
    if (config.verbose) {
      cerr << " ...i" << flush;
    }

    /* update w */

    typename F::Features& features = scores.f(); 
    typename S::W& w = scorer.w();

    bool mistake = false;
    // PERFORM A SET DIFFERENCE BETWEEN y AND yhat 
    Label<R> missed, wrong;
    typename Label<R>::const_iterator it_y = y.begin();
    typename Label<R>::const_iterator it_y_end = y.end();
    typename Label<R>::const_iterator it_yhat = yhat.begin();
    typename Label<R>::const_iterator it_yhat_end = yhat.end();
    while ((it_y!=it_y_end) and (it_yhat!=it_yhat_end)) {
      if (*it_y == *it_yhat) {
	++it_y;
	++it_yhat;
      }
      else if (*it_y < *it_yhat) {
	missed.push_back(*it_y);
	++it_y;
      }
      else {
	wrong.push_back(*it_yhat);
	++it_yhat;
      }
    }
    while (it_y != it_y_end) {
      missed.push_back(*it_y);
      ++it_y;
    }
    while (it_yhat != it_yhat_end) {
      wrong.push_back(*it_yhat);
      ++it_yhat;
    }

    if (config.verbose>1) {
      cerr << endl;
      //IO io; 
      //io.write(cerr, x, y, yhat); 
      cerr << "Y[" << y.size() << "]={";
      std::copy(y.begin(), y.end(), ostream_iterator<R>(cerr, ","));
      cerr << "}" << endl << "YHAT[" << yhat.size() << "]={";
      std::copy(yhat.begin(), yhat.end(), ostream_iterator<R>(cerr, ","));
      cerr << "}" << endl << "MISSD[" << missed.size() << "]={";
      std::copy(missed.begin(), missed.end(), ostream_iterator<R>(cerr, ","));
      cerr << "}" << endl << "WRONG[" << wrong.size() << "]={";
      std::copy(wrong.begin(), wrong.end(), ostream_iterator<R>(cerr, ","));
      cerr << "}" << endl;
    }

    typedef FeatureVector<typename F::FIdx> FVec;

    for (it_y = missed.begin(); it_y != missed.end(); ++it_y) {
      const FVec* const f_r = features.phi(*it_y);
      w.add(f_r);
      features.discard(f_r);
      mistake = true;
      ++status.hamming_loss;
    }
    for (it_y = wrong.begin(); it_y != wrong.end(); ++it_y) {
      const FVec* const f_r = features.phi(*it_y);
      w.sub(f_r);
      features.discard(f_r);
      mistake = true;
      ++status.hamming_loss;
    }

    if(mistake) { ++status.mistakes; }

    if (config.verbose)  cerr << "...u" << flush;

    if (config.verbose) {
      double cpu_time = ((double)(clock() - t0))/(double)CLOCKS_PER_SEC;
      cerr << fixed << setprecision(2) << " [" << cpu_time << "]" << endl;
    }
  }


} // namespace treeler

#undef myname
