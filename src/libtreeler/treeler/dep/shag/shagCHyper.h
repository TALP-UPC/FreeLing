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
 * \file   shagCHyper.h
 * \brief  
 * \author 
 */

#ifndef TREELER_SHAGCHYPER_H
#define TREELER_SHAGCHYPER_H

#include <queue>
#include <iomanip>
#include <cmath>
#include <limits>
#include <list>
#include <tuple>
#include <unordered_map>

namespace treeler {
    
  template <typename X, typename Y, typename R> 
  class CHyperTT {
  public:
    class Configuration {
    public:
      int L;
    };
    
    static const int N_COMB_T = 3;
    enum Combination_t { NON=0, LEFT, RIGHT };
    
    #define INI_MUL 0
    
    class Item;
    
    class Combination {
    public:
      const Item *l, *r;
      Combination_t ct;
      double *w;
      static int _L;
      
      Combination() {
	l = r = NULL;
	w = new double[_L];
	for (int l = 0; l < _L; l++) w[l] = INI_MUL;
      }
      
      Combination(const Item *l1, const Item *r1) {
	l = l1;
	r = r1;
	w = new double[_L];
	for (int l = 0; l < _L; l++) w[l] = INI_MUL;
      }
      
      Combination(const Combination& comb) {
	l = comb.l;
	r = comb.r;
	ct = comb.ct;
	w = new double[_L];
	for (int l = 0; l < _L; l++) w[l] = comb.w[l];
      }
      
      inline Combination operator()(Combination_t ct, double sc[][N_COMB_T]) {
	//DEFINE +
	this->ct = ct;
	for (int l = 0; l < _L; l++) this->w[l] += sc[l][ct];
	return *this;
      }
    };
    
    class Item {
    public:
      short l, r;
      list<Combination> co;
      
      Item() {
	l = r = -1;
	co = list<Combination>();
      }
      
      Item(short l1, short r1) {
	l = l1;
	r = r1;
	co = list<Combination>();
      }
      
      inline void operator+=(const Combination& comb) {
	//DEFINE INSERT
	co.push_back(comb);
      }
    
      inline Combination operator*(const Item& r) const {
	//DEFINE CONSTRUCTION
	Combination comb(this, &r);
	return comb;
      }
    };
    
    typedef pair<Item, Item> Chart;
    
    template <typename S>
    inline static void score(S& scores, int i, int k, double sc[][N_COMB_T]) {
      for (int l = 0; l < Combination::_L; l++) sc[l][NON] = 0;
      
      if (k != 0) {
	for (int l = 0; l < Combination::_L; l++) {
	  R r(i-1, k-1, l);
	  sc[l][LEFT] = scores(r);
	}
      }
      else {
	for (int l = 0; l < Combination::_L; l++) sc[l][LEFT] = -numeric_limits<double>::infinity();
      }
      
      if (i != 0) {
	for (int l = 0; l < Combination::_L; l++) {
	  R r(k-1, i-1, l);
	  sc[l][RIGHT] = scores(r);
	}
      }
      else {
	for (int l = 0; l < Combination::_L; l++) sc[l][RIGHT] = -numeric_limits<double>::infinity();
      }
    }
    
    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      double sc[c.L][N_COMB_T]; //score of the combination of 2 known items given a value of Combination_t (+Label)
      Combination::_L = c.L;
      int n = (int)x.size();
      Chart chart[n+1][n+1];
      R r;
      Combination comb;
      for (int i = 0; i < n+1; i++) {
	for (int k = 0; k < i; k++) chart[i][k] = Chart(Item(k, i), Item(k, i));
	for (int k = i; k < n+1; k++) chart[i][k] = Chart(Item(i, k), Item(i, k));
      }
      
      for (int width = 1; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  score(scores, i, k, sc);
	  for (int j = i; j < k; j++) {
	    comb = (chart[i][j].first*chart[k][j+1].first)(LEFT, sc);
	    chart[i][k].second += comb;
	  }
	  for (int j = i; j < k; j++) {
	    comb = (chart[i][j].first*chart[k][j+1].first)(RIGHT, sc);
	    chart[k][i].second += comb;
	  }
	  for (int j = i+1; j <= k; j++) {
	    comb = (chart[i][j].second*chart[j][k].first)(NON, sc);
	    chart[i][k].first += comb;
	  }
	  for (int j = i; j < k; j++) {
	    comb = (chart[j][i].first*chart[k][j].second)(NON, sc);
	    chart[k][i].first += comb;
	  }
	}
      }
      
