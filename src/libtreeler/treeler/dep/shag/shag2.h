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
 * \file   shag2.h
 * \brief  
 * \author 
 */

#ifndef TREELER_SHAG2_H
#define TREELER_SHAG2_H

#include <limits>
#include <queue>

namespace treeler {
    
  template <typename X, typename Y, typename R> 
  class Grandchildren {
  public:
    class Configuration {
    public:
      int L;
    };
    
    class Item {
    public:
      double w;
      short g;
      Item *left, *right;
      short j, ot;
      
      Item() {
      }
      
      Item(double ww, short gg) {
	w = ww; g = gg;
      }
    };
    
    typedef tuple<int,int,int> Tri;
/*
    struct TripletHash {
      size_t operator()(const Tri& t) const {
	hash<int> H;
	size_t h = H(get<0>(t));
	h = h ^ H(get<1>(t));
	return h ^ H(get<2>(t));
      }
    };
*/
    struct TripletHash_lookup3 {
      size_t operator()(const Tri& t) const {
	uint32_t a = get<0>(t);
	uint32_t b = get<1>(t);
	uint32_t c = get<2>(t);
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

    typedef unordered_map<Tri, Item, TripletHash_lookup3> Chart;
    
    template <typename S>
    inline static double score(S& scores, int g, int h, int m) {
      if (m == -1) return -numeric_limits<double>::infinity();
      R r(R::FO, h, m, -1);
      double res = scores(r);
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
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      int n = (int)x.size();
      R r;
      double nw;
      Item *litem, *ritem, *citem;
      Chart ta, ti;
      typename Chart::iterator citer;
      ti[Tri(0, 0, 0)] = Item(0, 0);
      for (int i = 1; i < n+1; i++) for (int g = 0; g < n+1; g++) if (i != g) ti[Tri(i, i, g)] = Item(0, g);
      for (int width = 1; width < n+1; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  for (int g = 0; g < n+1; g++) {
	    if (i == 0 and g != 0) break;
	    if (g == i and i != 0) { g = k; continue; }
	    double sc[3];
	    sc[0] = 0;
	    sc[1] = score(scores, g-1, i-1, k-1);
	    sc[2] = score(scores, g-1, k-1, i-1);
	    #define combine(o, t, a, b) \
	      citer = t.find(Tri(a, b, g)); \
	      if (citer == t.end()) citem = &t[Tri(a, b, g)]; \
	      else citem = &(citer->second); \
	      nw = litem->w + ritem->w + sc[o]; \
	      if (citer == t.end() or nw > citem->w) { \
		citem->w = nw; \
		citem->g = g; \
		citem->left = litem; \
		citem->right = ritem; \
		citem->j = j; \
		citem->ot = o; \
	      }
	      
	    for (int j = i; j < k; j++) {
	      litem = &ti[Tri(i, j, g)];
	      ritem = &ti[Tri(k, j+1, i)];
	      combine(1, ta, i, k);
	      
	      litem = &ti[Tri(i, j, k)];
	      ritem = &ti[Tri(k, j+1, g)];
	      combine(2, ta, k, i);
	    }
	    for (int j = i+1; j <= k; j++) {
	      litem = &ta[Tri(i, j, g)];
	      ritem = &ti[Tri(j, k, i)];
	      combine(0, ti, i, k);
	    }
	    for (int j = i; j < k; j++) {
	      litem = &ti[Tri(j, i, k)];
	      ritem = &ta[Tri(k, j, g)];
	      combine(0, ti, k, i);
	    }
	    #undef combine
	  }
	}
      }
      typedef pair<int, int> LR;
      typedef pair<Item*, LR> PI;
      queue<PI> q;
      q.push(PI(&ti[Tri(0, n, 0)], LR(0, n)));
      while (!q.empty()) {
	PI it = q.front();
	q.pop();
	int le = it.second.first;
	int ri = it.second.second;
	if (le == ri) continue; //Basic Item
	Item *i = it.first;
	if (i->ot == 0) {
	  q.push(PI(i->left, LR(le, i->j)));
	  q.push(PI(i->right, LR(i->j, ri)));
	}
	else {
	  q.push(PI(i->left, LR(le, i->j)));
	  q.push(PI(i->right, LR(i->j + 1, ri)));
	  if (le == 0) r = R(R::FO, le-1, ri-1, -1);
	  else if (i->ot == 1) {
	    if (1 or i->g == 0) r = R(R::FO, le-1, ri-1, -1);
	    else if (i->g < le) r = R(R::CMO, i->g-1, le-1, ri-1);
	    else r = R(R::CMI, i->g-1, le-1, ri-1);
	  }
	  else {
	    if (1 or i->g == 0) r = R(R::FO, ri-1, le-1, -1);
	    else if (i->g > ri) r = R(R::CMO, i->g-1, ri-1, le-1);
	    else r = R(R::CMI, i->g-1, ri-1, le-1);
	  }
	  y.push_back(r);
	}
      }
      return ti[Tri(0, n, 0)].w;
    }
    
/*
    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      int n = (int)x.size();
      R r;
      Item Ta[n+1][n+1][n+1], Ti[n+1][n+1][n+1]; //LLISTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
      for (int i = 0; i < n+1; i++) for (int j = 0; j < n+1; j++) for (int g = 0; g < n+1; g++) {
	Ta[i][j][g].w = -1.0/0.0;
	Ti[i][j][g].w = -1.0/0.0;
      }
      Ti[0][0][0].w = 0;
      Ti[0][0][0].g = 0;
      for (int i = 1; i < n+1; i++) for (int g = 0; g < n+1; g++) if (i != g) Ti[i][i][g].w = 0, Ti[i][i][g].g = g;
      for (int width = 1; width < n+1; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  for (int g = 0; g < n+1; g++) {
	    if (i == 0 and g != 0) break;
	    if (g == i and i != 0) { g = k; continue; }
	    double sgik, sgki;
	    sgik = score(scores, g-1, i-1, k-1);
	    sgki = score(scores, g-1, k-1, i-1);
	    for (int j = i; j < k; j++) {
	      double nw = Ti[i][j][g].w + Ti[k][j+1][i].w + sgik;
	      if (nw > Ta[i][k][g].w) {
		#define it Ta[i][k][g]
		it.w = nw;
		it.g = g;
		it.left = &Ti[i][j][g];
		it.right = &Ti[k][j+1][i];
		it.j = j;
		it.ot = 1;
		#undef it
	      }
	      nw = Ti[i][j][k].w + Ti[k][j+1][g].w + sgki;
	      if (nw > Ta[k][i][g].w) {
		#define it Ta[k][i][g]
		it.w = nw;
		it.g = g;
		it.left = &Ti[i][j][k];
		it.right = &Ti[k][j+1][g];
		it.j = j;
		it.ot = 2;
		#undef it
	      }
	    }
	    for (int j = i+1; j <= k; j++) {
	      double nw = Ta[i][j][g].w + Ti[j][k][i].w;
	      if (nw > Ti[i][k][g].w) {
		#define it Ti[i][k][g]
		it.w = nw;
		it.g = g;
		it.left = &Ta[i][j][g];
		it.right = &Ti[j][k][i];
		it.j = j;
		it.ot = 0;
		#undef it
	      }
	    }
	    for (int j = i; j < k; j++) {
	      double nw = Ti[j][i][k].w + Ta[k][j][g].w;
	      if (nw > Ti[k][i][g].w) {
		#define it Ti[k][i][g]
		it.w = nw;
		it.g = g;
		it.left = &Ti[j][i][k];
		it.right = &Ta[k][j][g];
		it.j = j;
		it.ot = 0;
		#undef it
	      }
	    }
	  }
	}
      }
      typedef pair<int, int> LR;
      typedef pair<Item*, LR> PI;
      queue<PI> q;
      q.push(PI(&Ti[0][n][0], LR(0, n)));
      while (!q.empty()) {
	PI it = q.front();
	q.pop();
	int le = it.second.first;
	int ri = it.second.second;
	if (le == ri) continue; //Basic Item
	Item *i = it.first;
	if (i->ot == 0) {
	  q.push(PI(i->left, LR(le, i->j)));
	  q.push(PI(i->right, LR(i->j, ri)));
	}
	else {
	  q.push(PI(i->left, LR(le, i->j)));
	  q.push(PI(i->right, LR(i->j + 1, ri)));
	  if (i->ot == 1) {
	    if (1 or i->g == 0) r = R(R::FO, le-1, ri-1, -1);
	    else if (i->g < le) r = R(R::CMO, i->g-1, le-1, ri-1);
	    else r = R(R::CMI, i->g-1, le-1, ri-1);
	  }
	  else {
	    if (1 or i->g == 0) r = R(R::FO, ri-1, le-1, -1);
	    else if (i->g > ri) r = R(R::CMO, i->g-1, ri-1, le-1);
	    else r = R(R::CMI, i->g-1, ri-1, le-1);
	  }
	  y.push_back(r);
	}
      }
      return Ti[0][n][0].w;
    }
    */
  };
}

#endif /* TREELER_SHAG_H */
