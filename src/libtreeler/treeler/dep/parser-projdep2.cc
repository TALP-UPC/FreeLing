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
 * \file   parser-projdep2.cc
 * \brief  Implementation of ProjDep2
 * \author Xavier Carreras
 */

#include "treeler/dep/parser-projdep2.h"

//#include "treeler/dep/part-dep2-index.h"
//#include "treeler/dep/scores-dep2.h"

//#include "treeler/base/sentence.h"
#include "treeler/dep/dep-tree.h"

#include "treeler/base/label.h"

//#include "treeler/util/math-utils.h"
//#include "treeler/util/options.h"


#include <iostream>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define TOL 1e-8


/* The part-numbering for labeled dependencies is defined through the class PartDep2Index */

using namespace std;

namespace treeler {
  
  void ProjDep2::unravel_tree(int N, Label<PartDep2>& y, const struct chart_values& CV,
			      const int h, const int e, const int m)
  {
    if (h==e) {
      // cerr << "unravel_tree_complete : h=" << h << " e=" << e << " m=" << m << " l=" << l << " : base case" << endl;
      return;
    }
    int l, r, ch, cmi, cmo;
    
    if (CV.get_cbp(h,e,m, &l,&cmo)) {
      //      cerr << "unravel_tree_com : h=" << h << " e=" << e << " m=" << m << " l=" << l << " : cm=" << cmo << " lm="<<lmo << endl;
      
      if (CV.get_ubp(h,m,l, &r,&ch,&cmi)) {
	// push factor in the solution
	//	int fid = 0;
	//      factors.push_back(o2_model::factor(h,m,l, ch,-2, cmi, -2, cmo,-2));
	// y.push_back(fid);
	
	// main dep
	// !! adding part objects instead of ids
	//int part_id = PartDep2Index::encode(N, PartDep2::FO, h, m, l);
	y.push_back(PartDep2(PartDep2::FO,h,m,-1,l));
	// head child
	//part_id = PartDep2Index::encode(N, PartDep2::SIB, h, m, l, (ch==h) ? -1 : ch);
	y.push_back(PartDep2(PartDep2::SIB,h,m,(ch==h) ? -1 : ch, l));
	// mod child inside
	//part_id = PartDep2Index::encode(N, PartDep2::CMI, h, m, l, (cmi==m) ? -1 : cmi);
	y.push_back(PartDep2(PartDep2::CMI, h, m, (cmi==m) ? -1 : cmi, l));
	// mod child outside
	//part_id = PartDep2Index::encode(N, PartDep2::CMO, h, m, l, (cmo==m) ? -1 : cmo);
	y.push_back(PartDep2(PartDep2::CMO, h, m, (cmo==m) ? -1 : cmo, l));
	
	if (h<m) {
	  unravel_tree(N,y,CV, h,r,ch);
	  unravel_tree(N,y,CV, m,r+1,cmi);
	}
	else {
	      unravel_tree(N,y,CV, m,r,cmi);
	      unravel_tree(N,y,CV, h,r+1,ch);
	    }
      }
      else {
	cerr << "unravel_tree : h=" << h << " m=" << m << " l=" << l << " : missing uncomplete back pointer" << endl;
	assert(0);
      }
      unravel_tree(N,y,CV, m,e,cmo);
    }
    else {
      cerr << "unravel_tree : h=" << h << " e=" << e << " m=" << m << " : missing complete back pointer" << endl;
      assert(0);
    }
  }
   


        
    /***************************************************************
     *
     * The following code implements auxiliary data structures to
     * perform inference with 2nd order relations
     *
     ***************************************************************/
    void ProjDep2::chart_values::set_cbp(int h, int e, int m,  int l, int cm)
    {
        C_signature s(h,e,m);
        C_backpointer bp(l,cm);

	//	cerr << "PDEP2 chart[C," << h << "," << e << "," << m << "]=[" << l << "," << cm << "]" << endl; 

        map<C_signature,C_backpointer>::value_type v(s,bp);
        CBP.insert(v);
    }
    
    void ProjDep2::chart_values::set_ubp(int h, int m, int l, int r, int ch, int cm)
    {
        U_signature s(h,m,l);
        U_backpointer bp(r,ch,cm);
        
	//	cerr << "PDEP2 chart[U," << h << "," << m << "]=[" << l << "," << ch << "," << cm << "]" << endl; 

        map<U_signature,U_backpointer>::value_type v(s,bp);
        UBP.insert(v);
    }
    
    bool
    ProjDep2::chart_values::get_cbp(int h, int e, int m, int *l, int *cm) const
    {
        C_signature s(h,e,m);
        
        map<C_signature,C_backpointer>::const_iterator i = CBP.find(s);
        if (i != CBP.end()) {
            *l = i->second._l;
            *cm = i->second._cm;
            return true;
        }
        return false;
    }
    
    bool
    ProjDep2::chart_values::get_ubp(int h, int m, int l, int *r, int *ch, int *cm) const
    {
        U_signature s(h,m,l);
        
        map<U_signature,U_backpointer>::const_iterator i = UBP.find(s);
        if ( i != UBP.end()) {
            *r = i->second._r;
            *ch = i->second._ch;
            *cm = i->second._cm;
            return true;
        }
        return false;
    }
    
    ProjDep2::chart_scores::chart_scores (int N0, int L0)
        : _CS(new double[N0*N0*N0]),
          _US(new double[N0*N0*L0]),
          _N(N0),
          _L(L0),
          _NL(_N*_L),
          _N2(_N*_N),
          _N2L(_NL*_N) {
    }
    
    ProjDep2::chart_scores::~chart_scores() {
        delete [] _CS;
        delete [] _US;
    }
    
    double
    ProjDep2::chart_scores::cscore (int h, int e, int m) const {
        if (h==-1) h=m;
        return _CS[h*_N2 + e*_N + m];
    }
    
    void
    ProjDep2::chart_scores::cscore_set (int h, int e, int m, double sco) {
        if (h==-1) h=m;
        _CS[h*_N2 + e*_N + m] = sco;
    }
    
    double
    ProjDep2::chart_scores::uscore (int h, int m, int l) const {
        if (h==-1) h=m;
        return _US[h*_NL + m*_L + l];
    }
    
    void
    ProjDep2::chart_scores::uscore_set (int h, int m, int l, double sco) {
        if (h==-1) h=m;
        _US[h*_NL + m*_L + l] = sco;
    }

