#ifndef TREELER_HYPERPARSER_H
#define TREELER_HYPERPARSER_H

#include <list>
#include "semiring.h"
#include "chart.h"

using namespace std;

namespace treeler {
  typedef Chart<Semiring<HYPER>::Item> HyperGraph;
  
  template<Semiring_t SR_t>
  void insideHyperRec(const HyperGraph& hyper, const typename Semiring<HYPER>::Item* item, Chart<typename Semiring<SR_t>::Item>& inside, Chart<bool>& done) {
    typedef Semiring<SR_t> SR;
    typedef typename SR::Edge Edge;
    typedef typename SR::Score_t Score_t;
    
    if (!done(item->k)) {
      if (item->ed.empty()) {
	inside(item->k).add(new Edge());
	done(item->k) = true;
	return;
      }
    
      for (list<typename Semiring<HYPER>::Edge*>::const_iterator edge = item->ed.begin(); edge != item->ed.end(); edge++) {
	//recursive call to the children
	if (!done((*edge)->l->k)) insideHyperRec<SR_t>(hyper, (*edge)->l, inside, done);
	if (!done((*edge)->r->k)) insideHyperRec<SR_t>(hyper, (*edge)->r, inside, done);
	
	Score_t dep_score;
	if ((*edge)->w.w.empty()) dep_score = SR::one();
	else {
	  dep_score = SR::zero();
	  for (list<pair<double, int> >::const_iterator it = (*edge)->w.w.begin(); it != (*edge)->w.w.end(); it++)
	    dep_score += Score_t(it->first, it->second);
	}

	inside(item->k).add(new Edge(inside((*edge)->l->k), inside((*edge)->r->k), dep_score));
      }
      done(item->k) = true;
    }
  }
  
  template<Semiring_t SR_t>
  void insideHyper(const HyperGraph& hyper, Chart<typename Semiring<SR_t>::Item>& inside) {
    //    typedef Semiring<SR_t> SR;
    //    typedef typename SR::Item Item;
    
    int n = ((int)hyper.get_n())-1;
    
    for (int i = 0; i < n+1; i++)
      for (int j = i; j < n+1; j++)
	for (int p = 0; p < nItem_t; p++)
	  inside(i, j, (Item_t)p).setKey(Key(i, j, (Item_t)p));
    
    Chart<bool> done(n+1, false);
    insideHyperRec<SR_t>(hyper, &hyper(0, n, RTRI), inside, done);
  }
  
  template<Semiring_t SR_t>
  void outsideHyperRec(const HyperGraph& hyper, const typename Semiring<HYPER>::Item* item, const Chart<typename Semiring<SR_t>::Item>& inside, Chart<typename Semiring<SR_t>::Item>& outside, Chart<bool>& done) {
    typedef Semiring<SR_t> SR;
    typedef typename SR::Edge Edge;
    typedef typename SR::Score_t Score_t;
    
    if (!done(item->k)) {
      for (list<typename Semiring<HYPER>::Edge*>::const_iterator p_edge = item->parents.begin(); p_edge != item->parents.end(); p_edge++) {
	//recursive call to the parents
	outsideHyperRec<SR_t>(hyper, (*p_edge)->p, inside, outside, done);
	
	Score_t dep_score;
	if ((*p_edge)->w.w.empty()) dep_score = SR::one();
	else {
	  dep_score = SR::zero();
	  for (list<pair<double, int> >::const_iterator it = (*p_edge)->w.w.begin(); it != (*p_edge)->w.w.end(); it++)
	    dep_score += Score_t(it->first, it->second);
	}
	
	if ((*p_edge)->l->k == item->k) outside(item->k).add(new Edge(outside((*p_edge)->p->k), inside((*p_edge)->r->k), dep_score));
	else outside(item->k).add(new Edge(outside((*p_edge)->p->k), inside((*p_edge)->l->k), dep_score));
      }
      done(item->k) = true;
    }
  }
  
  template<Semiring_t SR_t>
  void outsideHyper(const HyperGraph& hyper, const Chart<typename Semiring<SR_t>::Item>& inside, Chart<typename Semiring<SR_t>::Item>& outside) {
    typedef Semiring<SR_t> SR;
    typedef typename SR::Edge Edge;
    // typedef typename SR::Item Item;
    
    int n = (int)inside.get_n()-1;
    
    for (int i = 0; i < n+1; i++)
      for (int j = i; j < n+1; j++)
	for (int p = 0; p < nItem_t; p++)
	  outside(i, j, (Item_t)p).setKey(Key(i, j, (Item_t)p));
    
    outside(0, n, RTRI).add(new Edge());
    
    Chart<bool> done(n+1, false);
    outsideHyperRec<SR_t>(hyper, &hyper(0, 0, RTRI), inside, outside, done);
    for (int i = 1; i <= n; i++) {
      outsideHyperRec<SR_t>(hyper, &hyper(i, i, RTRI), inside, outside, done);
      outsideHyperRec<SR_t>(hyper, &hyper(i, i, LTRI), inside, outside, done);
    }
  }






 
  // private namespace for HyperGraph subroutines
  namespace _hypergraph {
    
