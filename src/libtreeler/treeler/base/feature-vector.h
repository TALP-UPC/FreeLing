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
 * \file   feature-vector.h
 * \brief  Declares the class FeatureVector
 * \author Terry Koo, Xavier Carreras
 * \note   This file was ported from egstra
 */

#ifndef BASE_FEATUREVECTOR_H
#define BASE_FEATUREVECTOR_H

#include <string>
#include <sstream>
#include "treeler/base/fidx.h"
#include <iostream>
#include <list>

using namespace std;

namespace treeler {

  /** 
   * \brief A sparse feature vector
   * \ingroup base
   * \author Xavier Carreras, Terry Koo
   *
   * FeatureVector is a plain struct for storing sparse feature
   * vectors, i.e. a list of feature-value pairs.  It supports two
   * types of features:
   * 
   * - *indicator features*, where the value of each feature is
   *   assumed to be 1. 
   *
   * - *real-valued features*, where each feature index is paired with
   *   a \c double value.
   *
   * The feature vector is implemented as two plain arrays of size \c
   * n: an array \c idx storing feature indices and an array \c val
   * storing values. For indicator features, \c val is set to \c NULL.
   *  
   * The main purpose of a feature vector is to compute the dot
   * product with a parameter vector \c W. In some applications the
   * parameter space where \c W operates is built as a (Kronecker)
   * product between a number of *parameter blocks* and the feature
   * space defined by feature indices. This is, \c W defines a logical
   * coordinate for each pair <tt><offset,index></tt> where \c offset
   * is a parameter block and \c index is a feature index, i.e. a
   * parameter value is at <tt>W[offset][index]</tt>. To support this
   * type of representations, FeatureVector includes a field \c
   * offset, of integer type, indicating the parameter block. A
   * classic example is multiclass classification, where one defines a
   * parameter block for each possible label.
   * 
   * When a feature vector represents a complex object, sometimes
   * several parameter blocks need to be used simultaneously. For
   * example, in dependency parsing one typically defines a feature
   * space to represent tokens, and then defines two separate
   * parameter spaces to represent the head token and the modifier
   * token of a dependency, using the same feature indices (associated
   * to token features) in each parameter space. To support these
   * representations, FeatureVector allows to concatenate several
   * structs, using a linked list (field \c next).
   *
   * This implementation is oriented towards having a cache of arrays
   * of feature indices (and values if necessary). Hence, many feature
   * generators will precompute feature indices fo the objects that
   * need to be represented. Then, on demand, they will create a
   * FeatureVector by making \c idx point to the appropriate array of
   * indices (cached somewhere), and setting the appropriate offset.
   *
   *
   *  The struct contains the following fields:
   *
   *  - \c n: the number of active features
   *
   *  - \c idx: A plain array of feature indices of type FIdx
   * 
   *  - \c val: A plain array of double values, aligned to \c idx. If \c NULL
   *    then indicator features are assumed
   * 
   *  - \c offset: a logical offset in the feature space
   *
   *  - \c next: a pointer to the next FeatureVector, or \c NULL if final. 
   *
   *
   * \todo \c offset should be \c unsigned \c int 
   * \todo implement an iterator of <tt><offset,index></tt> logical pairs
   * 
   */
  template<typename FIdx = FIdxV0>
  class FeatureVector {
  public: 
    int n;                     //!< Number of active features 
    const FIdx* idx;           //!< List of feature indices 
    const double* val;         //!< List of feature values. If this is NULL,
                               //!< then indicator features are assumed 
    int offset;                //!< A *logical* offset in the feature space.
			       //!< this is used to select different parameter
			       //!< spaces; thus (offset,index) is interpreted
			       //!< as corresponding to W[offset][index] 
    FeatureVector* next;       //!< A pointer to the next FeatureVector, or \c NULL if final
  };


  template <typename FIdx> 
  struct CollectFIdx {
  public:
    std::list<FIdx> idxlist;
    
    void operator()(FIdx& i) {
      idxlist.push_back(i);
    }
    
    size_t size() const { return idxlist.size(); }
    
    FIdx* create_array() const {
      FIdx* idx = new FIdx[idxlist.size()];
      FIdx* d = idx;
      for (auto s = idxlist.begin(); s!=idxlist.end(); ++s, ++d) {
	*d = *s;
      }
      return idx; 
    }
  };


}

#endif /* BASE_FEATUREVECTOR_H */
