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
 * \file   objectives.h
 * \brief  Declaration of loss-regularized objectives for learning
 * \author Terry Koo, adapted by Xavier Carreras for Treeler
 */

#ifndef TREELER_LEARNING_OBJECTIVES_H
#define TREELER_LEARNING_OBJECTIVES_H


namespace treeler {

  /**
   *
   * Obj is expected to provide the following methods
   *
   *   Obj::exprimal(example* ex, double* S, structured_model M)
   *   Obj::exgrad(parameters g, double scale, example* ex,
   *               double* S, fvec* F, structured_model M)
   *     These return the example-wise contribution to the primal
   *     objective and the primal gradient
   **/


#define TREELER_OBJECTIVEFUNCTION_PRIMAL_TOL 1e-8

  struct LogLinearObjective {
  public:
    
    static string name() { return "LogLinear"; }
    
    template<typename X>
    static void check_objective(const X& x, double& o) {
#ifdef TREELER_OBJECTIVEFUNCTION_PRIMAL_TOL 
      /* check the consistency of the log-probability */                        \
      assert(isfinite(o));                                                      \
      if(o > 0) {                                                               \
	if(o > TREELER_OBJECTIVEFUNCTION_PRIMAL_TOL/2) {                        \
	  cerr << "*** warning: gold_logp = " << scientific << setprecision(15) \
	       << o << " at pattern " << x.id()                                 \
	       << " (N=" << x.size() << ")"                                     \
	       << endl;                                                         \
	}                                                                       \
	assert(o <= TREELER_OBJECTIVEFUNCTION_PRIMAL_TOL);                      \
	/* truncate the log-probability to zero */                              \
	o = 0;                                                                  \
      }                                                                         
#endif
    }

    template <typename X, typename R, typename I, typename S>
    static double objective(const Example<X,Label<R>>& ex, 
			    I& inference,
			    S& scores) {
      double gold_score = scores.score(ex.y());
      double logz = inference.partition(ex.x(),scores); 
      const double gold_logp = gold_score - logz;
      check_objective(ex.x(), gold_logp);
      return gold_logp;
    }

    template <typename W, typename X, typename R, typename I, typename S, typename F>
    static double gradient(W& g,
			   const double scale,
			   const Example<X,Label<R>>& ex,
			   I& inference,
			   S& sco,
			   F& f) {
      const X& x = ex.x();
      const Label<R>& y = ex.y();

      /* compute the marginal probabilities */
      typename R::template map<double> mu; 
      const double log_Z = inference.marginals(x, sco, mu);
      
      double gold_score = 0; 
      auto rend  = y.end();
      for (auto r = y.begin(); r != rend; ++r) {
	gold_score += sco(*r);
	auto fv = f(*r);
	g.add(*fv, scale); 
	f.discard(fv);
      }

      for (auto r = inference.rbegin(x); r != inference.rend(); ++r) {
	const double mul = scale * mu(*r);
	auto fv = f(*r);
	g.add(*fv, -mul); 
	f.discard(fv);
      }
      double gold_logp = gold_score - log_Z;
      check_objective(x, gold_logp);            

      /* the objective is scaled */
      return scale*gold_logp;
    } // gradient
    
  };

#undef TREELER_OBJECTIVEFUNCTION_PRIMAL_TOL

}
#endif
