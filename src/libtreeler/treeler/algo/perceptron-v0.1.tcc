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
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
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
#include "treeler/util/options.h"
#include "treeler/learn/decoder.h"

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
#define myname							\
  (_averaged ? "A" : "")					\
  << RAND_SHORT_DESCRIPTION(_rand_type)				\
  << "Perc[" <<  model.name() << "]"

using namespace std;

namespace treeler {

  template <typename X, typename R>
  void Perceptron01<X,R>::usage(const char* const mesg) {
    cerr << "Perceptron-v0.1 options:" << endl;
    cerr << " --rand=<string> : randomization type (det,unif,perm)" << endl;
    cerr << " --seed=<int>    : random seed" << endl;
    cerr << " --averaged      : average the parameter vectors" << endl;
    cerr << " --sparse        : use sparse parameter vectors" << endl;
    cerr << " --params=<str>  : data type of parameter vectors" << endl;
    { /* print a list of available data type names */
      cerr << "                   (";
      vector<string> typenames; // = parameters_util::get_data_names();
      for(int i = 0; i < (int)typenames.size(); ++i) {
	cerr << "\"" << typenames[i] << "\"";
	if(i < (int)typenames.size() - 1) { cerr << ", "; }
      }
      cerr << ")" << endl;
    }
    cerr << " --margin=<real> : margin-sensitivity (default 0.0)" << endl;
    cerr << " --noparams      : do not write parameter vectors to disk" << endl;
    cerr << " --v=<int>       : verbosity level (through stderr)" << endl;
    cerr << endl;
    cerr << mesg << endl;
  }

  template <typename X, typename R>
  void Perceptron01<X,R>::process_options() {
    _rand_type = LearnUtils::RAND_UNIFORM;
     string rtype;
     if(Options::get("rand", rtype)) {
       if(strcmp(rtype.c_str(), "det") == 0) {
	 _rand_type = LearnUtils::DETERMINISTIC;
       } else if(strcmp(rtype.c_str(), "unif") == 0) {
	 _rand_type = LearnUtils::RAND_UNIFORM;
       } else if(strcmp(rtype.c_str(), "perm") == 0) {
	 _rand_type = LearnUtils::RAND_PERMUTATION;
       } else {
	 usage("unrecognized randomization type"); exit(1);
       }
     }

    int tmp;
    _seed = 1;
    if(Options::get("seed", tmp)) {
      _seed = tmp;
    }

    _averaged = false;
    if(Options::get("averaged", tmp)) {
      _averaged = true;
    }

    _sparse = false;
    if(Options::get("sparse", tmp)) {
      _sparse = true;
    }

    _paramtype = "double";
    string ptype;
    if(Options::get("params", ptype)) {
      //parameters_util::check_data_name(ptype);
      _paramtype = ptype;
    }

    if(!Options::get("verbose", _verbose)) {
      _verbose = 0;
    }

    _writeparams = true;
    if(Options::get("noparams", tmp)) {
      _writeparams = false;
    }
    _writeparams_every = -1;
    Options::get("savep", _writeparams_every);

    _resume_training = false;
    if (Options::get("resume", _resume_initial_parameters)) {
      _resume_training = true;
    }

  }


