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

#ifndef _ADABOOST
#define _ADABOOST

#include "freeling/omlet/weakrule.h"
#include "freeling/omlet/dataset.h"
#include "freeling/omlet/classifier.h"

#include <iostream>
#include <string>
#include <list>
#include <vector>

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class AdaBoost implement a generic AB learner/classifier,
  /// which may be based on any kind of weak rule.
  ////////////////////////////////////////////////////////////////

  class adaboost : public std::list<weak_rule*>, public classifier {
  private:
    /// class parameters
    bool option_initialize_weights;

    /// type of used weak rules
    std::wstring wr_type;

    adaboost::const_iterator pcl_pointer; // partial classification pointer
    int nrules;

    /// output 
    std::wostream *out;

    /// auxiliar learning functions
    void initialize_weights(dataset &ds);
    void update_weights(weak_rule *wr, double Z, dataset &ds);
    void add_weak_rule(weak_rule *wr);

    /// copy constructor forbidden
    adaboost(const adaboost &old_bab); 

  public:
    /// constructors, destructor and access methods
    adaboost(int nl, std::wstring t);
    adaboost(const std::wstring &file, const std::wstring &codes);
    int n_rules() const;

    /// classification methods. 
    /// Important: pred is an array of predictions, one for each label
    ///            the function *assigns* its predicion for each label.
    void classify(const example &i,  double pred[]) const;
    /// classification returning vector: useful for Java API
    std::vector<double> classify(const example &i) const;

    /// partial classification
    void pcl_ini_pointer();
    int  pcl_advance_pointer(int steps);
    /// Important: pred is an array of predictions, one for each label
    ///            the function *adds* its predicion for each label.
    void pcl_classify(const example &i, double *pred, int nrules);

    /// learning methods
    void learn(dataset &ds, int nrounds, bool init, wr_params *p);
    void learn(dataset &ds, int nrounds, bool init, wr_params *p, const std::wstring &outf);

    /// I/O methods
    void set_output(std::wostream *os);
    void read_from_stream(std::wistream *in);
    void read_from_file(const std::wstring &f);

    void set_initialize_weights(bool b);
  };

} // namespace

#endif 
