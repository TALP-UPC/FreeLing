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
 * \brief  Declaration of sparse parameters class
 * \author Xavier Carreras, Terry Koo
 * \note   This file was ported from egstra
 */

#ifndef TREELER_SPARSE_PARAMETERS_H
#define TREELER_SPARSE_PARAMETERS_H

#include "treeler/base/base-parameters.h"
#include "treeler/base/feature-vector.h"
#include "treeler/util/simple-hash-table.h"

#include <string>



namespace treeler {


  /***
   * \brief A sparse parameter vector
   *      
   * The class is templated on a value type Val.  Despite the variable
   * value type, the return values of the operations (e.g., dot
   * product) will always be double.  However, the internals will use
   * arithmetic on Val, allowing a potentially faster implementation.
   *
   *
   * The parameters are stored internally as a hashtable, which is
   * appropriate for sparse parameter vectors. 
   *
   */
  template <typename FIdx, typename Val = double>
  class SparseParameters : public BaseParameters< SparseParameters<FIdx,Val> > {
  public:
    typedef BaseParameters<SparseParameters<FIdx,Val>> BaseType; 
    typedef FeatureVector<FIdx> FVec; 

  private:

    typedef typename parameters_internal::Traits<FIdx>::Hash HashOp; 
    typedef typename parameters_internal::Traits<FIdx>::Eq EqOp; 
    typedef simple_hasht<FIdx, Val, HashOp, EqOp> hashtable; 

    /* number of spaces */
    size_t _nspaces;

    /* hashtable of parameter values */
    hashtable* _hasht;

    
  public:
    /* create zeroed parameters */
    SparseParameters(const size_t nspaces);

    /* destroy and delete memory */
    ~SparseParameters();


    /* return the number of nonzero parameters */
    size_t nnz() const;

    /* store/read to file */
    void save_parameters(gzFile out) const;

    // void load(const std::string& fname);
    //    void load(const std::string& fname, const int t,
    //	      const std::string& stem);

    /* return the inner product W . F */
    double dot(const FVec& F) const;

    /* return the inner product W . V */
    // template <typename P>
    // double dot(const P& V) const;

    // /* single-element access */
    // double get(const dim_int i) const;

    // /* find the max/min/max-magnitude parameter value and index */
    // double max(dim_int& argmax) const;
    // double min(dim_int& argmin) const;
    // double maxmag(dim_int& argmax) const;
    // /* find the k largest/smallest/largest-magnitude parameter values
    //    and indices, starting from the most superlative values */
    // void kmax(const int k, std::vector<double>& values,
    // 	      std::vector<dim_int>& indices) const;
    // void kmin(const int k, std::vector<double>& values,
    // 	      std::vector<dim_int>& indices) const;
    // void kmaxmag(const int k, std::vector<double>& values,
    // 		 std::vector<dim_int>& indices) const;


    /* perform W = 0 */
    void zero();

    /* perform W[i] += d */
    void add(const size_t offset, const size_t i, const Val& d);

    /* perform W += scale * F */
    void add(const FVec& F, const double scale);

    /* rescale the internal representation so that _alpha = 1.0 and
       recompute the norm _Wsq */
    void recenter();


    /* perform W += scale * V */
    //    void add(const parameters* const V, const double scale);

    /* perform W[i] *= \exp{scale * V[i]} for all indices i (i.e., a
       log-space additive update) */
    // void expmul(const parameters* const V, const double scale);

    // /* implementation of parameter averaging */
    // void avg_init();
    // void avg_flush();
  };


  // IMPLEMENTATION


/* (optional) parameter index bounds-checking */
#if 1
#define CHECK_INDEX(mac_i, mac_d) {             \
    assert((mac_i) >= 0);                       \
    assert((mac_i) < (mac_d));                  \
  }
#else
#define CHECK_INDEX(mac_i, mac_d)
#endif

/* (optional) deletion of zeroed parameters: it's best to leave this
   active.  although this involves additional checking, it turns out
   that at least for perceptron training, the reductions in hashtable
   size are worthwhile.  since the hashtable is less-populated, it
   consumes (slightly) less memory and there are also fewer hash
   collisions, so the code runs (slightly) faster.  */
#if 1
#define DELETE_ZERO(mac_k, mac_i, mac_v) {	\
    if((mac_v) == 0) { _hasht[mac_k].del((mac_i)); }   \
  }
