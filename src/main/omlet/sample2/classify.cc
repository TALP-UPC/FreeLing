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
//  usage of Omlet library to classify a set of input examples
//  using a previously acquired adaboost model.
//
//  The adaboost model must be previously learnt as shown in the
//  train.cc program
//
//------------------------------------------------------------------//

#include <fstream>
#include <sstream>

#include "omlet.h"

// include the definition of the Weak Rule we want to use. In this
// case, we will use our own WR code, not the WR predefined in the
// library.  See "dummy_rule.h" for details on how to define and
// register your onw WR type.
// Notice that this is the only line needed to have the classifier use
// a different wear rule, since the name of the actually used wr is
// specified in the adaboost model file.
#include "dummy_rule.h"


int main(int argc, char* argv[]) {
  
  // create AdaBoost classifier loading given model file
  adaboost *classifier = new adaboost(string(argv[1]));
  
  // allocate prediction array (reused for all examples)
  double pred[classifier->get_nlabels()];

  int n=0;
  int nok=0;
  string line;
  // read std input examples (one per line) and classify them
  while (std::getline(std::cin,line)) {

      int clas, feat;
      istringstream sin; sin.str(line);
      // first field in line is the class for the example (only useful
      // here to compute classifier performace)
      sin>>clas;
      // create new example.
      example ex(classifier->get_nlabels());
      // following fields are feature codes 
      while (sin>>feat) ex.add_feature(feat);
  
      // classify current example
      classifier->classify(ex,pred);

      // find out which class has highest weight (alternatively, we
      // could select all classes with positive weight, and propose
      // more than one class per example)
      double max=pred[0]; 
      string tag=classifier->get_label(0);
      for (int j=1; j<classifier->get_nlabels(); j++) {
	if (pred[j]>max) {
	  max=pred[j];
	  tag=classifier->get_label(j);
	}
      } 
      
      // if no label has a positive prediction and <others> is defined, select <others> label.
      string def = classifier->default_class();
      if (max<0 && def!="") tag = def;
      
      // output the result
      cout<<"Example #"<<++n<<".  Rigth class: "<<classifier->get_label(clas)<<".  Proposed class: "<<tag<<endl;
      if (classifier->get_label(clas)==tag) nok++;
  }

  // Print out the statistics. 
  cout<<"--------------------------"<<endl;
  cout<<"Classifier accuracy: "<<100*(double)nok/(double)n<<"%"<<endl;

  // NOTE that the statistics could be much more sophisticated. We
  // could be labelling each example with all classes with a positive
  // prediction, and then compute precision, recall and F1. Be could
  // be using a <others> label for those examples with no positive
  // predicion for any class. If we are interested on Named Entity
  // recognition (which is the case for the provided data) we will be
  // more interested in the ratio of correctly detected NEs rather
  // than in the individual word B-I-O classification, or we may want
  // to use obtained predictions as probabilites in a sequence search
  // algorithm (Viterbi, or the like...)


  // Nevertheless, this is a sample usage file to show how the library
  // works, not a general classification program, so, you can adapt it
  // to your needs, and share it if you feel like to.
}


