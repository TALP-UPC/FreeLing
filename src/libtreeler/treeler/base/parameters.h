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
 * \file   parameters.h
 * \brief  Declaration of the class Parameters
 * \author Terry Koo
 * \note   This file was ported from egstra
 */

#ifndef TREELER_AVG_PARAMETERS_H
#define TREELER_AVG_PARAMETERS_H

#include <stdint.h>
#include <string>
#include <functional>
#include "treeler/base/base-parameters.h"
#include "treeler/base/feature-vector.h"
#include "treeler/util/simple-hash-table.h"

// for input ouput
#include <iomanip>


namespace treeler {
    
  /**
   * \brief A parameter vector whose indices are feature
   * objects, eliminating the need for feature dictionaries.  
   * \author Terry Koo
   * \ingroup base
   * 
   * These are mostly only useful for perceptron training.  Note that
   * certain things are impossible in this setting, such as retrieving
   * the dimensionality of the vector or applying count cutoffs.
   * 
   * For convenience, the class is currently very specific in
   * application. In particular, the class only contains those
   * operations which are used by the perceptron algorithm, and
   * supports parameter averaging natively (but not operations like
   * fast scaling or squared-L2).  In the case that max-margin
   * subgradient is desired (which does not require averaging but does
   * require fast scaling/L2), a separate implementation may be
   * written.
   *
   * The parameters are stored internally as a hashtable, which is
   * appropriate for sparse parameter vectors.  Only those operations
   * which are important for sparse algorithms such as the perceptron
   * are defined.
   *
   * \note The name of this class will change in future versions to
   * reflect its form. Parameters will be used as an abstract class
   * name or concept class.
   *
   */
  template <typename FIdx, typename Val = double>
  class Parameters {
  public:
    typedef FeatureVector<FIdx> FVec; 
    
  private:

    struct avg_param { /* structure for averaged parameters */
      Val val;           /* parameter value at this index */
      Val sum;           /* summed parameter value at this index */
      int32_t timestamp;    /* time of last update to this index */
      avg_param();          /* construct a zeroed object */
      /* make this parameter consistent with time t */
      inline void update(const int t);
      /* make this parameter consistent with time t and add d */
      inline void add(const int t, const Val& d);
      /* make this parameter consistent with time t and subtract d */
      inline void sub(const int t, const Val& d);
    };
    /* a table of averaged parameters.  note that the avg_param
       structure is fairly large and may incur overhead when copying
       it back and forth into the hashtable.  hopefully, the cost of
       copying will be small, especially considering that simple_hasht
       uses many inlined functions.  in addition, this implementation
       is considerably faster than using three separate hashtables for
       parameters, summed parameters, and timestamps (as in the normal
       parameters classes).  such an implementation would incur three
       times the hashing overhead and memory overhead. */

    typedef typename parameters_internal::Traits<FIdx>::Hash HashOp; 
    typedef typename parameters_internal::Traits<FIdx>::Eq EqOp; 

    typedef simple_hasht<FIdx, avg_param, HashOp, EqOp> hashtable; 

    /* number of possible logical offsets */
    int _K;

    /* hashtables of parameter values, one for each possible logical
       offset */
    hashtable* _hasht;

    /* current timestamp for parameter averaging, which is always
       assumed to be active */
    int _now;
    /* whether to use averaged parameters in operations */
    bool _averaged;

      //Parameters() {} /* disallow default constructor */

  public:
    /* create zeroed parameters for the given number of offsets */
    Parameters(const int K);
    /* destroy and delete memory */
    ~Parameters();

    int spaces() const { return _K; }

    /* store/read to file */
    void save(const std::string& fname, const int t,
	      const std::string& stem = "parameters", bool verbose=true) const;
    void load(const char* fname, bool verbose = true);
    void load(const std::string& fname, const int t,
	      const std::string& stem = "parameters", bool verbose=true);


    /* set S[r] = this->dot(F[r]) */
    inline void dot(Val* const S, const FVec* const F,
		    const int R) const;
    /* inner product W . F or \sum W . F, depending on whether
       average-mode is active */
    inline Val dot(const FVec& F) const;


    /* perform W = 0 */
    void zero();

    /* perform W += F */
    inline void add(const FVec* F);
    /* perform W += c*F */
    inline void add(const FVec* F, const double c);
    /* perform W -= F */
    inline void sub(const FVec* F);

    /* advance the update counter */
    void avg_tick() { ++_now; }
    /* perform delayed updates to averaged parameters */
    void avg_flush();
    /* toggle use of averaged parameters for e.g. dot() */
    void avg_set(const bool b) { _averaged = b; }
  };




  /*** implementation of inlined functions ******************************/

