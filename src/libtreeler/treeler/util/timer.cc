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
 * \file   timer.cc
 * \brief  Utilities for timing runs
 * \author Terry Koo
 * \note   This file was ported from egstra
 */

#include "treeler/util/timer.h"
#include <stdio.h>

namespace treeler {
  /* global variables controlling report intervals */
  int TREELER_TICKER_REPINT_LOG = 5;
  int TREELER_TICKER_REPINT = 0x1f;
  int TREELER_TICKER_REPINT_MAJOR = 0xff;
  void ticker_adjust_repint(int log_factor) {
    TREELER_TICKER_REPINT_LOG += log_factor;
    if(log_factor < 0) {
      TREELER_TICKER_REPINT >>= -log_factor;
      TREELER_TICKER_REPINT_MAJOR >>= -log_factor;
    } else if(log_factor > 0) {
      for(int i = 0; i < log_factor; ++i) {
	TREELER_TICKER_REPINT = ((TREELER_TICKER_REPINT << 1) | 1);
	TREELER_TICKER_REPINT_MAJOR = ((TREELER_TICKER_REPINT_MAJOR << 1) | 1);
      }
    }
  }

  /* function that translates a time in seconds into a short
     descriptive string */
  const char* timestr(double seconds) {
    static char buf[9];
    sprintf(buf, "        "); /* clear buffer */
    if(seconds < 60) { /* < 1 minute */
      sprintf(buf, "%.4fs", seconds);
    } else if(seconds < 3600) { /* < 1 hour */
      int minutes = ((int)seconds)/60;
      seconds -= minutes*60;
      sprintf(buf, "%02d:%05.2f", minutes, seconds);
    } else if(seconds < 86400) { /* < 1 day */
      int hours  = ((int)seconds)/3600;
      seconds -= hours*3600;
      int minutes = ((int)seconds)/60;
      seconds -= minutes*60;
      sprintf(buf, "%02d:%02d:%02d", hours, minutes, (int)seconds);
    } else if(seconds < 864000) { /* < 10 days */
      int days = ((int)seconds)/86400;
      seconds -= days*86400;
      seconds /= 3600;
      sprintf(buf, "%dd %.1fh", days, seconds);
    } else if(seconds < 31536000) { /* < 1 year */
      seconds /= 86400;
      sprintf(buf, "%.2fd", seconds);
    } else {
      sprintf(buf, "> 1 year");
    }
    return buf;
  }
}
