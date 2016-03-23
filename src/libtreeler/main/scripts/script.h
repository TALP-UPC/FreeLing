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
 * \file   script.h
 * \brief  Declaration of base Script
 * \author Xavier Carreras
 */

#ifndef TREELER_SCRIPT
#define TREELER_SCRIPT

#include <string>
#include "treeler/util/options.h"

namespace treeler {

  /** 
   * \brief A base class for scripts in treeler
   */
  class Script {
  public:

  Script(std::string n)
    : _name(n)
    {} 
    
    virtual ~Script() {};
    
    virtual void usage(Options&) = 0;
    virtual void run(Options&) = 0;
    
  protected:
    std::string const _name;
  };
}

#endif
