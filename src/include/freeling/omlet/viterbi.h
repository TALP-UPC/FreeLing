//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#ifndef _VITERBI
#define _VITERBI

#include <math.h>
#include <vector>
#include <string>

namespace freeling {

  //---------- Visible Viterbi Class  ----------------------------------

  ////////////////////////////////////////////////////////////////
  ///  The class vis_viterbi implements the viterbi algorithm
  ///   given the weights of different labels, without
  ///   hidden information
  ////////////////////////////////////////////////////////////////

  class vis_viterbi {

  private:
    ///N:  number of classes
    int N;
    /// p_ini: vector with initial probabilities for each class
    std::vector<double> p_ini;
    /// p_trans: matrix with the probability transitions from one class to another
    /// e.g. P(B,B), P(O,B), etc
    std::vector<std::vector<double> > p_trans;
    /// whether to use softmax to convert given weights to probabilities
    bool use_softmax;
    /// convert weights to probabilities
    void softmax(double* p) const;
    /// logprob for probability zero
    double ZERO_logprob;    

  public:
    /// Constructor: Create dynammic storage for the best path, loading file with model probabilities
    vis_viterbi (const std::wstring &);

    /// find_best_path: perform viterbi algorithm given the weights matrix
    std::vector<int> find_best_path (std::vector<double*>&) const;
  };

} // namespace

#endif
