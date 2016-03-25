 /*********************************************************************
 *
 *  Treeler - Open-source Structured Prediction for NLP
 *
 *  Copyright (C) 2013   TALP Research Center
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
 * \file   forall.h
 * \brief  A generic script implementing forall schemes
 * \author Xavier Carreras
 */

#ifndef TREELER_CONTROL_FORALL
#define TREELER_CONTROL_FORALL

#include <string>

#include "treeler/control/control.h"

namespace treeler {

  namespace control {


    ////////////////////////////////////////////////////////////////////////////
    //  F O R   A L L
    //////////////////////////////////////


    /* 
     * \brief  A generic routine that initializes components of a model, and runs the functor on all sentences
     */
    template <typename Functor, typename Model>  
    void forallM(Functor& f, Options& options) {
      cerr << "Running f.name() " << " with ..." << endl;
      cerr << " X=" << typeid(Model::X).name() << endl
	   << " Y=" << typeid(Model::Y).name() << endl
	   << " R=" << typeid(Model::R).name() << endl
	   << " I=" << typeid(Model::I).name() << endl
	   << " S=" << typeid(Model::S).name() << endl
	   << " F=" << typeid(Model::F).name() << endl
	   << " IO=" << typeid(Model::IO).name() << endl << endl;
      
      typename Model::R::Configuration Rconfig; 
      typename Factory<typename Model::R>::configure(Rconfig, options, cerr);      
      // input output options

      typename Model::Symbols symbols; 
      Factory<typename Model::Symbols>::configure(symbols, options, cerr); 

      typename Model::IO io(symbols);
      Factory<typename Model::IO>::configure(io, options, cerr);
      
      typename Model::I parser(symbols);
      Factory<typename Model::I>::configure(parser, options, cerr);
      
      typename Model::S scorer(symbols);
      Factory<typename Model::S>::configure(symbols, scorer, options, cerr);
      
      DataStream<typename Model::X, typename Model::Y,  typename Model::IO> ds(io, std::cin);
      string input_file = ""; 
      options.get("data", input_file); 
      bool use_cin = input_file.empty() or input_file=="-";   
      if (!use_cin) {
	ds.set_file(input_file);
      }
      
      f.start(symbols, Rconfig, parser, scorer, io); 

      /* visit examples and process them */
      typename DataStream<typename Model::X,typename Model::Y,typename Model::IO>::const_iterator it = ds.begin(), it_end = ds.end();
      for (; it!=it_end; ++it) {
	const typename Model::X& x  = (*it).x();
	const typename Model::Y& y  = (*it).y();
	f(symbols, Rconfig, parser, scorer, io, x,y);
      } // for each example          

      f.end(symbols, Rconfig, parser, scorer, io); 

    } // generic routine


    template <template <typename> class Functor>
    struct ForAllMScript {
    public:
      template <typename Model>
      void run(Options& options, const string& model_name) {
	typedef Functor<Model> FF; 
	FF f(options, model_name); 
	forallM<FF,Model>(f,options); 	
      }       
    };


    /* 
     * \brief  A generic routine that initializes components of a model, and runs the functor on all sentences
     */
    template <typename Functor, typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
    void forall(Functor& f, Options& options) {
      cerr << "Running f.name() " << " with ..." << endl;
      cerr << " X=" << typeid(X).name() << endl
	   << " Y=" << typeid(Y).name() << endl
	   << " R=" << typeid(R).name() << endl
	   << " I=" << typeid(I).name() << endl
	   << " S=" << typeid(S).name() << endl
	   << " F=" << typeid(F).name() << endl
	   << " IO=" << typeid(IO).name() << endl << endl;
      
      typename R::Configuration Rconfig; 
      Factory<R>::configure(Rconfig, options, cerr);      
      // input output options

      Symbols symbols; 
      Factory<Symbols>::configure(symbols, options, cerr); 

      IO io(symbols);
      Factory<IO>::configure(io, options, cerr);
      
      I parser(symbols);
      Factory<I>::configure(parser, options, cerr);
      
      S scorer(symbols);
      Factory<S>::configure(symbols, scorer, options, cerr);
      
      DataStream<X,Y,IO> ds(io, std::cin);
      string input_file = ""; 
      options.get("data", input_file); 
      bool use_cin = input_file.empty() or input_file=="-";   
      if (!use_cin) {
	ds.set_file(input_file);
      }
      
      f.start(symbols, Rconfig, parser, scorer, io); 

      /* visit examples and process them */
      typename DataStream<X,Y,IO>::const_iterator it = ds.begin(), it_end = ds.end();
      for (; it!=it_end; ++it) {
	const X& x  = (*it).x();
	const Y& y  = (*it).y();
	f(symbols, Rconfig, parser, scorer, io, x,y);
      } // for each example          

      f.end(symbols, Rconfig, parser, scorer, io); 

    } // generic routine

    /* 
     * \brief Instantiates a forall functor and runs the forall script
     */
    template <template <typename, typename, typename, typename, typename, typename, typename, typename> class Functor>
    struct ForAllScript {
    public:
      template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>
      void run(Options& options, const string& model_name) {
	typedef Functor<Symbols,X,Y,R,I,S,F,IO> FF; 
	FF f(options, model_name); 
	forall<FF,Symbols,X,Y,R,I,S,F,IO>(f,options); 	
      }       
    };

  }
}

#endif
