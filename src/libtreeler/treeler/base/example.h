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
 * \file   example.h
 * \brief  Declares the class Example
 * \author Xavier Carreras
 */

#ifndef TREELER_EXAMPLE_H
#define TREELER_EXAMPLE_H

#include "treeler/base/label.h"
#include "treeler/io/io-basic.h"

namespace treeler {

  /**
   * \brief  A class representing a generic (x,y) pair
   * \ingroup base
   * \note   This will be templated on Y, not R
   * \author Xavier Carreras
   */
  template <typename X, typename Y>
  class Example {
  protected:
    X*  _x;
    Y* _y;

  public:
    Example(X* x, Y* y)
      : _x(x), _y(y) 
    {
      //      std::cerr << "Example::ctor : creating example with id " << _x->id() << std::endl; 
    }
    ~Example() {      
      //      std::cerr << "Example::dtor : destroying example with id " << _x->id() << std::endl; 
      delete _x;
      delete _y;
    }

    /* accessors */
    const X& x() const { return *_x; }
    const Y& y() const   { return *_y; }

    /* swap */
    void swap(Example<X,Y>& b) {
      std::cerr << "Example::swap : swapping examples " << std::endl; 
      X* tmpx = _x;
      Y* tmpy = _y; 
      _x = b._x; 
      _y = b._y; 
      b._x = tmpx; 
      b._y = tmpy; 
    }

  };

  

  /**
   * \brief  Default traits for an Example
   * \ingroup base
   * \author Xavier Carreras
   * 
   * Default traits for an Example. Example traits define:
   *   - IO : a type providing input-output methods (defaults to IOBasic)
   */
  template <typename X, typename R>
  class ExampleTraits {
  public:
    typedef IOBasic<X,R> IO;
  };

}

namespace std {
  template <typename X, typename Y>
  void swap(treeler::Example<X,Y>& a, treeler::Example<X,Y>& b) {
    a.swap(b); 
  }
}

#endif /* TREELER_EXAMPLE_H */

