//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
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
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


using namespace std;

// ####  Learn a abm model using an encoded training corpus
// ####  argv[2] must be a class coding string in a format like:
// ####         0 MyClass  1 MyOtherClass  2 MyThirdClass  3 MyLastClass  (...)
// ####  or either:
// ####         0 MyClass  1 MyOtherClass  2 MyThirdClass  (...) <others> MyDefaultClass
// ####
// ####  argv[1].lex is read and used as a lexicon to filter features
// ####  Output file to argv[1].abm
// ####
// ####  Input (stdin) is encoded training, one example per line:
// ####     class feat1 feat2 feat3...


#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include "freeling.h"

using namespace freeling;

// ----------------------------------------------------
int main(int argc, char* argv[]) {

  util::init_locale();

  map<wstring,unsigned int> ntrans,ninit,nocc;

  if (argc!=4) {
    wcerr<<L"Usage:  train lexicon.lex output.abm \"codes\" <encoded_text"<<endl;
    exit(1);
  }

  wstring lexFile = util::string2wstring(argv[1]);
  wstring abmFile = util::string2wstring(argv[2]);

  // get and analyze class codes string
  wstring cod=util::string2wstring(argv[3]);
  wistringstream ps(cod);

  int nlabels=0;
  wstring num,name;
  map<wstring,wstring> codes;
  while (ps>>num>>name) {
    if (num!=L"<others>") {
      nlabels++;
      codes.insert(make_pair(name,num));
    }
  }
  
  // create dataset to store examples;
  dataset ds(nlabels);
  
  // load feature lexicon
  fex_lexicon lex(lexFile);

  // load input into train data set, in the form of feature vectors.
  unsigned int ft=0,fu=0;
  unsigned int tinit=1;
  wstring text, BIOtag;

  while (getline(wcin,text)) {
    
    wistringstream sin; 
    sin.str(text);
    sin>>BIOtag;  // first field is the B-I-O tag
    
    
    // build an example 
    example exmp(ds.get_nlabels());
    
    // find out correct B-I-O tag for current word, and set it as the right
    // answer for this example
    map<wstring,wstring>::iterator q=codes.find(BIOtag);
    if (q!=codes.end()) exmp.set_belongs(util::wstring2int(q->second), true);
    
    // remanining fields are feature names. Convert to codes and add to example.
    wstring feat;
    while (sin>>feat){
      int x = lex.get_code(feat);
      if (x>0) {
	exmp.add_feature(x);
	fu++;
      }
      ft++;
    }
    
    // add example to train set
    ds.add_example(exmp);
  }

  wcerr<<L"Corpus loaded. "<<100.0*fu/(double)ft<<"% feature occurrences passed filter."<<endl;

  // output ner.dat file for test.cc 
  wcout<<L"<Lexicon>"<<endl<<L"../"<<lexFile.substr(lexFile.find(L"/")+1)<<endl<<L"</Lexicon>"<<endl;
  wcout<<L"<ModelFile>"<<endl<<abmFile.substr(abmFile.rfind(L"/")+1)<<endl<<L"</ModelFile>"<<endl;
  wcout<<L"<RGF>\nner.rgf\n</RGF>"<<endl;
  wcout<<L"<NE_Tag>\nNP00000\n</NE_Tag>"<<endl;
  wcout<<L"<Classifier>\nAdaBoost\n</Classifier>"<<endl;
  wcout<<L"<Classes>"<<endl<<cod<<endl<<L"</Classes>"<<endl;
  wcout<<L"<UseSoftMax>"<<endl<<"yes"<<endl<<L"</UseSoftMax>"<<endl;

  wcerr<<L"ner.dat file written to stdout"<<endl;

  // Set weak rule type to be used.  Note that the wr_type must
  // correspond to a registered weak rule type.  "mlDTree" is
  // preregistered in libomlet, but you can write code for your own WR
  // and register it without recompiling the library.  Check libomlet
  // documentation for details on how to do this.
  wstring wr_type=L"mlDTree";
  // Set parameters for WRs, nlab and epsilon are generic for all WRs.
  // Third parameter here is max_depth, specific to mlDTree.
  mlDTree_params wp(nlabels, 0.001, 3);
  // create and learn adaboost model
  adaboost learner(nlabels, wr_type);  
  // set learned model output file
  wofstream abm;
  util::open_utf8_file(abm, abmFile);

  wcerr<<L"building model"<<endl;     
  // write Weak rule type on second line
  abm<<wr_type<<endl;

  // learn model     
  learner.set_output((wostream*)&abm);
  learner.learn(ds, 1000, true, (wr_params *)&wp);
  wcerr<<L"model learnt"<<endl;

  abm.close();


}


