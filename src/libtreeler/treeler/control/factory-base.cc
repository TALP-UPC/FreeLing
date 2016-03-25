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
 * \file   factory-base.h
 * \brief  Static methods for creating basic components
 * \author Xavier Carreras
 */

#include "treeler/control/factory-base.h"

namespace treeler {


  namespace control {

    void display_options(ostream& out, const list<CLOption>& options, const string& prefix) {
      size_t maxl = 0; 
      for (auto& o : options) {
	size_t l = o.name.length(); 
	if (maxl<l) maxl = l; 
      }
      
      for (auto& o : options) {
	out << prefix; 
	out << "--" << o.name; 
	for (size_t i = o.name.length(); i<maxl; ++i) out << " "; 
	out << " : "; 	
	string line; 
	istringstream iss(o.help.str()); 
	getline(iss, line); 
	out << line << endl; 
	while (getline(iss, line)) {
	  out << string(prefix.length() + maxl + 5, ' ') << line << endl; 
	}
      }            
    }

  }
}