    /***************************************************************
     *
     * End of auxiliary data structures
     *
     ***************************************************************/
    
    

    static inline double log_sum(const double* const arr,
                                 const double* const end) {
        assert(end > arr);
        if(end == (arr + 1)) { return *arr; }
        const double* p = arr;
        /* compute max */
        double max = *p;
        ++p;
        for(; p < end; ++p) {
            const double tmp = *p;
            if(tmp > max) { max = tmp; }
        }
        /* compute normalized sum */
        double norm_sum = 0;
        for(p = arr; p < end; ++p) { norm_sum += exp(*p - max); }
        return max + log(norm_sum);
    }
    
    
  //   /* templated on a part_score functor, which returns the score of the
  //    given part. */
  //   template<class PartScorer>
  //   double
  //   ProjDep2::inside(const Pattern* const x,
  //                    chart_scores& IS,
  //                    PartScorer& part_score) {
        
  //       // number of tokens in the sentence
  //       const int N = x->size();
        
  //       /* workspace for enumerating split points */
  //       double* const work_r = (double*)malloc((N + 1)*sizeof(double));
  //       /* workspace for enumerating child indices */
  //       double* const work_c = (double*)malloc((N + 1)*sizeof(double));
  //       /* workspace for enumerating labels */
  //       double* const work_l = (double*)malloc(_L*sizeof(double));
  //       assert(work_r != NULL);
  //       assert(work_c != NULL);
  //       assert(work_l != NULL);
        
  //       /*
  //        * W is the max width of the spans it controls the outer loop of
  //        * the viterbi search,
  //        *
  //        * when multiple roots are allowed, its value is N+1, so that
  //        * dependencies from the root are visited from token 0 to token
  //        * N-1
  //        *
  //        */
  //       const int W = (_multiroot ? (N + 1) : N);
        
  //       for(int w = 1; w < W; ++w) {
  //           // cerr << "proj_2p_parser : main loop : w=" << w << endl;
  //           const int initial_s = _multiroot ? -1 : 0;
  //           for(int s = initial_s; s < N - w; ++s) {
  //               const int e = s + w;
  //               for(int l = 0; l < _L; ++l) {
  //                   { // UNCOMPLETE STRUCTURES HEADED AT S
  //                       // consider each possible splitting point
  //                       int nworkr = 0;
  //                       for(int r = s; r < e; ++r) {
                            
  //                           // here we need to sum over both cs and ce for the
  //                           // current r.  we can compute the summation efficiently
  //                           // by computing separate sums for cs/ce and multiplying
  //                           // the sums together
                            
  //                           // CHILDS OF S
  //                           double sumsc_cs = 0; /* sum of scores concerning cs */
  //                           if (r==s) { // in this case, e is the first child of s
  //                               sumsc_cs =
  //                                   part_score.sib(N,s,e,l, -1);
  //                           } else {
  //                               // this loop considers every cs between s and r
  //                               int nworkc = 0;
  //                               for(int cs = s + 1; cs <= r; ++cs) {
  //                                   const double part_sc =
  //                                       part_score.sib(N,s,e,l, cs);
  //                                   const double ins_sc = IS.cscore(s,r,cs);
  //                                   /* save in child workspace */
  //                                   work_c[nworkc++] = part_sc + ins_sc;
  //                               }
  //                               sumsc_cs = log_sum(work_c, work_c + nworkc);
  //                           }
  //                           // CHILDS OF E
  //                           double sumsc_ce = 0; /* sum of scores concerning ce */
  //                           const int r1 = r + 1;
  //                           if (r1==e) { // in this case, e has no childs to its left
  //                               sumsc_ce =
  //                                   part_score.cmi(N, s,e,l, -1);
  //                           } else {
  //                               // this loop considers every ce between r+1 and e
  //                               int nworkc = 0;
  //                               for(int ce = r1; ce < e; ++ce) {
  //                                   const double part_sc =
  //                                       part_score.cmi(N,s,e,l, ce);
  //                                   const double ins_sc = IS.cscore(e,r1,ce);
  //                                   /* save in child workspace */
  //                                   work_c[nworkc++] = part_sc + ins_sc;
  //                               }
  //                               sumsc_ce = log_sum(work_c, work_c + nworkc);
  //                           }
                            
  //                           /* save in the split point workspace */
  //                           work_r[nworkr++] = sumsc_cs + sumsc_ce;
  //                       }
                        
  //                       { /* set the chart score */
  //                           // score for main dependency, multiplied to each summation
  //                           const double maindep_sc =
  //                               part_score.fo(N, s,e,l);
  //                           const double sum_r = log_sum(work_r, work_r + nworkr);
  //                           IS.uscore_set(s,e,l, sum_r + maindep_sc);
  //                       }
  //                   }
                    
  //                   // UNCOMPLETE STRUCTURES HEADED AT E
  //                   if (s!=-1) {
  //                       // consider each possible splitting point
  //                       int nworkr = 0;
  //                       for(int r = s; r < e; ++r) {
  //                           // same as above, but with reversed dependencies
  //                           // CHILDS OF S
  //                           double sumsc_cs = 0; /* sum of scores concerning cs */
  //                           if (r==s) { // in this case, s has no childs to its right
  //                               sumsc_cs =
  //                                   part_score.cmi(N,e,s,l, -1);
  //                           } else {
  //                               // this loop considers every cs between s and r
  //                               int nworkc = 0;
  //                               for(int cs = s + 1; cs <= r; ++cs) {
  //                                   const double part_sc =
  //                                       part_score.cmi(N,e,s,l, cs);
  //                                   const double ins_sc = IS.cscore(s,r,cs);
  //                                   /* save in child workspace */
  //                                   work_c[nworkc++] = part_sc + ins_sc;
  //                               }
  //                               sumsc_cs = log_sum(work_c, work_c + nworkc);
  //                           }
                            
  //                           // CHILDS OF E
  //                           double sumsc_ce = 0; /* sum of scores concerning ce */
  //                           const int r1 = r + 1;
  //                           if (r1==e) { // s will be the first child of e to its left
  //                               sumsc_ce =
  //                                   part_score.sib(N,e,s,l, -1);
  //                           } else {
  //                               // this loop considers every ce between r+1 and e
  //                               int nworkc = 0;
  //                               for(int ce = r1; ce < e; ++ce) {
  //                                   const double part_sc =
  //                                       part_score.sib(N, e,s,l, ce);
  //                                   const double ins_sc = IS.cscore(e,r1,ce);
  //                                   /* save in child workspace */
  //                                   work_c[nworkc++] = part_sc + ins_sc;
  //                               }
  //                               sumsc_ce = log_sum(work_c, work_c + nworkc);
  //                           }
                            
