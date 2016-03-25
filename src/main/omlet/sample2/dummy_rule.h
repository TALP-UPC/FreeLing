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

//------------------------------------------------------------------//
//
//  This file contains a sample dummy weak rule class to illustrate 
//  usage of Omlet library with Adaboost and non-predefined WR types.
//
//------------------------------------------------------------------//

#include <cmath>

// get abstract classes to derive from
#include "weakrule.h"


// First, derive from wr_params a class containing all parameters
// your weak rule may need (e.g. a tree depth, a threshold biass..)
class myDummyRule_params : public wr_params {
 public:
  /// learning parameters
  double biass;
  double threshold;

  /// constructor: The first two are parameters needed by wr_params
  /// constructor, from third on, are specific parameters for this WR
  myDummyRule_params (int nl, double e, double b, double t);
};

// Then, derive from weak_rule your specific rule behaviour
// In this example, the WR is a set of dummy planes that 
// separate pos and neg examples of each class
class myDummyRule : public weak_rule {
 private:
  // here, define your rule internal stuff

  // This is required, a member to store learning parameters 
  // for this specific type of weak rule
  myDummyRule_params params;

  // separating hyperplane information for each class
  vector<example> point,vector;

  /// copy constructor forbidden
  myDummyRule(const myDummyRule &wr0);

public:
  // just specify virtual functions in weak_rule class

  // Constructor
  myDummyRule(myDummyRule_params *p);

  /// Classification
  /// Important: pred is an array of predictions, one for each label
  ///            the function must *add* its predicion for each label.
  void classify(const example &i, double *pred);

  ///  I/O operations
  void write_to_stream(ostream *os);
  void read_from_stream(istream *is);

  /// Learning
  void learn(const dataset &ds, double &Z);
};


//---------- Class myDummyRule_params ----------------------------------

///////////////////////////////////////////////////////////////
///  Constructor: Pass two first parameters to wr_params, 
/// store the rest as needed.
///////////////////////////////////////////////////////////////

myDummyRule_params::myDummyRule_params (int nl, double e, double b, double t) : wr_params(nl,e), biass(b), 
                                                                                threshold(t) {}


//---------- Class myDummyRule ----------------------------------

///////////////////////////////////////////////////////////////
///  Constructor. Fill 'params' member with provided params
///////////////////////////////////////////////////////////////

myDummyRule::myDummyRule(myDummyRule_params *p) : params(p->nlabels, p->epsilon, p->biass, p->threshold) {}


///////////////////////////////////////////////////////////////
///  Classify given example (just look at the feature and return 
///  the appropriate set of predictions )
///////////////////////////////////////////////////////////////

void myDummyRule::classify(const example &i, double pred[]) {

  for (int l=0; l<params.nlabels; l++) {
    example ex=i;
    // vector from middle mass center point to given example
    ex.add_vector(-1.0,point[l]);  
    // inner product with plane orthogonal vector gives us the answer
    pred[l] = vector[l].inner_product(ex);
  }
}


///////////////////////////////////////////////////////////////
///  Learn a weak rule that classifies the given dataset 
///////////////////////////////////////////////////////////////

void myDummyRule::learn(const dataset &ds, double &Z) {
example::iterator f,x;

  point.reserve(params.nlabels);
  vector.reserve(params.nlabels);

  // compute mass center for pos/neg examples of each class
  example pcentr(params.nlabels);
  example ncentr(params.nlabels);

  dataset::const_iterator ex;   
  for (int l=0; l<params.nlabels; l++) {
    for(ex=ds.begin(); ex!=ds.end(); ++ex) {
      // add example to positive or negative  mass center, with appropriate weight
      if (ex->belongs(l)) pcentr.add_vector(ex->get_weight(l),(*ex));
      else ncentr.add_vector(ex->get_weight(l),(*ex));
    }

    // middle point (biassed) between pos and neg mass centers.
    // (nonsense? well, just a silly example to use wr-specific params).
    example m(0.5+params.biass,pcentr,0.5-params.biass,ncentr);
    // vector from m to pcentr is orthogonal to separating plane
    example w(1.0,pcentr,-1.0,m);
    // normalize w
    double n=w.norm(); 
    for (f=w.begin(); f!=w.end(); f++) w[f->first] /= n;

    // ignore dimensions with very low values (for efficiency, and
    // to use another wr-specific parameter
    for (f=w.begin(); f!=w.end(); f++) 
      if (fabs(f->second) < params.threshold) {
        x=f; x--;
        w.erase(f);
        f=x;
      }
    for (f=m.begin(); f!=m.end(); f++) 
      if (fabs(f->second) < params.threshold) {
        x=f; x--;
        m.erase(f);
        f=x;
      }
   
    // store point and vector that identify class plane
    point.push_back(m);
    vector.push_back(w);
  }
  
  // This WR doesn't need to compute Z (e.g. as mlDTree does), but 
  // since we have to return a Z value, we use default computation
  // method provided in weak_rule class.
  Z = Zcalculus(ds);
}


///////////////////////////////////////////////////////////////
///  Write the learned WR to an ostream
///////////////////////////////////////////////////////////////

void myDummyRule::write_to_stream(ostream *os) {

  for (int l=0; l<params.nlabels; l++) {
    example::const_iterator p;    
    // output plane point
    for (p=point[l].begin(); p!=point[l].end(); p++) (*os) << " " << p->first << " " << p->second;
    (*os)<<endl;
    // output plane orthogonal vector
    for (p=vector[l].begin(); p!=vector[l].end(); p++) (*os) << " " << p->first << " " << p->second;
    (*os)<<endl;    
  }
  
}

///////////////////////////////////////////////////////////////
///  Read a WR from a istream
///////////////////////////////////////////////////////////////

void myDummyRule::read_from_stream(istream *is) {
string line;    
istringstream sin; 
int feat,val;

  point.reserve(params.nlabels);
  vector.reserve(params.nlabels);

  example e(params.nlabels);

  // read point+vector for each label.
  for (int l=0; l<params.nlabels; l++) {
    // input plane point
    getline((*is),line); sin.str(line);
    e.clear();
    while (sin>>feat>>val) e.insert(make_pair(feat,val));
    point.push_back(e);
    // input plane vector
    getline((*is),line); sin.str(line);
    e.clear();
    while (sin>>feat>>val) e.insert(make_pair(feat,val));
    vector.push_back(e);
  }
  
}


//----------------------------------------------------------------------
//---                 Weak Rule REGISTERING                          ---
//----------------------------------------------------------------------

/// Start up stuff to register WR type in the factory. We use an
/// anonymous namespace so this is not be visible from outside
namespace {
  weak_rule* create_myDummyRule(wr_params *p) {
    weak_rule* w=new myDummyRule((myDummyRule_params*)p);
    return(w);
  }

  const string name="myDummyRule";
  const bool registered=wr_factory::register_weak_rule_type(name,create_myDummyRule);
}


