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
 * \file   script-decode.cc
 * \brief  A script for decoding new inputs using a trained model
 * \author Xavier Carreras, Pranava
 */

#include <typeinfo>

#include "scripts/register.h"
#include "scripts/decode.h"
REGISTER_SCRIPT(decode, ScriptDecode);

#include "treeler/util/options.h"
#include "treeler/base/dataset.h"

using namespace std;

namespace treeler {
  
  using namespace control;
  
  ScriptDecode::ScriptDecode() : Script("decode") {}
  
  ScriptDecode::~ScriptDecode() {}
  
  typedef control::ModelSelector<ScriptDecode, TAG1, DEP1, DEP2> GenericModel; 

  //////////////////
  //
  //
  void ScriptDecode::usage(Options& options) {
    cerr << name() << " : runs inference on input data" << endl;
    cerr << endl;
    cerr << "Decoder options :" << endl;
    cerr << " --mod=<name>      : model name" << endl;
    cerr << " --dir=<path>      : model directory, defaults to $cwd" << endl;
    cerr << " --input=<path>    : path to input data (use \"-\" for std input)" << endl;
    cerr << " --output=<path>   : output path (use \"-\" for std output)" << endl;
    cerr << endl;
    cerr << " --oracle          : use oracle scorer" << endl;
    cerr << " --x               : display input pattern" << endl;
    cerr << " --ygold           : display correct output structure" << endl;
    cerr << " --eval            : evaluate predictions" << endl;
    cerr << endl;
    
    GenericModel::usage(options, "mod", cerr); 
    cerr << endl;
  }

  void ScriptDecode::configure(Options& options, Configuration& config) {
    config.ifile = ""; 
    options.get("input", config.ifile); 
    config.from_cin = config.ifile.empty() or config.ifile=="-";
    config.ofile = ""; 
    options.get("output", config.ofile); 
    config.to_cout = config.ofile.empty();
    config.display_x = true; 
    config.display_ygold = false; 
    options.get("x", config.display_x); 
    options.get("ygold", config.display_ygold); 
    config.verbose = 1;
    options.get("v", config.verbose); 
    options.get("eval", config.eval); 
  }

  
  void ScriptDecode::run(Options& options) {
    GenericModel::run(options, "mod", cerr); 
  }

}

