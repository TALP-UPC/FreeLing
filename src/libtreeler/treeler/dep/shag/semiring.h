#ifndef TREELER_SEMIRING_H
#define TREELER_SEMIRING_H

#include <queue>
#include <limits>
#include <list>
#include <string>
#include "chart.h"

using namespace std;

namespace treeler {
  
  enum Semiring_t { MAX=0, ADD, LOGADD, HYPER, STRING };

  template<Semiring_t S>
  class Semiring {
  };

  template<>
  class Semiring<MAX> {
  public:
    
    class Score_t {
    public:
      double w;
      int lab;
      
      inline Score_t() {
      }
      
      inline Score_t(const double a) {
	w = a;
	lab = -1;
      }
      
      inline Score_t(const double a, const int b) {
	w = a;
	lab = b;
      }
      
      inline Score_t operator*(const Score_t& s) const {
	return Score_t(this->w + s.w);
      }
      
      inline void operator*=(const Score_t& s) {
	this->w += s.w;
	this->lab = s.lab;
      }
      
      inline void operator+=(const Score_t& s) {
	if (this->w < s.w) {
	  this->w = s.w;
	  this->lab = s.lab;
	}
      }
      
      inline double value() const {
	return this->w;
      }
      
      inline bool operator<(const Score_t& s) const {
	return (this->w < s.w);
      }
    };
    
    inline static Score_t zero() { return Score_t(-numeric_limits<double>::infinity()); }
    inline static Score_t one() { return Score_t(0.0); }
    
    class Item;
    
    class Edge {
    public:
      const Item *l, *r;
      Score_t w;
      
      Edge() {
	l = NULL;
	r = NULL;
	w = one();
      }
      
      Edge(const Score_t a) {
	l = NULL;
	r = NULL;
	w = a;
      }
      
      Edge(const Item& l1, const Item& r1, const Score_t s) {
	l = &l1;
	r = &r1;
	w = l1.ed->w*r1.ed->w;
	w *= s;
      }
      
      inline bool operator<(const Edge& ed) const {
	return (this->w < ed.w);
      }
      
      inline double value() {
	return this->w.value();
      }
    };
    
    class Item {
    public:
      Key k;
      Edge *ed;
      
      Item() {
	k = Key();
	ed = new Edge(zero());
      }
      
      inline void setKey(const Key& a) {
	k = a;
      }
      
      inline void add(Edge* edge) {
	if (*ed < *edge) ed = edge;
	else delete edge;
      }
    
      inline double get_w() {
	return this->ed->value();
      }
      
      template<typename Y>
      inline void get_tree(Y& y) {
	queue<const Item*> q;
	q.push(this);
	while (!q.empty()) {
	  const Item *i = q.front();
	  q.pop();
	  if (get<0>(i->k) == get<1>(i->k)) continue; //Basic Item
	  q.push(i->ed->l);
	  q.push(i->ed->r);
	  if (get<2>(i->k) == RTRA) {
	    typename Y::R r = typename Y::R(get<0>(i->k)-1, get<1>(i->k)-1, i->ed->w.lab);
	    y.push_back(r);
	  }
	  else if (get<2>(i->k) == LTRA) {
	    typename Y::R r = typename Y::R(get<1>(i->k)-1, get<0>(i->k)-1, i->ed->w.lab);
	    y.push_back(r);
	  }
	}
      }
    };
    
  };

  template<>
  class Semiring<ADD> {
  public:
    
    class Score_t {
    public:
      double w;
      
      inline Score_t() {
      }
      
      inline Score_t(const double a) {
	w = a;
      }
      
      inline Score_t(const double a, const int b) { //just to be able to do: Score_t(scores(r), l);
	w = a;
      }
      
      inline Score_t operator*(const Score_t& s) const {
	return Score_t(this->w*s.w);
      }
      
      inline void operator*=(const Score_t& s) {
	this->w = this->w*s.w;
      }
      
      inline void operator+=(const Score_t& s) {
	this->w += s.w;
      }
      
      inline double value() const {
	return this->w;
      }
    };
    
    inline static Score_t zero() { return Score_t(0.0); }
    inline static Score_t one() { return Score_t(1.0); }
    
    class Item;
    
    class Edge {
    public:
      Score_t w;
      
      Edge() {
	w = one();
      }
      
      Edge(const Item& l1, const Item& r1, const Score_t s) {
	w = l1.w*r1.w;
	w *= s;
      }
    };
    
    class Item {
    public:
      Score_t w;
      
