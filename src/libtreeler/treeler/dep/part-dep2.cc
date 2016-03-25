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
 * \file   part-dep2.cc
 * \brief  Implementation of PartDep2 methods
 * \author Xavier Carreras
 */

#include "treeler/dep/part-dep2.h"

#include <cassert>
#include <iostream>

namespace treeler {

  std::ostream& operator<<(std::ostream & o, const PartDep2 & p) {
    if (p.type==PartDep2::FO) {
      o << "[FO " << p.head() << " " << p.mod() << " " << p.label() << "]"; 
    }
    else {   
      o << "[";
      switch (p.type) {
      case PartDep2::SIB:
	o << "SB "; break; 
      case PartDep2::CMI:
	o << "GI "; break; 
      case PartDep2::CMO:
	o << "GO "; break; 
      default:
	assert(0);
      }
      o << p.head() << " " << p.mod() << " " << p.child() << " " << p.label() << "]"; 
    }
    return o;
  }
  

  std::istream& operator>>(std::istream & i, PartDep2& p) {
    char c;
    string type;
    i >> type;
    if (type=="[FO") {
      p.type = PartDep2::FO;
      i >> p.h >> p.m >> p.l >> c;
      p.c = -1;
      if (c!=']') { /* !!!! */ }
      return i;
    }
    else {
      if (type=="[SB") p.type = PartDep2::SIB;
      else if (type=="[GI") p.type = PartDep2::CMI;
      else if (type=="[GO") p.type = PartDep2::CMO;
      else { /* !!!! */ }
      i >> p.h >> p.m >> p.c >> p.l >> c;
      if (c!=']') { /* !!!! */ }
      return i;
    }
  }
  
  
}
