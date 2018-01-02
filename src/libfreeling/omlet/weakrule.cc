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

#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>

#include "freeling/omlet/weakrule.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"WEAKRULE"
#define MOD_TRACECODE OMLET_TRACE

  //---------- Class weak_rule_handler ----------------------------------

  safe_map<std::wstring, wr_factory::WR_constructor> wr_factory::wr_types;

  ///////////////////////////////////////////////////////////////
  ///  Factory. Register new WR type
  ///////////////////////////////////////////////////////////////

  bool wr_factory::register_weak_rule_type(const std::wstring &type, WR_constructor bldr) {

    WR_constructor cs;
    bool found = wr_types.find_safe(type,cs);
    if (not found) wr_types.insert_safe(type,bldr);
    else WARNING(L"REGISTER_WR:  Weak rule type '"+type+L"' was already registered. Ignored."); 
    // insertion fails if type already existed
    return (not found);
  }

  ///////////////////////////////////////////////////////////////
  ///  Factory. Unregister WR type
  ///////////////////////////////////////////////////////////////

  bool wr_factory::unregister_weak_rule_type(const std::wstring &type) {

    WR_constructor cs;
    bool found = wr_types.find_safe(type,cs);
    if (found) wr_types.erase_safe(type);
    else WARNING(L"UNREGISTER_WR:  Weak rule type '"+type+L"' is not registered.");
    // deletion fails if type didn't exist.
    return found;
  }

  ///////////////////////////////////////////////////////////////
  ///  Factory. Create a WR of requested type, with given 
  /// learning parameters.
  ///////////////////////////////////////////////////////////////

  weak_rule* wr_factory::create_weak_rule(const std::wstring &type, wr_params *parm) {

    weak_rule *wr;

    // find requested wr type
    WR_constructor cs;
    bool found = wr_types.find_safe(type,cs);
    if (found) 
      wr = (cs)(parm);     // call appropriate constructor for WR
    else 
      ERROR_CRASH(L"CREATE_WR:  Weak rule type '"+type+L"' is not registered.");

    return(wr);
  }

  ///////////////////////////////////////////////////////////////
  ///  Factory. Create a WR of requested type with given nlabels
  /// (probably we are not going to learn it, no need for other params).
  ///////////////////////////////////////////////////////////////

  weak_rule* wr_factory::create_weak_rule(const std::wstring &type, int nlab) {

    weak_rule *wr;
    wr_params parm(nlab,0.0);

    // find requested wr type
    WR_constructor cs;
    bool found = wr_types.find_safe(type,cs);
    if (found)
      wr = (cs)(&parm);     // call appropriate constructor for WR
    else
      ERROR_CRASH(L"CREATE_WR: Unregistered weak rule type "+type);

    return(wr);
  }


  //---------- Class wr_params ----------------------------------

  ///////////////////////////////////////////////////////////////
  /// constructor
  ///////////////////////////////////////////////////////////////

  wr_params::wr_params (int nl, double e) : nlabels(nl), epsilon(e) {}



  //---------- Class weak_rule ----------------------------------
  //--- class weak_rule is abstract, so most methods 
  //--- have no implementation. 

  ///////////////////////////////////////////////////////////////
  ///  default Z-computation.
  ///////////////////////////////////////////////////////////////

  double weak_rule::Zcalculus(const dataset &ds) const {

    double Z = 0.0;  
    for (dataset::const_iterator ex=ds.begin(); ex!=ds.end(); ++ex) 
      for (int l=0; l<ds.get_nlabels(); l++) 
        Z += ex->get_weight(l);
  
    return Z; 
  }


  //---------- Class dt_node ----------------------------------

  ///////////////////////////////////////////////////////////////
  /// empty constructor 
  ///////////////////////////////////////////////////////////////

  dt_node::dt_node() {
    feature=0;
    predictions.clear();
  }

  ///////////////////////////////////////////////////////////////
  /// constructor for intermediate nodes: set feature
  ///////////////////////////////////////////////////////////////

  dt_node::dt_node(int f) {
    feature=f;
    predictions.clear();
  }

  ///////////////////////////////////////////////////////////////
  /// constructor for leaves: set predictions.
  ///////////////////////////////////////////////////////////////

  dt_node::dt_node(int nl, double pr[]) {
    feature = 0;
    predictions = vector<double>(nl,0.0);
    for (int l=0; l<nl; l++) 
      predictions[l] = pr[l];
  }

  ///////////////////////////////////////////////////////////////
  /// copy constructor
  ///////////////////////////////////////////////////////////////

  dt_node::dt_node(const dt_node &n) {
    feature = n.feature;
    predictions = n.predictions;
  }


  //---------- Class mlDTree_params ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  Constructor
  ///////////////////////////////////////////////////////////////

  mlDTree_params::mlDTree_params (int nl, double e, int mxd) : wr_params(nl,e), max_depth(mxd) {}


  //---------- Class mlDTree ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  Constructor
  ///////////////////////////////////////////////////////////////

  mlDTree::mlDTree(mlDTree_params *p) : params(p->nlabels, p->epsilon, p->max_depth) {}


  ///////////////////////////////////////////////////////////////
  ///  Classify given example
  ///////////////////////////////////////////////////////////////

  inline void mlDTree::classify(const example &i, double pred[]) {
    classify (i,pred,rule.begin());
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for classifying an example
  ///////////////////////////////////////////////////////////////

  void mlDTree::classify (const example &i, double pred[], tree<dt_node>::iterator t) {

    if (t->feature == 0) {
      // leaf reached
      for (int l=0; l<params.nlabels; l++)
        pred[l] += t->predictions[l];
    }
    else {
      // first child is for "false" case
      tree<dt_node>::sibling_iterator child = t.sibling_begin();   
      // second child is for "true" case
      if (i.get_feature_value(t->feature)) ++child; 
      // descend through the appropriate child
      classify(i, pred, child);
    }
  }


  ///////////////////////////////////////////////////////////////
  ///  Learn a Decision Tree
  ///////////////////////////////////////////////////////////////

  void mlDTree::learn(const dataset &ds, double &Z) {

    // adjust params depending on dataset, if necessary
    if (params.epsilon == -1.0) params.epsilon = 1.0/(ds.size()*params.nlabels);
    if (params.max_depth < 0) params.max_depth = int( fabs(float(rand())/(float(RAND_MAX)+1)) *(-params.max_depth+1));

    // prepare for learning process
    used_features.clear();

    // learn and store the rule
    rule = learn (ds, Z, 0);

    // clean up and return. "used_features" is a member, and 
    // may be used by other methods in the class.
    used_features.clear();
  }


  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning Decision Trees
  ///////////////////////////////////////////////////////////////

  tree<dt_node> mlDTree::learn (const dataset &ds, double &Z, int depth) {
    int l, c;

    if (stopping_criterion(ds, depth)) {
      // further splitting is worthless, stop
      double* W = new double[2*params.nlabels];
      for (l=0;l<params.nlabels;l++) {
        W[2*l]   = 0.0;
        W[2*l+1] = 0.0;
      }
      dataset::const_iterator ex;   
      for(ex=ds.begin(); ex!=ds.end(); ++ex) {
        for (l=0;l<params.nlabels;l++) {
          c =  ( ex->belongs(l) ? 1 : 0 );
          W[2*l+c] += ex->get_weight(l);
        }
      }
      Z = Zcalculus(W, 1);

      double* pred=new double[params.nlabels];
      // create leaf node, simple tree
      Cprediction(0, W, pred);
      dt_node nd(params.nlabels,pred);
      tree<dt_node> t(nd);
      delete [] W;
      delete [] pred;
      return t;
    }
    else {
      // select best feature
      double* Wfc=new double[4*params.nlabels];
      int bestf = best_feature(ds, Wfc);

      if (bestf == 0) {
        // no more features available
        double* pred=new double[params.nlabels];
        Z = Zcalculus(Wfc, 1);
        // create leaf node, simple tree
        Cprediction(0, Wfc, pred);
        dt_node nd(params.nlabels,pred);
        tree<dt_node> t(nd);
        delete [] Wfc;
        delete [] pred;
        return t;
      }
      else if (depth == params.max_depth) {
        // max depth reached, do not split further, just create leaves.
        double* pred=new double[params.nlabels];
        Z = Zcalculus(Wfc, 2);
        // create leaf node, simple tree
        Cprediction(0, Wfc, pred);
        dt_node nd0(params.nlabels,pred);
        tree<dt_node> wr0(nd0);
        // create leaf node, simple tree
        Cprediction(1, Wfc, pred);
        dt_node nd1(params.nlabels,pred);
        tree<dt_node> wr1(nd1);
        // build and return resulting tree
        dt_node nd(bestf); 
        tree<dt_node> t(nd); 
        t.add_child(wr0); t.add_child(wr1);
        delete [] Wfc;
        delete [] pred;      
        return t;
      }
      else {
        // split new level
        dataset ds0(params.nlabels);
        dataset ds1(params.nlabels);

        // in the first split (depth==0) the nodes of the dataset are newly created; 
        //  subsequent splits consist of a reindexation of the dataset nodes
        ds.split(bestf, ds0, ds1);

        used_features.insert(bestf);
        double Z0, Z1;
        tree<dt_node> wr0 = learn (ds0, Z0, depth+1);
        tree<dt_node> wr1 = learn (ds1, Z1, depth+1);
        used_features.erase(bestf);

        Z = Z0+Z1;      

        // build and return resulting tree
        dt_node nd(bestf); 
        tree<dt_node> t(nd); 
        t.add_child(wr0); t.add_child(wr1);
        delete [] Wfc;
        return t;
      }
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning Decision Trees
  ///////////////////////////////////////////////////////////////

  bool mlDTree::stopping_criterion(const dataset &ds, int depth) {
    int l;
    for (l=0; l<params.nlabels; l++) {
      if ((ds.get_positive_size(l)!=0) && (ds.get_negative_size(l)!=0)) {
        return false;
      }
    }
    return true; 
  }


  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning Decision Trees
  ///////////////////////////////////////////////////////////////

  int mlDTree::best_feature(const dataset &ds, double *Wflc) {
  
    int sizef = ds.get_dimension() + 1;
    double **feat = new double*[sizef];
  
    for(int i=0; i<sizef; i++) feat[i] = NULL;
  
    // create index
    double** wf=new double*[params.nlabels];
    for (int i=0; i!=params.nlabels; i++) wf[i]=new double[2];

    for (int l=0; l<params.nlabels; l++) {
      wf[l][0] = 0.0;
      wf[l][1] = 0.0;
    }

    dataset::const_iterator ex;   
    example::const_iterator f;

    double* w=new double[params.nlabels];
    int* c=new int[params.nlabels];
    for(ex=ds.begin(); ex!=ds.end(); ++ex) {
      for (int l=0; l<params.nlabels; l++) {
        w[l] = ex->get_weight(l);
        c[l] = ( ex->belongs(l) ? 1 : 0 );
        wf[l][c[l]] += w[l];
      }
      for (f=ex->begin(); f!=ex->end(); ++f) {
        int i = f->first;
        if (used_features.find(i)==used_features.end()) {
          if (feat[i]==NULL) {
            feat[i] = new double[2*params.nlabels];
            for (int l=0; l<params.nlabels; l++) {
              feat[i][2*l] = 0.0;
              feat[i][2*l+1] = 0.0;
            }
          }
          for (int l=0; l<params.nlabels; l++)
            feat[i][2*l+c[l]] += w[l];
        }
      }
    }
  
    //recorregut per a trobar el best feature
    int bestf = 0;
    double Z, Zbf=1;
    for (int i=0; i<sizef; i++) {
      if (feat[i]!=NULL) {
        for (int l=0;l<params.nlabels;l++) {
          Wflc[2*params.nlabels + 2*l]    = feat[i][2*l];
          Wflc[2*params.nlabels + 2*l +1] = feat[i][2*l+1];
          Wflc[2*l]    = wf[l][0] - Wflc[2*params.nlabels + 2*l];
          Wflc[2*l +1] = wf[l][1] - Wflc[2*params.nlabels + 2*l +1];
        }
        Z = mlDTree::Zcalculus(Wflc, 2);
        if (!bestf || (Z < Zbf)) {
          bestf = i;
          Zbf = Z;
        }
      }
    }

    if (bestf != 0) {
      for (int l=0; l<params.nlabels; l++) {
        Wflc[2*params.nlabels + 2*l]    = feat[bestf][2*l];
        Wflc[2*params.nlabels + 2*l +1] = feat[bestf][2*l+1];
        Wflc[2*l]    = wf[l][0] - Wflc[2*params.nlabels + 2*l];
        Wflc[2*l +1] = wf[l][1] - Wflc[2*params.nlabels + 2*l +1];
      }
    }
    else {
      for (int l=0; l<params.nlabels; l++) {
        Wflc[2*params.nlabels + 2*l]    = 0.0;
        Wflc[2*params.nlabels + 2*l +1] = 0.0;
        Wflc[2*l]    = wf[l][0];
        Wflc[2*l +1] = wf[l][1];
      }
    }

    // delete index;
    for(int i=0; i<sizef; i++) delete [] feat[i];
    delete [] feat;

    // free allocated memory
    delete [] w;
    delete [] c;
    for(int l=0; l<params.nlabels; l++) delete [] wf[l];
    delete [] wf;

    return bestf;
  }  

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning Decision Trees
  ///////////////////////////////////////////////////////////////

  double mlDTree::Zcalculus(double *W, int ndim) {
    int offset;
    double Z = 0.0;
    for (int l=0; l<params.nlabels; l++) {
      for (int i=0; i<ndim; i++) {
        offset = i * 2 *params.nlabels;
        Z += W[offset+2*l+1] * sqrt((W[offset+2*l]   + params.epsilon)/(W[offset+2*l+1]+params.epsilon));
        Z += W[offset+2*l]   * sqrt((W[offset+2*l+1] + params.epsilon)/(W[offset+2*l ]+params.epsilon));
      }
    }
    return Z;
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning Decision Trees
  ///////////////////////////////////////////////////////////////

  void mlDTree::Cprediction(int v, double *W, double result[]) {
    int offset = v * 2 * params.nlabels;
    for (int l=0; l<params.nlabels; l++) {
      result [l] = 0.5 * log((W[offset+2*l+1] + params.epsilon) / (W[offset+2*l] + params.epsilon));
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary to dump a tree
  ///////////////////////////////////////////////////////////////

  void mlDTree::write_to_stream(tree<dt_node>::iterator t, std::wostream *os) {

    if (t->feature==0) {
      (*os) << L"-";
      int l;
      for (l=0; l<params.nlabels; l++) {
        (*os) << L" " << t->predictions[l];
      }
      (*os) << endl;
    }
    else {
      (*os) << L"+ " << t->feature << endl;

      // get first child and dump it
      tree<dt_node>::sibling_iterator child = t.sibling_begin();   
      write_to_stream(child,os);
      // proceed to second child and dump it
      ++child;
      write_to_stream(child,os);
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Input/Output operations to dump/read decision trees  to/from a file,
  ///////////////////////////////////////////////////////////////

  void mlDTree::write_to_stream(std::wostream *os) {
    write_to_stream(rule.begin(),os);
  }

  ///////////////////////////////////////////////////////////////
  ///  Input/Output operations to dump/read decision trees
  ///  to/from a file,
  ///////////////////////////////////////////////////////////////

  inline void mlDTree::read_from_stream(std::wistream *is) {
    rule = read_dt(is);
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary to load a tree
  ///////////////////////////////////////////////////////////////

  tree<dt_node> mlDTree::read_dt(std::wistream *is) {

    wchar_t c;
    (*is) >> c;
    if (c == '-') {
      double* p=new double[params.nlabels];
      for (int l=0;l<params.nlabels;l++) (*is) >> p[l];
    
      dt_node nd(params.nlabels,p);
      tree<dt_node> t(nd);
      delete [] p;
      return t;
    }
    else {
      int f;
      (*is) >> f;
      dt_node nd(f); 
      tree<dt_node> t(nd); 

      tree<dt_node> wr0 = read_dt(is);
      tree<dt_node> wr1 = read_dt(is);
      t.add_child(wr0); t.add_child(wr1);

      return t;
    }
  }


  /// Start up stuff to register WR type in the factory. We use an
  /// anonymous namespace so this is not visible from outside
  namespace {
    weak_rule* create_mlDTree(wr_params *p) {
      weak_rule* w=new mlDTree((mlDTree_params *)p);
      return(w);
    }

    const wstring name=L"mlDTree";
    //const bool registered=wr_factory::register_weak_rule_type(name,create_mlDTree);
    const bool registered=wr_factory::register_weak_rule_type(name,create_mlDTree);
  }


} // namespace
