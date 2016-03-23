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
/// \author Xavier Carreras
/// Basic definitions for implementing standard classification models with the structured prediction framework

#ifndef CLASS_CLASSBASIC_H
#define CLASS_CLASSBASIC_H

#include <iostream>
#include "treeler/base/windll.h"

#include "treeler/base/example.h"
#include "treeler/base/feature-vector.h"

namespace treeler {

  /// A pattern for classification, i.e. a feature vector
  class ClassPattern : public FeatureVector<> {
  private:
    int _id;
  public:
    ClassPattern()
      {
	_id = -1;
	n=0;
	idx=NULL;
	val=NULL;
	offset=0;
	next=NULL;
      }
    
    ~ClassPattern() {
      if (idx!=NULL) {
	delete [] idx;
      }
      if (val!=NULL) {
	delete [] val;
      }
      if (next!=NULL) {
	delete next;
      }
    }

    void set_id(int i) { _id = i; }
    int id() const { return _id; }

  };

  class PartClass {
  private:
    int _l;
  public:
    PartClass() : _l(0) {}
    PartClass(int c) : _l(c) {}
    int label() const { return _l; }
    bool operator==(const PartClass& rhs) const { return _l==rhs._l; }
    bool operator<(const PartClass& rhs) const { return _l<rhs._l; }
    void increment(int L, int i) { _l += i; if (_l>=L) _l=-1; }

    //! An iterator of parts
    class iterator: public std::iterator<std::input_iterator_tag, PartClass> {
      int _L;
      PartClass* _r;
      int _step;
    public:
      iterator(int L) : _L(L), _r(new PartClass(0)), _step(1) {}
      iterator(int L, int l) : _L(L), _r(new PartClass(l)), _step(1) {}
      iterator(int L, int l, int s) : _L(L), _r(new PartClass(l)), _step(s) {}
      ~iterator() { delete _r; }	
      iterator& operator++() { 
	(*_r).increment(_L, _step);
	return *this;
      }
      bool operator==(const iterator& rhs) const {return *_r==*(rhs._r);}
      bool operator!=(const iterator& rhs) const { return not (*this==rhs);}
      PartClass const& operator*() {return *_r;}     
    };

    static iterator begin(int N, int L) { return iterator(L, 0); } 
    static iterator begin(int N, int L, int step) { return iterator(L, 0, step); } 
    static iterator end() { return iterator(0, -1); }

    /* //! An iterator of parts */
    /* class iterator { */
    /* public: */
    /*   iterator(int L) : _L(L), _p(new PartClass(0)) {} */
    /*   iterator() : _L(-1), _p(new PartClass()) {} */
    /*   ~iterator() { delete _p; } */
    /*   const PartClass& operator*() const { return *_p; } */
    /*   bool operator!=(const iterator& other) const { return !(*_p==*other); }       */
    /*   iterator& operator++() { */
    /* 	++_p->_l;  */
    /* 	if (_p->_l==_L) { */
    /* 	  _p->_l=-1; */
    /* 	}	 */
    /* 	return *this; */
    /*   } */
    /* private: */
    /*   int _L; */
    /*   PartClass* _p; */
    /* }; */
    /* static iterator begin(int x, int L) { return iterator(L); } */
    /* static iterator end() { return iterator(); } */
      
  };

/*   class FGenClass : public FeatureGenerator<ClassPattern, PartClass> { */
/*   public: */
/*     void phi_start_pattern(const ClassPattern& x) const {}; */
/*     void phi_end_pattern(const ClassPattern& x) const {}; */
/*     const FeatureVector* phi(const ClassPattern& x, const PartClass& r) const {  */
/*       FeatureVector* f = new FeatureVector(x); */
/*       f->offset = r.label(); */
/*       return f; */
/*     } */
/*     void discard(const FeatureVector* const f, const ClassPattern& x, const PartClass& r) const { */
/*       delete f; */
/*     } */
/*   }; */

  std::ostream& operator<<(std::ostream & o, const ClassPattern& p);
  std::ostream& operator<<(std::ostream & o, const PartClass& p);

  class IOClass;

  template <>
    class ExampleTraits<ClassPattern, PartClass> 
    {
    public:
      typedef IOClass IO;
 
    };

  
}

#endif /* CLASS_CLASSBASIC_H */
