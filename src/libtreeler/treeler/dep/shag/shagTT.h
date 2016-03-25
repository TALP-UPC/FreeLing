#ifndef TREELER_SHAGTT_H
#define TREELER_SHAGTT_H

#include "semiring.h"
#include "chart.h"

namespace treeler {
    
  template <typename X, typename Y, typename R, Semiring_t SR_t> 
  class ShagTT {
  public:
    class Configuration {
    public:
      int L;
    };
    
    static const int N_COMB_T = 3;
    enum Edge_t { NON=0, LEFT, RIGHT };
    
    static const Semiring_t _SR_t = SR_t;
    typedef Semiring<SR_t> SR;
    typedef typename SR::Item Item;
    typedef typename SR::Edge Edge;
    typedef typename SR::Score_t Score_t;
    
    template <typename S>
    inline static void score(const Configuration& c, S& scores, int i, int k, Score_t sc[]) {
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
    static void inside(const Configuration& c, const X& x, S& scores, Chart<Item>& inside) {
      Score_t sc[N_COMB_T]; //score of the combination of 2 known items given a value of Edge_t (+Label)
      sc[NON] = SR::one();
      int n = (int)x.size();
      
      for (int i = 0; i < n+1; i++)
	for (int k = i; k < n+1; k++)
	  for (int p = 0; p < nItem_t; p++)
	    inside(i, k, (Item_t)p).setKey(Key(i, k, (Item_t)p));
	  
      inside(0, 0, RTRI).add(new Edge());
      for (int i = 1; i < n+1; i++) {
	inside(i, i, RTRI).add(new Edge());
	inside(i, i, LTRI).add(new Edge());
      }
      for (int width = 1; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  score(c, scores, i, k, sc);
	  typename Chart<Item>::Pack& pack = inside(i, k);
	  for (int j = i; j < k; j++) {
	    Edge *edge = new Edge(inside(i, j, RTRI), inside(j+1, k, LTRI), sc[LEFT]);
	    get<RTRA>(pack).add(edge);
	    edge = new Edge(inside(i, j, RTRI), inside(j+1, k, LTRI), sc[RIGHT]);
	    get<LTRA>(pack).add(edge);
	  }
	  for (int j = i+1; j <= k; j++) {
	    Edge *edge = new Edge(inside(i, j, RTRA), inside(j, k, RTRI), sc[NON]);
	    get<RTRI>(pack).add(edge);
	  }
	  for (int j = i; j < k; j++) {
	    Edge *edge = new Edge(inside(i, j, LTRI), inside(j, k, LTRA), sc[NON]);
	    get<LTRI>(pack).add(edge);
	  }
	}
      }
    }
    
    template <typename S>
    static void outside(const Configuration& c, const X& x, S& scores, const Chart<Item>& inside, Chart<Item>& outside) {
      Score_t sc[N_COMB_T]; //score of the combination of 2 known items given a value of Edge_t (+Label)
      sc[NON] = SR::one();
      int n = (int)x.size();
      
      for (int i = 0; i < n+1; i++)
	for (int k = i; k < n+1; k++)
	  for (int p = 0; p < nItem_t; p++)
	    outside(i, k, (Item_t)p).setKey(Key(i, k, (Item_t)p));
	  
      outside(0, n, RTRI).add(new Edge());
      outside(0, n, RTRA).add(new Edge());
      
      //es criden scores iguals més de una vegada
      for (int width = n-1; width >= 0; width--) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  typename Chart<Item>::Pack& pack = outside(i, k);
	  //(i, k, RTRI)
	  for (int j = k+1; j <= n; j++) {
	    score(c, scores, i, j, sc);
	    Edge *edge = new Edge(outside(i, j, RTRA), inside(k+1, j, LTRI), sc[LEFT]);
	    get<RTRI>(pack).add(edge);
	    edge = new Edge(outside(i, j, LTRA), inside(k+1, j, LTRI), sc[RIGHT]);
	    get<RTRI>(pack).add(edge);
	  }
	  for (int j = 0; j < i; j++) {
	    Edge *edge = new Edge(outside(j, k, RTRI), inside(j, i, RTRA), sc[NON]);
	    get<RTRI>(pack).add(edge);
	  }
	  //(i, k, LTRI)
	  for (int j = 0; j < i; j++) {
	    score(c, scores, j, i, sc);
	    Edge *edge = new Edge(outside(j, k, RTRA), inside(j, i-1, RTRI), sc[LEFT]);
	    get<LTRI>(pack).add(edge);
	    edge = new Edge(outside(j, k, LTRA), inside(j, i-1, RTRI), sc[RIGHT]);
	    get<LTRI>(pack).add(edge);
	  }
	  for (int j = k+1; j <= n; j++) {
	    Edge *edge = new Edge(outside(i, j, LTRI), inside(k, j, LTRA), sc[NON]);
	    get<LTRI>(pack).add(edge);
	  }
	  if (i != k) {
	    //(i, k, RTRA)
	    for (int j = k; j <= n; j++) {
	      Edge *edge = new Edge(outside(i, j, RTRI), inside(k, j, RTRI), sc[NON]);
	      get<RTRA>(pack).add(edge);
	    }
	    //(i, k, LTRA)
	    for (int j = 0; j <= i; j++) {
	      Edge *edge = new Edge(outside(j, k, LTRI), inside(j, i, LTRI), sc[NON]);
	      get<LTRA>(pack).add(edge);
	    }
	  }
	}
      }
    }
    
    template <typename S>
    static void marginals(const Configuration& c, const X& x, S& scores, const Chart<Item>& inside, const Chart<Item>& outside, Chart<Item>& marginals) {
      int n = (int)x.size();
      
      for (int i = 0; i < n+1; i++) {
	for (int k = i; k < n+1; k++) {
	  for (int p = 0; p < nItem_t; p++) marginals(i, k, (Item_t)p).setKey(Key(i, k, (Item_t)p));
	}
      }
      
      for (int width = 0; width <= n; width++) {
	for (int i = 0; i < n+1-width; i++) {
	  int k = i + width;
	  Edge *edge = new Edge(outside(i, k, RTRI), inside(i, k, RTRI), SR::one());
	  marginals(i, k, RTRI).add(edge);
	  edge = new Edge(outside(i, k, LTRI), inside(i, k, LTRI), SR::one());
	  marginals(i, k, LTRI).add(edge);
	  edge = new Edge(outside(i, k, RTRA), inside(i, k, RTRA), SR::one());
	  marginals(i, k, RTRA).add(edge);
	  edge = new Edge(outside(i, k, LTRA), inside(i, k, LTRA), SR::one());
	  marginals(i, k, LTRA).add(edge);
	}
      }
    }
    
  };
  
}

#endif /* TREELER_SHAGTT_H */
