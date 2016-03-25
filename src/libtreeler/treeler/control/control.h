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
 * \file   control.h
 * \brief  Main include header for controlling treeler
 * \author Xavier Carreras
 */

#ifndef TREELER_CONTROL
#define TREELER_CONTROL


/**
 * \defgroup control Control 
 * 
 * This module defines the controller
 * components to create actual structured predictors out of treeler
 * generic componentns.  
 * 
 * @{ 
 * @}
 */  


#include <string>

#include "treeler/control/models.h"
#include "treeler/control/factory-base.h"
#include "treeler/control/factory-scores.h"
#include "treeler/control/factory-dep.h"
#include "treeler/control/factory-tag.h"
#include "treeler/control/factory-ioconll.h"

namespace treeler {
  namespace control {

  };
}

#endif
