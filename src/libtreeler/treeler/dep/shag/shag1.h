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
 * \file   shag1.h
 * \brief  
 * \author 
 */

#ifndef TREELER_SHAG1_H
#define TREELER_SHAG1_H

#include <queue>
#include <iomanip>
#include <cmath>
#include <limits>
#include <tuple>
#include <unordered_map>

namespace treeler {
  
  template <typename X, typename Y, typename R> 
  class Boxes {
  public:
    class Configuration {
    public:
      int L;
    };
  
    struct Item {
      double w;
      Item *left, *right;
      int j, ot;
      bool simple;
    };

    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      int n = (int)x.size();
      R r;
      Item chart[n+1][n+1][3];
      for (int i = 0; i < n+1; i++) {
	for (int j = 0; j < n+1; j++) {
	  for (int k = 0; k < 3; k++) chart[i][j][k].w = -numeric_limits<double>::infinity();
	}
      }
      for (int i = 0; i < n; i++) {
	chart[i][i+1][0].w = 0;
	chart[i][i+1][0].ot = 0;
	chart[i][i+1][0].left = chart[i][i+1][0].right = NULL;
	chart[i][i+1][0].simple = 1;
	r = R(i-1, i); //ROOT = -1
	chart[i][i+1][1].w = scores(r);
	chart[i][i+1][1].ot = 1;
	chart[i][i+1][1].left = chart[i][i+1][1].right = NULL;
	chart[i][i+1][1].simple = 1;
	r = R(i, i-1);
	chart[i][i+1][2].w = (i==0) ? -numeric_limits<double>::infinity() : scores(r);
	chart[i][i+1][2].ot = 2;
	chart[i][i+1][2].left = chart[i][i+1][2].right = NULL;
	chart[i][i+1][2].simple = 1;
      }
      for (int width = 2; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  double sik, ski;
	  r = R(i-1, k-1);
	  sik = scores(r);
	  r = R(k-1, i-1);
	  ski = (i==0) ? -numeric_limits<double>::infinity() : scores(r);
	  for (int j = i+1; j <= k-1; j++) {
	    #define le chart[i][j]
	    #define ri chart[j][k]
	    #define set_chart(m, o, bl, br) chart[i][k][m].w = nw; \
				    chart[i][k][m].j = j; \
				    chart[i][k][m].ot = o; \
				    chart[i][k][m].left = &le[bl]; \
				    chart[i][k][m].right = &ri[br]; \
				    chart[i][k][m].simple = (o != 0)
	    if (le[0].simple) {
	      double nw = le[0].w + ri[2].w;
	      if (nw > chart[i][k][0].w) {
		set_chart(0, 0, 0, 2);
		nw = chart[i][k][0].w + sik;
		set_chart(1, 1, 0, 2);
		nw = chart[i][k][0].w + ski;
		set_chart(2, 2, 0, 2);
	      }
	    }
	    if (le[1].simple) {
	      double nw = le[1].w + ri[0].w;
	      if (nw > chart[i][k][0].w) {
		set_chart(0, 0, 1, 0);
		nw = chart[i][k][0].w + sik;
		set_chart(1, 1, 1, 0);
		nw = chart[i][k][0].w + ski;
		set_chart(2, 2, 1, 0);
	      }
	      nw = le[1].w + ri[1].w;
	      if (nw > chart[i][k][1].w) {
		set_chart(1, 0, 1, 1);
	      }
	    }
	    if (le[2].simple) {
	      double nw = le[2].w + ri[2].w;
	      if (nw > chart[i][k][2].w) {
		set_chart(2, 0, 2, 2);
	      }
	    }
	    #undef set_chart
	    #undef ri
	    #undef le
	  }
	}
      }
      typedef pair<int, int> LR;
      typedef pair<Item*, LR> PI;
      queue<PI> q;
      q.push(PI(&chart[0][n][1], LR(0, n)));
      while (!q.empty()) {
	PI it = q.front();
	q.pop();
	int le = it.second.first;
	int ri = it.second.second;
	Item *i = it.first;
	if (i->left != NULL) { // NOT size-2 span
	  q.push(PI(i->left, LR(le, i->j)));
	  q.push(PI(i->right, LR(i->j, ri)));
	}
	if (i->ot == 0) continue;
	if (i->ot == 1) r = R(le-1, ri-1);
	else r = R(ri-1, le-1);
	y.push_back(r);
      }
      return chart[0][n][1].w;
    }
  };
  
  template <typename X, typename Y, typename R> 
  class TriTra {
  public:
    class Configuration {
    public:
      int L;
    };
  
    struct Item {
      double w;
      Item *left, *right;
      int j, ot;
    };

    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      int n = (int)x.size();
      R r;
      Item Ta[n+1][n+1], Ti[n+1][n+1];
      for (int i = 0; i < n+1; i++) {
	for (int j = 0; j < n+1; j++) {
	  Ta[i][j].w = -numeric_limits<double>::infinity();
	  Ti[i][j].w = -numeric_limits<double>::infinity();
	}
      }
      for (int i = 0; i < n+1; i++) Ti[i][i].w = 0;
      for (int width = 1; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  double sik, ski;
	  r = R(i-1, k-1);
	  sik = scores(r);
	  r = R(k-1, i-1);
	  ski = (i==0) ? -numeric_limits<double>::infinity() : scores(r);
	  for (int j = i; j < k; j++) {
	    double nw = Ti[i][j].w + Ti[k][j+1].w + sik;
	    if (nw > Ta[i][k].w) {
	      Ta[i][k].w = nw;
	      Ta[i][k].j = j;
	      Ta[i][k].ot = 1;
	      Ta[i][k].left = &Ti[i][j];
	      Ta[i][k].right = &Ti[k][j+1];
	    }
	    nw = Ti[i][j].w + Ti[k][j+1].w + ski;
	    if (nw > Ta[k][i].w) {
	      Ta[k][i].w = nw;
	      Ta[k][i].j = j;
	      Ta[k][i].ot = 2;
	      Ta[k][i].left = &Ti[i][j];
	      Ta[k][i].right = &Ti[k][j+1];
	    }
	  }
	  for (int j = i; j <= k; j++) {
	    double nw = Ta[i][j].w + Ti[j][k].w;
	    if (nw > Ti[i][k].w) {
	      Ti[i][k].w = nw;
	      Ti[i][k].j = j;
	      Ti[i][k].ot = 0;
	      Ti[i][k].left = &Ta[i][j];
	      Ti[i][k].right = &Ti[j][k];
	    }
	    nw = Ti[j][i].w + Ta[k][j].w;
	    if (nw > Ti[k][i].w) {
	      Ti[k][i].w = nw;
	      Ti[k][i].j = j;
	      Ti[k][i].ot = 0;
	      Ti[k][i].left = &Ti[j][i];
	      Ti[k][i].right = &Ta[k][j];
	    }
	  }
	}
      }
      typedef pair<int, int> LR;
      typedef pair<Item*, LR> PI;
      queue<PI> q;
      q.push(PI(&Ti[0][n], LR(0, n)));
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
	  if (i->ot == 1) r = R(le-1, ri-1);
	  else r = R(ri-1, le-1);
	  y.push_back(r);
	}
      }
      return Ti[0][n].w;
    }
  
  };
  
}

#endif /* TREELER_SHAG1_H */
