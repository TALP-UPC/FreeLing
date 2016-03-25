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
 * \file   script-training.cc
 * \brief  A script for training the parameters of a model, implementation
 * \author Xavier Carreras
 */

#include <typeinfo>

#include "scripts/register.h"
#include "scripts/train.h"
REGISTER_SCRIPT(train, control::ScriptTrain);

#include "treeler/util/options.h"
#include "treeler/base/dataset.h"


using namespace std;
using namespace treeler::control;

namespace treeler {

  namespace control {

    typedef ModelSelector<ScriptTrain, TAG1, DEP1, DEP2> GenericModel;
  
    void ScriptTrain::usage(Options& options) {
      cerr << Script::_name << " : trains the parameters of a model" << endl;
      cerr << Script::_name << " Options:" << endl;
      cerr << " --train=<path>    : training data" << endl;
      cerr << " --val=<path>      : held-out data file, to be classified" << endl;
      cerr << "                      after each iteration" << endl; 
      cerr << " --opt=[algorithm] : learning method " << endl;
      cerr << " --mod=[mod type]  : model type" << endl;
      cerr << " --T=<int>         : number of training iterations" << endl;    
      cerr << " --verbose=<int>   : verbosity level" << endl;
      cerr << endl;
      cerr << "Optimizers available (--opt=<value>):" << endl;
      cerr << " perc : perceptron " << endl;
      cerr << " --averaged=0/1   : use averaged updates" << endl;
      cerr << endl;

      GenericModel::usage(options, "mod", cerr); 
      cerr << endl;
    }
    
    void ScriptTrain::run(Options& options) {
      GenericModel::run(options, "mod", cerr);
    }

  }
 
}