  //                           /* save in the split point workspace */
  //                           work_r[nworkr++] = sumsc_cs + sumsc_ce;
  //                       }
                        
  //                       { /* set the chart score */
  //                           /* main dependency (reversed) */
  //                           const double maindep_sc =
  //                               part_score.fo(N,e,s,l);
  //                           const double sum_r = log_sum(work_r, work_r + nworkr);
  //                           IS.uscore_set(e,s,l, sum_r + maindep_sc);
  //                       }
  //                   }
  //               } // for each l
  //               // COMPLETE STRUCTURES
                
  //               // sum over complete dtree with head at s
  //               for(int m = s + 1; m <= e; ++m) {
  //                   for(int l = 0; l < _L; ++l) {
  //                       double sumsc_cm = 0; /* sum over scores concerning cm */
  //                       if(m==e) { /* null modifier child */
  //                           sumsc_cm =
  //                               part_score.cmo(N,s,m,l, -1);
  //                       } else { /* consider children */
  //                           int nworkc = 0;
  //                           for(int cm = m + 1; cm <= e; ++cm) {
  //                               const double part_sc =
  //                                   part_score.cmo(N,s,m,l, cm);
  //                               const double ins_sc = IS.cscore(m,e,cm);
  //                               /* save in child workspace */
  //                               work_c[nworkc++] = part_sc + ins_sc;
  //                           }
  //                           sumsc_cm = log_sum(work_c, work_c + nworkc);
  //                       }
  //                       /* save in label workspace */
  //                       work_l[l] = sumsc_cm + IS.uscore(s,m,l);
  //                   }
  //                   /* set the chart score */
  //                   IS.cscore_set(s,e,m, log_sum(work_l, work_l + _L));
  //               } // for each m (modifier)
                
                
  //               if (s!=-1) {
  //                   // sum over complete dtree with head at e
  //                   for(int m = s; m < e; ++m) {
  //                       for(int l = 0; l < _L; ++l) {
  //                           double sumsc_cm = 0; /* sum over scores concerning cm */
  //                           if (m==s) { /* null modifier child */
  //                               sumsc_cm =
  //                                   part_score.cmo(N, e,m,l, -1);
  //                           } else { /* consider children */
  //                               int nworkc = 0;
  //                               for(int cm = s; cm < m; ++cm) {
  //                                   const double part_sc =
  //                                       part_score.cmo(N, e,m,l, cm);
  //                                   const double ins_sc = IS.cscore(m,s,cm);
  //                                   /* save in child workspace */
  //                                   work_c[nworkc++] = part_sc + ins_sc;
  //                               }
  //                               sumsc_cm = log_sum(work_c, work_c + nworkc);
  //                           }
  //                           /* save in label workspace */
  //                           work_l[l] = sumsc_cm + IS.uscore(e,m,l);
  //                       }
  //                       /* set the chart score */
  //                       IS.cscore_set(e,s,m, log_sum(work_l, work_l + _L));
  //                   } // for each m (modifier)
  //               }
                
  //           } // for each s (start point)
  //       } // for each w (i.e. span width)

  //       /* sum over constructions spanning the entire sentence */
  //       double logZ = 0; /* partition function */
  //       if (_multiroot) {
  //           // sum over complete structures spanning 0,N
  //           for(int rr = 0; rr < N; ++rr) {
  //               work_c[rr] = IS.cscore(-1,N-1,rr);
  //           }
  //           logZ = log_sum(work_c, work_c + N);
  //       } else { /* single root */
  //           /* sum over possible roots and root deps with two complete parts
  //              on either side of the root */
  //           for(int rr = 0; rr < N; ++rr) { /* root index */
  //               for(int l = 0; l < _L; ++l) { /* label index */
  //                   // consider complete structures to the left
  //                   double sumsc_cl = 0; /* sum over left children */
  //                   if (rr==0) { /* no left child */
  //                       sumsc_cl =
  //                           part_score.cmi(N,-1,rr,l, -1);
  //                   } else { /* consider left children */
  //                       int nworkc = 0;
  //                       for(int cl = 0; cl < rr; ++cl) {
  //                           const double part_sc =
  //                               part_score.cmi(N,-1,rr,l, cl);
  //                           const double ins_sc = IS.cscore(rr,0,cl);
  //                           /* save in child workspace */
  //                           work_c[nworkc++] = part_sc + ins_sc;
  //                       }
  //                       sumsc_cl = log_sum(work_c, work_c + nworkc);
  //                   }
                    
  //                   // consider complete structures to the right
  //                   double sumsc_cr = 0; /* sum over right children */
  //                   if (rr==N-1) { /* no right child */
  //                       sumsc_cr =
  //                           part_score.cmo(N, -1,rr,l, -1);
  //                   } else { /* consider right children */
  //                       int nworkc = 0;
  //                       for(int cr = rr + 1; cr < N; ++cr) {
  //                     const double part_sc =
  //                         part_score.cmo(N,-1,rr,l, cr);
  //                     const double ins_sc = IS.cscore(rr,N-1,cr);
  //                     /* save in child workspace */
  //                     work_c[nworkc++] = part_sc + ins_sc;
  //                       }
  //                       sumsc_cr = log_sum(work_c, work_c + nworkc);
  //                   }
              
  //                   /* main root dependency */
  //                   const double maindep_sc =
  //                       part_score.fo(N, -1,rr,l);
  //                   /* save in label workspace */
  //                   work_l[l] = sumsc_cl + sumsc_cr + maindep_sc;
  //               }
  //               /* save in root workspace */
  //               work_r[rr] = log_sum(work_l, work_l + _L);
  //           }
  //           /* compute the partiton function */
  //           logZ = log_sum(work_r, work_r + N);
  //       }
        
  //       /* clean up */
  //       free(work_r);
  //       free(work_c);
  //       free(work_l);
        
  //       return logZ;
  //   }
    
    
  //   static inline double log_add(const double x, const double y) {
  //       if(x > y) {
  //           return x + log(1.0 + exp(y - x));
  //       } else {
  //           return y + log(1.0 + exp(x - y));
  //       }
  //   }

