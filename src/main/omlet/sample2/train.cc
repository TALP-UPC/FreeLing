//////////////////////////////////////////////////////////////////
//
//    Omlet - Open Machine Learning Enhanced Toolkit
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This file is part of the Omlet library
//
//    The Omlet library is free software; you can redistribute it 
//    and/or modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110-1301 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx Omega.S112 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


//------------------------------------------------------------------//
//
//  This file contains a sample main program to illustrate 
//  usage of Omlet library to train an Adaboost
//  model based on your own weak rule type.
//
//  The learned model can be used to classify as shown in the
//  sampleclassify.cc program
//
//------------------------------------------------------------------//

// ####  Learn a decision-tree adaboost model using a training corpus
// ####  
// ####  Usage:
// ####     train filename "class-codes-quoted-string"
// ####  
// ####  Where:
// ####     filename  is the name where the learned model will be output 
// ####               with extension .abm  (that is, output to filename.abm)
// ####     "class codes string" is a string enclosed in quotes with the number and descriptions of 
// ####              the classes in the classification problem. It has the format:
// ####                  0 MyClass  1 MyOtherClass  2 MyThirdClass ... n MyLastClass 
// ####              or either:
// ####                  0 MyClass  1 MyOtherClass  2 MyThirdClass ...  <others> MyDefaultClass


#include <iostream>
#include <fstream>
#include <sstream>

#include "omlet.h"


// include the definition of the Weak Rule we want to use. In this
// case, we will use our own WR code, not the WR predefined in the
// library.  See "dummy_rule.h" for details on how to define and
// register your onw WR type.
#include "dummy_rule.h"

int main(int argc, char* argv[]) {
 map<string,int> lexicon;
 map<string,string> codes;
 char snam[512];
 string num,name,wr_type,line;
 int nlab;
 
  // analyze class codes string
  string cod(argv[2]);
  istringstream ps(cod);
  nlab=0;
  while (ps>>num>>name)
    if (num!="<others>") nlab++;    

  // create dataset to store examples;
  dataset ds(nlab);

  // read std input examples (one per line) into train data set
  while (std::getline(std::cin,line)) {

      int clas,feat;
      istringstream sin; sin.str(line);
      // first field in line is the class for the example
      sin>>clas;
      // create new example with that class
      example ex(nlab);
      ex.set_label(clas,true,0,0);

      // following fields are feature codes 
      while (sin>>feat)
        ex.add_feature(feat);
  
      // add complete example to dataset
      ds.add_example(ex);
  }

  // Set weak rule type and parameters to be used.  Note that wr_type
  // must correspond to a registered weak rule type.
  // "mlDTree" (multi-label decision tree) is predefined in libomlet,
  // but as in this example, you can write code for your own WR and
  // register it without recompiling the library.  See "dummy_rule.h"
  // for details on how to define and register your onw WR type.
  wr_type="myDummyRule";

  // Set parameters for WRs, nlab and epsilon are generic for all WRs.
  // Third & fourth parameters are biass & threshold, specific to myDummyRule.  
  myDummyRule_params wp(nlab, 0.001, 0.05, 0.01);
  // create and learn adaboost model
  adaboost learner(nlab, wr_type);
  
  // open model output file
  strcpy(snam, argv[1]);
  ofstream abm(strcat(snam, ".abm"));

  // write class descriptions on first line
  abm<<argv[2]<<endl;
  // write Weak rule type on second line
  abm<<wr_type<<endl;
     
  // learn the model, up to 100 weak rules, incrementally writting it
  // to the file.
  learner.set_output((ostream*)&abm);

  learner.learn(ds, 100, true, (wr_params *)&wp);
  
  abm.close();
}