  template <typename X, typename R>
  void Perceptron01<X,R>::train(StructuredModel<X,R>& model,
			      DataSet<X,R>& trdata,
			      DataSet<X,R>& valdata,
			      const int T) {

    cerr << myname << " : training started for max " << T
	 << " rounds ... " << endl;

    if (_resume_training) {
      // cerr << myname << " : resuming training : reading parameters from "
      // 	   << _resume_initial_parameters << " ... " << flush;
      // _W = new Parameters(0);
      // _W->load(_resume_initial_parameters.c_str());
      // assert(_W->spaces() == gtrainer<P>::_fg->spaces());
      // cerr << " ok" << endl;
    }
    else {
      cerr << myname << " : creating sparse parameter vector with "
	   << model.fgen().spaces() << " feature spaces..." << flush;
      _W = new Parameters(model.fgen().spaces());
      _W->zero();
      cerr << " ok" << endl;
    }

    cerr << myname << " : data order is "
	 << RAND_LONG_DESCRIPTION(_rand_type) << endl;

    if(_rand_type != LearnUtils::DETERMINISTIC) {
      /* only report this for randomized EG */
      cerr << myname << " : seeding the RNG with value " << _seed << endl;
      srandom(_seed);
    }

    const int Ntr = trdata.size();

    /* log file for primal and #pass information */
    string objfilename =  model.dir() + "/obj.out";
    cerr << myname << " : logging to "<< objfilename << endl;
    ofstream obj_out(objfilename.c_str(), ios::out | ios::trunc);
    assert(obj_out.good());

//     if(_averaged) {
//       cerr << myname << " : enabling parameter averaging...";
//       //!!!!      _W->avg_enable();
//       cerr << " done" << endl;
//     }

    _num_processed = 0;
    _mistakes = 0;
    _hamming_loss = 0;
    double cpu_time = 0;
    for(int t = 1; t <= T; ++t) {
      cerr << myname << " : iter " << t << " " << flush;
      if (_verbose) cerr << endl;
      timer_start();
      const clock_t t0 = clock();
      for(int i = 0; i < Ntr; ++i) {
	/* first, pick an example according to the desired scheme.
	   NB: the first round of rand-uniform will be a permutation,
	   to ensure that all examples are visited. */
	const LearnUtils::randomization_t rt = (t == 1 && _rand_type == LearnUtils::RAND_UNIFORM && !_resume_training ?
						LearnUtils::RAND_PERMUTATION : _rand_type);
	const Example<X,R>* const ex = LearnUtils::randselect<X,R>(rt, i, trdata);

	/* perform a perceptron update */
	process_example(model, *ex);
	++_num_processed;
	_W->avg_tick();

// 	if (_writeparams_every>0 and ((i+1)%_writeparams_every==0)) {
// 	  ostringstream name;
// 	  name << "parameters." << i+1;
// 	  const parameters* const V = (_averaged ? _W->avg_params() : _W);
// 	  V->save( gtrainer<P>::_dir, t, name.str());
// 	}

	//ticker(i + 1, Ntr);
      }
      //ticker(Ntr, Ntr);
      timer_stop();
      cpu_time += ((double)(clock() - t0))/(double)CLOCKS_PER_SEC;
      //cerr << " [" << timestr(timediff) << "]" << endl;


      /* save the primals in the model directory */
      _W->avg_flush();
      if(_writeparams) { _W->save(model.dir(), t); }

      /* #passes is an abstract measure of computation */
      const double num_passes = ((double)_num_processed)/((double)Ntr);
      /* update the objective log with the current iteration */
      obj_out << scientific << setprecision(16)
	      << t << " " << num_passes << " "
	      << cpu_time << " "
	//	      << _W->sqL2() << " "
	      << -1 << " "
	      << _mistakes << endl;
      /* print a summary */
      cerr << myname << " : finished it = " << t
	   << scientific << setprecision(5)
	   << " ; ops = " << num_passes
	   << " ; cpu = " << cpu_time
	//	   << " ; W^2 = " << _W->sqL2()
	   << " ; W^2 = " << "n/a"
	   << " ; mistakes = " << _mistakes
	   << " ; hamming_loss = " << _hamming_loss <<  endl;

      /* parse the validation data if provided */
      if(!valdata.empty()) {
	/* NB: the perceptron is not a probabilistic learning
	   approach, so it does not make sense to compute minimum-risk
	   labels */
	 if(_averaged) { _W->avg_set(true); }
	 char* buf = new char[64];
	 sprintf(buf, "%03d", t);	  
	 string filename = model.dir() + '/' + "max-labels" + '.' + buf; 
	 Decoder::functor_ostream<X,R> f(filename);
	 Decoder::decode(model, valdata, *_W, f);
	 if(_averaged) { _W->avg_set(false); }
      }
    }

    /* tidy up */
    delete _W; _W = NULL;
    obj_out.close();
  }


  /* adjust the primals with the gradient on the given example */
  template <typename X, typename R>
  void Perceptron01<X,R>::process_example(StructuredModel<X,R>& model,
					const Example<X,R>& ex) {

    const X& x  = static_cast<const X&>(*ex.x());
    Label<R> y = *(ex.y());

    // sort(y.begin(), y.end());

    const clock_t t0 = clock();
    if (_verbose) {
      cerr << myname << "e " << x.id() << " (size " << x.size() << ") " << flush;
    }

    /* argmax solution */
    Label<R> yhat;

    model.fgen().phi_start_pattern(x);
    model.argmax(x, *_W, yhat);
    // sort(yhat.begin(), yhat.end());

    bool mistake = false;
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

    if (_verbose>1) {
      cerr << "\nY[" << y.size() << "]={";
      std::copy(y.begin(), y.end(), ostream_iterator<R>(cerr, ","));
      cerr << "\nYHAT[" << yhat.size() << "]={";
      std::copy(yhat.begin(), yhat.end(), ostream_iterator<R>(cerr, ","));
      cerr << "\nMISSD[" << missed.size() << "]={";
      std::copy(missed.begin(), missed.end(), ostream_iterator<R>(cerr, ","));
      cerr << "}\nWRONG[" << wrong.size() << "]={";
      std::copy(wrong.begin(), wrong.end(), ostream_iterator<R>(cerr, ","));
      cerr << "}\n";
    }

    for (it_y = missed.begin(); it_y != missed.end(); ++it_y) {
      const FeatureVector* const f_r = model.fgen().phi(x, *it_y);
      _W->add(f_r);
      model.fgen().discard(f_r, x, *it_y);
      mistake = true;
      ++_hamming_loss;
    }
    for (it_y = wrong.begin(); it_y != wrong.end(); ++it_y) {
      const FeatureVector* const f_r = model.fgen().phi(x, *it_y);
      _W->sub(f_r);
      model.fgen().discard(f_r, x, *it_y);
      mistake = true;
      ++_hamming_loss;
    }

    if(mistake) { ++_mistakes; }

    if (_verbose)  cerr << "!.." << flush;

    model.fgen().phi_end_pattern(x);

    if (_verbose) {
      double cpu_time = ((double)(clock() - t0))/(double)CLOCKS_PER_SEC;
      cerr << fixed << setprecision(2) << " [" << cpu_time << "]" << endl;
    }
  }


} // namespace treeler

#undef myname