  // class ProjDep2::outside_scores {
  // private:
  //   double* const _CS; // scores for complete structures
  //   double* const _US; // scores for uncomplete structures
  //   mutable bool* _usedCS; // scores for complete structures
  //   mutable bool* _usedUS; // scores for uncomplete structures
  //   const int _N, _L, _NL, _N2;
        
  // public:
  //   outside_scores(const int N0, const int L0)
  //     : _CS((double*)malloc(N0*N0*N0*sizeof(double))),
  // 	_US((double*)malloc(N0*N0*L0*sizeof(double))),
  // 	_N(N0),
  // 	_L(L0),
  // 	_NL(N0*L0),
  // 	_N2(N0*N0) {
  //     assert(_CS != NULL);
  //     assert(_US != NULL);
  //     /* set to -infinity */
  //     const double neginf = log(0);
  //     assert(exp(neginf) == 0);
  //     const int NNL = _L*_N2;
  //     const int NNN = _N*_N2;
  //     for(int i = 0; i < NNL; ++i) { _US[i] = neginf; }
  //     for(int i = 0; i < NNN; ++i) { _CS[i] = neginf; }
  //   }
  //   ~outside_scores() {
  //     free(_CS); free(_US);
  //   }
        
  //   double cscore(int h, const int e, const int m) const {
  //     if (h==-1) h=m;
  //     const int idx = h*_N2 + e*_N + m;
  //     return _CS[idx];
  //   }
        
  //   void cscore_add(int h, const int e, const int m, const double sco) {
  //     if (h==-1) h=m;
  //     const int idx = h*_N2 + e*_N + m;
  //     _CS[idx] = log_add(_CS[idx], sco);
  //   }
        
  //   double uscore(int h, const int m, const int l) const {
  //     if (h==-1) h=m;
  //     const int idx = h*_NL + m*_L + l;
  //     return _US[idx];
  //   }
        
  //   void uscore_add(int h, const int m, const int l, const double sco) {
  //     if (h==-1) h=m;
  //     const int idx = h*_NL + m*_L + l;
  //     _US[idx] = log_add(_US[idx], sco);
  //   }
  // };
  
  
  // /* templated on a part_score functor, which returns the score of the
  //    given part. */
  // template<class PartScorer>
  // void
  // ProjDep2::outside(const Pattern* const x,
  // 		    const chart_scores& IS,
  // 		    const double logZ,
  // 		    PartScorer& part_score,
  // 		    double* const ret_M) {
  //   // number of tokens in the sentence
  //   const int N = x->size();
    
  //   { /* zero the output marginals */
  //     const int nparts = x->nparts();
  //     for(int r = 0; r < nparts; ++r) { ret_M[r] = 0; }
  //   }
    
  //   /* workspace for enumerating child indices */
  //   double* const work_c = (double*)malloc((N + 1)*sizeof(double));
  //   /* workspace for child part scores */
  //   double* const work_cp1 = (double*)malloc((N + 1)*sizeof(double));
  //   /* workspace for child part scores */
  //   double* const work_cp2 = (double*)malloc((N + 1)*sizeof(double));
  //   assert(work_c != NULL);
  //   assert(work_cp1 != NULL);
  //   assert(work_cp2 != NULL);
    
  //   outside_scores OS(N, _L);
    
  //   /* make contributions to outside scores from constructions
  //      spanning the whole sentence */
  //   /* sentence-spanning complete structures have no outside */
  //   if(_multiroot) { /* multi root */
  //     /* set all sentence-spanning complete structures to have outside
  // 	 = log(1) = 0 as the base of recursion */
  //     for(int rr = 0; rr < N; ++rr) {
  // 	OS.cscore_add(-1,N-1,rr, 0);
  //     }
  //   } else { /* single root */
  //     /* go through all sentence spanning constructions and contribute
  // 	 to outside scores and marginals.  note that there is only one
  // 	 way to construct the first and second-order parts involving
  //        the root dependency (in the case of single-root parsing), so
  //        it is safe to assign marginals rather than accumulate them */
  //     for(int rr = 0; rr < N; ++rr) { /* root index */
  // 	for(int l = 0; l < _L; ++l) { /* label index */
  // 	  // consider complete structures to the left
  // 	  double sumsc_cl = 0; /* sum over left children */
  // 	  if (rr==0) { /* no left child */
  // 	    const double part_sc =
  //                           part_score.cmi(N,-1,rr,l, -1);
  // 	    sumsc_cl = part_sc;
  // 	  } else { /* consider left children */
  // 	    int nworkc = 0;
  // 	    for(int cl = 0; cl < rr; ++cl) {
  // 	      const double part_sc =
  // 		part_score.cmi(N, -1,rr,l, cl);
  // 	      work_cp1[nworkc] = part_sc;
  // 	      const double ins_sc = IS.cscore(rr,0,cl);
  // 	      /* save in child workspace */
  // 	      work_c[nworkc++] = part_sc + ins_sc;
  // 	    }
  // 	    sumsc_cl = log_sum(work_c, work_c + nworkc);
  // 	  }
	  
  // 	  // consider complete structures to the right
  // 	  double sumsc_cr = 0; /* sum over right children */
  // 	  if (rr==N-1) { /* no right child */
  // 	    const double part_sc =
  // 	      part_score.cmo(N,-1,rr,l, -1);
  // 	    sumsc_cr = part_sc;
  // 	  } else { /* consider right children */
  // 	    int nworkc = 0;
  // 	    for(int cr = rr + 1; cr < N; ++cr) {
  // 	      const double part_sc =
  // 		part_score.cmo(N,-1,rr,l, cr);
  // 	      work_cp2[nworkc] = part_sc;
  // 	      const double ins_sc = IS.cscore(rr,N-1,cr);
  // 	      /* save in child workspace */
  // 	      work_c[nworkc++] = part_sc + ins_sc;
  // 	    }
  // 	    sumsc_cr = log_sum(work_c, work_c + nworkc);
  // 	  }
          
  // 	  /* main root dependency */
  // 	  // have to change index calculation using PartDep2Index
  // 	  const int maindep_idx =
  // 	    PartDep2Index::encode(N, PartDep2::FO, -1,rr,-1,l);
  // 	  const double maindep_sc = part_score.fo(N,-1,rr,l);
          
  // 	  /* set marginal of main dep */
  // 	  ret_M[maindep_idx] =
  // 	    exp(sumsc_cl + sumsc_cr + maindep_sc - logZ);
	  
          
  // 	  /* compute second-order marginals and contribute to outside
  // 	     scores */
	  
