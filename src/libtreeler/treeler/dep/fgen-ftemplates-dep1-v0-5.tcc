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
 * \file   fgen-ftemplates-dep1.tcc
 * \brief  Implementation of FGenFTemplatesDep1v05
 * \author Xavier Carreras
 */

#include "treeler/dep/fgen-ftemplate-types.h"

#include <iostream>
#include <cstdlib>

using namespace std;

namespace treeler {

  template<class Sentence>
  void FGenFTemplatesDep1v05::phi_token(const Sentence& x, const int word_limit, const bool use_pos,  std::list<FeatureIdx> F[]) {
    
#define addfeat(type, data) {				     \
      const int x = data;				     \
      f_i.push_back(feature_idx_w(FG_TOK_ ## type, x));      \
    }

#define addfeat2(type, data1, data2) {                          \
      const int x1 = data1;                                     \
      const int x2 = data2;					\
      f_i.push_back(feature_idx_wp(FG_TOK_ ## type, x1, x2));	\
    }

#define addfeat2f(type, data1, data2) {                         \
      const int x1 = data1;                                     \
      const int x2 = data2;					\
      f_i.push_back(feature_idx_wf(FG_TOK_ ## type, x1, x2));	\
    }
    
    const int N = x.size();
    for(int i = 0; i < N; ++i) {
      list<FeatureIdx>& f_i = F[i];
      
      /* the token to be inspected */
      const typename Sentence::Token& tok = x.get_token(i);
      
      const int realword = tok.word();
      const int w = ((realword < word_limit) ? realword : -1);
      if(w != -1) {
	addfeat(WORD, w);
      }
      if(tok.lemma() >= 0) {
        addfeat(LEMMA, tok.lemma());
      }

      if(use_pos) {
        
        addfeat(CTAG, tok.coarse_pos());
        if(tok.fine_pos() >= 0) {
          addfeat(FTAG, tok.fine_pos());
        }
        addfeat2(WORD_CTAG, w, tok.coarse_pos());
      }
      
      /* the morpho-syntactic features, if available */
      /*for(list<int>::const_iterator it = tok.morphos().begin();
          it != tok.morphos().end(); ++it) {
        addfeat(MORPHO, *it);
        if(w != -1) {
          addfeat2f(WORD_MORPHO, w, *it);
        }
      }*/
    }
#undef addfeat
#undef addfeat2
#undef addfeatf
  }
  
  template<class Sentence>
  void FGenFTemplatesDep1v05::phi_root_token(const Sentence& x, 
					  const int word_limit, const bool use_pos,  
					  std::list<FeatureIdx>& F) {
#define addfeat(type, x)			\
    F.push_back(feature_idx_w(FG_TOK_ ## type, x)); 

    /* add null tokens */
    addfeat(WORD, feature_idx_nullw);
    addfeat(LEMMA, feature_idx_nullw);
    if(use_pos) {
      addfeat(CTAG, feature_idx_nullw);
      addfeat(FTAG, feature_idx_nullw);
    }

#undef addfeat
  }

  
  template<class Sentence>
  void FGenFTemplatesDep1v05::phi_token_context(const Sentence& x,
					     const int n,
					     const int word_limit,
               const bool use_pos, 
               list<FeatureIdx> F[]) {
    assert(n <= 2);

#define dupfeat(type, cur_fidx)					\
    f_i.push_back(feature_idx_catt(cur_fidx, FG_TOK_ ## type)); 
#define addfeat(type, data) {				\
  const int x = data;				\
  f_i.push_back(feature_idx_w(FG_TOK_ ## type, x));	\
}

    const int N = x.size();

    /* create features based on replication of the existing token
       features */
    list<FeatureIdx>* Fnew = new list<FeatureIdx>[N];
    for(int i = 0; i < N; ++i) {
      list<FeatureIdx>& f_i = Fnew[i];
      /* features from +1/-1 token */
      if(n >= 1) {
        /* previous token features */
        if(i > 0) {
          const list<FeatureIdx>& fprev = F[i - 1];
          list<FeatureIdx>::const_iterator it = fprev.begin();
          const list<FeatureIdx>::const_iterator it_end = fprev.end();
          for(; it != it_end; ++it) {
            dupfeat(PREV1, *it);
          }
        } else {
          addfeat(PREVNULL, 1);
        }
        /* next token features */
        if(i < N - 1) {
          const list<FeatureIdx>& fnext = F[i + 1];
          list<FeatureIdx>::const_iterator it = fnext.begin();
          const list<FeatureIdx>::const_iterator it_end = fnext.end();
          for(; it != it_end; ++it) {
            dupfeat(NEXT1, *it);
          }
        } else {
          addfeat(NEXTNULL, 1);
        }
      }
      /* features from +2/-2 token */
      if(n >= 2) {
        /* previous token features */
        if(i > 1) {
          const list<FeatureIdx>& fprev = F[i - 2];
          list<FeatureIdx>::const_iterator it = fprev.begin();
          const list<FeatureIdx>::const_iterator it_end = fprev.end();
          for(; it != it_end; ++it) {
            dupfeat(PREV2, *it);
          }
        } else {
          addfeat(PREVNULL, 2);
        }
        /* next token features */
        if(i < N - 2) {
          const list<FeatureIdx>& fnext = F[i + 2];
          list<FeatureIdx>::const_iterator it = fnext.begin();
          const list<FeatureIdx>::const_iterator it_end = fnext.end();
          for(; it != it_end; ++it) {
            dupfeat(NEXT2, *it);
          }
        } else {
          addfeat(NEXTNULL, 2);
        }
      }
    }

    for(int i = 0 ; i < N; ++i) {
      /* splice the new features into the current features.  note that
         this is constant time for linked lists. */
      list<FeatureIdx>& fnew = Fnew[i];
      list<FeatureIdx>& f = F[i];
      f.splice(f.begin(), fnew);
    }
    delete [] Fnew;


#define addbigram(type, data1, data2) {					\
  const int x1 = data1;						\
  const int x2 = data2;						\
  f_i.push_back(feature_idx_pp(FG_TOK_ ## type ## _BIGRAM, x1, x2));	\
}
#define addtrigram(type, data1, data2, data3) {				\
  const int x1 = data1;						\
  const int x2 = data2;						\
  const int x3 = data3;						\
  f_i.push_back(feature_idx_ppp(FG_TOK_ ## type ## _TRIGRAM,		\
        x1, x2, x3));			\
}

    if(use_pos) {
      int* const arr = (int*)malloc((N + 2)*sizeof(int));
      arr[0] = arr[N + 1] = feature_idx_nullp;
      for(int i = 0; i < N; ++i) {
        arr[i + 1] = x.get_token(i).coarse_pos();
      }
      const int* const ctag = arr + 1;
      for(int i = 0; i < N; ++i) {
        list<FeatureIdx>& f_i = F[i];
        if(n >= 1) {
          if(i >= 0) {
            addbigram(PREV_CTAG, ctag[i], ctag[i - 1]);
          }
          if(i < N) {
            addbigram(NEXT_CTAG, ctag[i], ctag[i + 1]);
          }
        }
        if(n >= 2) {
          if(i >= 1) {
            addtrigram(PREV_CTAG, ctag[i], ctag[i - 1], ctag[i - 2]);
          }
          if(i < N - 1) {
            addtrigram(NEXT_CTAG, ctag[i], ctag[i + 1], ctag[i + 2]);
          }
        }

      }
      free(arr);
    }
#undef dupfeat
#undef addfeat
#undef addbigram
#undef addtrigram
}



  
  template<class Sentence>
  void FGenFTemplatesDep1v05::phi_dependency(const Sentence& x, 
					  const int word_limit, const bool use_pos,  
					  std::list<FeatureIdx> F[]) {
    
#define addpair(type, in_ft) {				\
      const FeatureIdx ft =					\
 	feature_idx_catt(feature_idx_catd(in_ft, 0),	\
			 FG_DEP_ ## type);		\
      f_se.push_back(ft);					\
      f_es.push_back(feature_idx_setd(ft));			\
    }
#define addroot(type, in_ft) {						\
      f_i.push_back(feature_idx_catt(feature_idx_catd(in_ft, 0),	\
				     FG_DEP_ ## type));			\
    }


#define addrootww(type, w1, w2) {				\
      if(w1 != -1 && w2 != -1) {					\
	addroot(type, feature_idx_catw(w1, w2));			\
      }									\
    }
#define addrootwwp(type, w1, w2, p3) {				\
      if(w1 != -1 && w2 != -1 && p3 != -1) {				\
	addroot(type, feature_idx_catp(feature_idx_catw(w1, w2), p3));	\
      }									\
    }
#define addrootwwpp(type, w1, w2, p3, p4) {				\
      if(w1 != -1 && w2 != -1 && p3 != -1 && p4 != -1) {		\
	addroot(type, feature_idx_catp(feature_idx_catp(feature_idx_catw(w1, w2), \
							p3),		\
				       p4));				\
      }									\
    }

    const int N = x.size();

    int* const ctag = (int*)malloc(N*sizeof(int));
    int* const word = (int*)malloc(N*sizeof(int));
    for(int i = 0; i < N; ++i) { /* root position */
      list<FeatureIdx>& f_i = F[i + N*i];

      const typename Sentence::Token& tok = x.get_token(i);
      const int et = (use_pos ? tok.coarse_pos() : -1);
      const int ew = (tok.word() < word_limit ? tok.word() : -1);

      addrootww(SW_EW, feature_idx_nullw, ew);
      addrootww(ST_ET, feature_idx_nullp, et);
      addrootwwp(ST_EWT, feature_idx_nullp, ew, et);
      addrootwwp(SW_EWT, feature_idx_nullw, ew, et);
      addrootwwp(SWT_EW, feature_idx_nullw, ew, feature_idx_nullp);
      addrootwwp(SWT_ET, feature_idx_nullw, et, feature_idx_nullp);
      addrootwwpp(SWT_EWT, feature_idx_nullw, ew, feature_idx_nullp, et);

      const list<int>& em = tok.morpho_tags();
      const list<int>::const_iterator iend = em.end();
      for(list<int>::const_iterator it = em.begin(); it != iend; ++it) {
        const int emorph = *it;
        addrootww(ST_EM, feature_idx_nullp, emorph);
        addrootwwp(ST_EMT, feature_idx_nullp, emorph, et);
      }

      ctag[i] = et;
      word[i] = ew;
    }


#define addpairww(type, w1, w2) {					\
      if(w1 != -1 && w2 != -1) {					\
	addpair(type, feature_idx_catw(w1, w2));			\
      }									\
    }
#define addpairpp(type, w1, w2) {					\
	addpair(type, feature_idx_catp(w1, w2));			\
    }

#define addpairwwp(type, w1, w2, p3) {					\
      if(w1 != -1 && w2 != -1 && p3 != -1) {				\
	addpair(type, feature_idx_catp(feature_idx_catw(w1, w2), p3));	\
      }									\
    }

#define addpairwpp(type, w1, w2, p3) {					\
      if(w1 != -1 && w2 != -1 && p3 != -1) {				\
	addpair(type, feature_idx_catp(feature_idx_catp(w1, w2), p3));	\
      }									\
}

#define addpairwwpp(type, w1, w2, p3, p4) {				\
      if(w1 != -1 && w2 != -1 && p3 != -1 && p4 != -1) {		\
	addpair(type, feature_idx_catp(feature_idx_catp(feature_idx_catw(w1, w2), \
							p3),		\
				       p4));				\
      }									\
    }
#define addpairwppp(type, w1, w2, p3, p4) {				\
      if(w1 != -1 && w2 != -1 && p3 != -1 && p4 != -1) {		\
	addpair(type, feature_idx_catp(feature_idx_catp(feature_idx_catp(w1, w2), \
							p3),		\
				       p4));				\
      }									\
    }

    for(int e = 1; e < N; ++e) { /* end position */
      const int et = ctag[e];
      const int ew = word[e];
      const list<int>& em = x.get_token(e).morpho_tags();
      if(et == -1 && ew == -1 && em.empty()) { continue; }

      for(int s = 0; s < e; ++s) { /* start position */
        const int st = ctag[s];
        const int sw = word[s];
        const list<int>& sm = x.get_token(s).morpho_tags();
        if(st == -1 && sw == -1 && sm.empty()) { continue; }

        list<FeatureIdx>& f_se = F[s + N*e];
        list<FeatureIdx>& f_es = F[e + N*s];

        addpairww(SW_EW, sw, ew);
        addpairpp(ST_ET, st, et); //
        addpairwwp(ST_EWT, st, ew, et); 
        addpairwwp(SW_EWT, sw, ew, et);
        addpairwwp(SWT_EW, sw, ew, st);
        addpairwpp(SWT_ET, sw, et, st);
        addpairwwpp(SWT_EWT, sw, ew, st, et);

        const list<int>::const_iterator eend = em.end();
        for(list<int>::const_iterator ei = em.begin(); ei != eend; ++ei) {
          const int emorph = *ei;
          addpairpp(ST_EM, st, emorph);
          addpairwpp(ST_EMT, st, emorph, et);
        }
        const list<int>::const_iterator send = sm.end();
        for(list<int>::const_iterator si = sm.begin(); si != send; ++si) {
          const int smorph = *si;
          addpairpp(SM_ET, smorph, et);
          addpairwpp(SMT_ET, smorph, et, st);
          for(list<int>::const_iterator ei = em.begin(); ei != eend; ++ei) {
            const int emorph = *ei;
            addpairpp(SM_EM, smorph, emorph);
            addpairwpp(SMT_EM, smorph, emorph, st);
            addpairwpp(SM_EMT, smorph, emorph, et);
            addpairwppp(SMT_EMT, smorph, emorph, st, et);
          }
        }
      }
    }
    free(ctag);
    free(word);

#undef addpair
#undef addroot
#undef addrootww
#undef addrootwwp
#undef addrootwwpp
#undef addpairww
#undef addpairwwp
#undef addpairwwpp
  }


