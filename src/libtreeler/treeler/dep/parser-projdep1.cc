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

#include "treeler/dep/parser-projdep1.h"

#include <cstdlib>

namespace treeler {


  void ProjDep1::unravel_tree(Label<PartDep1>& y, const backp* const BP,
      const int h, const int m,
      const bool complete, const int N) {
    //    cout << "utree : h " << h << " m " << m << " c? " << complete << flush;
    if (h==m) {
      //      cout << endl;
      return;
    }

    if (complete) {
      int r = BP->get_r_com(h,m);
      //      cout << " ; complete r " << r << endl;
      unravel_tree(y, BP, h, r, false, N);
      unravel_tree(y, BP, r, m, true, N);
    }
    else {
      int r = BP->get_r_inc(h,m);
      int k = BP->get_k(h,m);
      //      cout << " ; incomplete r " << r << " k " << k << endl;
      //      y.push_back((h == -1 ? m : h) + N*m + N*N*k);
      y.push_back(PartDep1(h, m, k));
      /*dtree.set_dependency(h, m, k);*/
      if (h<m) {
        unravel_tree(y, BP, h, r, true, N);
        unravel_tree(y, BP, m, r+1, true, N);
      }
      else {
        unravel_tree(y, BP, m, r, true, N);
        unravel_tree(y, BP, h, r+1, true, N);
      }
    }
  }


}
