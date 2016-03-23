#ifndef CHART_H
#define CHART_H

#include <tuple>

const int nItem_t = 4;
enum Item_t { RTRI=0, LTRI, RTRA, LTRA };

typedef tuple<int, int, Item_t> Key;

ostream& operator<< (ostream& out, const Key& k ) {
  out << '(' << get<0>(k) << ',' << get<1>(k) << ',' << get<2>(k) << ')';
  return out;
}

template <typename Item>
class Chart {
public:
  typedef tuple<Item, Item, Item, Item> Pack;
  Pack **table;
  int n;
  
  Chart() {}
  
  Chart(const int m) {
    table = new Pack*[m];
    for (int i = 0; i < m; i++)
      table[i] = new Pack[m-i];
    n = m;
  }
  
  Chart(const int m, const Item& a) {
    table = new Pack*[m];
    for (int i = 0; i < m; i++)
      table[i] = new Pack[m-i];
    n = m;
    
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n-i; j++) {
	Pack& p = table[i][j];
	get<0>(p) = a;
	get<1>(p) = a;
	get<2>(p) = a;
	get<3>(p) = a;
      }
    }
  }
  
  ~Chart() {
    for (int i = 0; i < n; i++)
      delete[] table[i];
    delete[] table;
  }
  
  inline Pack & operator()(const int a, const int b) const {
    return table[a][b-a];
  }
  
  inline Item & operator()(const int a, const int b, const Item_t t) const {
    switch (t) {
      case RTRI: return get<0>(table[a][b-a]);
      case LTRI: return get<1>(table[a][b-a]);
      case RTRA: return get<2>(table[a][b-a]);
      case LTRA: return get<3>(table[a][b-a]);
    }    
    return get<0>(table[a][b-a]);    
  }
  
  inline Item & operator()(const Key& k) const {
    return (*this)(get<0>(k), get<1>(k), get<2>(k));
  }
  
  inline int get_n() const {
    return n;
  }
  
};

#endif
