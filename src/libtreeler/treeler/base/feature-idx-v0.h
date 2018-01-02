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
 * \file   feature-idx.h
 * \brief  Defines basic type for feature index
 * \author Terry Koo
 * \note   This file was ported from egstra
 */

#ifndef TREELER_FEATUREIDX_H
#define TREELER_FEATUREIDX_H

#include "treeler/base/windll.h"

#include <stdint.h>
#include <cstring>

namespace treeler {
  /**  
   * \brief A bit-string representation of a tuple of values, together
   *        with macros for setting the values of the tuple.
   * \author Terry Koo
   * 
   * The current implementation uses a 64-bit integer.  The
   * lowest-order 8 bits are reserved for the "type" of the feature
   * (e.g., pos-tag-trigram, word-word bilexical).  It is up to the
   * user to define the types and keep them consistent (as well as
   * define less than 256 of them).  The rest of the bits can be used
   * in any fashion, although it may be easiest to use the utilities
   * to divide the remaining 56 bits into orderly blocks.
   */
  typedef uint64_t FeatureIdx;

  /** 
   *  \brief A hash function for FeatureIdx, based on the high-quality hash
   *         function from Bob Jenkins' "lookup3"
   *  \author Terry Koo
   * 
   */
  class FeatureIdxHash {
  public:
    inline size_t operator()(const FeatureIdx& t) const {
      /* grab low-order bits */
      register uint32_t a = (uint32_t)(t & 0xffffffff);
      /* grab high-order bits */
      register uint32_t b = (uint32_t)((t >> 32) & 0xffffffff);
      /* nothing to grab */
      register uint32_t c = 0;

#define rot(x, k) ((x << k) | (x >> (32 - k)))

      /* initial stage; mix (a, b, c) */
      a -= c; a ^= rot(c, 4);  c += b;
      b -= a; b ^= rot(a, 6);  a += c;
      c -= b; c ^= rot(b, 8);  b += a;
      a -= c; a ^= rot(c, 16); c += b;
      b -= a; b ^= rot(a, 19); a += c;
      c -= b; c ^= rot(b, 4);  b += a;
      /* final stage; (a, b, c) => c */
      c ^= b; c -= rot(b, 14);
      a ^= c; a -= rot(c, 11);
      b ^= a; b -= rot(a, 25);
      c ^= b; c -= rot(b, 16);
      a ^= c; a -= rot(c, 4);
      b ^= a; b -= rot(a, 14);
      c ^= b; c -= rot(b, 24);

#undef rot

      return c;
    }
  };  // FeatureIdxHash


  /// Equality function, it just uses numerical equality
  class FeatureIdxEquality {
  public:
    inline bool operator()(const FeatureIdx& t1, const FeatureIdx& t2) const {
      return (t1 == t2);
    }
  };


}


/* define this to add checks for overflow */
#define TREELER_FEATUREIDX_CHECK_BOUNDS

/******************************************************************
 * higher-level convenience macros for various NLP data types     *
 ******************************************************************/

/* these macros have the general calling style:
     feature_idx_w(ftype t, int w);
     feature_idx_ww(ftype t, int w1, int w2);
   where ftype is an integer feature-type identifier, and produce an
   feature_idx as a return value */

/* a tuple of one word */
#define feature_idx_w(t, w1)				\
  feature_idx_catt(	\
  feature_idx_catw(	\
  feature_idx_zero,	\
  w1),		\
  t)
/* a tuple of two words */
#define feature_idx_ww(t, w1, w2)			\
  feature_idx_catt(	\
  feature_idx_catw(	\
  feature_idx_catw(	\
  feature_idx_zero,	\
  w2),		\
  w1),		\
  t)
/* a tuple of two words with direction */
#define feature_idx_dww(t, d, w1, w2)		\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catw(	\
  feature_idx_catw(	\
  feature_idx_zero,	\
  w2),		\
  w1),		\
  d),		\
  t)

/* a tuple of one part of speech */
#define feature_idx_p(t, p1)				\
  feature_idx_catt(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p1),		\
  t)
/* a tuple of two parts of speech */
#define feature_idx_pp(t, p1, p2)			\
  feature_idx_catt(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p2),		\
  p1),		\
  t)



/* a tuple of three parts of speech */
#define feature_idx_ppp(t, p1, p2, p3)		\
  feature_idx_catt(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p3),		\
  p2),		\
  p1),		\
  t)
/* a tuple of four parts of speech */
#define feature_idx_pppp(t, p1, p2, p3, p4)		\
  feature_idx_catt(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p4),		\
  p3),		\
  p2),		\
  p1),		\
  t)

