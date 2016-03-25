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
 * \file   script-dep.h
 * \brief  A script for dependency parsing
 * \author Xavier Carreras
 */

#ifndef TREELER_SCRIPT_DEP
#define TREELER_SCRIPT_DEP


#include "scripts/script.h"
#include "treeler/control/control.h"
#include <string>

namespace treeler {

  namespace control {
    
    /** 
     *  \brief A script for dependency parsing
     *  \ingroup control
     */  
    class ScriptDep: public Script {
    private:
      //      typedef ModelSelector<ScriptDep, DEP1, DEP2> DepModel; 
      typedef ModelSelector<ScriptDep, DEP1> DepModel; 
      
    public:
      ScriptDep()
	: Script("dep")
      {} 
      ~ScriptDep()
      {} 
            
      /** 
       *  \brief The name of the script
       */ 
      static std::string name() { return "dep"; }
            
      /** 
       *  \brief Usage information; the actual type of model is decided via command-line options
       */       
      void usage(Options& options);

      /** 
       *  \brief Runs the script, the actual model and the configuration is set from command-line options
       */       
      void run(Options& options);


      /** 
       *  \brief Generic version of the usage
       */       
      template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
      static void usage(Options& options, const string& model_name, ostream& log, const string& prefix);
      
      template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
      static void run(Options& options, const string& model_name);

      template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
      static void parse(Options& options, const string& model_name);
            
      template <typename Symbols, typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
      static void print_all_derivations(Options& options, const string& model_name);

    };
  }
}


#endif
