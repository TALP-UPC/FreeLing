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
 * \file   label.h
 * \brief  Declaration of the class Label
 * \author Terry Koo, Xavier Carreras
 * \note   This file was ported from egstra
 */

#ifndef TREELER_LABEL_H
#define TREELER_LABEL_H

#include <set>
#include <iostream>

namespace treeler {

  /**
   * \brief An output value, representing a set of abstract parts
   * \author Xavier Carreras, Terry Koo
   * \ingroup base
   */
  template<class Part>
  class Label : public std::set<Part> {
  public:
    typedef Part R;
    inline void push_back(const Part& r) { this->insert(r); }    
  };
 
}

#endif /* TREELER_LABEL_H */

