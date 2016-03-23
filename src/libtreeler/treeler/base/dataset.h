/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2014   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with Treeler.  If not, see <http: *www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   dataset.h
 * \brief  Declares the class Dataset
 * \author Xavier Carreras
 */

#ifndef TREELER_DATASET_H
#define TREELER_DATASET_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <treeler/base/example.h>
#include <treeler/base/exception.h>

using namespace std;

namespace treeler {

  /** 
   *  \brief A vector of (x,y) pairs, with the same interface as \c std::vector
   *  \ingroup base
   *  \author Xavier Carreras 
   *
   *  Examples are kept as pointers in the vector to avoid copy costs
   *  when reallocating the container. 

   *  New examples are added to the dataset as pointers, but then they
   *  are kept by the dataset.  When the dataset is deleted, the
   *  examples are deleted too.
   *
   *  \todo Add iterators to loop over examples, using different
   *  randomization schemes used by learning methods (see
   *  learn/learn-utils::randselect(...)
   *
   */
  template <typename X, typename Y>
  class DataSet: public std::vector<const Example<X,Y>*> {
  private:
    typedef std::vector<const Example<X,Y>*> _basetype;
  public:
    /**
     * Deleting a dataset deletes its examples as well
     */
    ~DataSet() {
      auto i = this->_basetype::begin(); 
      auto i_end = this->_basetype::end();       
      for (; i!=i_end; ++i) {
	delete *i;
      }
    }

    // Below, I overload the const_iterator so that *it returns a
    // reference to an example (instead of a reference to a pointer to an example)

    class const_iterator;

    //! \brief Returns an iterator pointing to the first available example in the stream
    const_iterator begin() {
      typename _basetype::const_iterator i = _basetype::begin();
      return const_iterator(i); 
    }
    
    //! \brief Returns an iterator pointing to a special final value
    const_iterator end() {
      typename _basetype::const_iterator i = _basetype::end();
      return const_iterator(i); 
    }
    
    // a subclass for constant iterators
    class const_iterator {
    private:
      typename _basetype::const_iterator _it; 
    public:
      const_iterator(typename _basetype::const_iterator& i) : _it(i) {}
      
      const_iterator& operator++() { 
	_it++;
	return *this;
      }

      bool operator==(const const_iterator& rhs) const {
	return _it==rhs._it; 
      }
      bool operator!=(const const_iterator& rhs) const { return _it != rhs._it;}
      const Example<X,Y>& operator*() {return *(*_it);}
    };


    
  };


  /** 
   *  \brief A stream of (x, y) pairs
   *  \ingroup base
   *  \author Xavier Carreras 
   *
   *  Examples are read from an input stream, which can be a file or a generic input stream.  
   * 
   *  An IO class must be given, which needs to provide the following method
   * 
   *  bool read(istream& i, X*& x, Y*& y)
   * 
   *  If the method succeeds in reading an example, x and y point to
   *  the pair of values. If the method does not succeed it returns
   *  false, and x and y remain undefined.
   * 
   *  Provides an iterator to iterate over examples in the standard
   *  fashion. Each iterator numbers its examples starting at 0. In
   *  case examples are read from a file, each iterator will iterate
   *  over all examples starting from the beginning. In case examples
   *  are read from a stream and multiple iterators are defined
   *  simultaneously, examples will be read according to how the
   *  iterators are called.
   *  
   */
  template <typename X, typename Y, typename IO>
  class DataStream {
  private:
    IO& _io;
    string   _file;
    istream* _is;    
  public:
    //! \brief Default constructor, input source is left unspecified
    DataStream(IO& io) : _io(io), _file(""), _is(NULL) {}
    //! \brief Constructor using a file as a source of input examples
    DataStream(IO& io, const string& file) : _io(io), _file(file), _is(NULL) {}
    //! \brief Constructor using a general input stream as a source of examples
    DataStream(IO& io, istream& is) : _io(io), _file(""), _is(&is) {}