  template <class Sentence>
  void FGenFTemplatesDep1v05::phi_dependency_context(const Sentence& x,
						  const int word_limit,
						  const bool use_pos, 
						  list<FeatureIdx> F[]) {
    if(!use_pos) { return; }
    
#define addrootfeat2(type, x1, x2) {				\
      f_i.push_back(feature_idx_pp(FG_DEP_CTXT_ROOT_ ## type,	\
				   x1, x2));			\
    }
#define addrootfeat3(type, x1, x2, x3) {			\
      f_i.push_back(feature_idx_ppp(FG_DEP_CTXT_ROOT_ ## type,	\
				    x1, x2, x3));		\
    }
    
#define addpair(type, in_fidx) {				\
      const FeatureIdx fidx =					\
	feature_idx_catt(feature_idx_catd(in_fidx, 0),		\
			 FG_DEP_CTXT_ ## type);			\
      f_se.push_back(fidx);					\
      f_es.push_back(feature_idx_setd(fidx));			\
    }
    
    const int N = x.size();
    int* ctag = (int*)malloc((N + 2)*sizeof(int));
    ctag[0] = ctag[N + 1] = feature_idx_nullp;
    ++ctag;

    /* collect coarse tags */
    for(int i = 0; i < N; ++i) {
      ctag[i] = x.get_token(i).coarse_pos();
    }

    for(int i = 0; i < N; ++i) { /* root position */
      list<FeatureIdx>& f_i = F[i + N*i];
      const int ept = ctag[i - 1];
      const int et  = ctag[i];
      const int ent = ctag[i + 1];
      addrootfeat2(EP, et, ept);
      addrootfeat2(EN, et, ent);
      addrootfeat3(EPN, et, ept, ent);
      /*if(i > 0) { // adjacent dependencies 
        cerr << "adding adjacent " << endl;
        list<FeatureIdx>& f_se = F[(i - 1) + N*i];
        list<FeatureIdx>& f_es = F[i + N*(i - 1)];
        const int spt = ctag[i - 2];
        const FeatureIdx fidx_e_s =
          feature_idx_catp(feature_idx_catp(feature_idx_zero, et), ept);
        addpair(ADJ_S_EN,  feature_idx_catp(fidx_e_s, ent));
        const FeatureIdx fidx_e_sp = feature_idx_catp(fidx_e_s, spt);
        addpair(ADJ_SP_E,  fidx_e_sp);
        addpair(ADJ_SP_EN, feature_idx_catp(fidx_e_sp, ent));
      }*/
    }

    for(int e = 2; e < N; ++e) { /* end position */
      const int et = ctag[e];
      const int ept = ctag[e - 1];
      const int ent = ctag[e + 1];

      const FeatureIdx fidx_e = feature_idx_catp(feature_idx_zero, et);
      const FeatureIdx fidx_ep = feature_idx_catp(fidx_e, ept);
      const FeatureIdx fidx_en = feature_idx_catp(fidx_e, ent);

      for(int s = e - 2; s >= 0; --s) { /* start position */
        list<FeatureIdx>& f_se = F[s + N*e];
        list<FeatureIdx>& f_es = F[e + N*s];

        const int st = ctag[s];
        const int spt = ctag[s - 1];
        const int snt = ctag[s + 1];

        {
          const FeatureIdx fidx_e_s = feature_idx_catp(fidx_e, st);
          addpair(SP_E, feature_idx_catp(fidx_e_s, spt));
          addpair(SN_E, feature_idx_catp(fidx_e_s, snt));
        }
        {
          const FeatureIdx fidx_ep_s = feature_idx_catp(fidx_ep, st);
          addpair(S_EP, fidx_ep_s);
          addpair(SP_EP, feature_idx_catp(fidx_ep_s, spt));
          addpair(SN_EP, feature_idx_catp(fidx_ep_s, snt));
        }
        {
          const FeatureIdx fidx_en_s = feature_idx_catp(fidx_en, st);
          addpair(S_EN, fidx_en_s);
          addpair(SP_EN, feature_idx_catp(fidx_en_s, spt));
          addpair(SN_EN, feature_idx_catp(fidx_en_s, snt));
        }
      }

    }

    /* recover the original pointer for deletion */
    free((void*)(ctag - 1));

