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
 * \file   oracle-scores.h
 */

#ifndef TREELER_ORACLE_SCORES_H
#define TREELER_ORACLE_SCORES_H

#include "treeler/base/label.h"
#include "treeler/base/example.h"
#include "treeler/base/scores.h"
#include <string>
#include <list>
#include <iostream>
#include <cassert>

namespace treeler {


  /** 
   * \brief  Oracle scores 
   * \author Xavier Carreras
   * \ingroup base
   */
  template <typename Symbols, typename X, typename R>
  class OracleScorer; 

  template <typename Symbols, typename X, typename R>
  class OracleScores : public BaseScores<OracleScores<Symbols,X,R>> {
  private:
    const Symbols* _symbols; 
    const X* _x;
    const Label<R>* _y;
    Label<R>* _tmpy;
  public:
    typedef OracleScorer<Symbols, X,R> Scorer;
    OracleScores() : _symbols(NULL), _x(NULL), _y(NULL), _tmpy(NULL)
    {}
    void set_symbols(const Symbols* sym) {
      _symbols = sym;
    }

    void new_xy(const X& x, const Label<R>& y) { 
      clear(); 
      _x = &x;
      _y = &y;
    }   

    void new_xy(const Example<X,Label<R>>& xy) { 
      clear(); 
      _x = &(xy.x());
      _y = &(xy.y());
    }

    template <typename Y> 
    void new_xy(const X& x, const Y& y){
      _tmpy = new Label<R>(); 
      R::decompose(*_symbols, x, y, *_tmpy); 
      _x = &(x); 
      _y = _tmpy; 
    }

    template <typename Y> 
    void new_xy(const Example<X,Y>& xy) {
      _tmpy = new Label<R>(); 
      R::decompose(*_symbols, xy.x(), xy.y(), *_tmpy); 
      _x = &(xy.x()); 
      _y = _tmpy; 
    }

    void clear() {
      _x = NULL; 
      _y = NULL;
      if (_tmpy!=NULL) {
	delete _tmpy;
	_tmpy = NULL; 
      }
    }   
    double operator()(const R& r) const {
      typename Label<R>::const_iterator s = _y->find(r);
      if (s==_y->end()) {
	return -1;
      }
      return +1;
    }    
  };

  template <typename Symbols, typename X, typename R>
  class OracleScorer {
  private:
    const Symbols& _symbols; 
  public:
    typedef OracleScores<Symbols,X,R> Scores;
    //! \brief
    OracleScorer(const Symbols& sym)
      : _symbols(sym)
    {};

    //! \brief
    void scores(const X& x, const Label<R>& y, Scores& scores) const {
      scores.set_symbols(&_symbols); 
      scores.new_xy(x, y); 
    }

    //! \brief
    template <typename Y>
    void scores(const X& x, const Y& y, Scores& scores) const {
      scores.set_symbols(&_symbols); 
      scores.new_xy(x, y); 
    }

    //! \brief
    void scores(const Example<X,Label<R>>& xy, Scores& scores) const {
      scores.set_symbols(&_symbols); 
      scores.new_xy(xy); 
    }
    //! \brief
    template <typename Y>
    void scores(const Example<X,Y>& xy, Scores& scores) const {
      scores.set_symbols(&_symbols); 
      scores.new_xy(xy); 
    }

  };
  
}

#endif
