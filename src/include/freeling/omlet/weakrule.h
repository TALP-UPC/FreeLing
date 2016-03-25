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

#ifndef _WEAKRULE
#define _WEAKRULE

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "freeling/tree.h"
#include "freeling/safe_map.h"
#include "freeling/omlet/dataset.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class wr_params is a dummy class used to derive the 
  /// set of parameters of each WR type. It contains parameters
  /// needed by any WR type
  ////////////////////////////////////////////////////////////////

  class wr_params {
  public:
    int nlabels;
    double epsilon;

    /// constructor
    wr_params (int nl, double e);
  };

  ////////////////////////////////////////////////////////////////
  ///  Class weak_rule is an abstract class generalizing any kind
  /// of weak rule that adaboost can use.
  ////////////////////////////////////////////////////////////////

  class weak_rule {

  public:
    /// Destructor
    virtual ~weak_rule() {};
 
    /// Classification. Pred is an array of predictions, one for each label,
    ///                 the function *adds* its predicion for each label.
    virtual void classify(const example &i,double pred[]) = 0;

    ///  I/O operations
    virtual void read_from_stream(std::wistream *is) = 0;
    virtual void write_to_stream(std::wostream *os) = 0;
    
    /// learn a WR (and compute normalization factor Z)
    virtual void learn(const dataset &ds, double &Z) = 0;

    /// Compute normalization factor (default procedure,
    /// each weak rule can redefine (or ignore) this function 
    /// if it has a more efficeint way to compute Z factor
    virtual double Zcalculus(const dataset &ds) const;
  };




  ////////////////////////////////////////////////////////////////
  ///  Class wr_factory is a factory enabling to register new
  /// types of weak_rules (provided they are derived from 
  /// weak_rule class below
  ////////////////////////////////////////////////////////////////

  class wr_factory {

  public:
    typedef weak_rule* (*WR_constructor)(wr_params*);
    static void initialize();
    static bool register_weak_rule_type(const std::wstring &type, WR_constructor builder);
    static bool unregister_weak_rule_type(const std::wstring &type);
    static weak_rule* create_weak_rule(const std::wstring &type, wr_params *p);
    static weak_rule* create_weak_rule(const std::wstring &type, int nlabels);

  private:
    // store weakrule types registered by user apps
    static safe_map<std::wstring, WR_constructor> wr_types;

  };


  ////////////////////////////////////////////////////////////////
  ///  Class mlDTree_params stores the set of params for
  /// this kind of weak rules.
  ////////////////////////////////////////////////////////////////

  class mlDTree_params : public wr_params {
  public:
    /// learning parameters
    int    max_depth;

    /// constructor
    mlDTree_params (int nl, double e, int mxd);
  };

  ////////////////////////////////////////////////////////////////
  ///  Class dt_node stores the info in one node of the decision tree
  ////////////////////////////////////////////////////////////////

  class dt_node {
    friend class mlDTree;
    //protected:
  public:
    int     feature;              // 0 when leaf
    std::vector<double>  predictions;  // empty when not leaf (when leaf, array of predictions, one for each class)

  public:
    // empty constructor
    dt_node();
    /// create non-leaf node, with given feature.
    dt_node(int f);
    /// create leaf node, with given predictions.
    dt_node(int nl, double pr[]);
    /// copy constructor
    dt_node(const dt_node &n);
  };

  ////////////////////////////////////////////////////////////////
  ///  Class mlDTree implements a multilabel decision tree that
  /// can be used by adaboost as a weak rules.
  ////////////////////////////////////////////////////////////////

  class mlDTree : public weak_rule {

  private:
    // learning parameters for the specific type of weak rule
    mlDTree_params params;

    // decision tree itself
    tree<dt_node> rule;
    // learning auxiliary list.
    std::set<int> used_features; 

    /// auxiliar classifying function
    void classify (const example &i, double pred[], tree<dt_node>::iterator t);

    /// auxiliar I/O functions
    void write_to_stream(tree<dt_node>::iterator t, std::wostream *os);
    tree<dt_node> read_dt(std::wistream *is);
  
    /// auxiliar learning functions
    tree<dt_node> learn (const dataset &ds, double &Z, int depth);

    bool stopping_criterion(const dataset &ds, int depth);
    /// W is W[2][nlabels][2]
    int best_feature(const dataset &ds, double *W);
    /// W is W[v][nlabels][2]; result is result[nlabels]
    void Cprediction(int v, double *W, double result[]);
    /// We will re-use Z computed during decision-tree building
    /// so we ignore default Zcalculus and do it our way.
    /// W is W[ndim][nlabels][2]
    double Zcalculus(double *W, int ndim);

    /// copy constructor forbidden
    mlDTree(const mlDTree &wr0);

  public:

    // Constructor
    mlDTree(mlDTree_params *p);

    /// Classification
    /// Important: pred is an array of predictions, one for each label
    ///            the function *adds* its predicion for each label.
    void classify(const example &i, double pred[]);

    ///  I/O operations
    void write_to_stream(std::wostream *os);
    void read_from_stream(std::wistream *is);

    /// Learning
    void learn(const dataset &ds, double &Z);
  };

} // namespace

#endif 