  // 	  // consider complete structures to the left
  // 	  if (rr==0) { /* no left child */
  // 	    /* no outside contributions to make; just compute marginal */
  // 	    const double out_sc = sumsc_cr + maindep_sc;
  // 	    const double part_sc = sumsc_cl;
  // 	    const int part_idx =
  // 	      PartDep2Index::encode(N, PartDep2::CMI, -1,rr,l, -1);
  // 	    ret_M[part_idx] = exp(out_sc + part_sc - logZ);
  // 	  } else { /* consider left children */
  // 	    const double out_sc = sumsc_cr + maindep_sc;
  // 	    int nworkc = 0;
  // 	    for(int cl = 0; cl < rr; ++cl) {
  // 	      const double part_sc = work_cp1[nworkc++];
  // 	      /* contribution to outside score */
  // 	      const double out_contrib = out_sc + part_sc;
  // 	      OS.cscore_add(rr,0,cl, out_contrib);
  // 	      /* compute marginal */
  // 	      const int part_idx =
  // 		PartDep2Index::encode(N, PartDep2::CMI, -1,rr,l, cl);
  // 	      const double ins_sc = IS.cscore(rr,0,cl);
  // 	      ret_M[part_idx] = exp(out_contrib + ins_sc - logZ);
  // 	    }
  // 	  }
          
  // 	  // consider complete structures to the right
  // 	  if (rr==N-1) { /* no right child */
  // 	    /* no outside contributions to make; just compute marginal */
  // 	    const double out_sc = sumsc_cl + maindep_sc;
  // 	    const double part_sc = sumsc_cr;
  // 	    const int part_idx =
  // 	      PartDep2Index::encode(N, PartDep2::CMO, -1,rr,l, -1);
  // 	    ret_M[part_idx] = exp(out_sc + part_sc - logZ);
  // 	  } else { /* consider right children */
  // 	    const double out_sc = sumsc_cl + maindep_sc;
  // 	    int nworkc = 0;
  // 	    for(int cr = rr + 1; cr < N; ++cr) {
  // 	      const double part_sc = work_cp2[nworkc++];
  // 	      /* contribution to outside score */
  // 	      const double out_contrib = out_sc + part_sc;
  // 	      OS.cscore_add(rr,N-1,cr, out_contrib);
  // 	      /* compute marginal */
  // 	      const int part_idx =
  // 		PartDep2Index::encode(N, PartDep2::CMO, -1,rr,l, cr);
  // 	      const double ins_sc = IS.cscore(rr,N-1,cr);
  // 	      ret_M[part_idx] = exp(out_contrib + ins_sc - logZ);
  // 	    }
  // 	  }
  // 	}
  //     }
  //   }
            
	
  //   /*
  //    * W is the max width of the spans it controls the outer loop of
  //    * the viterbi search,
  //    *
  //    * when multiple roots are allowed, its value is N+1, so that
  //    * dependencies from the root are visited from token 0 to token
  //    * N-1
  //    *
  //    */
  //   const int W = (_multiroot ? (N + 1) : N);
    
  //   for(int w = W - 1; w > 0; --w) {
      
  //     // cerr << "proj_2p_parser : main loop : w=" << w << endl;
      
  //     const int initial_s = _multiroot ? -1 : 0;
      
  //     for(int s = initial_s; s < N - w; ++s) {
  // 	const int e = s + w;
        
  // 	/* for the marginals of second-order parts, in general there
  // 	   may be many constructions which result in the same second
  // 	   order part.  for example, the grandparent part (1, 3, 5)
  // 	   can be produced from Uncomplete(1, 3) + Complete(3, 10, 5)
  // 	   or Uncomplete(1, 3) + Complete(3, 6, 5).  thus we add
  // 	   contributions to the marginals instead of assigning them */
	
  // 	// COMPLETE STRUCTURES (NB before open)
        
  // 	{ // sum over complete dtree with head at s
  // 	  for(int m = s + 1; m <= e; ++m) {
  // 	    const double cout_sc = OS.cscore(s,e,m);
  // 	    for(int l = 0; l < _L; ++l) {
  // 	      const double uins_sc = IS.uscore(s,m,l);
  // 	      /* make contributions to the complete halves and compute
  // 		 marginals; the sum will be used to contribute to the
  // 		 uncomplete half */
  // 	      double sumsc_cm = 0; /* sum over scores concerning cm */
  // 	      if(m==e) { /* null modifier child */
  // 		/* no complete structure to contribute to, so just
  // 		   compute marginal contribution */
  // 		const int part_idx =
  // 		  PartDep2Index::encode(N, PartDep2::CMO, s,m,l, -1);
  // 		const double part_sc = part_score.cmo(N,s,m,l, -1);
  // 		sumsc_cm = part_sc;
  // 		ret_M[part_idx] += exp(cout_sc + uins_sc + part_sc - logZ);
  // 	      } else { /* consider children */
  // 		int nworkc = 0;
  // 		for(int cm = m + 1; cm <= e; ++cm) {
  // 		  const int part_idx =
  // 		    PartDep2Index::encode(N, PartDep2::CMO, s,m,l, cm);
  // 		  /* compute contribution to outside of complete */
  // 		  const double part_sc = part_score.cmo(N,s,m,l,cm);
  // 		  const double cout_contrib = cout_sc + uins_sc + part_sc;
  // 		  OS.cscore_add(m,e,cm, cout_contrib);
  // 		  /* compute contribution to sumsc_cm */
  // 		  const double cins_sc = IS.cscore(m,e,cm);
  // 		  work_c[nworkc++] = part_sc + cins_sc;
  // 		  /* compute marginal contribution */
  // 		  ret_M[part_idx] += exp(cout_contrib + cins_sc - logZ);
  // 		}
  // 		sumsc_cm = log_sum(work_c, work_c + nworkc);
  // 	      }
              
  // 	      /* compute contribution to outside of uncomplete */
  // 	      const double uout_contrib = cout_sc + sumsc_cm;
  // 	      OS.uscore_add(s,m,l, uout_contrib);
  // 	    }
  // 	  } // for each m (modifier)
  // 	}

