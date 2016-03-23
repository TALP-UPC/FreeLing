//////////////////////////////////////////////////////////////////
//
//    Treeler - Open-source Structured Prediction for NLP
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//                          02111-1307 USA
//
//    contact: Xavier Carreras (carreras@lsi.upc.es)
//             TALP Research Center
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////
#ifndef TREELER_PART_DEP2_INDEX_H
#define TREELER_PART_DEP2_INDEX_H

#include <assert.h>
#include <math.h>

#include "treeler/dep/part-dep2.h"

/* PartDep2Index : functions to compute integer indices of second-order parts in dependency parsing */

namespace treeler {

    class PartDep2Index {
        
    public:
        
      /*  Total parts, unlabeled : 2*N*N + N*N*N + 0.5*(N*N+N) 
          The first N*N values are for first-order parts (h,m), with index : 
          r = h + m*N   (when h=root, then h=m)
          
          The following N*N*N values are for second-order relations with non-root head,
          according to the following mapping : 
          
          A triple <x1,x2,x3> represents all these relations, with 0 <= x_i < N
          
          - r = N*N + x1*N*N + x2*N + x3
          
          - For SIB relations, non-null child : x1 = c ; x2 = m ; x3 = h
          - For SIB relations, null child :     x1 = x3 = h ; x2 = m 
          
          - For mod-child-in (CMI) relations, non-null child : x1 = h ; x2 = m ; x3 = c
          - For mod-child-in (CMI) relations, null child :     x1 = h ; x2 = x3 = m 
          
          - For mod-child-out (CMO) relations, non-null child : x1 = h ; x2 = m ; x3 = c
          - For mod-child-out (CMO) relations, null child :     x1 = x2 = h ; x3 = m    
          
          Note: N values of the N*N*N are not used for these relations,
          specifically those where x1=x2=x3
          
          The following N*N values correspond to second-order relations with the root, 
          for children of the modifier, according to : 
          - r = N*N + N*N*N + m*N + c
          
          - c=m corresponds to null child inside the dependency
          - for null children outside the dependency, the unused values in the N*N*N block
          are used

          The final 0.5*(N*N + N) values represent 2nd order children of the root : 
          - c<=m non-negative values, with c==m meaning null child
          - r = 2*N*N + N*N*N + 0.5*m*(m+1) + c 
          - decoding can be done as follows : 
          r' = r - 2*N*N + N*N*N
          m = 0.5*(-1 + sqrt(1+ 8*r'))
          c = r' - 0.5*m*(m+1)

      */
        
      
      static int nparts(const int N, 
			const int L) {	
	if (N != _N) update(N);
	return L*_U;
      }
      
      static int unlabeled_nparts(const int N) {	
	if (N != _N) update(N);
	return _U; 
      }
      
        
      static void decode(const int N, const int L, const int part, 
			 PartDep2::type_t *type, int *h, int *m, int *l, int *c) {	
	if (N != _N) update(N);        
	*l = part / _U;
	int upart = part % _U;        
	if (upart < _NN) {
	  // first-order part
	  *type = PartDep2::FO;
	  *m = upart / _N;
	  *h = upart % _N;
	  if (*h == *m) {
	    *h = -1;
	  }
	  *c = -1;
	  return;
	}
	
	// move to the next block of indices     
	upart = upart - _NN;
        
	if (upart < _NNN) {
	  // second-order part with non-root head 
	  const int x1 = upart / _NN;
	  upart = upart % _NN;
	  const int x2 = upart / _N;
	  const int x3 = upart % _N;
	  
	  //      assert(x1!=x2 or x2!=x3 or x1!=x3);
	  // 
	  // x1=x2=x3 encodes 2nd order CMO relations with root head and null child
	  if (x1==x2 and x2==x3) {
	    *type = PartDep2::CMO; 
	    *h = -1;
	    *m = x1;
	    *c = -1;
	  }
	  else if (x1==x2) {
	    // mod's child outside, with null child
	    *type = PartDep2::CMO;
	    *h = x1;
	    *m = x3;
	    *c = -1;
	  }
	  else if (x2==x3) {
	    // mod's child inside, with null child
	    *type = PartDep2::CMI;
	    *h = x1;
	    *m = x2;
	    *c = -1;
	  }
	  else if (x1==x3) {
	    // head's child, with null child
	    *type = PartDep2::SIB;
	    *h = x1;
	    *m = x2;
	    *c = -1;
	  }
	  else if ((x1<x2 and x2<x3) or (x3<x2 and x2<x1)) {
	    // mod's child outside
	    *type = PartDep2::CMO;
	    *h = x1;
	    *m = x2;
	    *c = x3;
	  }
	  else if ((x1<x3 and x3<x2) or (x2<x3 and x3<x1)) {
	    // mod's child inside
	    *type = PartDep2::CMI;
	    *h = x1;
	    *m = x2;
	    *c = x3;
	  }
	  else {  // ((x3<x1 and x1<x2) or (x2<x1 and x1<x3))
	    // head's child
	    *type = PartDep2::SIB;
	    *h = x3;
	    *m = x2;
	    *c = x1;
	  }      
	  return;
	}
        
	// move to the next block of indices
	upart = upart - _NNN;
        
	if (upart < _NN) {
	  // second-order part, head=root, modifier children 
	  *h = -1;
	  *m = upart / _N;
	  *c = upart % _N;
	  if (*m==*c) {
	    *type = PartDep2::CMI;
	    *c = -1;
	  }
	  else if (*c<*m) { 
	    *type = PartDep2::CMI;
	  }
	  else {
	    *type = PartDep2::CMO;
	  }	       
	  return;
	}
        
	// move to the next block of indices
	upart = upart - _NN;
        
	// second-order part, head=root, root children 
	*type = PartDep2::SIB;
	*h = -1;
	*m = (int)(0.5*(-1+sqrt((double)(1+8*upart))));
	*c = upart - (int)(0.5*(*m)*((*m)+1));
	if (*c==*m) {
	  *c = -1;
	}
        
      }
      
