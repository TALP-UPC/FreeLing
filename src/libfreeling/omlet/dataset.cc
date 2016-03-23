//////////////////////////////////////////////////////////////////
//
//    Omlet - Open Machine Learning Enhanced Toolkit
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This file is part of the Omlet library
//
//    The Omlet library is free software; you can redistribute it 
//    and/or modify it under the terms of the GNU Affero General Public
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
//    Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110-1301 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx Omega.S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

//
// Author: Xavier Carreras
//

#include "freeling/omlet/dataset.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DATASET"
#define MOD_TRACECODE OMLET_TRACE


  //---------- Class dataset ----------------------------------

  /// Static class stuff
  std::list<example> dataset::all_examples;

  ///////////////////////////////////////////////////////////////
  ///  Constructor
  ///////////////////////////////////////////////////////////////

  dataset::dataset(int nlab) {

    size_pos = vector<int>(nlab,0);
    size_neg = vector<int>(nlab,0);
    nlabels = nlab;
    dimension = 0;
  }


  ///////////////////////////////////////////////////////////////
  ///  Add example to the repository and to current dataset
  ///////////////////////////////////////////////////////////////

  void dataset::add_example(const example &e) {

    if (e.get_nlabels() != nlabels) {
      WARNING(L"Example ignored: number of labels different from dataset nlabels.");
    }
    else {
      // store new element in global repository
      all_examples.push_front(e);
      // add iterator to new element into current dataset
      add_member(all_examples.begin());
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Add example in repository as member of current dataset
  ///////////////////////////////////////////////////////////////

  void dataset::add_member(std::list<example>::iterator e) {

    // dimension of data set is that of largest example
    if (e->get_dimension() > dimension) dimension = e->get_dimension();
  
    // count number of pos/neg examples for each class
    for (int l=0; l<nlabels; l++) 
      if (e->belongs(l)) size_pos[l]++; else size_neg[l]++;
    
    this->push_back(e);
  }

  ///////////////////////////////////////////////////////////////
  ///  split dataset according to a feature
  ///////////////////////////////////////////////////////////////

  void dataset::split(int feature, dataset &ds0, dataset &ds1) const {

    list<list<example>::iterator>::const_iterator ex;
    for (ex=this->begin(); ex!=this->end(); ex++) {
      if ((*ex)->get_feature_value(feature))
        ds1.add_member(*ex);
      else 
        ds0.add_member(*ex);
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Get size of negative examples subset for class l
  ///////////////////////////////////////////////////////////////

  int dataset::get_negative_size(int l) const { return size_neg[l]; }

  ///////////////////////////////////////////////////////////////
  ///  Get size of positive examples subset for class l
  ///////////////////////////////////////////////////////////////

  int dataset::get_positive_size(int l) const { return size_pos[l]; }

  ///////////////////////////////////////////////////////////////
  ///  Get number of labels
  ///////////////////////////////////////////////////////////////

  int dataset::get_nlabels() const { return nlabels; }

  ///////////////////////////////////////////////////////////////
  ///  Get dataset dimension
  ///////////////////////////////////////////////////////////////

  int dataset::get_dimension() const { return dimension; }

















} // namespace
