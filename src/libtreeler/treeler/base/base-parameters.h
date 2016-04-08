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
 * \file   base-parameters.h
 * \brief  Declaration of base parameters class
 * \author Xavier Carreras, Terry Koo
 * \note   This file was ported from egstra
 */

#ifndef TREELER_BASE_PARAMETERS_H
#define TREELER_BASE_PARAMETERS_H

/* cutoff point at which the parameters should be recentered */
#define TREELER_PARAMETERS_SCALE_CUTOFF   0.1


#include <string>
#include <vector>

#include "treeler/base/feature-vector.h"

//#include "treeler/util/gzfile.h"
#include "zlib.h"

namespace treeler {

  /* 
   * \brief Base abstract class for parameters, using CRTP
   * 
   * The parameter representation is augmented with _alpha and _Wsq
   * values that allow a fast implementation of certain operations,
   * such as scalar multiplication and squared L2-norm.  The
   * underlying implementation may be a dense or sparse vector, and
   * the internal data type may be integral or floating-point, of
   * varying precision. 
   *
   * D is the derived type, and must implement the following functions:
   *
   * - rescale() : rescale the internal representation so that _alpha
   *               = 1.0 and recompute the norm _Wsq
   *
   * - zero() : sets w = 0 
   * 
   */
  template <typename D>
  class BaseParameters {
  public:
    typedef FIdxBits FIdx; 
    typedef FeatureVector<FIdx> FVec; 
    
  protected:
    /* identifiers for what kind of parameter vector this is */
    const int _vtype;        /* vector type (e.g., sparse) */
    const int _dtype;        /* data type */
    const std::string _name; /* name identifying the object */

    /* basic parameter information */
    size_t _dim;  /* feature dimensionality */
    double _alpha; /* scalar multiplier of this */
    double _Wsq;   /* squared-L2 of this */

    /* data structures for parameter averaging */
    // bool        _averaged;
    // int         _avg_tick;
    // parameters* _avg_sum;
    // parameters* _avg_time;

    /* create a parameter vector of the given type */
    BaseParameters(const int vtype, const int dtype);

  public:
    /* destroy and delete memory */
    ~BaseParameters() {}


    /* store to/read from file */
    void save(const std::string& fname, const int t,
	      const std::string& stem = "parameters") const;
    void load(const std::string& fname);
    void load(const std::string& fname, const int t,
	      const std::string& stem = "parameters");
    
    /* return the parameter dimensionality */
    size_t dim() const { return _dim; }


    /* return the number of nonzero (active) parameters.  the
       implementation is not required to be efficient. */
    size_t nnz() const { return D::nnz(); }


    /* compute the inner product of W with a batch of feature vectors */
    //    void dot(double* const S, 
    //	     const struct Fvec* const F,
    //	     const int R) const {
    //      for (int r = 0; r < R; ++r) { S[r] = D::dot(F + r); }
    //      }
    
    /* compute the inner product of W with a batch of feature vectors */
    void dot(double* const S, 
	     const FVec* const F,
	     const int R, const double scale) const {
      for (int r = 0; r < R; ++r) { S[r] = scale*D::dot(F + r); }
    }

    /* return W . W */
    double sqL2() const { return _alpha*_alpha*_Wsq; }
    /* return W . W, potentially recentering the parameters first */

    double sqL2(const bool force_recenter) {
      if (force_recenter) { static_cast<D const*>(this)->recenter(); }
      return sqL2();
    }

    // /* single-element access (read-only).  this will not in general be
    //    a fast operation, so use sparingly */
    // virtual double get(const dim_int i) const = 0;

    // /* find the max/min/max-magnitude parameter value and index */
    // virtual double max(dim_int& argmax) const = 0;
    // virtual double min(dim_int& argmin) const = 0;
    // virtual double maxmag(dim_int& argmax) const = 0;
    // /* find the k largest/smallest/largest-magnitude parameter values
    //    and indices, starting from the most superlative values */
    // virtual void kmax(const int k, std::vector<double>& values,
    // 		      std::vector<dim_int>& indices) const = 0;
    // virtual void kmin(const int k, std::vector<double>& values,
    // 		      std::vector<dim_int>& indices) const = 0;
    // virtual void kmaxmag(const int k, std::vector<double>& values,
    // 			 std::vector<dim_int>& indices) const = 0;


    // /* perform W = 0 */
    // virtual void zero() = 0;

    /* perform W *= s using the _alpha variable.  recenter if
       necessary to prevent excessive numerical error */
    void scale(const double s);

    // /* perform W[i] += d */
    // virtual void add(const dim_int i, const double d) = 0;
    // /* perform W += scale * F */
    // virtual void add(const struct fvec* F,
    // 		     const double scale = 1.0) = 0;
    // /* perform W += scale * V */
    // virtual void add(const parameters* const V,
    // 		     const double scale = 1.0) = 0;
    // /* perform W[i] *= \exp{scale * V[i]} for all indices i (i.e., a
    //    log-space additive update) */
    // virtual void expmul(const parameters* const V,
    // 			const double scale = 1.0) = 0;


