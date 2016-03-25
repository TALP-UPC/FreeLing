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
 * \file   learn-utils.tcc
 * \brief  Implementation of a number of boilerplate utilities for
 *         optimization algorithms. Will likely disappear from treeler.
 * \author Terry Koo
 */

/* detailed name of the trainer */
#define myname								\
  "LearnUtils[" << M.name() << "]"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "treeler/io/io-conll.h"
#include "treeler/util/timer.h"

using namespace std;
namespace treeler {

  template <typename X, typename Y>
  const Example<X,Y>& LearnUtils::randselect(const randomization_t t,
					     const int i,
					     DataSet<X,Y>& data) {
    const int N = data.size();
    /* produce a random value in [0, 1) */
    const double rand_01 = ((double)random())/(1.0 + (double)RAND_MAX);
    if(t == RAND_UNIFORM) {
      /* in this case, just pick an example at random */
      const double rand_0n = N*rand_01;
      const int rand_idx = (int)rand_0n;
      return *data[rand_idx];
    }
    else if(t == RAND_PERMUTATION) {
      /* in this case, compute a random permutation on-the-fly by
	 picking a random example out of [i, n) and swapping it
	 with the current example (i.e., the Knuth shuffle). */
      const double rand_in = i + (N - i)*rand_01;
      const int rand_idx = (int)rand_in;
      /* swap and store the random selection in x */
      std::swap(data[i], data[rand_idx]);
      // const Example<X,R>* ex = data[rand_idx];
      // data[rand_idx] = data[i];
      // data[i] = ex;
      return *data[i];
    } else if(t == DETERMINISTIC) {
      /* deterministic ascending order */
      return *data[i];
    }
    
    cerr << "unrecognized randomization type "
      	 << t << endl;
    assert(0);
  }

  template <typename X, typename R>
  int LearnUtils::count_matches_sparse(const Example<X,R>* const ex,
				       const Label<R>& yhat) {   
     /* now count the number of matches */
     int match = 0;
     // Label<PartDep1>
     typename Label<R>::const_iterator yi = (*ex->y()).begin();
     const typename Label<R>::const_iterator y_end = (*ex->y()).end();
     typename Label<R>::const_iterator yhi = yhat.begin();
     const typename Label<R>::const_iterator yh_end = yhat.end();
     while (yi!=y_end and yhi!=yh_end) {
       if (*yi == *yhi) {
 	  match++;
	  ++yi; ++yhi;
      }
      else if (*yi < *yhi) {
  	  ++yi;
      }
      else {
	  ++yhi;
      }
    }
    return match;
  }

  // template <typename X,typename R>
  // int LearnUtils::hamming_loss_sparse(StructuredModel<X,R>& M,
  // 				      const Example<X,R>* const ex,
  // 				      const Label<R>& yhat) {
  //   int loss = 0;

  //   typename Label<R>::const_iterator yi = (*ex->y()).begin();
  //   const typename Label<R>::const_iterator y_end = (*ex->y()).end();
  //   typename Label<R>::const_iterator yhi = yhat.begin();
  //   const typename Label<R>::const_iterator yh_end = yhat.end();

  //   while (yi!=y_end and yhi!=yh_end) {
  //     if (*yi == *yhi) {
  // 	++yi; ++yhi;
  //     }
  //     else if (*yi < *yhi) {
  // 	// loss += M.loss_penalty(*ex, *yi);
  // 	++yi;
  //     }
  //     else {
  // 	loss += M.loss_penalty(*ex, *yhi);
  // 	++yhi;
  //     }
  //   }

  //   //    while (yi != y_end) {
  //   //      loss += M.loss_penalty(ex, *yi);
  //   //      ++yi;
  //   //    }

  //   while (yhi != yh_end) {
  //     loss += M.loss_penalty(*ex, *yhi);
  //     ++yhi;
  //   }
  //   return loss;
  // }



  // template <typename X, typename R, typename Traits >
  // void LearnUtils::lazy_classify_val_set_obsolete(const Parameters& V,
  // 						  const DataSet<X,R>& data,
  // 						  StructuredModel<X,R>& M,
  // 						  const string& dir,
  // 						  const int t,
  // 						  const string& stem) {
  //   // cerr << myname << " : decoding validation set : " << flush;
  //   //timer_start();
    
  //   /* open two output files */
  //   char* buf = new char[64 + stem.size()];
  //   sprintf(buf, "%s.%03d", stem.c_str(), t);
  //   string maxfilename = dir + "/" + buf;
  //   delete [] buf;
       
  //   /* use these to compute the part-wise precision and recall */
  //   int correct_parts = 0;
  //   int max_matching_parts = 0;
  //   int max_predicted_parts = 0;
  //   int hamming_tot = 0;
  //   int max_hamming_sum = 0;
         
  //   const int N = data.size();
  //   int cnt = 0;
  //   typename DataSet<X,R>::const_iterator it = data.begin();
  //   const typename DataSet<X,R>::const_iterator it_end = data.end();
  //   for(; it != it_end; ++it) {
  //     const Example<X,R>* ex = *it;
      
  //     const X* x  = static_cast<const X*>(ex->x());
  //     const Label<R>* y = ex->y();
      
  //     correct_parts += y->size();
  //     hamming_tot += ex->x()->size();
      
  //     /* find the argmax (viterbi) label */
  //     Label<R> ymax;
  //     M.fgen().phi_start_pattern(*x);
  //     M.argmax(*x, V, ymax);
  //     M.fgen().phi_end_pattern(*x);
  //     /// this needs to be rewritten for generic X
  
  //     Traits::IO::write(cerr, *x, ymax);
      
  //     max_predicted_parts += ymax.size();
  //     max_matching_parts += count_matches_sparse<X,R>(ex, ymax);
  //     max_hamming_sum += hamming_loss_sparse<X,R>(M, ex, ymax);
      
  //     ++cnt;
  //     ticker(cnt, N);
  //   }
  //   ticker(N, N);
    
  //   //timer_stop();
  //   //cerr << " done [" << timestr(timediff) << "]" << endl;
  //   assert(hamming_tot > 0);
  //   const double maxH = 100.0 - (100.0*(double)max_hamming_sum/hamming_tot);
  //   cerr << myname << " : argmax hamming score = "
  // 	 << fixed << setprecision(2) << maxH << endl;
  //   assert(correct_parts > 0);
  //   assert(max_predicted_parts > 0);
  //   const double maxP =
  //     100.0*(double)max_matching_parts/(double)max_predicted_parts;
  //   const double maxR =
  //     100.0*(double)max_matching_parts/(double)correct_parts;
  //   cerr << myname << " : argmax prec/rec = "
  // 	 << fixed << setprecision(2)
  // 	 << maxP << " " << maxR << endl;
  // }

}

#undef myname
