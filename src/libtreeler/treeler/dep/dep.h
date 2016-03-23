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
 * \file   dep.h
 * \brief  Main declarations for Dependency Parsing
 * \author Xavier Carreras
 * 
 *
 */

#ifndef TREELER_DEP
#define TREELER_DEP

#include "treeler/base/scores.h"
#include "treeler/base/basic-sentence.h"
#include "treeler/dep/dep-tree.h"
#include "treeler/dep/fgen-dep-v1.h"

#include "treeler/dep/part-dep1.h"
#include "treeler/dep/parser-projdep1.h"

#include "treeler/dep/part-dep2.h"
#include "treeler/dep/parser-projdep2.h"

#include "treeler/dep/fgen-dep-v0.h"
#include "treeler/dep/fgen-dep-v1.h"

#include "treeler/io/io-conll.h"
#include "treeler/io/io-dep.h"
#include "treeler/dep/dep-eval.h"

namespace treeler {

  /**
   * \defgroup dep Dependency Parsing
   * This module implements models and algorithms for dependency parsing. 
   * @{
   */


  /**
   * @}
   */
  
}


#endif
