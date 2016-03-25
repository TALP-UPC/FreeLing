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
 * \file   part-dep1.cc
 * \brief  Implementation of PartDep1 methods
 * \author Xavier Carreras
 */
#include "treeler/dep/part-dep1.h"

std::ostream& treeler::operator<<(std::ostream & o, const PartDep1 & p) {
  o << "<" << p.head() << "," << p.mod() << "," << p.label() << ">"; 
  return o;
}

namespace treeler {

  void PartDep1::decompose(const DepVector<int>& yin, Label<PartDep1>& yout) {
    yout.clear(); 
    for (size_t m = 0; m < yin.size(); ++m) {
      yout.push_back(PartDep1(yin[m].h, m, yin[m].l)); 
    }
  }

  Label<PartDep1>&& PartDep1::decompose(const DepVector<int>& y) {
    Label<PartDep1> parts; 
    for (size_t m = 0; m < y.size(); ++m) {
      parts.push_back(PartDep1(y[m].h, m, y[m].l)); 
    }
    return std::move(parts);
  }

  void PartDep1::compose(const Label<PartDep1>& yin, DepVector<int>& yout) {
    auto r = yin.begin(); 
    auto r_end = yin.end(); 
    for (; r!=r_end; ++r) {
      if (r->m >= (int)yout.size()) {
	yout.resize(r->m+1, HeadLabelPair<int>()); 
      }
      HeadLabelPair<int>& hl = yout[r->m];
      hl.h = r->h; 
      hl.l = r->l; 
    }    
  }


}
