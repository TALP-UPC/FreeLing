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
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   scores.h
 * \brief  A number of generic score components
 * \author Xavier Carreras
 */

#ifndef TREELER_SCORES_H
#define TREELER_SCORES_H

#include "treeler/base/parameters.h"
#include "treeler/base/label.h"
#include "treeler/base/example.h"
#include "treeler/io/io-fvec.h"
#include <string>
#include <list>
#include <iostream>
#include <cassert>

namespace treeler {

  // CRTP 
  template <typename Derived> 
  class BaseScores {
  public:
    //    typedef typename Derived::R R; 

    template <typename R>
    inline double operator()(Label<R> const & y) {
      double s = 0; 
      for (auto r=y.begin(); r!=y.end(); ++r) {
	s += (*static_cast<Derived>(this))(*r);
      }
      return s;
    }

    template <typename R>
    inline double score(const Label<R>& y) {
      double s = 0; 
      for (auto r=y.begin(); r!=y.end(); ++r) {
	s += (*static_cast<Derived*>(this))(*r);
      }
      return s;
    }

  };


  /** 
   * \brief  Constant scores: return a constant score for any part
   * \author Xavier Carreras
   * \ingroup base
   */
  template <typename X, typename R>
  class ConstantScores : public BaseScores<ConstantScores<X,R>> {
  private:
    const double _s; 
    bool  _random; 
  public:

    //! \brief Constructor with default score set to 1
    ConstantScores(const X& x) : _s(1.0), _random(false) {}

    //! \brief Constructor specifying the score to be returned
    ConstantScores(const X& x, double s) : _s(s), _random(false) {}

    //! \brief Sets the score of a part
    void set_score(double s) { _s = s; }

    //! \brief Sets the score to be random or not 
    void set_random(bool r) { _random = r; }

    //! \brief Scores given part 
    template <typename ...A>
    double operator()(A&&... args) const {
      if (_random) {
	double sign = +1; 
	if (rand()%2==1) sign=-1; 
	return (double)rand() * sign/ (double) RAND_MAX; 
      }
      else 
	return _s;
    }

    class Scorer {
    private:
      const double _s; 
    public:
      Scorer() : _s(1) {};
      Scorer(double s) : _s(s) {};
      
      void scores(const X& x, ConstantScores<X,R>& scores) const {
	scores.set_score(_s); 
      }
      void scores(const Example<X,Label<R>>& xy, ConstantScores<X,R>& scores) const {
	scores.set_score(_s); 
      }
    };
  };
  


  /** 
   * \brief  Inference methods that return an empty solution for any input
   * \author Xavier Carreras
   * \ingroup base
   */
  template <typename X, typename Y, typename R> 
  class NullInference {
  public:
    class Configuration {};

    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      return 0;
    }
  };


}

#include "treeler/base/oracle-scores.h"
#include "treeler/base/wf-scores.h"

#endif /* TREELER_SCORES_H */