      static int encode(const int N, 
			PartDep2::type_t type, 
			const int h, 
			const int m, 
			const int l,
			const int c=-1) {
	if (N!=_N) update(N);
	
	if (type == PartDep2::FO) 
	  return encode_FO(h,m,l);
	else if (type == PartDep2::SIB) 
	  return encode_SIB(h,m,l,(c == h ? -1 : c));
	else if (type == PartDep2::CMI) 
	  return encode_CMI(h,m,l,(c == m ? -1 : c));
	else if (type == PartDep2::CMO) 
	  return encode_CMO(h,m,l,(c == m ? -1 : c));
	else 
	  assert(0);
	return -1;
      };
      
      static int encode_FO(const int h, 
			   const int m, 
			   const int l) {
	int haux = (h==-1) ? m : h;
	return _U*l + haux + _N*m;
      }
      
      static int encode_SIB(const int h, 
			    const int m, 
			    const int l, 
			    const int c) {
	
	if (h==-1) { 
	  int caux = (c==-1) ? m : c;      
	  return _U*l + 2*_NN + _NNN + (int)((0.5*m*(m+1))) + caux;
	}
        
	int x1 = (c==-1) ? h : c;
	int x2 = m;
	int x3 = h;
        
	return _U*l + _NN + x1*_NN + x2*_N + x3;
      }
      
      static int encode_CMI(const int h, 
			    const int m,
			    const int l,
			    const int c) {	  
	if (h==-1) { 
	  int caux = (c==-1) ? m : c;      
	  return _U*l + _NN + _NNN + _N*m + caux;
	}          
	int x1 = h;
	int x2 = m;
	int x3 = (c==-1) ? m : c;;          
	return _U*l + _NN + x1*_NN + x2*_N + x3;
      }
        
      static int encode_CMO(const int h, 
			    const int m,
			    const int l,
			    const int c) {
	if (h==-1) { 
	  if (c==-1) {
	    return _U*l + _NN + m*_NN + m*_N + m;
	  }
	  return _U*l + _NN + _NNN + _N*m + c;
	}          
	int x1 = h;
	int x2 = (c==-1) ? h : m;
	int x3 = (c==-1) ? m : c;          
	return _U*l + _NN + x1*_NN + x2*_N + x3;
      }
      
    private: 
      
      /* these are stored values for :  */
      /*  N, i.e. current sentence length*/
      static int _N; 
      /*  N*N */
      static int _NN; 
      /*  N*N*N */
      static int _NNN; 
      /*  number of unlabeled parts */
      static int _U; 
      
      /* the values are updated each time N changes */
      static void update(const int N) {
	_N = N;
	_NN = N*N; 
	_NNN = _NN*N;
	_U = 2*_NN + _NNN + (int)(0.5*(_NN+_N));
      }
    };
        
}

#endif



