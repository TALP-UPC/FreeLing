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

#include "freeling/omlet/example.h"

#include <cstdlib>
#include <iostream>
#include <cmath>

#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"EXAMPLE"
#define MOD_TRACECODE OMLET_TRACE


  //---------- Class category ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  Constructor
  ///////////////////////////////////////////////////////////////

  category::category(bool b, double w, double p) {
    belongs=b; weight=w; prediction=p;
  }

  ///////////////////////////////////////////////////////////////
  ///  Copy constructor
  ///////////////////////////////////////////////////////////////

  category::category(const category &c) {
    belongs=c.belongs; weight=c.weight; prediction=c.prediction;
  }


  //---------- Class example ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  Constructor
  ///////////////////////////////////////////////////////////////

  example::example(int nlab) { 
    nlabels = nlab;
    dimension = 0;
    labels = vector<category>(nlab,category(false,0,0));
  }

  ///////////////////////////////////////////////////////////////
  ///  copy constructor
  ///////////////////////////////////////////////////////////////

  example::example(const example &e) {
    map<int,double>::const_iterator i; 
    this->clear(); 
    for (i=e.begin(); i!=e.end(); i++) this->insert(*i);
    dimension = e.dimension;
    nlabels = e.get_nlabels();
    labels.reserve(nlabels); 
    for (int j=0; j<nlabels; j++) labels[j] = e.labels[j];
  }


  ///////////////////////////////////////////////////////////////
  ///  constructor: add two vectors to get a new one
  ///////////////////////////////////////////////////////////////

  example::example(double f1, const example& i1, double f2, const example& i2) {

    nlabels=i1.get_nlabels();
    labels = vector<category>(nlabels,category(false,0,0));
   
    // compute entries for all features in i1 (either also in i2 or not)
    example::const_iterator it1;
    for (it1=i1.begin(); it1!=i1.end(); it1++) {
      add_feature(it1->first, f1*it1->second + f2*i2.get_feature_value(it1->first));
    }

    // add entries for features in i2 missing in i1
    example::const_iterator it2;
    for (it2=i2.begin(); it2!=i2.end(); it2++) {
      it1 = i1.find(it2->first);
      if (it1 == i1.end())
        add_feature(it2->first, f2*it2->second);
    }
  }


  ///////////////////////////////////////////////////////////////
  /// compute norm of example as a vector
  ///////////////////////////////////////////////////////////////

  double example::norm() const {
    example::const_iterator it;
    double y=0.0;

    // add entries for features in i1 (either also in i2 or not)
    // if not in i2, value will be 0.
    for (it=this->begin(); it!=this->end(); it++)
      y += it->second * it->second;

    return sqrt(y);
  }


  ///////////////////////////////////////////////////////////////
  ///  Add a feature (given as label+value) to an example
  ///////////////////////////////////////////////////////////////

  void example::add_feature(int l, double v) {

    this->insert(make_pair(l,v));
    if (dimension < l) dimension = l;
  }

  ///////////////////////////////////////////////////////////////
  ///  Get value for a feature
  ///////////////////////////////////////////////////////////////

  double example::get_feature_value(int label) const {

    map<int,double>::const_iterator i = this->find(label);

    if (i != this->end()) return (i->second);
    else return 0.0;
  }


  ///////////////////////////////////////////////////////////////
  ///  Get number of possible labels of the example
  ///////////////////////////////////////////////////////////////

  int example::get_nlabels() const {
    return nlabels;
  }

  ///////////////////////////////////////////////////////////////
  ///  Get dimension of the example
  ///////////////////////////////////////////////////////////////

  int example::get_dimension() const { 
    return dimension; 
  }

  ///////////////////////////////////////////////////////////////
  ///  vector dot product
  ///////////////////////////////////////////////////////////////

  double example::inner_product(const example &i2) const {
    example::const_iterator it;
    double y = 0.0;

    // use shortest vector to scan intersection
    if (i2.size() > this->size()) {
      for (it=this->begin(); it!=this->end(); it++)
        y += it->second * i2.get_feature_value(it->first);
    }
    else {
      for (it=i2.begin(); it!=i2.end(); it++)
        y += it->second * this->get_feature_value(it->first);
    }
    
    return y;
  }


  ///////////////////////////////////////////////////////////////
  /// add given feature vector with given weight
  ///////////////////////////////////////////////////////////////
  void example::add_vector(double f, const example &i2) {

    // for each features in i2, appropiately add to curent vector
    example::const_iterator it2;
    for (it2=i2.begin(); it2!=i2.end(); it2++)
      (*this)[it2->first] = this->get_feature_value(it2->first) + f*it2->second;
  }


  ///////////////////////////////////////////////////////////////
  ///  Set all information about a class
  ///////////////////////////////////////////////////////////////

  void example::set_label(int l, bool b, double w, double pr) {
    labels[l].belongs=b; 
    labels[l].weight=w; 
    labels[l].prediction=pr; 
  }

  ///////////////////////////////////////////////////////////////
  ///  Set on/of the belonging to a class 
  ///////////////////////////////////////////////////////////////

  void example::set_belongs(int l, bool b) { 
    labels[l].belongs=b; 
  }

  ///////////////////////////////////////////////////////////////
  ///  Find out whether the example belongs to a class 
  ///////////////////////////////////////////////////////////////

  bool example::belongs(int l) const { 
    return (labels[l].belongs); 
  }

  ///////////////////////////////////////////////////////////////
  ///  Find out sign of belonging
  ///////////////////////////////////////////////////////////////

  int example::sign(int l) const { 
    return (labels[l].belongs ? +1 : -1); 
  }

  ///////////////////////////////////////////////////////////////
  ///  set weight for the class
  ///////////////////////////////////////////////////////////////

  void example::set_weight(int l, double w) { 
    labels[l].weight=w; 
  }

  ///////////////////////////////////////////////////////////////
  ///  set weight for the class
  ///////////////////////////////////////////////////////////////

  double example::get_weight(int l) const { 
    return labels[l].weight; 
  }
  
  ///////////////////////////////////////////////////////////////
  ///  set prediction for the class
  ///////////////////////////////////////////////////////////////

  void example::set_prediction(int l, double pr) { 
    labels[l].prediction=pr; 
  }

  ///////////////////////////////////////////////////////////////
  ///  get prediction for the class
  ///////////////////////////////////////////////////////////////

  double example::get_prediction(int l) const { 
    return labels[l].prediction; 
  }














} // namespace
