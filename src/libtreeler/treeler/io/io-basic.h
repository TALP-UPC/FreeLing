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
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   io-basic.h
 * \brief  Declaration of class IOBasic
 * \author Xavier Carreras
 */
#ifndef TREELER_IO_BASIC
#define TREELER_IO_BASIC

#include <iostream>
#include "treeler/base/sentence.h"
#include "treeler/base/label.h"

namespace treeler {

  std::ostream& operator<<(std::ostream & o, const Sentence& s);

  template <typename T>
  std::ostream& operator<<(std::ostream & o, const Label<T>& y);

  template <typename X, typename R>
  class IOBasic {
  public:
    static void write(std::ostream& o, const X& x) {
      o << "(iobasic: writing x)";
    }
    static void write(std::ostream& o, const Label<R>& y) {
      o << "(iobasic: writing y)";
    }
    static void write(std::ostream& o, const X& x, const Label<R>& y) {
      o << "(iobasic: writing x and y)";
    }
    static void write(std::ostream& o, const X& x, const Label<R>& y1, const Label<R>& y2) {
      o << "(iobasic: writing x and y1 and y2)";
    }
  };


  // Implementation of template functions

  template <typename T>
  std::ostream& operator<<(std::ostream & o, const Label<T>& y) {    
    typename Label<T>::const_iterator i = y.begin(), i_end = y.end();
    bool first = true;
    for( ; i!=i_end; ++i) {
      if (first) first = false;
      else o << " ";
      o << *i;
    }
    return o;
  }


}

#endif
