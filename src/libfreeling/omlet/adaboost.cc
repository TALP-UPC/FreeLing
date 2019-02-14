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

#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/omlet/adaboost.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"ADABOOST"
#define MOD_TRACECODE OMLET_TRACE


  //---------- Class AdaBoost ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  Set initialize weights option on/off
  ///////////////////////////////////////////////////////////////

  void adaboost::set_initialize_weights(bool b) {
    option_initialize_weights = b;
  }

  ///////////////////////////////////////////////////////////////
  ///  Constructor. Empty adaboost
  ///////////////////////////////////////////////////////////////

  adaboost::adaboost(int nl, std::wstring t) : classifier(L"") {
    nrules = 0;
    out   = NULL;
    wr_type = t;
  }

  ///////////////////////////////////////////////////////////////
  ///  Constructor. Create a classifier loading given file
  ///////////////////////////////////////////////////////////////

  adaboost::adaboost(const wstring &file, const wstring &codes) : classifier(codes) {

    nrules = 0;
    out   = NULL;

    // open file
    wifstream in;
    util::open_utf8_file(in,file);
    if (in.fail()) ERROR_CRASH(L"Error opening file "+file);

    // next line is the type of Weak Rules
    in>>wr_type;

    // following lines are the model itself
    read_from_stream((wistream *)&in);
  }

  ///////////////////////////////////////////////////////////////
  ///  Destructor
  ///////////////////////////////////////////////////////////////

  adaboost::~adaboost() {
    while (!this->empty()) {
      delete this->front();
      this->pop_front();
    }
  } 

  ///////////////////////////////////////////////////////////////
  ///  Get number of rules of the classifier
  ///////////////////////////////////////////////////////////////

  int adaboost::n_rules() const {
    return nrules;
  }


  ///////////////////////////////////////////////////////////////
  ///  Classify given example
  ///////////////////////////////////////////////////////////////

  void adaboost::classify(const example &i, double pred[]) const {

    for (int l=0; l<this->get_nlabels(); l++) pred[l] = 0.0;

    adaboost::const_iterator w;
    for (w=this->begin(); w!=this->end(); w++)
      (*w)->classify(i, pred);
  }


  ///////////////////////////////////////////////////////////////
  ///  Classify given example. Useful for Java API
  ///////////////////////////////////////////////////////////////

  vector<double> adaboost::classify(const example &i) const {

    double *pred = new double[this->get_nlabels()];
    for (int l=0; l<this->get_nlabels(); l++) pred[l] = 0.0;

    adaboost::const_iterator w;
    for (w=this->begin(); w!=this->end(); w++)
      (*w)->classify(i, pred);

    vector<double> p;
    for (int l=0; l<this->get_nlabels(); l++) p.push_back(pred[l]);
    delete [] pred;

    return p;   
  }


  ///////////////////////////////////////////////////////////////
  ///  Set starting rule for partial classification
  ///////////////////////////////////////////////////////////////

  void adaboost::pcl_ini_pointer() {
    pcl_pointer = this->begin();
  }

  ///////////////////////////////////////////////////////////////
  ///  Advance to next rule during partial classification
  ///////////////////////////////////////////////////////////////

  int  adaboost::pcl_advance_pointer(int steps) {
    if (steps<0)
      ERROR_CRASH(L"adaboost->pcl_advance_pointer: steps is negative ("+util::int2wstring(steps)+L") !\n");

    int i = 0;
    while (i<steps && pcl_pointer!=this->end()) {
      pcl_pointer++;
      i++;
    }
    return i;
  }

  ///////////////////////////////////////////////////////////////
  ///  Perform incremental partial classification (using only 
  /// a subrange of weak rules, and summing the weigths to the 
  /// current state.
  ///////////////////////////////////////////////////////////////

  void adaboost::pcl_classify(const example &i, double *pred, int nrules) {

    adaboost::const_iterator wr;

    for (wr=pcl_pointer; nrules>0 && wr!=this->end(); wr++) {
      (*wr)->classify(i, pred);
      nrules--;
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Learn an adaboost model, writing to the stream defined
  ///  with "set_output"
  ///////////////////////////////////////////////////////////////

  void adaboost::learn(dataset &ds, int nrounds, bool init, wr_params *p) {

    if (init) initialize_weights(ds);

    int T = 0;
    double Z;

    while ( T < nrounds ) {
      weak_rule* wr = wr_factory::create_weak_rule(wr_type,p);
      wr->learn(ds, Z);
      add_weak_rule(wr);
      update_weights(wr, Z, ds);    
      T++;
    }
  }


  ///////////////////////////////////////////////////////////////
  ///  Learn an adaboost model, writing to the given file
  ///   (Useful for Java API)
  ///////////////////////////////////////////////////////////////

  void adaboost::learn(dataset &ds, int nrounds, bool init, wr_params *p, const std::wstring &outf) {

    wofstream abm;
    util::open_utf8_file(abm,outf);
    if (abm.fail()) ERROR_CRASH(L"Error opening file "+outf);

    set_output((wostream*)&abm);

    learn(ds,nrounds,init,p);

    abm.close();
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning AB model
  ///////////////////////////////////////////////////////////////

  void adaboost::initialize_weights(dataset &ds) {
    double w = 1.0 / (ds.size()*ds.get_nlabels());

    for (dataset::iterator ex=ds.begin(); ex!=ds.end(); ++ex) {
      for (int l=0; l<ds.get_nlabels(); l++) {
        ex->set_weight(l,w);
      }
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning AB model
  ///////////////////////////////////////////////////////////////

  void adaboost::update_weights(weak_rule *wr, double Z, dataset &ds) {
    double w, margin;
    dataset::iterator ex;     
    double* out = new double[ds.get_nlabels()];
    int l;

    for(ex=ds.begin(); ex!=ds.end(); ++ex) {

      // classify example using new wr
      for (l=0; l<ds.get_nlabels(); l++) out[l] = 0.0;
      wr->classify((*ex), out);

      // re-weight example according to errors commited
      for (l=0; l<ds.get_nlabels(); l++) {
        w = ex->get_weight(l);
        margin = ex->sign(l) * out[l];
        ex->set_weight(l, (w * exp(-margin)) / Z);
      }
    }
    delete [] out;
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning AB model
  ///////////////////////////////////////////////////////////////

  void adaboost::add_weak_rule(weak_rule *wr) {

    this->push_back(wr);

    if (out!=NULL) {
      (*out) << L"---" << endl;;
      wr->write_to_stream(out);
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Auxiliary for learning AB model
  ///////////////////////////////////////////////////////////////

  void adaboost::set_output(std::wostream *os) {
    out = os;
  }


  ///////////////////////////////////////////////////////////////
  ///  Input/Output operations to dump/read models to/from a file
  ///////////////////////////////////////////////////////////////

  void adaboost::read_from_stream(std::wistream *in) {
    wstring token;
    weak_rule *wr;

    if (not in->eof()) {
      (*in) >> token;
    }

    while (not in->eof() and token==L"---") {
      wr = wr_factory::create_weak_rule(wr_type,this->get_nlabels());
      wr->read_from_stream(in);
      this->push_back(wr);

      nrules++;
      if (not in->eof()) {
        (*in) >> token;
      }
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Input/Output operations to dump/read models to/from a file
  ///////////////////////////////////////////////////////////////

  void adaboost::read_from_file(const std::wstring &f) {

    wifstream abm;
    util::open_utf8_file(abm,f);
    if (abm.fail()) ERROR_CRASH(L"Error opening file "+f);

    read_from_stream((wistream*)&abm);
    abm.close();
  }

} // namespace