  /* for avg_param() */
  template <typename FIdx, typename Val>
  inline void Parameters<FIdx,Val>::avg_param::update(const int t) {
    sum += val*(t - timestamp); timestamp = t;
  }
  template <typename FIdx, typename Val>
  inline void Parameters<FIdx,Val>::avg_param::add(const int t, const Val& d) {
    update(t); val += d;
  }
  template <typename FIdx, typename Val>
  inline void Parameters<FIdx,Val>::avg_param::sub(const int t, const Val& d) {
    update(t); val -= d;
  }


  template <typename FIdx, typename Val>
  Parameters<FIdx,Val>::Parameters(const int K)
    : _K(K), _hasht(NULL), _now(0), _averaged(false) {
    assert(_K >= 0);
    if(_K > 0) { _hasht = new hashtable[K]; }
  }

  template <typename FIdx, typename Val>
  Parameters<FIdx,Val>::~Parameters() {
    if(_hasht != NULL) { delete [] _hasht; }
  }

  /* nested class for averaged parameters */
  template <typename FIdx, typename Val>
  Parameters<FIdx, Val>::avg_param::avg_param()
    : val(0), sum(0), timestamp(0) {}


/* (optional) deletion of zeroed parameters: it's best to leave this
   active.  although this involves additional checking, it turns out
   that at least for perceptron training, the reductions in hashtable
   size are worthwhile.  since the hashtable is less-populated, it
   consumes (slightly) less memory and there are also fewer hash
   collisions, so the code runs (slightly) faster.  */
#if 1
#define DELETE_ZERO(ft, parm) {					\
    if(parm->val == 0 && parm->sum == 0) { ht.del(ft); }	\
  }
#else
#define DELETE_ZERO(mac_i, mac_v)
#endif

/* (optional) checks for consistency of offsets */
#if 1
#define CHECK_OFFSET(k) 			\
    assert(k >= 0); assert(k < _K);