  // 	if (s!=-1) {
  // 	  // sum over complete dtree with head at e
  // 	  for(int m = s; m < e; ++m) {
  // 	    const double cout_sc = OS.cscore(e,s,m);
  // 	    for(int l = 0; l < _L; ++l) {
  // 	      const double uins_sc = IS.uscore(e,m,l);
  // 	      /* make contributions to the complete halves and compute
  // 		 marginals; the sum will be used to contribute to the
  // 		 uncomplete half */
  // 	      double sumsc_cm = 0; /* sum over scores concerning cm */
  // 	      if (m==s) { /* null modifier child */
  // 		/* only marginal contribution */
  // 		const int part_idx =
  // 		  PartDep2Index::encode(N, PartDep2::CMO, e,m,l, -1);
  // 		const double part_sc = part_score.cmo(N,e,m,l,-1);
  // 		sumsc_cm = part_sc;
  // 		ret_M[part_idx] += exp(cout_sc + uins_sc + part_sc - logZ);
  // 	      } else { /* consider children */
  // 		int nworkc = 0;
  // 		for(int cm = s; cm < m; ++cm) {
  // 		  const int part_idx =
  // 		    PartDep2Index::encode(N, PartDep2::CMO, e,m,l, cm);
  // 		  /* compute contribution to outside of complete */
  // 		  const double part_sc = part_score.cmo(N,e,m,l, cm);
  // 		  const double cout_contrib = cout_sc + uins_sc + part_sc;
  // 		  OS.cscore_add(m,s,cm, cout_contrib);
  // 		  /* compute contribution to sumsc_cm */
  // 		  const double cins_sc = IS.cscore(m,s,cm);
  // 		  work_c[nworkc++] = part_sc + cins_sc;
  // 		  /* compute marginal contribution */
  // 		  ret_M[part_idx] += exp(cout_contrib + cins_sc - logZ);
  // 		}
  // 		sumsc_cm = log_sum(work_c, work_c + nworkc);
  // 	      }
              
  // 	      /* compute contribution to outside of uncomplete */
  // 	      const double uout_contrib = cout_sc + sumsc_cm;
  // 	      OS.uscore_add(e,m,l, uout_contrib);
  // 	    }
  // 	  } // for each m (modifier)
  //               }
  // 	/* check complete+uncomplete sums */
                      
	
  // 	/* UNCOMPLETE STRUCTURES */
  // 	for(int l = 0; l < _L; ++l) {
  // 	  { // UNCOMPLETE STRUCTURES HEADED AT S
  // 	    const double uout_sc = OS.uscore(s,e,l);
  // 	    const int maindep_idx =
  // 	      PartDep2Index::encode(N, PartDep2::FO, s,e,l);
  // 	    const double maindep_sc = part_score.fo(N,s,e,l);
  // 	    { /* compute the marginal of the main dependency; there is
  // 		 only one way to construct this part so assignment is
  // 		 okay */
  // 	      const double ins_sc = IS.uscore(s,e,l);
  // 	      ret_M[maindep_idx] = exp(ins_sc + uout_sc - logZ);
  // 	    }
                        
  // 	    // consider each possible splitting point
  // 	    for(int r = s; r < e; ++r) {
	      
  // 	      // here we need to sum over both cs and ce for the
  // 	      // current r.  we can compute the summation efficiently
  // 	      // by computing separate sums for cs/ce and multiplying
  // 	      // the sums together
              
  // 	      // CHILDS OF S
  // 	      double sumsc_cs = 0; /* sum of scores concerning cs */
  // 	      if (r==s) { // in this case, e is the first child of s
  // 		sumsc_cs =
  // 		  part_score.sib(N,s,e,l, -1);
  // 	      } else {
  // 		// this loop considers every cs between s and r
  // 		int nworkc = 0;
  // 		for(int cs = s + 1; cs <= r; ++cs) {
  // 		  const double part_sc =
  // 		    part_score.sib(N,s,e,l, cs);
  // 		  work_cp1[nworkc] = part_sc;
  // 		  const double ins_sc = IS.cscore(s,r,cs);
  // 		  /* save in child workspace */
  // 		  work_c[nworkc++] = part_sc + ins_sc;
  // 		}
  // 		sumsc_cs = log_sum(work_c, work_c + nworkc);
  // 	      }
  // 	      // CHILDS OF E
  // 	      double sumsc_ce = 0; /* sum of scores concerning ce */
  // 	      const int r1 = r + 1;
  // 	      if (r1==e) { // in this case, e has no childs to its left
  // 		sumsc_ce =
  // 		  part_score.cmi(N,s,e,l, -1);
  // 	      } else {
  // 		// this loop considers every ce between r+1 and e
  // 		int nworkc = 0;
  // 		for(int ce = r1; ce < e; ++ce) {
  // 		  const double part_sc =
  // 		    part_score.cmi(N,s,e,l, ce);
  // 		  work_cp2[nworkc] = part_sc;
  // 		  const double ins_sc = IS.cscore(e,r1,ce);
  // 		  /* save in child workspace */
  // 		  work_c[nworkc++] = part_sc + ins_sc;
  // 		}
  // 		sumsc_ce = log_sum(work_c, work_c + nworkc);
  // 	      }
              
              
  // 	      /* now go through the children again, computing outside
  // 		 and marginal contributions */
	      
  // 	      // CHILDS OF S
  // 	      if (r==s) { // in this case, e is the first child of s
  // 		/* only marginal contribution */
  // 		const int part_idx =
  // 		  PartDep2Index::encode(N, PartDep2::SIB, s,e,l, -1);
  // 		ret_M[part_idx] +=
  // 		  exp(uout_sc + maindep_sc + sumsc_ce + sumsc_cs - logZ);
  // 	      } else {
  // 		// this loop considers every cs between s and r
  // 		int nworkc = 0;
  // 		for(int cs = s + 1; cs <= r; ++cs) {
  // 		  const double part_sc = work_cp1[nworkc++];
  // 		  /* contribution to outside */
  // 		  const double out_contrib =
  // 		    uout_sc + sumsc_ce + maindep_sc + part_sc;
  // 		  OS.cscore_add(s,r,cs, out_contrib);
  // 		  /* contribution to marginal */
  // 		  const double ins_sc = IS.cscore(s,r,cs);
  // 		  const int part_idx =
  // 		    PartDep2Index::encode(N, PartDep2::SIB, s,e,l, cs);
		  