/* a tuple of one part of speech with direction */
#define feature_idx_dp(t, d, p1)			\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p1),		\
  d),		\
  t)
/* a tuple of two parts of speech with direction */
#define feature_idx_dpp(t, d, p1, p2)		\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p2),		\
  p1),		\
  d),		\
  t)
/* a tuple of three parts of speech with direction */
#define feature_idx_dppp(t, d, p1, p2, p3)		\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p3),		\
  p2),		\
  p1),		\
  d),		\
  t)
/* a tuple of four parts of speech with direction */
#define feature_idx_dpppp(t, d, p1, p2, p3, p4)	\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p4),		\
  p3),		\
  p2),		\
  p1),		\
  d),		\
  t)


/* a tuple of one word and one part of speech */
#define feature_idx_wp(t, w1, p2)			\
  feature_idx_catt(	\
  feature_idx_catw(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p2),		\
  w1),		\
  t)
/* a tuple of one word and one frequent word */
#define feature_idx_wf(t, w1, f2)			\
  feature_idx_catt(	\
  feature_idx_catw(	\
  feature_idx_catf(	\
  feature_idx_zero,	\
  f2),		\
  w1),		\
  t)
/* a tuple of one word and one part of speech with direction */
#define feature_idx_dwp(t, d, w1, p2)		\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catw(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p2),		\
  w1),		\
  d),		\
  t)
/* a tuple of two words and one part of speech with direction */
#define feature_idx_dwwp(t, d, w1, w2, p3)		\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catw(	\
  feature_idx_catw(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p3),		\
  w2),		\
  w1),		\
  d),		\
  t)
/* a tuple of two words and two parts of speech with direction */
#define feature_idx_dwwpp(t, d, w1, w2, p3, p4)	\
  feature_idx_catt(	\
  feature_idx_catd(	\
  feature_idx_catw(	\
  feature_idx_catw(	\
  feature_idx_catp(	\
  feature_idx_catp(	\
  feature_idx_zero,	\
  p4),		\
  p3),		\
  w2),		\
  w1),		\
  d),		\
  t)

/* concatenate a word onto an existing feature_idx. */
#define feature_idx_catw(ft, w) FEATUREIDX_CAT(ft, w, FEATUREIDX_WORDSIZE)

/* concatenate a large-vocabulary word onto an existing feature_idx. */
#define feature_idx_catW(ft, w) FEATUREIDX_CAT(ft, w, FEATUREIDX_BIGWORDSIZE)

/* concatenate a word onto an existing feature_idx. */
#define feature_idx_catf(ft, f) FEATUREIDX_CAT(ft, f, FEATUREIDX_FREQWORDSIZE)

/* concatenate a part of speech tag onto an existing feature_idx. */
#define feature_idx_catp(ft, p) FEATUREIDX_CAT(ft, p, FEATUREIDX_POSSIZE)

/* concatenate a 2-bit header onto an existing feature_idx. */
#define feature_idx_cat2(ft, h) FEATUREIDX_CAT(ft, h, 2)

/* concatenate a 3-bit header onto an existing feature_idx. */
#define feature_idx_cat3(ft, h) FEATUREIDX_CAT(ft, h, 3)

/* concatenate a freqword+2-bit header onto an existing feature_idx. */
#define feature_idx_catf2(ft, f2) FEATUREIDX_CAT(ft, f2, (FEATUREIDX_FREQWORDSIZE + 2))

/* concatenate a freqword+3-bit header onto an existing feature_idx. */
#define feature_idx_catf3(ft, f3) FEATUREIDX_CAT(ft, f3, (FEATUREIDX_FREQWORDSIZE + 3))

/* concatenate a freqword+3-bit header onto an existing feature_idx. */
#define feature_idx_catw3(ft, w3) FEATUREIDX_CAT(ft, w3, (FEATUREIDX_WORDSIZE + 3))

/* concatenate a direction bit onto an existing feature_idx. */
#define feature_idx_catd(ft, d) FEATUREIDX_CAT(ft, d, 1)

/* concatenate a feature type identifier onto an existing tuple. */
#define feature_idx_catt(ft, t) FEATUREIDX_CAT(ft, t, FEATUREIDX_TYPESIZE)

/* special macro to set the direction bit on a pre-existing feature_idx.
   this is a very special-case macro and is primarily used to quickly
   generate features for both forward- and backward-pointing
   dependencies. */
#define feature_idx_setd(ft) (((FeatureIdx)ft) | (1 << FEATUREIDX_TYPESIZE))

/* macro that returns a "null" token that is not within the normal
   range of the given datatype */
