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
 * \file   learn-utils.h
 * \brief  Class from taken from egstra containing a number of boilerplate utilities for
 *         optimization algorithms. Will likely disappear from treeler.
 * \author Xavier Carreras, Terry Koo
 */


#ifndef LEARN_LEARNUTILS_H
#define LEARN_LEARNUTILS_H

#include "treeler/base/example.h"
#include "treeler/base/dataset.h"

#include "treeler/base/label.h"
#include "treeler/base/sentence.h"
#include "treeler/dep/part-dep1.h"
#include "treeler/io/io-conll.h"

#include <vector>
#include <string>



namespace treeler {

  template <typename X, typename Y>
    class LazyClassifyTraits {
  public:
      class IO {
      public:
	static void write(ostream& o, const X& x, const Y& y) {
	  cerr << "(default LazyClassifyTraits: call to write(o, x, y))" << endl;
	}
      };      
  };
  
  
  class LearnUtils {
  public:

    enum randomization_t {
      RAND_UNIFORM, 
      RAND_PERMUTATION, 
      DETERMINISTIC
    };
    
    /* convert a rand-type to a string description */
#define RAND_LONG_DESCRIPTION(randtype)                               \
    (randtype == LearnUtils::RAND_UNIFORM			\
     ? "random with replacement"				\
     : (randtype == LearnUtils::RAND_PERMUTATION		\
	? "random permutations"					\
	: (randtype == LearnUtils::DETERMINISTIC		\
	   ? "deterministic (cyclic)"				\
	   : "<unknown randomization, error!>")))		\
      
    /* convert a rand-type to an abbreviation */
#define RAND_SHORT_DESCRIPTION(randtype)					\
    (randtype == LearnUtils::RAND_UNIFORM ? "R"				\
     : (randtype == LearnUtils::RAND_PERMUTATION ? "P"			\
	: (randtype == LearnUtils::DETERMINISTIC ? "D"			\
	   : "<error>")))                  
    
    
    /* choose the i'th random example from the dataset, based on the
       randomization type */
    template <typename X, typename Y>
      static const Example<X,Y>& randselect(const LearnUtils::randomization_t t,
					    const int i,
					    DataSet<X,Y> & data);
    /* template <typename X,typename R> */
    /*   static int hamming_loss_sparse(StructuredModel<X,R>& M, */
    /* 				     const Example<X,R>* const ex, */
    /* 				     const Label<R>& yhat); */

    template <typename X, typename R>
    static int count_matches_sparse(const Example<X,R>* const ex,
				    const Label<R>& yhat);



    /* template <typename X, typename R, typename Traits = LazyClassifyTraits<X,Label<R>> > */
    /* static void lazy_classify_val_set_obsolete(const Parameters& V, */
    /* 					       const DataSet<X,R>& data, */
    /* 					       StructuredModel<X,R>& M, */
    /* 					       const string& dir, */
    /* 					       const int t, */
    /* 					       const string& stem);  */
  };
}

#include "treeler/algo/learn-utils.tcc"

#endif /* LEARN_LEARNUTILS_H */
