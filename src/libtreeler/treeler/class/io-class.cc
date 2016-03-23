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
#include "treeler/class/io-class.h"
#include "treeler/io/io-basic.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <list>

using namespace std;

namespace treeler {

  void IOClass::process_options() {

  }


  void IOClass::read(const std::string& file, DataSet<ClassPattern,Label<PartClass>>& ds) {
    if (file == "-") {
      read_dataset(std::cin, ds);
    }
    else {
      ifstream iii(file.c_str());
      if (iii) {
	read_dataset(iii, ds);
      }
      else {
	cerr << "IOClass:: error opening file " << file << endl;
	exit(-1);
      }
    }
  }

  void IOClass::read(std::istream& iii, DataSet<ClassPattern,Label<PartClass>>& ds) {    
    string line;
    int id = 0; 
    ClassPattern *x; 
    Label<PartClass> *y;
    while (read(iii, x, y)) {
      x->set_id(id++);
      ds.emplace_back(new Example<ClassPattern,Label<PartClass>>(x, y)); 
    }    
  }    


  bool IOClass::read(std::istream& iii, ClassPattern*& x, Label<PartClass>*& y) {
    string line;
    if (getline(iii, line)) {
      istringstream iss(line); 
      int label; 
      iss >> label;
      // parse features (idx-value pairs)
      list<FeatureIdx> fidx;
      list<double> values;
      string fstr;
      while (iss >> fstr) {
	istringstream fss(fstr); 
	int idx; 
	char c; 
	double value; 
	if (!(fss >> idx >> c >> value) or c!=':') {
	  cerr << "IOClass::read_dataset() : bad format in " << fstr << endl; 
	  exit(-1);
	}
	fidx.push_back(feature_idx_catW(feature_idx_zero, idx));
	values.push_back(value);
      } 
      // create fvec structure 
      int n = fidx.size();
      FeatureIdx* didx0 = new FeatureIdx[n];
      double* dval0 = new double[n];
      list<FeatureIdx>::const_iterator iidx = fidx.begin(), eidx = fidx.end(); 
      list<double>::const_iterator ival = values.begin();
      FeatureIdx* didx = didx0;
      double* dval = dval0;
      while (iidx!=eidx) {
	*didx = *iidx;
	*dval = *ival;
	++iidx; ++ival; ++didx; ++dval;
      }

      x = new ClassPattern;
      x->n = n;
      x->idx = didx0;
      x->val = dval0;
      x->offset = 0; 
      x->next = NULL;

      Label<PartClass>* y = new Label<PartClass>; 
      y->push_back(PartClass(label));
      return true;
    }
    else {
      return false; 
    }      
  }
  
  void IOClass::write(std::ostream& o, const ClassPattern& x) {
    o << x;
  }
  
  void IOClass::write(std::ostream& o, const Label<PartClass>& y) {
    o << "{";
    Label<PartClass>::const_iterator r = y.begin(), r_end = y.end(); 
    for (; r!=r_end; ++r) {
      o << " " << r->label(); 
    }
    o << " }"; 
  }

  void IOClass::write(std::ostream& o, const ClassPattern& x, const Label<PartClass>& y) {
    o << x.id() << " "; 
    write(o, y); 
    o << endl;
  }

  void IOClass::write(std::ostream& o, const ClassPattern& x, const Label<PartClass>& y, const Label<PartClass>& yhat) {
    o << x.id() << " "; 
    write(o, y); 
    o << " "; 
    write(o, yhat); 
    o << endl;
  }

}