      Item() {
	w = zero();
      }
      
      inline void setKey(const Key& a) {
      }
      
      inline void add(Edge* edge) {
	this->w += edge->w;
	delete edge;
      }
    
      inline double get_w() {
	return this->w.value();
      }
      
      template<typename Y>
      inline void get_tree(Y& y) { }
    };
    
  };

  template<>
  class Semiring<LOGADD> {
  public:
    
    class Score_t {
    public:
      double w;
      
      inline Score_t() {
      }
      
      inline Score_t(double a) {
	w = a;
      }
      
      inline Score_t(const double a, const int b) { //just to be able to do: Score_t(scores(r), l);
	w = a;
      }
      
      inline Score_t operator*(const Score_t& s) const {
	return Score_t(this->w + s.w);
      }
      
      inline void operator*=(const Score_t& s) {
	this->w += s.w;
      }
      
      inline void operator+=(const Score_t& s) {
	if (this->w > s.w) this->w = this->w + log(1 + exp(s.w - this->w));
	else this->w = s.w + log(1 + exp(this->w - s.w));
      }
      
      inline double value() const {
	return this->w;
      }
    };
    
    inline static Score_t zero() { return Score_t(-numeric_limits<double>::infinity()); }
    inline static Score_t one() { return Score_t(0.0); }
    
    class Item;
    
    class Edge {
    public:
      Score_t w;
      
      Edge() {
	w = one();
      }
      
      Edge(Item& l1, Item& r1) {
	w = l1.w*r1.w;
      }
      
      Edge(Item& l1, Item& r1, Score_t s) {
	w = l1.w*r1.w;
	w *= s;
      }
    };
    
    class Item {
    public:
      Score_t w;
      
      Item() {
	w = zero();
      }
      
      inline void setKey(const Key& a) {
      }
      
      inline void add(Edge* edge) {
	this->w += edge->w;
	delete edge;
      }
      
      inline double get_w() const {
	return exp(this->w.value());
      }
      
      template<typename Y>
      inline void get_tree(Y& y) { }
    };
    
  };
  
  template<Semiring_t SR_t, typename IChart>
  typename Semiring<SR_t>::Score_t insideHyper(const IChart& ch, const Key& k);
  template<Semiring_t SR_t, typename IChart>
  typename Semiring<SR_t>::Score_t outsideHyper(const IChart& ch, const Key& k);
  
  template<>
  class Semiring<HYPER> {
  public:
    
    class Score_t {
    public:
      list<pair<double, int> > w;
      
      inline Score_t() {
	w = list<pair<double, int> >();
      }
      
      inline Score_t(const list<pair<double, int> >& a) {
	w = list<pair<double, int> >(a.begin(), a.end());
      }
      
      inline Score_t(double a, const int b) {
	w = list<pair<double, int> >(1, pair<double, int>(a, b));
      }
      
      inline void operator*=(const Score_t& s) {
	this->w.insert(this->w.end(), s.w.begin(), s.w.end());
      }
      
      inline void operator+=(const Score_t& s) {
	this->w.insert(this->w.end(), s.w.begin(), s.w.end());
      }
    };
    
    inline static Score_t zero() { return Score_t(); }
    inline static Score_t one() { return Score_t(); }
    
    class Item;
    
    class Edge {
    public:
      const Item *p;
      const Item *l, *r;
      Score_t w;
      
      Edge() {
	p = NULL;
	l = NULL;
	r = NULL;
	w = one();
      }
      
      Edge(Item& l1, Item& r1, const Score_t s) {
	p = NULL;
	l = &l1;
	r = &r1;
	w = one();
	w *= s;
	l1.parents.push_back(this);
	r1.parents.push_back(this);
      }
      
      ~Edge() {
      }
    };
    
    class Item {
    public:
      Key k;
      list<Edge*> parents;
      list<Edge*> ed;
      
      Item() {
	k = Key();
      }
      
      inline void setKey(const Key& a) {
	k = a;
      }
      
      ~Item() {
	for (list<Edge*>::iterator it = ed.begin(); it != ed.end(); it++)
	  delete (*it);
      }
      
      inline void add(Edge* edge) {
	if (edge->l != NULL) {
	  edge->p = this;
	  ed.push_back(edge);
	}
	else delete edge;
      }
      
      inline double get_w() {
	return 0;
      }
      
      template<typename Y>
      inline void get_tree(Y& y) { }
    };
    
  };

}
  
#include "hyperParser.h"

#endif