#else
#define CHECK_OFFSET(k)
#endif


  /* batch inner product */
  template <typename FIdx, typename Val>
  inline void Parameters<FIdx,Val>::dot(Val* const S,
				    const FVec* const F,
				    const int R) const {
    Val* s = S;
    Val* const end = S + R;
    const FVec* f = F;
    for(; s < end; ++s, ++f) { (*s) = dot(f); }
  }

  /* return the inner product W . F */
  template <typename FIdx, typename Val>
  inline Val Parameters<FIdx,Val>::dot(const FVec& f) const {
    Val ret = 0; /* inner product value */
    if(_averaged) {
      for(const FVec* fp = &f; fp != NULL; fp = fp->next) {
	CHECK_OFFSET(fp->offset);
	const hashtable& ht = _hasht[fp->offset]; /* select parameter space */
	const FIdx* fi = fp->idx;
	const FIdx* const fend = fi + fp->n;
	const double* fv = fp->val;
	const struct avg_param zero;
	/* NB: use .sum parameter value */
	if(fv == NULL) { /* indicator features */
	  for(; fi < fend; ++fi)       { ret += ht.get(*fi, zero).sum; }
	} else { /* real-valued features */
	  for(; fi < fend; ++fi, ++fv) { ret += ht.get(*fi, zero).sum*(*fv); }
	}
      }
    } else {
      for(const FVec* fp = &f; fp != NULL; fp = fp->next) {
	CHECK_OFFSET(fp->offset);
	const hashtable& ht = _hasht[fp->offset]; /* select parameter space */
	const FIdx* fi = fp->idx;
	const FIdx* const fend = fi + fp->n;
	const double* fv = fp->val;
	const struct avg_param zero;
	/* NB: use .val parameter value */
	if(fv == NULL) { /* indicator features */
	  for(; fi < fend; ++fi)       { ret += ht.get(*fi, zero).val; }
	} else { /* real-valued features */
	  for(; fi < fend; ++fi, ++fv) { ret += ht.get(*fi, zero).val*(*fv); }
	}
      }
    }
    return ret;
  }


  /* perform W = 0 */
  template <typename FIdx, typename Val>
  void Parameters<FIdx,Val>::zero() {
    for(int k = 0; k < _K; ++k) { _hasht[k].clear(); }
    _now = 0; /* might as well reset this too */
  }
  


  /* flush updates to averaged vectors */
  template <typename FIdx, typename Val>
  void Parameters<FIdx,Val>::avg_flush() {
    for(int k = 0; k < _K; ++k) {
      typename hashtable::enumerator e = _hasht[k].mbegin();
      struct avg_param* ret = NULL;
      while((ret = e.next()) != NULL) {
	ret->update(_now);
	ret->timestamp = 0; /* reset all timestamps to zero */
      }
    }
    _now = 0; /* reset all timestamps to zero */
  }


  /* perform W += F */
  template <typename FIdx, typename Val>
  inline void Parameters<FIdx,Val>::add(const FVec* F) {
    assert(!_averaged);
    assert(F != NULL);
    for(; F != NULL; F = F->next) {
      CHECK_OFFSET(F->offset);
      hashtable& ht = _hasht[F->offset]; /* select parameter space */
      const FIdx* fi = F->idx;
      const FIdx* const fend = fi + F->n;
      const double* fv = F->val;
      const struct avg_param zero;
      /* NB: op is "+" for addition */
      if(fv == NULL) { /* indicator features */
	for(; fi < fend; ++fi) {
	  struct avg_param* const ret = ht.pgetput(*fi, zero);
	  ret->add(_now, 1);
	  DELETE_ZERO(*fi, ret);
	}
      } else { /* real-valued features */
	for(; fi < fend; ++fi, ++fv) {
	  struct avg_param* const ret = ht.pgetput(*fi, zero);
	  ret->add(_now, *fv);
	  DELETE_ZERO(*fi, ret);
	}
      }
    }
  }
  /* perform W += s*F */
  template <typename FIdx, typename Val>
  inline void Parameters<FIdx,Val>::add(const FVec* F,
					const double s) {
    assert(!_averaged);
    assert(F != NULL);
    for(; F != NULL; F = F->next) {
      CHECK_OFFSET(F->offset);
      hashtable& ht = _hasht[F->offset]; /* select parameter space */
      const FIdx* fi = F->idx;
      const FIdx* const fend = fi + F->n;
      const double* fv = F->val;
      const struct avg_param zero;
      /* NB: op is "+" for addition */
      if(fv == NULL) { /* indicator features */
	for(; fi < fend; ++fi) {
	  struct avg_param* const ret = ht.pgetput(*fi, zero);
	  ret->add(_now, s);
	  DELETE_ZERO(*fi, ret);
	}
      } else { /* real-valued features */
	for(; fi < fend; ++fi, ++fv) {
	  struct avg_param* const ret = ht.pgetput(*fi, zero);
	  ret->add(_now, s*(*fv));
	  DELETE_ZERO(*fi, ret);
	}
      }
    }
  }

  /* perform W -= F */
  template <typename FIdx, typename Val>
  inline void Parameters<FIdx,Val>::sub(const FVec* F) {
    assert(!_averaged);
    assert(F != NULL);
    for(; F != NULL; F = F->next) {
      CHECK_OFFSET(F->offset);
      hashtable& ht = _hasht[F->offset]; /* select parameter space */
      const FIdx* fi = F->idx;
      const FIdx* const fend = fi + F->n;
      const double* fv = F->val;
      const struct avg_param zero;
      /* NB: op is "+" for subtraction */
      if(fv == NULL) { /* indicator features */
	for(; fi < fend; ++fi) {
	  struct avg_param* const ret = ht.pgetput(*fi, zero);
	  ret->sub(_now, 1);
	  DELETE_ZERO(*fi, ret);
	}
      } else { /* real-valued features */
	for(; fi < fend; ++fi, ++fv) {
	  struct avg_param* const ret = ht.pgetput(*fi, zero);
	  ret->sub(_now, *fv);
	  DELETE_ZERO(*fi, ret);
	}
      }
    }
  }


  // Input/Output operations
  
  #define MAXLINE 1024
  #define myname "Parameters"

  /* write to disk */
  template <typename FIdx, typename Val>
  void Parameters<FIdx,Val>::save(const string& dir, const int t,
				  const string& stem, bool verbose) const {
    char* const fname = new char[64 + stem.size() + dir.size()];
    sprintf(fname, "%s/%s.%03d.gz", dir.c_str(), stem.c_str(), t);

    int hash_tot = 0;
    int hash_active = 0;
    for(int k = 0; k < _K; ++k) {
      hash_tot += _hasht[k].n();
      hash_active += _hasht[k].nactive();
    }

    if (verbose) {
      cerr << myname << " : bucket ratio = " << hash_tot
	   << " / " << hash_active << " = "
	   << fixed << setprecision(2)
	   << (100.0*(double)hash_tot)/((double)hash_active)
	   << "%" << endl;
      
      cerr << myname << " : saving to \"" << fname << "\" " << flush;
    }
    
    gzFile out = gzopen(fname, "w");

    gzprintf(out, "%d\n", _K);

    FIdx f;
    struct avg_param p;
    int nnz = 0;
    int nnz_sum = 0;
    int cnt = 0;
    for(int k = 0; k < _K; ++k) {
      typename hashtable::const_enumerator e = _hasht[k].begin();
      while(e.next(f, p)) {
	if(p.val != 0 || p.sum != 0) { /* only write nonzero parameters */
	  if(p.val != 0) { ++nnz; }
	  if(p.sum != 0) { ++nnz_sum; }	  
	  parameters_internal::Traits<FIdx>::fwrite(out, f); 	  
	  // feature_idx_fwrite(out, f.fidx);
	  gzprintf(out, " %.16g %.16g\n", p.val, p.sum);
	  if(((++cnt) & 0xfffff) == 0) { cerr << "." << flush; }
	}
      }
      /* separate different feature spaces with blank lines */
      gzprintf(out, "\n");
    }
    gzclose(out);
    delete [] fname;

    if (verbose) {
      cerr << " done" << endl;
      cerr << myname << " : nnz = " << nnz
	   << " avg_nnz = " << nnz_sum << endl;
    }
  }

  /* read from disk */
  template <typename FIdx, typename Val>
  void Parameters<FIdx,Val>::load(const string& dir, const int t,
				  const string& stem, bool verbose) {
    char* const fname = new char[64 + stem.size() + dir.size()];
    sprintf(fname, "%s/%s.%03d.gz", dir.c_str(), stem.c_str(), t);

    load(fname, verbose);

    delete [] fname;
  }


  template <typename FIdx, typename Val>
  void Parameters<FIdx,Val>::load(const char* fname, bool verbose){

    if (verbose) 
      cerr << myname << " : loading from \"" << fname << "\" " << flush;

    _now = 0; /* reset the time counter */

    gzFile in = gzopen(fname, "r");

    char line[MAXLINE];
    char ftbuf[64];

    line[0] = '\0';
    const char* ret = gzgets(in, line, MAXLINE);
    assert(ret != NULL);
    int nscanned = sscanf(line, "%d", &_K);
    assert(nscanned == 1);
    assert(_K > 0);

    if(_hasht != NULL) { delete [] _hasht; }
    _hasht = new hashtable[_K];

    FIdx f;
    struct avg_param p;
    int nnz = 0;
    int nnz_sum = 0;
    int cnt = 0;

    for(int k = 0; k < _K; ++k) {
      hashtable& ht = _hasht[k];
      while(1) {
	line[0] = '\0';
	ret = gzgets(in, line, MAXLINE);
	assert(ret != NULL);
	const int len = strlen(line);
	assert(len > 0); /* always expect input */

	/* check if the final newline was stored */
	if(line[len - 1] != '\n') {
	  cerr << "detected line overflow when reading data" << endl;
	  assert(0);
	}

	/* check for blank lines */
	bool blank = true;
	for(int i = 0; i < len; ++i) {
	  const char c = line[i];
	  if(c != ' ' && c != '\t' && c != '\n') {
	    blank = false; break;
	  }
	}
	/* blank lines separate the parameter spaces */
	if(blank) { break; }

	/* otherwise we have a FIdx double double line; grab only
	   the FIdx string and process it later */
	ftbuf[0] = '\0';
	int nread = sscanf(line, "%s %lf %lf", ftbuf, &(p.val), &(p.sum));
	assert(nread == 3);
	assert(strlen(ftbuf) == 16); /* 64-bit integer encoded in hex */
	/* process the FIdx string */
	nread = parameters_internal::Traits<FIdx>::sscan(ftbuf, f); 
	assert(nread == 1);

	if(p.val != 0) { ++nnz; }
	if(p.sum != 0) { ++nnz_sum; }

	/* finally, insert it into the hash */
	const struct avg_param* const ret = ht.pgetput(f, p);
	/* check uniqueness */
	assert(ret->val == p.val);
	assert(ret->sum == p.sum);
	assert(ret->timestamp == 0);

	
	if(verbose and ((++cnt) & 0xfffff) == 0) { cerr << "." << flush; }
      }
    }
    /* check EOF */
    assert(gzgets(in, line, MAXLINE) == NULL);
    assert(gzeof(in));
    gzclose(in);

    if (verbose) {
      cerr << " done" << endl;
      cerr << myname << " : nnz = " << nnz
	   << " avg_nnz = " << nnz_sum << endl;
    }

    int hash_tot = 0;
    int hash_active = 0;
    for(int k = 0; k < _K; ++k) {
      hash_tot += _hasht[k].n();
      hash_active += _hasht[k].nactive();
    }
    
    if (verbose) {
      cerr << myname << " : bucket ratio = " << hash_tot
	   << " / " << hash_active << " = "
	   << fixed << setprecision(2)
	   << (100.0*(double)hash_tot)/((double)hash_active)
	   << "%" << endl;
    }
  }

#undef MAXLINE
#undef myname
#undef CHECK_OFFSET
#undef DELETE_ZERO

} // namespace


#endif /* TREELER_PARAMETERS_H */