    // /* enable parameter averaging */
    // void avg_enable();
    // /* signal the end of the current update phase */
    // void avg_tick();
    // /* retrieve the cumulative-sum version of this parameter vector */
    // const parameters* avg_params();
    // /* initialize averaged vectors, etc. */
    // virtual void avg_init() = 0;
    // /* flush all pending updates to the averaged vector */
    // virtual void avg_flush() = 0;

    // /* make a runtime assertion that this object is of the named data
    //    type or vector type */
    // void assert_data_type(const std::string& name) const;
    // void assert_vector_type(const std::string& name) const;

    /* accessors */
    double alpha() const { return _alpha; }
    int data_type() const { return _dtype; }
    int vector_type() const { return _vtype; }
  };


  class FIdxBitsHash {
  public:	
    inline size_t operator()(const FIdxBits& t) const {
      FeatureIdxHash h;
      return h(t());
    }

    inline size_t operator()(const FIdxPair& t) const {
      FeatureIdxHash h;
      return h(t.idx.first.idx);
    }
  }; 
  
  class FIdxBitsEquality {
  public:	
    inline bool operator()(const FIdxBits& t1, const FIdxBits& t2) const {
      return (t1() == t2());
    }

    inline bool operator()(const FIdxPair& t1, const FIdxPair& t2) const {
      return (*this)(t1.idx.first,t2.idx.first);
    }
  }; 

  class FIdxCharsHash {
  public:	
    inline size_t operator()(const FIdxChars& t) const {      
      return std::hash<std::string>()(t());
    }
  }; 
  
  class FIdxCharsEquality {
  public:	
    inline bool operator()(const FIdxChars& t1, const FIdxChars& t2) const {
      return (t1() == t2());
    }
  }; 


  namespace parameters_internal {

    template<typename T> 
    class Traits {}; 

    template<> 
    class Traits<FIdxV0> {
    public: 
      typedef FeatureIdxHash Hash; 
      typedef FeatureIdxEquality Eq; 
      
      static inline void fwrite(gzFile out, const FIdxV0& f) {
	feature_idx_fwrite(out, f);
      }
      
      static inline int sscan(char* buf, FIdxV0& f) {
	int n; 
	feature_idx_sscan(buf, n, f);
	return n; 
      }      
    }; 
    
    template<> 
    class Traits<FIdxBits> {
    public: 
      typedef FIdxBitsHash Hash; 
      typedef FIdxBitsEquality Eq; 
      
      static inline void fwrite(gzFile out, const FIdxBits& f) {
	feature_idx_fwrite(out, f());
      }      

      static inline int sscan(char* buf, FIdxBits& f) {
	int n; 
	feature_idx_sscan(buf, n, f.idx);
	return n; 
      }      
    }; 

    template<> 
    class Traits<FIdxChars> {
    public: 
      typedef FIdxCharsHash Hash; 
      typedef FIdxCharsEquality Eq; 

      static inline void fwrite(gzFile out, const FIdxChars& f) {
	//feature_idx_fwrite(out, f());
	gzprintf(out, "%s", f.idx.c_str());
      }      

      static inline int sscan(char* buf, FIdxChars& f) {
	int n=0; 
	// n = sscan(buf, "%s", f.idx);
	return n; 
      }      
    }; 

    template<> 
    class Traits<FIdxPair> {
    public: 
      typedef FIdxBitsHash Hash; 
      typedef FIdxBitsEquality Eq; 

      static inline void fwrite(gzFile out, const FIdxPair& f) {
	feature_idx_fwrite(out, f.idx.first.idx);
      }      

      static inline int sscan(char* buf, FIdxPair& f) {
	int n; 
	feature_idx_sscan(buf, n, f.idx.first.idx);
	return n; 
      }      
    }; 
    
  }


  /* IMPLEMENTATION */

  template <typename D>
  BaseParameters<D>::BaseParameters(const int vtype, const int dtype)
    : _vtype(vtype), _dtype(dtype), _name(""), _dim(0), _alpha(1), _Wsq(0)
  {}


  template <typename D>
  void BaseParameters<D>::scale(const double s) {
    if (s == 0.0) { 
      static_cast<D*>(this)->zero(); 
      return; 
    }
    _alpha *= s;
    const double mag = (_alpha < 0 ? -_alpha : _alpha);
    if(mag < TREELER_PARAMETERS_SCALE_CUTOFF ||
       mag > 1.0/TREELER_PARAMETERS_SCALE_CUTOFF) { 
      static_cast<D*>(this)->recenter(); 
    }
  }



  template <typename D>
  void BaseParameters<D>::save(const std::string& dir, const int t,
					       const std::string& stem) const {
    char* const fname = new char[64 + stem.size() + dir.size()];
    sprintf(fname, "%s/%s.%03d.gz", dir.c_str(), stem.c_str(), t);

    gzFile out = gzopen(fname, "w");

    static_cast<D const*>(this)->save_parameters(out); 

    gzclose(out);
    delete [] fname;
  }

  template <typename D>
  void BaseParameters<D>::load(const std::string& fname) {

  }

  template <typename D>
  void BaseParameters<D>::load(const std::string& fname, const int t,
			       const std::string& stem) {

  }



}

#undef TREELER_PARAMETERS_SCALE_CUTOFF 

#endif /* BASE_PARAMETERS_H */
