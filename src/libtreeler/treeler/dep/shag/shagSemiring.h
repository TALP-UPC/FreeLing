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
 * \file   shagSemiring.h
 * \brief  
 * \author 
 */

#ifndef TREELER_SHAGSEMIRING_H
#define TREELER_SHAGSEMIRING_H

#include <queue>
#include <iomanip>
#include <cmath>
#include <list>
#include <tuple>
#include <unordered_map>

namespace treeler {
    
  template <typename X, typename Y, typename R> 
  class Semiring {
  public:
    class Configuration {};
    
    enum Combination_t { NON, LEFT, RIGHT };
    
    #define INI_SUM -1.0/0.0
    #define INI_MUL 0
    
    class Item;
    
    class Combination {
    public:
      Item *l, *r;
      Combination_t ct;
      double w;
      
      Combination() {
      }
      
      inline Combination operator()(Combination_t ct, double sc[]) {
	//DEFINES +
	
      }
    };
    
    enum Item_t { RTra, LTra, RTri, LTri };
    
    class Item {
    public:
      int l, r;
      Item_t it;
      double w;
      list<Combination> co;
      
      Item() {
      }
      
      inline void operator+=(const Combination& comb) {
	//DEFINES +
	
      }
    
      inline Combination operator*(const Item& r) const {
	//DEFINES x
	
      }
    };
    
    typedef tuple<int,int,int> Tri;

    struct TripletHash_lookup3 {
      size_t operator()(const Tri& t) const {
	
      }
    };

    typedef unordered_map<Tri, Item, TripletHash_lookup3> Chart;
    
    template <typename S>
    inline static double score(S& scores, int g, int h, int m) {
      
    }
    
    template <typename S>
    static double argmax(const Configuration& c, const X& x, S& scores, Y& y) {
      double sc[K]; //score of the combination of 2 known items given a value of Combination_t
      
    }
    
  };

}

#endif /* TREELER_SHAG_H */
