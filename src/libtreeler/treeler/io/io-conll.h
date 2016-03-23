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
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   io-conll.h
 * \brief  Declaration of class IOCoNLL
 * \author Xavier Carreras
 */
#ifndef TREELER_IOCONLL_H
#define TREELER_IOCONLL_H


#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "treeler/base/dataset.h"
#include "treeler/io/conllstream.h"

using namespace std;

namespace treeler {

  /** 
   *  \brief An IO class based on CoNLLStreams
   */
  template<typename Symbols, typename Format=CoNLLFormat, typename CharType=char>
  class IOCoNLL {
  public:
    
    typedef CoNLLBasicStream<Symbols,Format,CharType> MyCoNLLStream;
    typedef CoNLLBasicStream<SimpleMapping,Format,CharType> CoNLLStreamNoMap;
    
    struct Configuration {
      bool ids;
      int offset; 
      bool add_padding; 
      bool map_input, map_output; 
      Format format;
      // CoNLLFormat oformat;
      bool debug;

      Configuration() 
	: ids(true), offset(1), add_padding(true), map_input(true),  map_output(true), debug(false)
      {}
    };

    Symbols& symbols;
    Configuration config; 

    IOCoNLL(Symbols& sym)
      : symbols(sym)
    {}

    template <typename X, typename Y>
    bool read(istream& in, X*& new_x, Y*& new_y) const {
      MyCoNLLStream c(symbols,config.format);
      if (in >> c) {
	if (config.debug) {
	  c.prefix = "IOCONLL_READ";
	  c.add_padding = true;
	  cerr << c;
	}
	vector<int> ids;
	X* x = new X; 
	Y* y = new Y; 
	bool good = true; 
	if (config.ids) {
	  good = (c >> ids >> *x >> *y);
	}
	else {
	  good = (c >> *x >> *y);
	}
	if (good) {
	  new_x = x; 
	  new_y = y; 
	  return true;
	}
	delete x; 
	delete y; 
      }
      return false; 
    }

    template <typename ...Args>
    void write(ostream& o, Args&&... args) {
      if (config.map_output) {
	MyCoNLLStream s(symbols,config.format);
	if (config.add_padding) {
	  s.add_padding = true;
	  s.offset = config.offset;
	}
	write_impl(s, args...);
	o << s;
      }
      else {
	SimpleMapping mapping;
	CoNLLStreamNoMap s(mapping,config.format);
	if (config.add_padding) {
	  s.add_padding = true;
	  s.offset = config.offset;
	}
	write_impl(s, args...);
	o << s;
      }
    }
    

    template <typename CStream, typename A1, typename ...Args>
    void write_impl(CStream& s, A1& a1, Args&&... args) {
      // cerr << ">> ioconll : writing " << typeid(A1).name() << endl; 
      s << a1; 
      write_impl(s, args...);
    }

    template <typename CStream, typename ...Args>
    void write_impl(CStream& s, Args&&... args) {
    }
     

    template <typename X, typename Y>
    void read(istream& input, DataSet<X,Y>& d, bool quiet) const {
      int id = d.size();
      X* x = NULL;
      Y* y = NULL;
      while(read(input, x, y)) {
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
    }
    
   
    struct OStreamCached {
      ostream& o; 
      IOCoNLL<Symbols>& io;
      OStreamCached(ostream& oo, IOCoNLL<Symbols>& ii) 
	: o(oo), io(ii) {}
    };
    
    OStreamCached operator()(ostream& o) {
      OStreamCached oc(o,*this);
      return oc;
    }


  };

  template <typename Symbols, typename T>
  typename IOCoNLL<Symbols>::OStreamCached& operator<<(typename IOCoNLL<Symbols>::OStreamCached& oc, const T& t) {
    oc.o << t; 
    return oc; 
  }
  
}

namespace std {
  
}


#endif /* TREELER_IOCONLL_H */
