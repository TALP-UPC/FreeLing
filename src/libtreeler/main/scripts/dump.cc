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
 * \file   script-dump.cc
 * \brief  A script for dumping part-factored structures
 * \author Xavier Carreras
 */

#include "scripts/register.h"
#include "scripts/dump.h"
REGISTER_SCRIPT(dump, control::ScriptDump);

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "treeler/util/options.h"

using namespace std;

namespace treeler {

  namespace control {

    //! 
    //
    void ScriptDump::usage(Options& options) {
      cerr << name() << " : dumps part-factored structures" << endl;
      cerr << endl;
      cerr << name() << " options:" << endl;

      ScriptDump::Configurer::dump_options(name(), cerr, " "); 
      cerr << endl;
      cerr << " --mod=<name>    : model name" << endl;      
      GenericModel::usage(options, "mod", cerr); 
      cerr << endl;
    }
    
    // this is the main non-templated script, instantiates the templated script via a generic model selector
    void ScriptDump::run(Options& options) {
      GenericModel::run(options, "mod", cerr);
    }
    
    
    void ScriptDump::Configurer::dump_options(const string& name, ostream& log, const string& prefix) {

      log << prefix << "--dir=<path>    :  model directory, defaults to $cwd" << endl;
      log << prefix << "--input=<path>  : path to input data (use \"-\" for std input)" << endl;
      log << prefix << "--parts=[list]  : dump parts" << endl;
      log << prefix << "                   y   : gold parts" << endl;
      log << prefix << "                   all : all parts" << endl; 
      log << prefix << "                   max : parts in max solution" << endl;
      //    log << prefix << "                   cin: parts read from standard input" << endl;
      log << prefix << "--fidx=[type]   : type of feature indices (experimental)" << endl;
      log << prefix << "--fvec=[1|p]    : dump part feature vectors (one-liner, pretty)" << endl;
      log << prefix << "--score         : dump part scores from a model" << endl;
      log << prefix << "--oracle        : dump part scores from an oracle" << endl;
      //    log << prefix << "--marg          : dump part marginals" << endl;
      log << prefix << "--max           : dump max solution" << endl;	  
    }


    void ScriptDump::Configurer::configure(Configuration& config, Options& options, const string& name, ostream& log) {
      // general display 
      log << name << " : configuring parameters of the script" << endl;
      config.display_x = 0; 
      config.display_y = 0; 
      config.display_xy = 1; 
      options.get("x", config.display_x); 
      options.get("y", config.display_y); 
      options.get("xy", config.display_xy); 
      
      // parts
      config.run_parts_y = 0;
      config.run_parts_all = 0;
      config.run_parts_max = 0;
      config.run_parts_cin = 0;
      string tmp;
      if (options.get("parts", tmp)) {
	if (tmp=="1" or tmp=="y") {
	  config.run_parts_y = 1;
	}
	else if (tmp=="all") {
	  config.run_parts_all = 1; 
	}
	else if (0 and tmp=="max") {
	  config.run_parts_max = 1; 
	}
	else {
	  log << name << " : bad parts spec! (--parts=\"" << tmp <<"\")" << endl;
	  exit(1);	
	}
      }
      else {
	log << name << " : no parts spec!!!" << endl; 
      }
      
      config.display_fvec = 0; 
      tmp = ""; 
      if (options.get("fvec", tmp)) {
	if (tmp == "0") {
	  
	}
	else if (tmp == "" or tmp == "1") {
	  config.display_fvec = 1; 
	  config.fvec_pretty = 0; 
	}
	else if (tmp == "p") {
	  config.display_fvec = 1; 
	  config.fvec_pretty = 1; 
	}
	else {
	  log << name << " : unknown option for --fvec=\"" << tmp <<"\"" << endl;
	  exit(1);	
	}
      }
      config.display_score = 0;
      
      options.get("score", config.display_score); 
      config.display_oracle = 0;
      options.get("oracle", config.display_oracle); 
      config.display_marginal = 0;
      options.get("marg", config.display_marginal); 
      
      config.run_max = 0; 
      options.get("max", config.run_max); 
      config.display_max = config.run_max!=0;	  
      
      config.ifile = ""; 
      options.get("input", config.ifile); 
      
      config.fidx_type = "";
      options.get("fidx", config.fidx_type);       
    };


    
    ///////////////////
    ////// 
    /// EXPERIMENTAL, NOT USED 

    /** 
     * \brief This template is experimental and under construction
     *
     * Selects inference via command-line and runs the \c Script::run() with arguments \c Args. 
     * The option name is fixed to \c "mod"
     * 
     * \param args A pack of parameters to pass to the method \c Script::run()
     *
     */

/*  COMMENTED OUT TO AVOID ERRORS WITH g++ 4.8

    template <typename Script, typename ...IChoices> 
    struct ISelector {      
    private:      
      template <typename ...emptylist>
      struct tmp {	
      };
    public:      
      static string name() { return Script::name()+"ISelector"; }
      template <typename X, typename Y, typename R, typename I, typename S, typename F, typename IO>  
      static void run(Options& options, const string& model_name, bool help) {
	if (help) {
	  cout << "ISelector : listing inference methods here ... " << endl; 
	  Script::template run<X,Y,R,I,S,F,IO>(options, model_name, help); 
	}
	else {
	  string choice; 
	  if (!options.get("inf", choice)) {
	    cerr << "please name a type of inference" << endl; 	  
	    exit(0);
	  }
	  cout << "ISelector : option \"" << choice << "\" has been selected" << endl; 
	  Script::template run<X,Y,R,I,S,F,IO>(options, model_name, help); 
	  // tmp<IChoices...>::select(choice, help);
	}
      }      
    };
*/
       
  }
}
