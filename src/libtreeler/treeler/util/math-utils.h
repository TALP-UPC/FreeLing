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
 * \file math-utils.h
 * \brief Defines an assortment of math utilities
 * \author Terry Koo
 * \note This file was ported from egstra
 */

#ifndef TREELER_MATH_UTILS_H
#define TREELER_MATH_UTILS_H

namespace treeler {

  /** 
   * Computes a signed "relative" difference; i.e., a difference that
   * is scaled by the magnitudes of the two values 
   */
  static inline double reldiff(const double x, const double y) {
    const double absx = (x < 0 ? -x : x);
    const double absy = (y < 0 ? -y : y);
    const double avgxy = 0.5*(absx + absy);
    const double diff = x - y;
    /* scale diffs for weights greater than 1 */
    return (avgxy > 1.0 ? diff/avgxy : diff);
  }

  /** 
   * Computes an unsigned "relative" distance; i.e., a distance that is
   * scaled by the magnitudes of the two values 
   */
  static inline double reldist(const double x, const double y) {
    const double absx = (x < 0 ? -x : x);
    const double absy = (y < 0 ? -y : y);
    const double avgxy = 0.5*(absx + absy);
    const double diff = x - y;
    const double dist = (diff < 0 ? -diff : diff);
    /* scale diffs for weights greater than 1 */
    return (avgxy > 1.0 ? dist/avgxy : dist);
  }
}

#endif /* TREELER_MATH_UTILS_H */