  // 		  ret_M[part_idx] += exp(out_contrib + ins_sc - logZ);
  // 		}
  // 	      }
              
              
  // 	      // CHILDS OF E
  // 	      if (r1==e) { // in this case, e has no childs to its left
  // 		/* only marginal contribution */
  // 		const int part_idx =
  // 		  PartDep2Index::encode(N, PartDep2::CMI, s,e,l, -1);
  // 		ret_M[part_idx] +=
  // 		  exp(uout_sc + maindep_sc + sumsc_ce + sumsc_cs - logZ);
  // 	      } else {
  // 		// this loop considers every ce between r+1 and e
  // 		int nworkc = 0;
  // 		for(int ce = r1; ce < e; ++ce) {
  // 		  const double part_sc = work_cp2[nworkc++];
  // 		  /* contribution to outside */
  // 		  const double out_contrib =
  // 		    uout_sc + sumsc_cs + maindep_sc + part_sc;
  // 		  OS.cscore_add(e,r1,ce, out_contrib);
  // 		  /* contribution to marginal */
  // 		  const double ins_sc = IS.cscore(e,r1,ce);
  // 		  const int part_idx =
  // 		    PartDep2Index::encode(N, PartDep2::CMI, s,e,l, ce);
  // 		  ret_M[part_idx] += exp(out_contrib + ins_sc - logZ);
  // 		}
  // 	      }
  // 	    }
  // 	  }
                    
  // 	  // UNCOMPLETE STRUCTURES HEADED AT E
  // 	  if (s!=-1) {
  // 	    const double uout_sc = OS.uscore(e,s,l);
  // 	    const int maindep_idx =
  // 	      PartDep2Index::encode(N, PartDep2::FO, e,s,l);
  // 	    const double maindep_sc = part_score.fo(N,e,s,l);
  // 	    { /* compute the marginal of the main dependency; there is
  // 		 only one way to construct this part so assignment is
  // 		 okay */
  // 	      const double ins_sc = IS.uscore(e,s,l);
  // 	      ret_M[maindep_idx] = exp(ins_sc + uout_sc - logZ);
  // 	    }
            
  // 	    // consider each possible splitting point
  // 	    for(int r = s; r < e; ++r) {
                            
  // 	      // same as above, but with reversed dependencies
              
  // 	      // CHILDS OF S
  // 	      double sumsc_cs = 0; /* sum of scores concerning cs */
  // 	      if (r==s) { // in this case, s has no childs to its right
  // 		sumsc_cs =
  // 		  part_score.cmi(N,e,s,l, -1);
  // 	      } else {
  // 		// this loop considers every cs between s and r
  // 		int nworkc = 0;
  // 		for(int cs = s + 1; cs <= r; ++cs) {
  // 		  const double part_sc =
  // 		    part_score.cmi(N,e,s,l, cs);
  // 		  work_cp1[nworkc] = part_sc;
  // 		  const double ins_sc = IS.cscore(s,r,cs);
  // 		  /* save in child workspace */
  // 		  work_c[nworkc++] = part_sc + ins_sc;
  // 		}
  // 		sumsc_cs = log_sum(work_c, work_c + nworkc);
  // 	      }
                            
  // 	      // CHILDS OF E
  // 	      double sumsc_ce = 0; /* sum of scores concerning ce */
  // 	      const int r1 = r + 1;
  // 	      if (r1==e) { // s will be the first child of e to its left
  // 		sumsc_ce =
  // 		  part_score.sib(N,e,s,l, -1);
  // 	      } else {
  // 		// this loop considers every ce between r+1 and e
  // 		int nworkc = 0;
  // 		for(int ce = r1; ce < e; ++ce) {
  // 		  const double part_sc =
  // 		    part_score.sib(N,e,s,l, ce);
  // 		  work_cp2[nworkc] = part_sc;
  // 		  const double ins_sc = IS.cscore(e,r1,ce);
  // 		  /* save in child workspace */
  // 		  work_c[nworkc++] = part_sc + ins_sc;
  // 		}
  // 		sumsc_ce = log_sum(work_c, work_c + nworkc);
  // 	      }
              
              
  // 	      /* go through the children again, computing outside
  // 		 contributions and marginals */
                            
  // 	      // CHILDS OF S
  // 	      if (r==s) { // in this case, s has no childs to its right
  // 		const int part_idx =
  // 		  PartDep2Index::encode(N, PartDep2::CMI, e,s,l, -1);
  // 		ret_M[part_idx] +=
  // 		  exp(uout_sc + maindep_sc + sumsc_ce + sumsc_cs - logZ);
  // 	      } else {
  // 		// this loop considers every cs between s and r
  // 		int nworkc = 0;
  // 		for(int cs = s + 1; cs <= r; ++cs) {
  // 		  const double part_sc = work_cp1[nworkc++];
  // 		  /* contribution to outside */
  // 		  const double out_contrib =
  // 		    uout_sc + sumsc_ce + maindep_sc + part_sc;
  // 		  OS.cscore_add(s,r,cs, out_contrib);
  // 		  /* contribution to marginal */
  // 		  const double ins_sc = IS.cscore(s,r,cs);
  // 		  const int part_idx =
  // 		    PartDep2Index::encode(N, PartDep2::CMI, e,s,l, cs);
  // 		  ret_M[part_idx] += exp(out_contrib + ins_sc - logZ);
  // 		}
  // 	      }
              
  // 	      // CHILDS OF E
  // 	      if (r1==e) { // s will be the first child of e to its left
  // 		const int part_idx =
  // 		  PartDep2Index::encode(N, PartDep2::SIB, e,s,l, -1);
  // 		ret_M[part_idx] +=
  // 		  exp(uout_sc + maindep_sc + sumsc_ce + sumsc_cs - logZ);
  // 	      } else {
  // 		// this loop considers every ce between r+1 and e
  // 		int nworkc = 0;
  // 		for(int ce = r1; ce < e; ++ce) {
  // 		  const double part_sc = work_cp2[nworkc++];
  // 		  /* contribution to outside */
  // 		  const double out_contrib =
  // 		    uout_sc + sumsc_cs + maindep_sc + part_sc;
  // 		  OS.cscore_add(e,r1,ce, out_contrib);
  // 		  /* contribution to marginal */
  // 		  const double ins_sc = IS.cscore(e,r1,ce);
  // 		  const int part_idx =
  // 		    PartDep2Index::encode(N, PartDep2::SIB, e,s,l, ce);
  // 		  ret_M[part_idx] += exp(out_contrib + ins_sc - logZ);
  // 		}
  // 	      }
  // 	    }
  // 	  }
  // 	} // for each l
        