#else
#define DELETE_ZERO(mac_k, mac_i, mac_v)
#endif



  template <typename FIdx, typename Val>
  SparseParameters<FIdx,Val>::SparseParameters(const size_t nspaces) 
    : BaseType(0,0), _nspaces(nspaces)
  {
    assert(nspaces>0);
    _hasht = new hashtable[_nspaces];
  }

  template <typename FIdx, typename Val>
  SparseParameters<FIdx,Val>::~SparseParameters() 
  {
    delete [] _hasht;
  }

  /* return the number of nonzero parameters */
  template <typename FIdx, typename Val>
  size_t SparseParameters<FIdx,Val>::nnz() const {
    size_t cnt = 0;
    size_t index = 0;
    Val value = 0;
    for (size_t k = 0; k<_nspaces; ++k) {
      typename hashtable::const_enumerator e = _hasht[k].begin();
      while(e.next(index, value)) {
	if(value != 0) { ++cnt; }
      }
    }
    return cnt;
  }
  
  /* return the inner product W . F */
  template <typename FIdx, typename Val>
  double SparseParameters<FIdx,Val>::dot(const FVec& f) const {
    Val ret = 0; /* inner product value */
    for(const FVec* fp = &f; fp != NULL; fp = fp->next) {
      // CHECK_OFFSET(fp->offset);
      const hashtable& ht = _hasht[fp->offset]; /* select parameter space */
      const FIdx* fi = fp->idx;
      const FIdx* const fend = fi + fp->n;
      const double* fv = fp->val;
      const Val zero = 0;
      /* NB: use .val parameter value */
      if(fv == NULL) { /* indicator features */
	for(; fi < fend; ++fi)  { ret += ht.get(*fi, zero); }
      } else { /* real-valued features */
	for(; fi < fend; ++fi, ++fv) { ret += ht.get(*fi, zero)*(*fv); }
      }
    }
    
    return ret;
  };


  template <typename FIdx, typename Val>
  void SparseParameters<FIdx,Val>::zero() {
    BaseType::_alpha = 1.0; 
    BaseType::_Wsq = 0.0; 
    for (size_t k = 0; k<_nspaces; ++k) _hasht[k].clear();
  }

  /* perform W[i] += d */
  template <typename FIdx, typename Val>
  void SparseParameters<FIdx,Val>::add(const size_t offset, const size_t i, const Val& d) {
    Val* const pv = _hasht[offset].pgetput(i, 0);
    const Val cur = *pv;
    if(BaseType::_alpha == 1.0) {
      *pv += (Val)d;
    } else if(BaseType::_alpha == -1.0) {
      *pv -= (Val)d;
    } else {
      assert(BaseType::_alpha != 0.0);
      *pv += (Val)(d/BaseType::_alpha);
    }
    const Val v = *pv;
    BaseType::_Wsq += (double)(v*v - cur*cur);
    DELETE_ZERO(offset, i, v);

  }


  
  /* perform W += scale * F */
  template <typename FIdx, typename Val>
  void SparseParameters<FIdx,Val>::add(const FVec& fv, const double scale) {
    assert(BaseType::_alpha != 0.0);
    if(scale == 0.0) { return; }

    const double realscale = scale/BaseType::_alpha;
    for(const FVec* f = &fv; f != NULL; f = f->next) {
      const int n = f->n;
      const size_t k = f->offset;
      const FIdx* const fidx = f->idx;

      const double* const fval = f->val;
      if(fval == NULL) { /* indicators */
        if(realscale == 1.0) {
          //    cerr << _name << " : add 1.0 * F" << endl;
          /* use the formula:
             (w_i + 1)^2 = w_i^2 + 2*w_i + 1
             = w_i^2 + 2*(w_i + 1) - 1
             summing across all i, the update to w^2 is:
             \sum_i [(w_i + 1)^2 - w_i^2]
             = \sum_i [2*(w_i + 1) - 1]
             = 2*\sum_i [w_i + 1] - n */
          Val sum_w = 0;
          for(int i = 0; i < n; ++i) {
            const size_t idx = fidx[i];
	    //            CHECK_INDEX(idx, _dim);
            Val* const pv = _hasht[k].pgetput(idx, 0);
            const Val newval = (++(*pv));
            sum_w += newval;
            DELETE_ZERO(k, idx, newval);
          }
          BaseType::_Wsq += 2*sum_w - n;
        } else if(realscale == -1.0) {
          //    cerr << _name << " : add -1.0 * F" << endl;
          /* use the formula:
             (w_i - 1)^2 = w_i^2 - 2*w_i + 1
             = w_i^2 - 2*(w_i - 1) - 1
             summing across all i, the update to w^2 is:
             \sum_i [(w_i - 1)^2 - w_i^2]
             = \sum_i [-2*(w_i - 1) - 1]
             = -2*\sum_i [w_i - 1] - n */
          Val sum_w = 0;
          for(int i = 0; i < n; ++i) {
            const dim_int idx = fidx[i];
	    //            CHECK_INDEX(idx, _dim);
            Val* const pv = _hasht[k].pgetput(idx, 0);
            const Val newval = (--(*pv));
            sum_w += newval;
            DELETE_ZERO(k, idx, newval);
          }
          BaseType::_Wsq -= 2*sum_w + n;
        } else {
          //    cerr << _name << " : add scale * F" << endl;
          /* use the formula:
             (w_i + x)^2 = w_i^2 + 2*x*w_i + x^2
             = w_i^2 + 2*x*(w_i + x) - x^2
             summing across all i, the update to w^2 is:
             \sum_i [(w_i + x)^2 - w_i^2]
             = \sum_i [2*x*(w_i + x) - x^2]
             = 2*x*\sum_i [w_i + x] - n*x^2 */
          const Val x = (Val)realscale;
          assert(x == realscale); /* check precision */
          Val sum_w = 0;
          for(int i = 0; i < n; ++i) {
            const dim_int idx = fidx[i];
            // CHECK_INDEX(idx, _dim);
            Val* const pv = _hasht[k].pgetput(idx, 0);
            const Val newval = (*pv += x);
            sum_w += newval;
            DELETE_ZERO(k, idx, newval);
          }
          BaseType::_Wsq += 2*x*sum_w - n*x*x;
        }
     } else { /* full feature vector */
        double Wsq_adjust = 0;
        for(int i = 0; i < n; ++i) {
          const dim_int idx = fidx[i];
          // CHECK_INDEX(idx, _dim);
          const double x_i = realscale*(fval[i]);
          Val* const pv = _hasht[k].pgetput(idx, 0);
          const double w_i = (double)(*pv);
          const double newval = w_i + x_i;
          *pv = (Val)newval;
          /* use the formula:
             (w_i + x_i)^2 - w_i^2 = 2*x_i*w_i + x_i^2 */
          Wsq_adjust += x_i*(newval + w_i);
          DELETE_ZERO(k, idx, newval);
        }
        BaseType::_Wsq += Wsq_adjust;
      }
    }
  }
  

  template <typename FIdx, typename Val>
  void SparseParameters<FIdx,Val>::recenter() {
    const Val vscale = (Val)BaseType::_alpha;
    assert(vscale == BaseType::_alpha); /* check precision */
    if(vscale == 1) {
      //      cerr << _name << " : recenter() 1.0" << endl;
      Val value = 0;
      Val wsq = 0;
      for (size_t k = 0; k<_nspaces; ++k) {
	typename hashtable::const_enumerator e = _hasht[k].begin();
	while(e.next(value)) {
	  wsq += value*value;
	}
      }
      BaseType::_Wsq = (double)wsq;
    } 
    else if(vscale == -1) {
      //      cerr << _name << " : recenter() -1.0" << endl;
      Val* pv = NULL;
      Val wsq = 0;
      for (size_t k = 0; k<_nspaces; ++k) {
	typename hashtable::enumerator e = _hasht[k].mbegin();
	while((pv = e.next()) != NULL) {
	  const Val value = *pv;
	  wsq += value*value;
	  *pv = -value;
	}
      }
      BaseType::_Wsq = (double)wsq;
    } 
    else {
      assert(vscale != 0);
      //      cerr << _name << " : recenter() alpha" << endl;
      /* rescale and accumulate W^2 */
      Val* pv = NULL;
      Val wsq = 0;
      for (size_t k = 0; k<_nspaces; ++k) {
	typename hashtable::enumerator e = _hasht[k].mbegin();
	while((pv = e.next()) != NULL) {
	  const Val value = vscale*(*pv);
	  wsq += value*value;
	  *pv = value;
	}
      }
      BaseType::_Wsq = (double)wsq;
    }
    BaseType::_alpha = 1.0;
  }


  template <typename FIdx, typename Val>
  void SparseParameters<FIdx,Val>::save_parameters(gzFile out) const {
    gzprintf(out, "%lu\n", _nspaces);    
    FIdx f;
    Val v;
    int nnz = 0;
    //    int cnt = 0;
    for(size_t k = 0; k < _nspaces; ++k) {
      typename hashtable::const_enumerator e = _hasht[k].begin();
      while(e.next(f, v)) {
	if(v != 0) { /* only write nonzero parameters */
	  ++nnz;
	  parameters_internal::Traits<FIdx>::fwrite(out, f); 	  
	  // feature_idx_fwrite(out, f.fidx);
	  gzprintf(out, " %.16g\n", v);
	  //	  if(((++cnt) & 0xfffff) == 0) { cerr << "." << flush; }
	}
      }
      /* separate different feature spaces with blank lines */
      gzprintf(out, "\n");
    }

  }

#undef CHECK_INDEX
#undef DELETE_ZERO


}




#endif /* TREELER_SPARSE_PARAMETERS_H */