    void set_file(const string& file) { _file=file; _is=NULL; }
    void set_input(istream& is) { _file=""; _is=&is; }
    
    class const_iterator;

    //! \brief Returns an iterator pointing to the first available example in the stream
    const_iterator begin() {
      if (_file!="") {
	ifstream* fs = new ifstream(); 
	fs->open(_file.c_str(), ifstream::in); 
	if (!fs->good()) {
	  // the file does not open correctly, throw an exception
	  delete fs;
	  ostringstream oss; 
	  oss << "error opening data file \"" << _file << "\"";
	  TreelerException e("DataStream", oss.str());
	  throw(e);
	}
	else {
	  return const_iterator(_io, fs);
	}
      }
      else if (_is!=NULL) {
	return const_iterator(_io, _is); 
      }
      else {
	// return directly the end
	return const_iterator(_io);
      }
    }
    
    //! \brief Returns an iterator pointing to a special final value
    const_iterator end() {
      return const_iterator(_io);
    }

    // a subclass for constant iterators
    class const_iterator {
    private:
      IO& _io;
      Example<X,Y>* _e; 
      istream *_is;
      ifstream *_ifs;
      int _n; // example ids
    public:
      const_iterator(IO& io) : _io(io), _e(NULL), _is(NULL), _ifs(NULL), _n(0) {}
      const_iterator(IO& io, istream* is) : _io(io), _e(NULL), _is(is), _ifs(NULL), _n(0) {
	// read the first available example by advancing the iterator
	++(*this);
      }
      const_iterator(IO& io, ifstream* is) : _io(io), _e(NULL), _is(is), _ifs(is), _n(0) {
	// read the first available example by advancing the iterator
	++(*this);
      }
      ~const_iterator() { 
	delete _e; 
	if (_ifs!=NULL) _ifs->close();
      }
      
      const_iterator& operator++() { 
	delete _e; 	
	// advance to the next example
	X* x = NULL; 
	Y* y = NULL;
	if (_io.read(*_is, x, y)) {
	  x->set_id(_n); 
	  ++_n;
	  // _io.write(cout,*x,*y);
	  _e = new Example<X,Y>(x,y);
	}
	else {
	  /* no more examples, set iterator to null */
	  if (_ifs!=NULL) _ifs->close();
	  _ifs = NULL; 
	  _is = NULL; 
	  _e = NULL;
	  _n = 0;
	}
	return *this;
      }

      bool operator==(const const_iterator& rhs) const {
	if (_e!=rhs._e) return false; 
	if (_is!=rhs._is) return false; 
	if (_ifs!=rhs._ifs) return false; 
	if (_n!=rhs._n) return false; 
	return true;
      }
      bool operator!=(const const_iterator& rhs) const { return not (*this==rhs);}
      const Example<X,Y>& operator*() {return *_e;}
    };
  };


  template <typename ...ARGS>
  bool read_dataset(const string& file, ARGS&&... args) {
    istream* in = &std::cin; 
    ifstream fin; 
    if (file != "-") {
      fin.open(file.c_str(), ifstream::in);
      if (!fin.good()) {
	return false; 
      }
      in = &fin;
    }    
    bool b = read_dataset(*in, args...); 
    if (file!="-") {
      fin.close();
    }
    return b; 
  }


  template <typename X, typename Y, typename IO>
  bool read_dataset(istream& input, const IO& io, DataSet<X,Y>& d, bool quiet=true) {
    int id = 0;
    assert(d.size() == 0);    
    X* x = NULL;
    Y* y = NULL;
    while(io.read(input, x, y)) {
      x->set_id(id++); // incrementing id
      d.emplace_back(new Example<X,Y>(x,y)); 
      x = NULL;
      y = NULL;
      if(!quiet and (id & 0x7ff) == 0) {
	cerr << "(" << (double)id/1000.0 << "k)" << flush;
      } else if((id & 0xff) == 0) {
	cerr << "." << flush;
      }
    }    
    return true; 
  }


}


#endif /* TREELER_DATASET_H */