#undef addrootfeat2
#undef addrootfeat3
#undef addpair
  }


  template <typename Sentence>
  void FGenFTemplatesDep1v05::phi_dependency_distance(const Sentence& x, const int word_limit, const bool use_pos,
						   std::list<FeatureIdx> F[]) {
#define addpair(type, in_fidx) {				\
      const FeatureIdx fidx =					\
	feature_idx_catt(feature_idx_catd(in_fidx, 0),		\
			 FG_DEP_DIST_ ## type);			\
      f_se.push_back(fidx);					\
      f_es.push_back(feature_idx_setd(fidx));			\
    }
    
    const int N = x.size();
    /* prefetch coarse tags */
    int* const ctag = (int*)malloc(N*sizeof(int));
    for(int i = 0; i < N; ++i) {
      ctag[i] = x.get_token(i).coarse_pos();
    }

    for(int e = 1; e < N; ++e) { /* end position */
      const int ctag_e = ctag[e];
      const FeatureIdx fidx_e = feature_idx_catp(feature_idx_zero, ctag_e);
      for(int s = 0; s < e; ++s) { /* start position */
        list<FeatureIdx>& f_se = F[s + N*e];
        list<FeatureIdx>& f_es = F[e + N*s];

        const int dist = e - s;

        /* feature for the actual length of the dependency */
        const int plen = (dist > TREELER_FTEMPLATES_TOKEN_DISTANCE_THRESHOLD
            ? TREELER_FTEMPLATES_TOKEN_DISTANCE_THRESHOLD : dist);
        addpair(EXACT, feature_idx_catw(feature_idx_zero, plen));

        /* length-binned features */
#define FGbin1(len)						\
        if(dist > len) {						\
          addpair(BINGT, feature_idx_catp(feature_idx_zero, len));	\
        }

        FGbin1(2);
        FGbin1(5);
        FGbin1(10);
        FGbin1(20);
        FGbin1(30);
        FGbin1(40);

        if(use_pos) {
          const int ctag_s = ctag[s];
          const FeatureIdx fidx_e_s = feature_idx_catp(fidx_e, ctag_s);
          const FeatureIdx fidx_s = feature_idx_catp(feature_idx_zero, ctag_s);
#define FGbin2(len)						\
          if(dist > len) {					\
            addpair(BINGT_SCTAG, feature_idx_catp(fidx_s, len));	\
            addpair(BINGT_ECTAG, feature_idx_catp(fidx_e, len));	\
            addpair(BINGT_SECTAG, feature_idx_catp(fidx_e_s, len));	\
          }

          FGbin2(2);
          FGbin2(5);
          FGbin2(10);
          FGbin2(20);
          FGbin2(30);
          FGbin2(40);
        }
      }

    }

    free(ctag);

#undef addpair
#undef FGbin1
#undef FGbin2
  }