#define feature_idx_nullw FEATUREIDX_MASK(FEATUREIDX_WORDSIZE)
#define feature_idx_nullW FEATUREIDX_MASK(FEATUREIDX_BIGWORDSIZE)
#define feature_idx_nullf FEATUREIDX_MASK(FEATUREIDX_FREQWORDSIZE)
#define feature_idx_nullt FEATUREIDX_MASK(FEATUREIDX_TYPESIZE)
#define feature_idx_nullp FEATUREIDX_MASK(FEATUREIDX_POSSIZE)

/* zeroed feature tuple */
#define feature_idx_zero ((FeatureIdx)0)


/* macros to read and write tuples.  since ISO C++ does not explicitly
   support 64-bit integers, we do this by two separate prints on the
   high and low order bits */
#define feature_idx_fread(f, nread, ft) {		\
    /* assert(sizeof(ft) == sizeof(FeatureIdx)); */	\
    uint32_t x;					\
    int ft_fr_nread = fscanf(f, "%8x", &x);	\
    if(ft_fr_nread <= 0) {			\
      nread = ft_fr_nread;			\
    } else {					\
      assert(ft_fr_nread == 1);			\
      ft = 0;					\
      ft |= x;					\
      ft_fr_nread = fscanf(f, "%8x", &x);	\
      assert(ft_fr_nread == 1);			\
      ft = ((ft << 32) | x);			\
      nread = 1;				\
    }						\
  }
#define feature_idx_sscan(str, nread, ft) {		\
    /* assert(sizeof(ft) == sizeof(FeatureIdx)); */	\
    uint32_t x;					\
    int ft_fr_nread = sscanf(str, "%8x", &x);	\
    if(ft_fr_nread <= 0) {			\
      nread = ft_fr_nread;			\
    } else {					\
      assert(ft_fr_nread == 1);			\
      ft = 0;					\
      ft |= x;					\
      ft_fr_nread = sscanf(str + 8, "%8x", &x);	\
      assert(ft_fr_nread == 1);			\
      ft = ((ft << 32) | x);			\
      nread = 1;				\
    }						\
  }
#define feature_idx_sprint(str, ft) {		\
    /* assert(sizeof(ft) == sizeof(FeatureIdx)); */	\
    uint32_t x = (uint32_t)(ft >> 32);		\
    int ft_fw_nwrite = sprintf(str, "%08x", x);	\
    (void)ft_fw_nwrite;                         \
    assert(ft_fw_nwrite == 8);			\
    x = (uint32_t)(ft & 0xffffffff);		\
    ft_fw_nwrite = sprintf(str + 8, "%08x", x);	\
    assert(ft_fw_nwrite == 8);			\
  }
#define feature_idx_fwrite(f, ft) {			\
    /* assert(sizeof(ft) == sizeof(FeatureIdx)); */	\
    uint32_t x = (uint32_t)(ft >> 32);		\
    int ft_fw_nwrite = gzprintf(f, "%08x", x);	\
    (void)ft_fw_nwrite;                         \
    assert(ft_fw_nwrite == 8);			\
    x = (uint32_t)(ft & 0xffffffff);		\
    ft_fw_nwrite = gzprintf(f, "%08x", x);	\
    assert(ft_fw_nwrite == 8);			\
  }


/* bit-width of the representation */
#define FEATUREIDX_BITS 64

/* bit-width of the feature-type field */
#define FEATUREIDX_TYPESIZE 8
/* bit-width of a word field */
#define FEATUREIDX_WORDSIZE 18
/* bit-width of a large-vocabulary word field */
#define FEATUREIDX_BIGWORDSIZE 20
/* bit-width of a frequent-word field */
#define FEATUREIDX_FREQWORDSIZE 10
/* bit-width of a part-of-speech field */
#define FEATUREIDX_POSSIZE  8

#ifdef TREELER_FEATUREIDX_CHECK_BOUNDS
#include <assert.h>
/* check for overflow before concatenating */
#define FEATUREIDX_CAT(ft, x, size) \
  (assert((x) < (1 << (size))), ((((FeatureIdx)(ft)) << (size)) | (x)))
#else /* TREELER_FEATUREIDX_CHECK_BOUNDS */
/* no check for overflow */
#define FEATUREIDX_CAT(ft, x, size) \
  ((((FeatureIdx)(ft)) << (size)) | (x))
#endif /* TREELER_FEATUREIDX_CHECK_BOUNDS */

#define FEATUREIDX_MASK(width) ((((FeatureIdx)1) << width) - 1)



#endif /* BASE_FEATUREIDX_H */
