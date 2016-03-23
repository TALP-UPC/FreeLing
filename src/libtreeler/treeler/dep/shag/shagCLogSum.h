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
 * \file   shagCLogSum.h
 * \brief  
 * \author 
 */

#ifndef TREELER_SHAGCLOGSUM_H
#define TREELER_SHAGCLOGSUM_H

#include <queue>
#include <iomanip>
#include <cmath>
#include <limits>
#include <unordered_map>

#include "semiring.h"

namespace treeler {
    
  template <typename X, typename Y, typename R> 
  class CLogSumTT {
  public:
    class Configuration {
    public:
      int L;
    };
    
    static const int N_COMB_T = 3;
    enum Combination_t { NON=0, LEFT, RIGHT };
    
    typedef Semiring<LOGADD, Y, R> SR;
    
    typedef typename SR::Item Item;
    typedef typename SR::Combination Combination;
    typedef typename SR::Score_t Score_t;
    
    typedef pair<Item, Item> Chart;
    
    template <typename S>
    inline static void score(const Configuration& c, S& scores, int i, int k, Score_t sc[]) {
      sc[NON] = SR::one();
      
      if (k != 0) {
	Score_t acum = SR::zero();
	for (int l = 0; l < c.L; l++) {
	  R r(i-1, k-1, l);
	  Score_t s(scores(r));
	  acum += s;
	}
	sc[LEFT] = acum;
      }
      else sc[LEFT] = SR::zero();
      
      if (i != 0) {
	Score_t acum = SR::zero();
	for (int l = 0; l < c.L; l++) {
	  R r(k-1, i-1, l);
	  Score_t s(scores(r));
	  acum += s;
	}
	sc[RIGHT] = acum;
      }
      else sc[RIGHT] = SR::zero();
    }
    
    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      Score_t sc[N_COMB_T]; //score of the combination of 2 known items given a value of Combination_t (+Label)
      int n = (int)x.size();
      Chart chart[n+1][n+1];
      
      for (int i = 0; i < n+1; i++) {
	for (int k = 0; k < n+1; k++) chart[i][k] = Chart(Item(i, k), Item(i, k));
      }
      for (int i = 0; i < n+1; i++) chart[i][i].first += Combination();
      
      for (int width = 1; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  score(c, scores, i, k, sc);
	  for (int j = i; j < k; j++) {
	    Combination comb = (chart[i][j].first*chart[k][j+1].first)(LEFT, sc);
	    chart[i][k].second += comb;
	  }
	  for (int j = i; j < k; j++) {
	    Combination comb = (chart[i][j].first*chart[k][j+1].first)(RIGHT, sc);
	    chart[k][i].second += comb;
	  }
	  for (int j = i+1; j <= k; j++) {
	    Combination comb = (chart[i][j].second*chart[j][k].first)(NON, sc);
	    chart[i][k].first += comb;
	  }
	  for (int j = i; j < k; j++) {
	    Combination comb = (chart[j][i].first*chart[k][j].second)(NON, sc);
	    chart[k][i].first += comb;
	  }
	}
      }
      
      return chart[0][n].first.value(y);
    }
    
  };
  
}

#endif /* TREELER_SHAG_H */
