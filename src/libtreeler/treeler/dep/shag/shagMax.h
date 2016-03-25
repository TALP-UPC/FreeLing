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
 * \file   shagMax.h
 * \brief  
 * \author 
 */

#ifndef TREELER_SHAGMAX_H
#define TREELER_SHAGMAX_H

#include <limits>
#include <queue>

namespace treeler {
    
  template <typename X, typename Y, typename R> 
  class MaxTT { //FIRST ORDER PARSER
  public:
    class Configuration {
    public:
      int L;
    };
    
    enum Combination_t { NON, LEFT, RIGHT };
    
    #define INI_SUM -numeric_limits<double>::infinity()
    #define INI_MUL 0
    
    class Item;
    
    class Combination {
    public:
      const Item *l, *r;
      Combination_t ct;
      double w;
      
      Combination() {
	l = r = NULL;
	w = INI_MUL;
      }
      
      Combination(const Item *l1, const Item *r1) {
	l = l1;
	r = r1;
	w = INI_MUL;
      }
      
      Combination(const Combination& comb) {
	l = comb.l;
	r = comb.r;
	ct = comb.ct;
	w = comb.w;
      }
      
      inline Combination operator()(Combination_t ct, double sc[3]) {
	//DEFINE +
	this->ct = ct;
	this->w += sc[ct];
	return *this;
      }
    };
    
    class Item {
    public:
      short l, r;
      double w;
      Combination co;
      
      Item() {
	l = r = -1;
	w = INI_SUM;
	co = Combination();
      }
      
      Item(short l1, short r1) {
	l = l1;
	r = r1;
	w = INI_SUM;
	co = Combination();
      }
      
      inline void operator+=(const Combination& comb) {
	//DEFINE MAX
	if (comb.w > w) {
	  co = comb;
	  w = comb.w;
	}
      }
    
      inline Combination operator*(const Item& r) const {
	//DEFINE +
	Combination comb(this, &r);
	comb.w = this->co.w + r.co.w;
	return comb;
      }
    };
    
    template <typename S>
    inline static double score(S& scores, int h, int m) {
      R r(h, m);
      return scores(r);
    }
    
    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      double sc[3]; //score of the combination of 2 known items given a value of Combination_t
      int n = (int)x.size();
      R r;
      Combination comb;
      Item Ta[n+1][n+1], Ti[n+1][n+1];
      for (int i = 0; i < n+1; i++) {
	for (int k = 0; k < i; k++) {
	  Ta[i][k] = Item(k, i);
	  Ti[i][k] = Item(k, i);
	}
	for (int k = i; k < n+1; k++) {
	  Ta[i][k] = Item(i, k);
	  Ti[i][k] = Item(i, k);
	}
      }
      for (int i = 0; i < n+1; i++) Ti[i][i].w = INI_MUL;
      
      for (int width = 1; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  sc[NON] = 0;
	  sc[LEFT] = (k == 0) ? -numeric_limits<double>::infinity() : score(scores, i-1, k-1);
	  sc[RIGHT] = (i == 0) ? -numeric_limits<double>::infinity() : score(scores, k-1, i-1);
	  for (int j = i; j < k; j++) {
	    comb = (Ti[i][j]*Ti[k][j+1])(LEFT, sc);
	    Ta[i][k] += comb;
	    
	    comb = (Ti[i][j]*Ti[k][j+1])(RIGHT, sc);
	    Ta[k][i] += comb;
	  }
	  for (int j = i+1; j <= k; j++) {
	    comb = (Ta[i][j]*Ti[j][k])(NON, sc);
	    Ti[i][k] += comb;
	  }
	  for (int j = i; j < k; j++) {
	    comb = (Ti[j][i]*Ta[k][j])(NON, sc);
	    Ti[k][i] += comb;
	  }
	}
      }
      
      queue<const Item*> q;
      q.push(&Ti[0][n]);
      while (!q.empty()) {
	const Item *i = q.front();
	q.pop();
	int le = i->l;
	int ri = i->r;
	if (le == ri) continue; //Basic Item
	q.push(i->co.l);
	q.push(i->co.r);
	if (i->co.ct != NON) {
	  if (i->co.ct == LEFT) r = R(le-1, ri-1);
	  else r = R(ri-1, le-1);
	  y.push_back(r);
	}
      }
      return Ti[0][n].w;
    }
    #undef INI_SUM
    #undef INI_MUL

  };
  
}

#endif /* TREELER_SHAG_H */
