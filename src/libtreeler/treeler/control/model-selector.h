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
 * \file   model-selector.h
 * \brief  A template function to select a model by type 
 * \author Xavier Carreras
 */

#ifndef TREELER_MODELSELECTOR
#define TREELER_MODELSELECTOR

#include <string>
#include <iostream>
using namespace std;



namespace treeler {

  namespace control {

    /**
     * \brief Selects a model type and runs the \c Script using the
     * component types of the selected model 
     * 
     * \ingroup control 
     * \author Xavier Carreras
     * 
     * \tparam Script The script to run after selecting the types of components
     * \tparam MList A pack of model types 
     * 
     */ 
    template <typename Script, int ...MList>
    struct ModelSelector {
    private:
      
      // this is a recursive struct that represents a number of model choices listed by integer type
      template<int ...emptylist> 
      struct mlist {
	static void list(ostream& o, const string& prefix="") {
	}
	
	template <typename ...Args>
	static void usage(Options& options, const string& choice, ostream& o, Args... args) {
	  o << Script::name() << " : model name \"" << choice << "\" is not available" << endl;
	  exit(0);
	}

	template <typename ...Args>
	static void run(Options& options, const string& choice, ostream& o, Args... args) {
	  o << Script::name() << " : model name \"" << choice << "\" is not available" << endl;
	  exit(0);
	}

      };

      // specialization to get the head of the list \c T
      template<int T, int ...Rest> 
      struct mlist<T,Rest...> {    

	// list model T by its name, and iterate
	static void list(ostream& o, const string& prefix="") {
	  o << prefix << "  " << Model<T>::name() << " : " << Model<T>::brief() << endl;
	  mlist<Rest...>::list(o, prefix); 
	}
	
	template <typename ...Args>
	static void usage(Options& options, const string& choice, ostream& o, Args... args) {
	  if (choice==Model<T>::name()) {
	    Runner<T>::usage(options, o, args...);
	  }
	  else {
	    mlist<Rest...>::usage(options, choice, o, args...);
 	  }
	}	

	template <typename ...Args>
	static void run(Options& options, const string& choice, ostream& o, Args... args) {
	  if (choice==Model<T>::name()) {
	    Runner<T>::run(options, o, args...);
	  }
	  else {
	    mlist<Rest...>::run(options, choice, o, args...);
 	  }
	}	

      };


      // this struct maps a model type to its component types, and runs the \c Script with necessary \c Args
      template <int T>
      struct Runner {

	template <typename ...Args>
	inline static void usage(Options& options, ostream& o, Args... args) {	  
	  Script::template usage<Model<T>>(options, o, args...);
	}
	
        template <typename ...Args>
	inline static void run(Options& options, ostream& o, Args... args) {	  
	  Script::template run<Model<T>>(options, o, args...); 
	}
	

      };
	   
    public:

      /** 
       * \brief Outputs the list of available models through output stream \o
       * 
       * \param opt The name of the option specifying the model name
       * \param o the output stream
       * \param prefix a string written to the beginning of each line (empty string by default)
       *
       */
      static void usage(Options& options, const string& option_name, ostream& o, const string& prefix = "") {
	string model_name;
	if (!options.get(option_name, model_name)) {
	  o << prefix << "Models available:" << endl; 
	  mlist<MList...>::list(o, prefix); 
	  o << prefix << endl; 
	}
	else {
	  mlist<MList...>::usage(options, model_name, o, prefix);
	}
      }
      
      /** 
       * \brief Selects a model via command-line and runs the \c Script::run() with arguments \c Args
       * 
       * \param opt The name of the option specifying the model name
       * \param args A pack of parameters to pass to the method \c Script::run()
       *
       */
      template <typename ...Args>
      static void run(Options& options, const string& option_name, ostream& o, Args... args) {
	string model_name; 
	if (!options.get(option_name, model_name)) {
	  o << Script::name() << " : please name a model type" << endl; 	  
	  exit(0);
	}
	mlist<MList...>::run(options, model_name, o, args...); 
      }

    };

  }

}

#endif
