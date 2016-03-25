#include "Timer.h"
#include "Options.h"
#include "Constants.h"
#include "OptUtils.h"
#include "RegPrimal.h"
#include "ParametersFactory.h"


#include <iostream>
#include <fstream>
#include <iomanip>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* detailed name of the trainer */
#define myname								\
  (_anneal >= 0 ? "A" : "")						\
  << RAND_SHORT_GLOSS(_rand_type)					\
  << "SGD[" << _M->name() << "," << Obj::name << "]"

using namespace std;

namespace egstra {
  template<class Obj>
  void sgd_trainer<Obj>::usage(const char* const mesg) {
    cerr << "SGDTrainer[" << Obj::name << "] options:" << endl;
    cerr << " --rand=<string>  : randomization type (det,unif,perm)" << endl;
    cerr << " --seed=<int>     : random seed" << endl;
    cerr << " --C=<real>       : regularization constant" << endl;
    cerr << " --E=<real>       : initial learning rate (fixed-rate unless an" << endl;
    cerr << "                    annealing option is specified)" << endl;
    cerr << " --anneal=<real>  : anneal the learning rate using the formula" << endl;
    cerr << "                    eta(t) = E/(1 + A*t)" << endl;
    cerr << "                    where A is the provided value" << endl;
    cerr << " --anneal-lecun   : synonym for anneal=1/(# training examples)" << endl;
    cerr << " --anneal-pegasos : synonym for anneal=1" << endl;
    cerr << " --projl2=<real>  : after each SGD update, project onto an" << endl;
    cerr << "                    L2-ball of the given radius R" << endl;
    cerr << " --projl2-pegasos : synonym for projl2=1/sqrt(C)" << endl;
    cerr << " --noparams       : do not write weight vectors to disk" << endl;
    cerr << " --noprimal       : do not compute primal objective" << endl;
    cerr << " --sparse         : use sparse parameter vectors" << endl;
    cerr << " --reproduce-ordering  : use the example order contained in the files" << endl;
    cerr << "                         <dir>/example-ordering.<NNN>" << endl;
    cerr << "                         where <dir> is the output directory and <NNN>" << endl;
    cerr << "                         is the 3-digit-padded iteration number.  this" << endl;
    cerr << "                         is mostly only useful for debugging." << endl;
    cerr << endl;
    cerr << mesg << endl;
  }

  template<class Obj>
  void sgd_trainer<Obj>::process_options() {
    int tmpi;
    _seed = 1;
    if(options::get("seed", tmpi)) {
      _seed = tmpi;
    }

    _rand_type = EGSTRA_RAND_UNIFORM;
    string rtype;
    if(options::get("rand", rtype)) {
      if(strcmp(rtype.c_str(), "det") == 0) {
	_rand_type = EGSTRA_DETERMINISTIC;
      } else if(strcmp(rtype.c_str(), "unif") == 0) {
	_rand_type = EGSTRA_RAND_UNIFORM;
      } else if(strcmp(rtype.c_str(), "perm") == 0) {
	_rand_type = EGSTRA_RAND_PERMUTATION;
      } else {
	usage("unrecognized randomization type"); exit(1);
      }
    }

    if(!options::get("C", _C)) {
      usage("please provide a regularization constant"); exit(1);
    }
    if(_C <= 0) {
      usage("C: please provide a value > 0"); exit(1);
    }

    if(!options::get("E", _lrate)) {
      usage("please provide an initial learning rate"); exit(1);
    }
    if(_lrate <= 0) {
      usage("E: please provide a value > 0"); exit(1);
    }

    _anneal = -1; /* special meaning: no annealing */
    double tmpd;
    if(options::get("anneal", tmpd)) {
      _anneal = tmpd;
      if(_anneal <= 0) {
	usage("anneal: please providea a value > 0"); exit(1);
      }
    }
    if(options::get("anneal-lecun", tmpd)) {
      _anneal = 0; /* special meaning: lecun-style annealing */
    }
    if(options::get("anneal-pegasos", tmpd)) {
      _anneal = 1.0;
    }

    _projl2 = -1;  /* special meaning: no projection */
    if(options::get("projl2", tmpd)) {
      _projl2 = tmpd*tmpd; /* NB: _projl2 contains the squared radius */
      if(_projl2 <= 0) {
	usage("projl2: please provide a value > 0"); exit(1);
      }
    }
    if(options::get("projl2-pegasos", tmpd)) {
      _projl2 = 1.0/_C;    /* NB: _projl2 contains the squared radius */
    }

    _writeparams = true;
    if(options::get("noparams", tmpi)) {
      _writeparams = false;
    }

    _doprimal = true;
    if(options::get("noprimal", tmpi)) {
      _doprimal = false;
    }

    _sparse = false;
    if(options::get("sparse", tmpi)) {
      _sparse = true;
    }

    _reproduce_ordering = false;
    if(options::get("reproduce-ordering", tmpi)) {
      _reproduce_ordering = true;
    }
  }