  //     } // for each s (start point)
  //   } // for each w (i.e. span width)

    
  //   if(false) { /* validate marginals */
  //     for(int m = 0; m < N; ++m) {
  // 	double inbound_m = 0;
  // 	for(int h = -1; h < N; ++h) {
  // 	  if(h == m) { continue; }
  // 	  for(int l = 0; l < _L; ++l) {
  // 	    const int maindep_idx =
  // 	      PartDep2Index::encode(N, PartDep2::FO, h,m,l);
  // 	    const double marg_dep = ret_M[maindep_idx];
  // 	    inbound_m += marg_dep;
  // 	    double sum_ch = 0;
  // 	    double sum_cmi = 0;
  // 	    const int inc = (m < h ? 1 : -1);
  // 	    for(int c = m; c != h; c += inc) {
  // 	      const int cc = (c == m ? -1 : c);
  // 	      const int ch_idx =
  // 		PartDep2Index::encode(N, PartDep2::SIB, h,m,l, cc);
  // 	      sum_ch += ret_M[ch_idx];
  // 	      const int cmi_idx =
  // 		PartDep2Index::encode(N, PartDep2::CMI, h,m,l, cc);
  // 	      sum_cmi += ret_M[cmi_idx];
  // 	    }
  // 	    double sum_cmo = 0;
  // 	    const int end = (m < h ? -1 : N);
  // 	    for(int c = m; c != end; c -= inc) {
  // 	      const int cc = (c == m ? -1 : c);
  // 	      const int cmo_idx =
  // 		PartDep2Index::encode(N, PartDep2::CMO, h,m,l, cc);
  // 	      sum_cmo += ret_M[cmo_idx];
  // 	    }
  // 	    const double absdiff_ch  =
  // 	      ((_multiroot || h != -1) ? reldist(marg_dep, sum_ch) : 0);
  // 	    const double absdiff_cmi = reldist(marg_dep, sum_cmi);
  // 	    const double absdiff_cmo = reldist(marg_dep, sum_cmo);
  // 	    cerr << "marg h,m,l=" << h << "," << m << "," << l
  // 		 << " diff ch = " << absdiff_ch
  // 		 << " diff cmi = " << absdiff_cmi
  // 		 << " diff cmo = " << absdiff_cmo << endl;
  // 	    assert(absdiff_ch  <= TOL);
  // 	    assert(absdiff_cmi <= TOL);
  // 	    assert(absdiff_cmo <= TOL);
  // 	  }
  // 	}
  // 	// const double diff_in = inbound_m - 1.0;
  // 	// const double absdiff_in = (diff_in < 0 ? -diff_in : diff_in);
  // 	const double absdiff_in = reldist(inbound_m, 1.0);
  // 	cerr << "marg m=" << m << " inbound sum = " << inbound_m
  // 	     << " ; diff = " << absdiff_in << endl;
  // 	assert(absdiff_in <= TOL);
  //     }
  //   }
        
  //   /* clean up */
  //   free(work_c);
  //   free(work_cp1);
  //   free(work_cp2);
  // }

    
  // double ProjDep2::partition(const Pattern* x, const double* const S) {
  //   scores_dep2_array scores(S);
  //   chart_scores IS(x->size(), _L);
  //   return inside(x, IS, scores);
  // }
  
  // double ProjDep2::marginals(const Pattern* x, const double* const S,
  // 			     double* const out_M) {
  //   scores_dep2_array scores(S);
  //   chart_scores IS(x->size(), _L);
  //   const double logZ = inside(x, IS, scores);
  //   outside(x, IS, logZ, scores, out_M);
  //   return logZ;
  // }
  
   
  // void ProjDep2::deptree2label(const bool multiroot,
  // 			       const int N, const DepTree* const t,
  // 			       Label<PartDep2>& y) {
  //   const int h = t->idx();
  //   const int nl = t->nlc();
  //   if(nl > 0) { /* left children */
  //     int sib = -1; /* initialize to null sibling */
  //     for(int i = 0; i < nl; ++i) {
  // 	const DepTree* const tm = t->lc(i);
  // 	const int m = tm->idx();
  // 	const int l = tm->label();
  // 	/* first-order part */
  // 	y.push_back(PartDep2(PartDep2::FO, h, m, -1, l));
  // 	/* sibling part */
  // 	y.push_back(PartDep2(PartDep2::SIB, h, m, sib, l));
  // 	sib = m; /* save for next iteration */
  // 	/* grandparent parts (for outermost grandchildren only) */
  // 	int cmo = -1, cmi = -1;
  // 	/* left-left is cmo, left-right is cmi */
  // 	if(tm->nlc() > 0) { cmo = tm->lc(tm->nlc() - 1)->idx(); }
  // 	if(tm->nrc() > 0) { cmi = tm->rc(tm->nrc() - 1)->idx(); }
  // 	y.push_back(PartDep2(PartDep2::CMO, h, m, cmo, l));
  // 	y.push_back(PartDep2(PartDep2::CMI, h, m, cmi, l));
  // 	/* recurse */
  // 	deptree2label(multiroot, N, tm, y);
  //     }
  //   }
  //   const int nr = t->nrc();
  //   if(nr > 0) { /* right children */
  //     int sib = -1; /* initialize to null sibling */
  //     for(int i = 0; i < nr; ++i) {
  // 	const DepTree* const tm = t->rc(i);
  // 	const int m = tm->idx();
  // 	const int l = tm->label();
  // 	/* first-order part */
  // 	y.push_back(PartDep2(PartDep2::FO, h, m, -1, l));
  // 	/* sibling part : special case, for root dependencies in the
  // 	   single-root case, ignore the root-sibling part */
  // 	if(multiroot || h != -1) {
  // 	  y.push_back(PartDep2(PartDep2::SIB, h, m, sib, l));
  // 	}
  // 	sib = m; /* save for next iteration */
  // 	/* grandparent parts (for outermost grandchildren only) */
  // 	int cmo = -1, cmi = -1;
  // 	/* right-right is cmo, right-left is cmi */
  // 	if(tm->nrc() > 0) { cmo = tm->rc(tm->nrc() - 1)->idx(); }
  // 	if(tm->nlc() > 0) { cmi = tm->lc(tm->nlc() - 1)->idx(); }
  // 	y.push_back(PartDep2(PartDep2::CMO, h, m, cmo, l));
  // 	y.push_back(PartDep2(PartDep2::CMI, h, m, cmi, l));
  // 	/* recurse */
  // 	deptree2label(multiroot, N, tm, y);
  //     }
  //   }
  // }
  
 
}