    typedef Semiring<HYPER>::Item Item; 
    typedef Semiring<HYPER>::Edge Edge; 

  
    // a derivation is just a vector of heads
    struct HeadsVector {			
    public:
      enum { FIELD_i=0, FIELD_j=1, FIELD_type=2 };	
      vector<int> head; 
  
      HeadsVector(int n) : head(n, -2) {}
      
      // add an item to the derivation
      inline void operator+=(const Item& i) {
	switch (get<FIELD_type>(i.k)) {
	case RTRA: 
	  head[get<FIELD_j>(i.k)-1] = get<FIELD_i>(i.k); 
	  break; 
	case LTRA: 
	  head[get<FIELD_i>(i.k)-1] = get<FIELD_j>(i.k); 
	  break; 
	default:
	  break; 
	}
      }
      // substract an item from the derivation
      inline void operator-=(const Item& i) {
	switch (get<FIELD_type>(i.k)) {
	case RTRA: 
	  head[get<FIELD_j>(i.k)-1] = -2;
	  break; 
	case LTRA: 
	  head[get<FIELD_i>(i.k)-1] = -2;
	  break; 
	default:
	  break;
	}
      }
    };
  

    // forward declarations
    template <typename F, typename D>
      class ADFunctor;

    template<typename F,typename D>
      void all_derivations_rec(const Item& i, ADFunctor<F,D>& f, D& d); 
    
  
    
    /**
     * \brief A wrapper functor used in the recursive enumeration of
     * all derivations.
     * 
     * F is the base functor, the one applied to all
     * derivations. D is the type of derivations.
     *
     * An ADFunctor<F> defines an operation over derivations, and can
     * take two forms. In the base form it is just a pointer to an F
     * functor, and the operation simply applies the base functor to
     * the derivation.
     * 
     * In the recursive form, it remembers a location in the
     * hypergraph (that is, an item) together with a pointer to a
     * ADFunctor<F>. The operation on a (partial) derivation
     * recursively calls the all_derivations starting form the pointed
     * item, using the pointed functor as functor.
     * 
     */
    template <typename F, typename D=HeadsVector>
    class ADFunctor {
    public:
      F *base_f; 
      const Item *i; 
      ADFunctor<F,D> *rec_f;
      
      ADFunctor(F& base) {
	base_f = &base; 
	rec_f = NULL; 
	i = NULL; 
      }
      
      ADFunctor(const Item& it, ADFunctor<F,D>& rec) {
	base_f = NULL;
	i = &it; 
	rec_f = &rec; 
      }
      
      void operator()(D& d) {
	if (base_f!=NULL) {
	  (*base_f)(d); 
	}
	else {
	  all_derivations_rec<F,D>(*i, *rec_f, d); 
	}
      }    
    };
  
  

   /* 
    * This method applies f to all derivations of i
    * 
    */      
    template<typename F,typename D>
    void all_derivations_rec(const Item& i, ADFunctor<F,D>& f, D& d) {	
      // add the item to the derivation
      d += i;
      
      if (i.ed.empty()) {
	f(d);
      }
      else {
	list<Edge*>::const_iterator e = i.ed.begin(); 
	list<Edge*>::const_iterator e_end = i.ed.end(); 
	for (; e!=e_end; ++e) {
	  // this functor will apply f to all derivations of the right item 
	  ADFunctor<F> fprime(*((*e)->r), f); 
	  // we apply fprime to all derivations of the left item
	  all_derivations_rec(*((*e)->l), fprime, d);
	}
      }
      
      // remove the item from the derivation
      d -= i;
    }
    
 
  }
  
  /**
   * \brief Explicitly enumerates all derivations of a hypergraph 
   * \author Xavier Carreras
   * 
   * This method applies functor f to all derivations of a
   * hypergraph's item.
   *
   * The type Derivation needs to support adding and substracting
   * items, by defining operator+=(const Item&) and operator-=(const Item&)
   * 
   * The type Functor needs to define operator()(const Derivation& d).
   *
   */
   template<typename Functor, typename Derivation=_hypergraph::HeadsVector>
   void all_derivations(const Semiring<HYPER>::Item& item, 
			Functor& f, 
			Derivation& d) {      
     _hypergraph::ADFunctor<Functor> fprime(f);
     _hypergraph::all_derivations_rec(item, fprime, d); 
   }


}

#endif /* TREELER_SHAGHYPERPARSER_H */
