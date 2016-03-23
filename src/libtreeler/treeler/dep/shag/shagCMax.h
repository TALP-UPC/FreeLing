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
 * \file   shagCMax.h
 * \brief  
 * \author 
 */

#ifndef TREELER_SHAGCMAX_H
#define TREELER_SHAGCMAX_H

#include <queue>
#include <iomanip>
#include <cmath>
#include <limits>
#include <tuple>
#include <unordered_map>

#include "semiring.h"

namespace treeler {
    
  template <typename X, typename Y, typename R> 
  class CMaxTT {
  public:
    class Configuration {
    public:
      int L;
    };
    
    static const int N_COMB_T = 3;
    enum Combination_t { NON=0, LEFT, RIGHT };
    
    typedef Semiring<MAX, Y, R> SR;
    
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
	  Score_t s(scores(r), l);
	  acum += s;
	}
	sc[LEFT] = acum;
      }
      else sc[LEFT] = SR::zero();
      
      if (i != 0) {
	Score_t acum = SR::zero();
	for (int l = 0; l < c.L; l++) {
	  R r(k-1, i-1, l);
	  Score_t s(scores(r), l);
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
    
  template <typename X, typename Y, typename R> 
  class CMaxGrandchildren {
  public:
    class Configuration {
    public:
      int L;
    };
    
    static const int N_COMB_T = 3;
    enum Combination_t { NON=0, LEFT, RIGHT };
    
    #define INI_SUM -numeric_limits<double>::infinity()
    #define INI_MUL 0
    
    class Item;
    
    class Combination {
    public:
      const Item *l, *r;
      Combination_t ct;
      double w;
      int L;
      
      Combination() {
	l = r = NULL;
	w = INI_MUL;
	L = -1;
      }
      
      Combination(const Item *l1, const Item *r1) {
	l = l1;
	r = r1;
	w = INI_MUL;
	L = -1;
      }
      
      Combination(const Combination& comb) {
	l = comb.l;
	r = comb.r;
	ct = comb.ct;
	w = comb.w;
	L = comb.L;
      }
      
      inline Combination operator()(Combination_t ct, double sc[][N_COMB_T]) {
	//DEFINE +
	this->ct = ct;
	this->w += sc[0][ct];
	this->L = sc[1][ct];
	return *this;
      }
    };
    
    enum Item_t { TRI, TRA };
    
    class Item {
    public:
      short l, r;
      short g;
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
      
      Item(short l1, short r1, short g1) {
	l = l1;
	r = r1;
	g = g1;
	w = INI_SUM;
	co = Combination();
      }
      
      inline void operator+=(const Combination& comb) {
	//DEFINE MAX
	if (comb.w > w) {
	  l = comb.l->l;
	  r = comb.r->r;
	  if (comb.ct == NON) {
	    if (comb.l->g < l or comb.l->g > r) g = comb.l->g;
	    else g = comb.r->g;
	  }
	  else if (comb.ct == LEFT) g = comb.l->g;
	  else g = comb.r->g;
	  w = comb.w;
	  co = comb;
	}
      }
    
      inline Combination operator*(const Item& r) const {
	//DEFINE +
	Combination comb(this, &r);
	comb.w = this->co.w + r.co.w;
	return comb;
      }
    };
    
    typedef tuple<int, int> Key;
    
    struct TripletHash_lookup3 {
      size_t operator()(const Key& t) const {
	uint32_t a = get<0>(t);
	uint32_t b = get<1>(t);
	uint32_t c = 0;
	#define rot(x, k) ((x << k) | (x >> (32 - k)))

	/* initial stage; mix (a, b, c) */
	a -= c; a ^= rot(c, 4);  c += b;
	b -= a; b ^= rot(a, 6);  a += c;
	c -= b; c ^= rot(b, 8);  b += a;
	a -= c; a ^= rot(c, 16); c += b;
	b -= a; b ^= rot(a, 19); a += c;
	c -= b; c ^= rot(b, 4);  b += a;
	/* final stage; (a, b, c) => c */
	c ^= b; c -= rot(b, 14);
	a ^= c; a -= rot(c, 11);
	b ^= a; b -= rot(a, 25);
	c ^= b; c -= rot(b, 16);
	a ^= c; a -= rot(c, 4);
	b ^= a; b -= rot(a, 14);
	c ^= b; c -= rot(b, 24);

	#undef rot
	return c;
	
      }
    };
    
    typedef unordered_map<Key, Item, TripletHash_lookup3> Chart;
    
    template <typename S>
    inline static double score2(S& scores, int g, int h, int m) {
      R r;
      double res = 0;
      if (h != -1) {
	if (h < m) {
	  if (g < h) {
	    r = R(R::CMO, g, h, m);
	    res += scores(r);
	  }
	  else {
	    r = R(R::CMI, g, h, m);
	    res += scores(r);
	  }
	}
	else {
	  if (g > h) {
	    r = R(R::CMO, g, h, m);
	    res += scores(r);
	  }
	  else {
	    r = R(R::CMI, g, h, m);
	    res += scores(r);
	  }
	}
      }
      return res;
    }
    
    template <typename S>
    inline static void score(const Configuration& c, S& scores, int i, int k, const Key& t, double sc[][N_COMB_T]) {
      double max;
      int amax;
      R r;
      
      int g = get<1>(t);
      
      sc[0][NON] = 0;
      sc[1][NON] = -1;
      
      if (k != 0) {
	double base = score2(scores, g-1, i-1, k-1);
	max = -numeric_limits<double>::infinity();
	amax= -1;
	for (int l = 0; l < c.L; l++) {
	  r = R(R::FO, i-1, k-1, -1);
	  double s = base + scores(r);
	  if (s > max) {
	    max = s;
	    amax = l;
	  }
	}
	sc[0][LEFT] = max;
	sc[1][LEFT] = amax;
      }
      else sc[0][LEFT] = -numeric_limits<double>::infinity();
      
      if (i != 0) {
	double base = score2(scores, g-1, k-1, i-1);
	max = -numeric_limits<double>::infinity();
	amax= -1;
	for (int l = 0; l < c.L; l++) {
	  r = R(R::FO, k-1, i-1, -1);
	  double s = base + scores(r);
	  if (s > max) {
	    max = s;
	    amax = l;
	  }
	}
	sc[0][RIGHT] = max;
	sc[1][RIGHT] = amax;
      }
      else sc[0][RIGHT] = -numeric_limits<double>::infinity();
    }
    
    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      double sc[2][N_COMB_T]; //score of the combination of 2 known items given a value of Combination_t (+Label)
      int n = (int)x.size();
      Chart chart[n+1][n+1];
      R r;
      Combination comb;
      
      chart[0][0][Key(TRI, 0)] = Item(0, 0, 0);
      for (int i = 1; i < n+1; i++) for (int g = 0; g < n+1; g++) if (i != g) chart[i][i][Key(TRI, g)] = Item(i, i, g);
      for (int width = 1; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  for (int g = 0; g < n+1; g++) {
	    if (i == 0 and g != 0) break;
	    if (g == i and i != 0) { g = k; continue; }
	    score(c, scores, i, k, Key(-1, g), sc);
	      
	    for (int j = i; j < k; j++) {
	      comb = (chart[i][j][Key(TRI, g)]*chart[k][j+1][Key(TRI, i)])(LEFT, sc);
	      chart[i][k][Key(TRA, g)] += comb;
	    }
	    for (int j = i; j < k; j++) {
	      comb = (chart[i][j][Key(TRI, k)]*chart[k][j+1][Key(TRI, g)])(RIGHT, sc);
	      chart[k][i][Key(TRA, g)] += comb;
	    }
	    for (int j = i+1; j <= k; j++) {
	      comb = (chart[i][j][Key(TRA, g)]*chart[j][k][Key(TRI, i)])(NON, sc);
	      chart[i][k][Key(TRI, g)] += comb;
	    }
	    for (int j = i; j < k; j++) {
	      comb = (chart[j][i][Key(TRI, k)]*chart[k][j][Key(TRA, g)])(NON, sc);
	      chart[k][i][Key(TRI, g)] += comb;
	    }
	  }
	}
      }
      
      queue<const Item*> q;
      q.push(&chart[0][n][Key(TRI, 0)]);
      while (!q.empty()) {
	const Item *i = q.front();
	q.pop();
	int le = i->l;
	int ri = i->r;
	if (le == ri) continue; //Basic Item
	q.push(i->co.l);
	q.push(i->co.r);
	if (i->co.ct != NON) {
	  if (i->co.ct == LEFT) r = R(R::FO, le-1, ri-1, i->co.L);
	  else r = R(R::FO, ri-1, le-1, i->co.L);
	  y.push_back(r);
	}
      }
      return chart[0][n][Key(TRI, 0)].w;
    }
    
    #undef INI_SUM
    #undef INI_MUL
    
  };
  
}

#endif /* TREELER_SHAG_H */