  /* adjust the primals with the gradient on the given sentence */
  template<class Obj>
  void sgd_trainer<Obj>::process_example(const example* const ex) {
    const pattern* x = ex->x();

    const int R = x->nparts();

    /* get the inner products */
    const struct fvec* const F = _fg->phi(x);
    double* const S = (double*)malloc(R*sizeof(double));
    _W->dot(S, F, R);

    /* regularizer gradient is -(_C*V) */
    _W->scale(1.0 - (_lrate * _C));
    /* write the gradient update directly onto the primals */
    Obj::exgrad(*_W, _lrate, ex, S, F, *_M);

    _fg->discard(F, x);
    free(S);

    /* optionally project the primals onto an L2-ball */
    if(_projl2 > 0) {
      const double sqL2 = _W->sqL2();
      if(sqL2 > _projl2) {
	_W->scale(sqrt(_projl2/sqL2));
	cerr << "p" << flush;
      }
    }
  }


  /* helper function for reading example orderings */
  static void read_example_ordering(const string dir, const int t,
				    int* const arr, const int n) {
    char buf[256];
    sprintf(buf, "/example-ordering.%03d", t);
    const string fname = dir + buf;
    cerr << "loading example ordering from \"" << fname << "\"" << endl;
    ifstream fin(fname.c_str());
    if(!(fin.good())) {
      cerr << "*** problem opening " << fname << endl;
      assert(0);
    }
    for(int j = 0; j < n; ++j) {
      if(!(fin.good())) {
	cerr << " : *** malformed file " << fname << endl;
	assert(0);
      }
      fin >> arr[j];
      if(fin.fail()) {
	cerr << " : *** malformed file " << fname << endl;
	assert(0);
      }
    }
    int tmp;
    fin >> tmp;
    if(!fin.eof()) {
      cerr << " : *** malformed file " << fname << endl;
      assert(0);
    }
    fin.close();
  }


