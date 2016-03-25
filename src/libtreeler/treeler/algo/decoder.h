/*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2011   TALP Research Center
 *                       Universitat Politecnica de Catalunya
 *
 *  This file is part of Treeler.
 *
 *  Treeler is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Treeler is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Treeler.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  contact: Xavier Carreras (carreras@lsi.upc.edu)
 *           TALP Research Center
 *           Universitat Politecnica de Catalunya
 *           08034 Barcelona
 *
 ********************************************************************/

/** 
 * \file   decoder.h
 * \brief  Declaration of generic methods for decoding
 * \author Xavier Carreras, Terry Koo
 */
#ifndef TREELER_DECODER_H
#define TREELER_DECODER_H

#include "treeler/base/parameters.h"
#include "treeler/base/example.h"
#include "treeler/util/timer.h"
#include <vector>
#include <string>


namespace treeler {

  /**
   * \brief A generic decoder, i.e. goes through each example of a
   *  dataset and performs inference
   *  \author Xavier Carreras, Terry Koo
   *  \ingroup base
   *
   *  The decoding algorithm takes a prediction model and a
   *  dataset. For each example (x,y) in the dataset, it predicts the
   *  best value y', and outputs the triple (x,y,y'). The decoding
   *  algorithm accepts policies for outputing the resulting triples. Available policies are:
   *  - Evaluate: compute evaluation metrics
   *  - Save: create a new dataset of (x,y') pairs
   *  - Output: (x,y,y') through an output stream (like a file, or cout)
   *  - Combo: combine these policies into one
   *
   *
   *  \todo Document available result policies after decoding, and how to
   *  create new policies or extend existing ones.
   *       
   */
  class Decoder {     
  public:

    /** 
     * \brief Performs decoding on an number of input patterns
     * 
     * \tparam X The type of input patterns
     * \tparam Y The type of output patterns
     * \tparam I The class providing inference algorithms
     * \tparam S The class calculating scores for parts
     * \tparam Iterator An interator type of examples <tt><X,Y></tt>
     * \tparam F A functor to process decoded structures, providing an <tt>operator(const X& x, const Y& ystar, const Y& yhat)</tt>
     * 
     * \param IConfig Configuration of the inference
     * \param scorer  A scorer calculating scores for input patterns 
     * \param first   An iterator pointing to the first pattern to be decoded
     * \param functor A functor to process decoded structures
     */ 
    template <typename X, typename Y, typename I, typename S, typename Iterator, typename F>
    static 
    void decode(I& parser,
		S& scorer,
		Iterator first, 
		Iterator last,
		F& functor);    
    
    
    //////////////////////////////////
    // DECODING FUNCTORS
    
    /**
     * \brief A dummy functor (does not do anything)
     * 
     * \tparam X The type of input patterns 
     * \tparam Y The type of output patterns
     *
     */
    template <typename X, typename Y>
    class functor_dummy {
    public:
      functor_dummy(ostream& o, const string& s) : _o(o), _s(s) {}
      ~functor_dummy() {}
      void operator()(const X& x, const Y& y, const Y& yhat) {
	_o << _s;
      }
    private:
      ostream& _o;
      string _s;
    };

    /**
     * \brief Functor that writes a decoded structure to an output stream
     * 
     * \tparam X The type of input patterns 
     * \tparam R The type of parts
     * \tparam R Traits providing a class to write structures to an output stream
     *
     */
    template <typename X, typename Y, typename IO>
    class functor_ostream {
    public:
      functor_ostream(IO& io, ostream& o) : _io(io), _display_x(true), _display_ygold(false), _o(o), _fo(NULL) {}
      functor_ostream(IO& io, const string& filename) : _io(io), _display_x(true), _display_ygold(false), _o(cerr) {
	_fo.open(filename.c_str());
	if (!_fo.good()) {
	  cerr << "Decoder::functor_ostream : error opening file " << filename << endl;
	  exit(1);
	}
	//_o = _fo;
      }
      ~functor_ostream() {
	if (_fo) {
	  _fo.close();
	}
      }
      void operator()(const X& x, const Y& y, const Y& yhat) {
	ostream& tmp = _fo ? _fo : _o; 
	if (_display_x and _display_ygold) {
	  _io.write(tmp, x, y, yhat);
	}
	else if (_display_x) {
	  _io.write(tmp, x, yhat);
	}
	else if (_display_ygold) {
	  _io.write(tmp, y, yhat);
	}
	else {
	  _io.write(tmp, yhat);
	}
      }

      void display_x(bool b) { _display_x = b; }
      void display_ygold(bool b) { _display_ygold = b; }
      
    private:
      IO& _io;
      bool _display_x;
      bool _display_ygold;
      ostream& _o;
      ofstream _fo;
    };

    /**
     * \brief Functor that evaluates solutions
     * 
     * \tparam X The type of input patterns 
     * \tparam Y The type of parts
     * \tparam Eval An evaluation functor
     *
     */
    template <typename X, typename Y, typename Eval>
    class functor_eval : public Eval {
    public:
      void operator()(const X& x, const Y& y, const Y& yhat) {
	Eval::operator()(x,y,yhat); 
      }
    };

    /**
     * \brief Functor pair : a pair of functors, called one after the other
     * 
     * \tparam F1 The type of the first functor
     * \tparam F2 The type of the second functor
     */
    template <typename X, typename Y, typename F1, typename F2>
    class functor_pair {
    public:

      functor_pair(F1& f1, F2& f2) : _f1(f1), _f2(f2) {}

      void operator()(const X& x, const Y& y, const Y& yhat) {
	_f1(x,y,yhat);
	_f2(x,y,yhat);
      }

    private:
      F1& _f1;
      F2& _f2;
    }; 
    

    
  };

  
  template <typename X, typename Y, typename I, typename S, typename Iterator, typename F>
  void Decoder::decode(I& parser, 
		       S& scorer,
		       Iterator first, 
		       Iterator last,
		       F& functor) {    
    timer_start();    
    int cnt = 0;
    for(; first != last; ++first) {
      const Example<X,Y>& ex = *first;
      
      typename S::Scores scores;
      scorer.scores(ex, scores);      

      Y yhat;      
      parser.argmax(ex.x(), scores, yhat); 
      functor(ex.x(), ex.y(), yhat);
      
      ++cnt;
    }
    timer_stop();
  }

}

#endif /* TREELER_DECODER_H */
