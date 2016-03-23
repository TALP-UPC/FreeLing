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
 * \file   gzfile.cc
 * \brief  Implementation of methods of class GzFile
 * \author Terry Koo
 * \note   This file was ported from egstra
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "treeler/util/gzfile.h"

#define myname "GzFile"

using namespace std;

namespace treeler {
  FILE* GzFile::gzopen(const char* const fn, const char* const mode) {
    FILE* f = NULL;
    const int n = strlen(fn);
    if(n > 3 &&
       (fn[n - 1] == 'z' || fn[n - 1] == 'Z') &&
       (fn[n - 2] == 'g' || fn[n - 2] == 'G') &&
       fn[n - 3] == '.') {
      char* const cmd = (char*)malloc((n + 64)*sizeof(char));
      if(mode[0] == 'r') {
	snprintf(cmd, n + 64, "gunzip -c '%s'", fn);
      } else if(mode[0] == 'w') {
	snprintf(cmd, n + 64, "gzip -c > '%s'", fn);
      } else {
	cerr << myname << " : unrecognized mode \"" << mode << "\"" << endl;
	assert(0);
      }
      f = popen(cmd, mode);
      free(cmd);
    } else {
      f = fopen(fn, mode);
    }
    if(f == NULL) {
      cerr << myname << " : error opening \"" << fn << "\"" << endl;
      assert(0);
    }
    return f;
  }

  void GzFile::gzclose(const char* const fn, FILE* const f) {
    const int n = strlen(fn);
    if(n > 3 &&
       (fn[n - 1] == 'z' || fn[n - 1] == 'Z') &&
       (fn[n - 2] == 'g' || fn[n - 2] == 'G') &&
       fn[n - 3] == '.') {
      const int ret = pclose(f);
      if(ret == -1) {
	cerr << myname << " : error closing \"" << fn << "\"" << endl;
	assert(0);
      }
    } else {
      const int ret = fclose(f);
      if(ret != 0) {
	cerr << myname << " : error closing \"" << fn << "\"" << endl;
	assert(0);
      }
    }
  }

  
  int GzFile::fprint_int(FILE* const f, const int32_t i) {
    return fprintf(f, "%d", i);
  }

  int GzFile::fprint_int(FILE* const f, const int64_t i) {
    ostringstream sstr;
    sstr << i;
    string istr = sstr.str();
    return fprintf(f, "%s", istr.c_str());
  }

  
  int GzFile::fread_int(FILE* const f, int32_t *i) {
    return fscanf(f, "%d", i);
  }
  
  int GzFile::fread_int(FILE* const f, int64_t *i) {
    char s1[256];
    int n = fscanf(f, "%s", s1);

    if (n>0) {
      istringstream str(s1);
      str >> *i;
    }    
    return n;
  }


}
