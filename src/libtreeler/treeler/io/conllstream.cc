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
 * \file   conllstream.cc
 * \brief  Implementation of class CoNLLStream
 * \author Xavier Carreras
 */
#include "treeler/io/conllstream.h"

namespace std {

  ostream& operator<<(ostream& o, const treeler::CoNLLColumn& v) {
    bool first = true;
    for (auto it = v.begin(); it!=v.end(); ++it) {
      if (!first) o << " ";
      o << *it;
      first = false;
    }
    return o;
  }

  ostream& operator<<(ostream& o, const treeler::CoNLLStream& s) {
    s.write(o);
    return o;
  }

  istream& operator>>(istream& i, treeler::CoNLLStream& s) {
    bool b = s.read(i);
    if (!b) {
      i.setstate(std::ios::failbit);
    }
    return i;
  }


}