template <typename Sentence>
void FGenFTemplatesDep1v05::phi_dependency_between(const DepSymbols& symbols, 
    const Sentence& x, 
    const int word_limit, const bool use_pos,
    std::list<FeatureIdx> F[]) {
  if(!use_pos) { return; }
#define addpair(type, in_fidx) {				\
  const FeatureIdx fidx =					\
  feature_idx_catt(feature_idx_catd(in_fidx, 0),		\
      FG_DEP_BETW_ ## type);			\
  f_se.push_back(fidx);					\
  f_es.push_back(feature_idx_setd(fidx));			\
}

  const int N = x.size();
  const int NCT = symbols.d_coarse_pos.size() + 1;

  char* const flags = (char*)calloc(NCT, sizeof(char));   
  int* const ctag = (int*)malloc(N*sizeof(int));
  int* const vcnt = (int*)malloc(N*sizeof(int));
  int* const pcnt = (int*)malloc(N*sizeof(int));
  int* const ccnt = (int*)malloc(N*sizeof(int));

  /* First traverse the sentence to precompute things */
  for(int i = 0 ; i < N; ++i) {
    const typename Sentence::Token& t = x.get_token(i);
    ctag[i] = t.coarse_pos();
    if(i > 0) {
      vcnt[i] = vcnt[i - 1];
      pcnt[i] = pcnt[i - 1];
      ccnt[i] = ccnt[i - 1];
    } else {
      vcnt[i] = pcnt[i] = ccnt[i] = 0;
    }
    if(symbols.is_verb(t.fine_pos()))  { ++(vcnt[i]); }
    if(symbols.is_punc(t.fine_pos()))  { ++(pcnt[i]); }
    if(symbols.is_coord(t.fine_pos())) { ++(ccnt[i]); }
  }

/* Second, visit each dependency and generate its features */
for(int s = N - 2; s >= 0; --s) {
  const int st = ctag[s];
  const FeatureIdx fidx_s = feature_idx_catp(feature_idx_zero, st);
  for(int e = s + 1; e < N; ++e) {
    list<FeatureIdx>& f_se = F[s + N*e];
    list<FeatureIdx>& f_es = F[e + N*s];

    const int et = ctag[e];
    const FeatureIdx fidx_s_e = feature_idx_catp(fidx_s, et);

    /* only check POS-between features if there is at least one
       intervening token */
    if(e > s + 1) {
      /* use the flags to ensure that only one feature is generated
         for each unique s,e,b tag triple */
      for(int b = s + 1; b < e; ++b) {
        const int bt = ctag[b];
        if(flags[bt] == 0) {
          flags[bt] = 1;
          addpair(SEB, feature_idx_catp(fidx_s_e, bt));
        }
      }

      /* reset flags, either by retracing the ctag array or just
         zeroing the array, depending on which implies fewer
         operations */
      if(e - s < (NCT/2)) {
        for(int b = s + 1; b < e; ++b) { flags[ctag[b]] = 0; }
      } else {
        for(int i = 0; i < NCT; ++i) { flags[i] = 0; }
      }
    }

    /* number of verbs that appear between the two tokens
       (i.e. from token s+1 to token e-1 both included) */
    int vcount = vcnt[e - 1] - vcnt[s];
    if(vcount > TREELER_FTEMPLATES_DEP1_VCOUNT_MAX) { vcount = TREELER_FTEMPLATES_DEP1_VCOUNT_MAX; }
    addpair(SE_VCNT, feature_idx_catp(fidx_s_e, vcount));
    addpair(VCNT,    feature_idx_catp(feature_idx_zero, vcount));

    /* number of punctuation that appear between the two tokens
       (i.e. from toxken s+1 to token e-1 both included) */
    int pcount = pcnt[e - 1] - pcnt[s];
    if(pcount > TREELER_FTEMPLATES_DEP1_PCOUNT_MAX) { pcount = TREELER_FTEMPLATES_DEP1_PCOUNT_MAX; }
    addpair(SE_PCNT, feature_idx_catp(fidx_s_e, pcount));
    addpair(PCNT,    feature_idx_catp(feature_idx_zero, pcount));

    /* number of coordinations that appear between the two tokens
       (i.e. from token s+1 to token e-1 both included) */
    int ccount = ccnt[e - 1] - ccnt[s];
    if(ccount > TREELER_FTEMPLATES_DEP1_CCOUNT_MAX) { ccount = TREELER_FTEMPLATES_DEP1_CCOUNT_MAX; }
    addpair(SE_CCNT, feature_idx_catp(fidx_s_e, ccount));
    addpair(CCNT,    feature_idx_catp(feature_idx_zero, ccount));

  }
}

free(ctag);
free(vcnt);
free(pcnt);
free(ccnt);
free(flags);
#undef addpair
}



} // treeler