      int result = 1;
      if (result == 0) { /* RECOVER THE MAX VALUE */
	pair<double, double> score_t[n+1][n+1];
	for (int i = 0; i < n+1; i++) {
	  for (int j = 0; j < n+1; j++) {
	    score_t[i][j].first = -numeric_limits<double>::infinity();
	    score_t[i][j].second = -numeric_limits<double>::infinity();
	  }
	}
	for (int i = 0; i < n+1; i++) {
	  score_t[i][i].first = 0.0;
	  score_t[i][i].second = 0.0;
	}
	for (int width = 1; width <= n; width++) {
	  for (int i = 0; i < n+1-width; i++) {
	    int k = i + width;
	    typename list<Combination>::iterator list_iter;
	    #define l_i list_iter->l
	    #define r_i list_iter->r
	    for (list_iter = chart[i][k].second.co.begin(); list_iter != chart[i][k].second.co.end(); list_iter++) {
	      double items_score = score_t[l_i->l][l_i->r].first + score_t[r_i->r][r_i->l].first;
	      for (int l = 0; l < Combination::_L; l++) {
		double aux_score = items_score + list_iter->w[l];
		if (aux_score > score_t[i][k].second) score_t[i][k].second = aux_score;
	      }
	    } 
	    for (list_iter = chart[k][i].second.co.begin(); list_iter != chart[k][i].second.co.end(); list_iter++) {
	      double items_score = score_t[l_i->l][l_i->r].first + score_t[r_i->r][r_i->l].first;
	      for (int l = 0; l < Combination::_L; l++) {
		double aux_score = items_score + list_iter->w[l];
		if (aux_score > score_t[k][i].second) score_t[k][i].second = aux_score;
	      }
	    }
	    //When building triangles the score of the union proces is always 0
	    for (list_iter = chart[i][k].first.co.begin(); list_iter != chart[i][k].first.co.end(); list_iter++) {
	      double aux_score = score_t[l_i->l][l_i->r].second + score_t[r_i->l][r_i->r].first;
	      if (aux_score > score_t[i][k].first) score_t[i][k].first = aux_score;
	    }
	    for (list_iter = chart[k][i].first.co.begin(); list_iter != chart[k][i].first.co.end(); list_iter++) {
	      double aux_score = score_t[l_i->r][l_i->l].first + score_t[r_i->r][r_i->l].second;
	      if (aux_score > score_t[k][i].first) score_t[k][i].first = aux_score;
	    }
	    #undef l_i
	    #undef r_i
	  }
	}
	return score_t[0][n].first;
      }
      else if (result == 1) { /* RECOVER THE ADD VALUE */
	pair<double, double> score_t[n+1][n+1];
	pair<double, double> nt_t[n+1][n+1]; //n_trees
	for (int i = 0; i < n+1; i++) {
	  for (int j = 0; j < n+1; j++) {
	    score_t[i][j].first = 0.0;
	    score_t[i][j].second = 0.0;
	    nt_t[i][i].first = 0.0;
	    nt_t[i][i].second = 0.0;
	  }
	}
	for (int i = 0; i < n+1; i++) {
	  score_t[i][i].first = 0.0;
	  score_t[i][i].second = 0.0;
	  nt_t[i][i].first = 1.0;
	  nt_t[i][i].second = 1.0;
	}
	for (int width = 1; width <= n; width++) {
	  for (int i = 0; i < n+1-width; i++) {
	    int k = i + width;
	    typename list<Combination>::iterator list_iter;
	    #define l_i list_iter->l
	    #define r_i list_iter->r
	    for (list_iter = chart[i][k].second.co.begin(); list_iter != chart[i][k].second.co.end(); list_iter++) {
	      double items_score = nt_t[r_i->r][r_i->l].first*score_t[l_i->l][l_i->r].first
				 + nt_t[l_i->l][l_i->r].first*score_t[r_i->r][r_i->l].first;
	      double combs_score = 0.0;
	      for (int l = 0; l < Combination::_L; l++) combs_score += list_iter->w[l];
	      score_t[i][k].second += Combination::_L*items_score + nt_t[l_i->l][l_i->r].first*nt_t[r_i->r][r_i->l].first*combs_score;
	      nt_t[i][k].second += Combination::_L*nt_t[l_i->l][l_i->r].first*nt_t[r_i->r][r_i->l].first;
	    }
	    for (list_iter = chart[k][i].second.co.begin(); list_iter != chart[k][i].second.co.end(); list_iter++) {
	      double items_score = nt_t[r_i->r][r_i->l].first*score_t[l_i->l][l_i->r].first
				 + nt_t[l_i->l][l_i->r].first*score_t[r_i->r][r_i->l].first;
	      double combs_score = 0.0;
	      for (int l = 0; l < Combination::_L; l++) combs_score += list_iter->w[l];
	      score_t[k][i].second += Combination::_L*items_score + nt_t[l_i->l][l_i->r].first*nt_t[r_i->r][r_i->l].first*combs_score;
	      nt_t[k][i].second += Combination::_L*nt_t[l_i->l][l_i->r].first*nt_t[r_i->r][r_i->l].first;
	    }
	    //When building triangles the score of the union proces is always 0
	    for (list_iter = chart[i][k].first.co.begin(); list_iter != chart[i][k].first.co.end(); list_iter++) {
	      double items_score = nt_t[r_i->l][r_i->r].first*score_t[l_i->l][l_i->r].second
				 + nt_t[l_i->l][l_i->r].second*score_t[r_i->l][r_i->r].first;
	      score_t[i][k].first += items_score;
	      nt_t[i][k].first += nt_t[l_i->l][l_i->r].second*nt_t[r_i->l][r_i->r].first;
	    }
	    for (list_iter = chart[k][i].first.co.begin(); list_iter != chart[k][i].first.co.end(); list_iter++) {
	      double items_score = nt_t[r_i->r][r_i->l].second*score_t[l_i->r][l_i->l].first
				 + nt_t[l_i->r][l_i->l].first*score_t[r_i->r][r_i->l].second;
	      score_t[k][i].first += items_score;
	      nt_t[k][i].first += nt_t[l_i->r][l_i->l].first*nt_t[r_i->r][r_i->l].second;
	    }
	    #undef l_i
	    #undef r_i
	  }
	}
	return score_t[0][n].first;
      }
      else if (result == 2) { /* RECOVER THE LOGSUM VALUE */
	#define logsum(a, b) if (a > b) a = a + log(1 + exp(b - a)); \
	else a = b + log(1 + exp(a - b))
	pair<double, double> score_t[n+1][n+1];
	for (int i = 0; i < n+1; i++) {
	  for (int j = 0; j < n+1; j++) {
	    score_t[i][j].first = -numeric_limits<double>::infinity();
	    score_t[i][j].second = -numeric_limits<double>::infinity();
	  }
	}
	for (int i = 0; i < n+1; i++) {
	  score_t[i][i].first = 0.0;
	  score_t[i][i].second = 0.0;
	}
	for (int width = 1; width <= n; width++) {
	  for (int i = 0; i < n+1-width; i++) {
	    int k = i + width;
	    typename list<Combination>::iterator list_iter;
	    #define l_i list_iter->l
	    #define r_i list_iter->r
	    for (list_iter = chart[i][k].second.co.begin(); list_iter != chart[i][k].second.co.end(); list_iter++) {
	      double items_score = score_t[l_i->l][l_i->r].first + score_t[r_i->r][r_i->l].first;
	      double combs_score = -numeric_limits<double>::infinity();
	      for (int l = 0; l < Combination::_L; l++) {
		logsum(combs_score, list_iter->w[l]);
	      }
	      double total_score = items_score + combs_score;
	      logsum(score_t[i][k].second, total_score);
	    }
	    for (list_iter = chart[k][i].second.co.begin(); list_iter != chart[k][i].second.co.end(); list_iter++) {
	      double items_score = score_t[l_i->l][l_i->r].first + score_t[r_i->r][r_i->l].first;
	      double combs_score = -numeric_limits<double>::infinity();
	      for (int l = 0; l < Combination::_L; l++) {
		logsum(combs_score, list_iter->w[l]);
	      }
	      double total_score = items_score + combs_score;
	      logsum(score_t[k][i].second, total_score);
	    }
	    //When building triangles the score of the union proces is always 0
	    for (list_iter = chart[i][k].first.co.begin(); list_iter != chart[i][k].first.co.end(); list_iter++) {
	      double items_score = score_t[l_i->l][l_i->r].second + score_t[r_i->l][r_i->r].first;
	      logsum(score_t[i][k].first, items_score);
	    }
	    for (list_iter = chart[k][i].first.co.begin(); list_iter != chart[k][i].first.co.end(); list_iter++) {
	      double items_score = score_t[l_i->r][l_i->l].first + score_t[r_i->r][r_i->l].second;
	      logsum(score_t[k][i].first, items_score);
	    }
	    #undef l_i
	    #undef r_i
	  }
	}
	return score_t[0][n].first;
      }
      else return 0;
    }
    
    #undef INI_MUL
    
  };
  template <typename X, typename Y, typename R> int CHyperTT<X, Y, R>::Combination::_L;
  
}

#endif /* TREELER_SHAG_H */
