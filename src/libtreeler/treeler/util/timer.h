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
 * \file   timer.h
 * \brief  Utilities for timing runs
 * \author Terry Koo
 * \note   This file was ported from egstra
 */

#ifndef TREELER_TIMER_H
#define TREELER_TIMER_H

#if defined WIN32 || defined WIN64	
#include <time.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

#include <assert.h>
#include <iomanip>

/* macro that starts a timer (also allocates local vars) */
#define timer_start()				\
  struct timeval tv1;				\
  gettimeofday(&tv1, NULL)

/* THESE TWO MACROS ASSERT IN CASE TIMING IS NEGATIVE; ASSERTIONS ARE 
   PROBLEMATIC, HENCE WE REDEFINE THEM WITHOUT THE ASSERTIONS */
/* #define timer_split(ret) {						\ */
/*     struct timeval tvsplit;						\ */
/*     gettimeofday(&tvsplit, NULL);					\ */
/*     ret = (double)tvsplit.tv_sec   - (double)tv1.tv_sec +		\ */
/*       ((double)tvsplit.tv_usec - (double)tv1.tv_usec)/1000000.0;	\ */
/*     assert(ret > 0);							\ */
/*   } */
/**/
/* /\* macro that stores elapsed seconds in timediff *\/ */
/* #define timer_stop()						\ */
/*   struct timeval tv2;						\ */
/*   gettimeofday(&tv2, NULL);					\ */
/*   const double timediff =					\ */
/*    (double)tv2.tv_sec   - (double)tv1.tv_sec +			\ */
/*    ((double)tv2.tv_usec - (double)tv1.tv_usec)/1000000.0;		\ */
/*    assert(timediff > 0) */

#define timer_split(ret) {						\
    struct timeval tvsplit;						\
    gettimeofday(&tvsplit, NULL);					\
    ret = (double)tvsplit.tv_sec   - (double)tv1.tv_sec +		\
      ((double)tvsplit.tv_usec - (double)tv1.tv_usec)/1000000.0;	\
  }

/* macro that stores elapsed seconds in timediff */
#define timer_stop()						\
  struct timeval tv2;						\
  gettimeofday(&tv2, NULL);					

/* macro that prints out a dot every 32 units and a percentage every
   256 units.  ii and nn are the current tick and the total ticks */
#define ticker(ii, nn)							\
  ticker4(ii, nn, TREELER_TICKER_REPINT, TREELER_TICKER_REPINT_MAJOR)

/* macro that prints out a dot every 32 units and thousands every 256
   units.  ii and nn are the current tick and the total ticks */
#define tickerK(ii)							\
  tickerK3(ii, TREELER_TICKER_REPINT, TREELER_TICKER_REPINT_MAJOR)

/* macro that prints out a dot every 32 units and millions every 256
   units.  ii and nn are the current tick and the total ticks */
#define tickerM(ii)							\
  tickerM3(ii, TREELER_TICKER_REPINT, TREELER_TICKER_REPINT_MAJOR)

/* macro that prints out a dot every n1 units and a percentage every
   n2 units.  ii and nn are the current tick and the total ticks */
#define ticker4(ii, nn, n1, n2)						\
  if(ii==nn) {						                \
    cerr << "(100%)" << flush;						\
  }                                                                     \
  else if (((ii) & (n1)) == 0) {					\
    if(((ii) & (n2)) == 0) {						\
      const double pct = 100.0*(double)(ii)/(double)(nn);		\
      cerr << fixed << setprecision(0)					\
	   << '(' << pct << "%)" << flush;				\
    } else {								\
      cerr << '.' << flush;						\
    }									\
  }

/* macro that prints out a dot every n1 units and a value in thousands
   every n2 units.  ii is the current tick */
#define tickerK3(ii, n1, n2)						\
  if(((ii) & (n1)) == 0) {						\
    if(((ii) & (n2)) == 0) {						\
      const double k = (double)(ii)/(double)(1000.0);			\
      cerr << fixed << setprecision(1)					\
	   << '(' << k << "k)" << flush;				\
    } else {								\
      cerr << '.' << flush;						\
    }									\
  }

/* macro that prints out a dot every n1 units and a value in millions
   every n2 units.  ii is the current tick */
#define tickerM3(ii, n1, n2)						\
  if(((ii) & (n1)) == 0) {						\
    if(((ii) & (n2)) == 0) {						\
      const double m = (double)(ii)/(double)(1000000.0);		\
      cerr << fixed << setprecision(1)					\
	   << '(' << m << "m)" << flush;				\
    } else {								\
      cerr << '.' << flush;						\
    }									\
  }


#define ticker64_start()                                                \
  const int ticker64_mask1 = TREELER_TICKER_REPINT;                      \
  const int ticker64_mask2 = (0x3f << TREELER_TICKER_REPINT_LOG);	\
  timer_start()

#define ticker64(ii, nn) 						\
  if(((ii) & ticker64_mask1) == 0) {                                    \
    if(((ii) & ticker64_mask2) == 0) {					\
      /* cerr << "\r              [                                " */	\
      /* "                            ]\r";*/				\
      cerr << "\r             [                                "	\
	"                                ]\r";				\
      /* print percentages and ETC */					\
      const double ratio = (double)(ii)/(double)(nn);			\
      int percent = (int)(100.0*ratio);					\
      if(percent >= 100) { percent = 99; }				\
      char buf[16];							\
      sprintf(buf, "%2d%% ", percent);					\
      cerr << buf;							\
      if(ratio != 0) {							\
	double split = 0;						\
	timer_split(split);						\
	cerr << timestr((1 - ratio)*(split/ratio));		\
      } else {								\
	cerr << "??:??:??";						\
      }									\
      cerr << " [";							\
    }									\
    cerr << '.' << flush;						\
  }

#define ticker64_finish(nn) {				\
    double split;					\
    timer_split(split);					\
    cerr << "\rEND " << timestr(split) << endl;		\
  }


namespace treeler {
  /* variables controlling how often ticks are printed */
  extern int TREELER_TICKER_REPINT_LOG;
  extern int TREELER_TICKER_REPINT;
  extern int TREELER_TICKER_REPINT_MAJOR;
  /* adjust the reporting intervals by 2^{log_factor} */
  void ticker_adjust_repint(int log_factor);

  /* (non-reentrant) helper function that pretty-prints a time
     interval. */
  const char* timestr(double seconds);
}


#endif /* TREELER_TIMER_H */