  template<class Obj>
  void sgd_trainer<Obj>::train(dataset& trdata, dataset& valdata,
			       const int T) {
    /* due to the scaling from the regularizer gradient, the primals
       should always be double */
    _W = (_sparse
	  ? parameters_factory::construct_sparse("double", _fg->dim())
	  : parameters_factory::construct_array("double", _fg->dim()));

    cerr << myname << " : training started for max " << T
	 << " rounds ... " << endl;
    cerr << myname << " : data order is "
	 << RAND_LONG_GLOSS(_rand_type) << endl;

    if(_rand_type != EGSTRA_DETERMINISTIC) {
      /* only report this for randomized EG */
      cerr << myname << " : seeding the RNG with value " << _seed << endl;
      srandom(_seed);
    }

    cerr << myname << " : provided eta0 = " << _lrate;
    /* scale the learning rate by 1/C so that the learning rate has a
       clear interpretation as the downweighting of the current
       primals; i.e., the update w' = w - C*_lrate*w + _lrate*f'(w)
       results in w' = (1 - lrate)*w + _lrate*f'(w) */
    _lrate = _lrate/_C;
    cerr << " => internal eta0 = " << _lrate << endl;

    /* save the initial value */
    const double eta0 = _lrate;

    const int Ntr = trdata.size();

    /* special case for lecun-style learning rate */
    if(_anneal == 0) { _anneal = 1.0/(double)Ntr; }

    if(_anneal < 0) {
      cerr << myname << " : learning rate is constant" << endl;
    } else {
      cerr << myname << " : learning rate is annealed using "
	   << _anneal << endl;
      assert(_anneal > 0);
    }

    if(_projl2 < 0) {
      cerr << myname << " : no L2 projection" << endl;
    } else {
      cerr << myname << " : L2-projection with R = "
	   << sqrt(_projl2) << endl;
    }

    /* log file for primal and #pass information */
    string objfilename = _dir + "/obj.out";
    ofstream obj_out(objfilename.c_str(), ios::out | ios::trunc);
    assert(obj_out.good());

    _W->zero();

    int num_processed = 0;
    double cpu_time = 0;
    for(int t = 1; t <= T; ++t) {
      cerr << myname << " : iter " << t << " " << flush;

      /* NB: the first round of rand-uniform will be a permutation,
	 to ensure that all examples are visited. */
      const int rt = (t == 1 && _rand_type == EGSTRA_RAND_UNIFORM
		      ? EGSTRA_RAND_PERMUTATION : _rand_type);

      /* load the example order if needed (see below) */
      int* example_ordering = NULL;
      if(_reproduce_ordering) {
	example_ordering = new int[Ntr];
	read_example_ordering(_dir, t, example_ordering, Ntr);
      }

      timer_start();
      const clock_t t0 = clock();
      for(int i = 0; i < Ntr; ++i) {
	/* pick an example according to the desired scheme.  NB: as a
	   special case, we allow the algorithm to reproduce an
	   example order stored in specially-named files */
	const example* const ex =
	  (_reproduce_ordering
	   ? trdata[example_ordering[i]]
	   : optutils::randselect(rt, i, trdata));

	/* adjust the learning rate if necessary */
	if(_anneal > 0) {
	  /* formula is eta = eta0/(1 + A*t) */
	  _lrate = eta0/(1.0 + _anneal*((double)num_processed));
	}

	/* perform a stochastic update */
	process_example(ex);
	++num_processed;

	ticker(i + 1, Ntr);
      }
      ticker(Ntr, Ntr);
      timer_stop();
      cpu_time += ((double)(clock() - t0))/(double)CLOCKS_PER_SEC;
      cerr << " [" << timestr(timediff) << "]" << endl;

      /* delete the ordering if needed */
      if(_reproduce_ordering) {
	delete [] example_ordering; example_ordering = NULL;
      }

      /* save the primals in the model directory */
      if(_writeparams) { _W->save(_dir, t); }

      const double primal =
	(_doprimal
	 ? reg_primal<Obj>::objective(*_W, _C, trdata, *_fg, *_M)
	 : 0.0);

      /* #passes is an abstract measure of computation */
      const double num_passes = ((double)num_processed)/((double)Ntr);
      /* update the objective log with the current iteration */
      obj_out << scientific << setprecision(16)
	      << t << " " << num_passes << " "
	      << cpu_time << " "
	      << _W->sqL2() << " " << primal << endl;
      /* print a summary */
      cerr << myname << " : finished it = " << t
	   << scientific << setprecision(5)
	   << " ; ops = " << num_passes
	   << " ; cpu = " << cpu_time
	   << " ; W^2 = " << _W->sqL2()
	   << " ; P = " << primal << endl;

      /* parse the validation file if provided */
      if(valdata.size() > 0) {
	optutils::classify_val_set(*_W, valdata, *_fg, *_M, _dir, t,
				   Obj::probabilistic());
      }
    }

    delete _W; _W = NULL;
    obj_out.close();
  }
}

#undef myname
