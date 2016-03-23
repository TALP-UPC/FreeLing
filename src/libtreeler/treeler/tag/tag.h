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
 * \file   tag.h
 * \brief  Main declarations for Tagging
 * \author Xavier Carreras
 */

#ifndef TREELER_TAG
#define TREELER_TAG

#include "treeler/base/sentence.h"
#include "treeler/base/label.h"
#include "treeler/tag/part-tag.h"
#include "treeler/tag/viterbi.h"
#include "treeler/tag/fgen-tag.h"
#include "treeler/base/scores.h"

#include "treeler/tag/tag-symbols.h"
#include "treeler/tag/tag-eval.h"
#include "treeler/tag/tuple-seq.h"
#include "treeler/tag/fgen-ttag.h"
//#include "treeler/tag/io-ttag.h" 

#include "treeler/io/io-sentence.h" 
#include "treeler/tag/io-tag.h" 


namespace treeler {

  /**
   * \defgroup tag Tagging
   * This module implements models and algorithms for sequence tagging. 
   * @{
   */

  template <typename X>
  class ExampleTraits<X, TagSeq> {
  public:
    typedef TagEval Eval;
  };


  template <typename X>
  class ExampleTraits<X, Label<PartTag>> {
  public:
    typedef TagEval Eval;
//    typedef IOTTag IO;
  };



}

#endif
