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
 * \file   fidx.cc
 * \brief  Implementation of ostream operators for \c FIdx types
 * \author Andreu Mayo 
 */

#include "treeler/base/fidx.h"

namespace treeler {

  ostream& operator<<(ostream& os, const FIdxBits& f) {
    char tmpidx[17];
    tmpidx[16] = '\0';
    feature_idx_sprint(tmpidx, f());
    os << tmpidx;
    return os;
  }

  ostream& operator<<(ostream& os, const FIdxChars& f) {
    os << f(); 
    return os;
  }

  ostream& operator<<(ostream& os, const FIdxPair& f) {
    const pair<FIdxBits, FIdxChars>& p = f();
    os << "<" << p.first << "," << p.second << ">";
    return os;
  }



}
