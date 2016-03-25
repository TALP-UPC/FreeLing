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
 * \file   gzfile.h
 * \brief  Declaration of class GzFile
 * \author Terry Koo
 * \note   This file was ported from egstra
 */

#ifndef TREELER_GZFILE_H
#define TREELER_GZFILE_H

#include <stdio.h>
#include <inttypes.h>

#include "treeler/base/windll.h"

namespace treeler {
  
  /** 
   *  \brief A class with some utilities for opening (optionally) gzipped files
   *  
   *  \author Terry Koo
   */ 
  class WINDLL GzFile {

  public:
    static FILE* gzopen(const char* const fn, const char* const mode);
    static void gzclose(const char* const fn, FILE* const f);

    static int fprint_int(FILE* const f, const int32_t i);
    static int fprint_int(FILE* const f, const int64_t i);
    static int fread_int(FILE* const f, int32_t *i);
    static int fread_int(FILE* const f, int64_t *i);
  };
}

#endif /* TREELER_GZFILE_H */
