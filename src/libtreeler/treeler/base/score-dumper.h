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
 * \file   score-dumper.h
 * \brief  Declares the class ScoreDumper
 * \author Xavier Carreras
 */

#ifndef TREELER_SCORE_DUMPER_H
#define TREELER_SCORE_DUMPER_H

#include <iostream>
#include <string>

namespace treeler {

  /**
   *  A generic dumper of part-scores
   * 
   * \tparam R The type of parts
   * \tparam T The type of scores
   */
  template <typename R, typename T=double>
  class ScoreDumper {
  public:  
    
    /**
     *  Dumps part-scores to an output stream
     *
     * \tparam X The type of patterns
     * \tparam S The type of part-score container
     * 
     * \param o The output stream
     * \param rconfig Configuration of the space of parts
     * \param x The input pattern
     * \param s The scores to be dumped
     * \param prefix A prefix to that is writen to the beginning of each line (empty by default)
     *
     */    
    template <typename X, typename S>
    static void dump(std::ostream& o, const typename R::Configuration rconfig, const X& x, S& s, const std::string& prefix = "") {
      auto r = R::begin(rconfig, x);
      auto r_end = R::end();
      for (; r!=r_end; ++r) {
	o << prefix << *r << " " << s(*r) << std::endl;
      }
    }
  };
  
}

#endif
