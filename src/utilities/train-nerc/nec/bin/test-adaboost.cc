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

// ####  Test a abm model using an encoded  corpus
// ####
// ####  argv[1] - lexicon to filter features
// ####  argv[2] - abm model
// ####
// ####  Input (stdin) is encoded training, one example per line:
// ####     class feat1 feat2 feat3...
// ####  Output (stdout) is the answer classes, one per line

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>

#include "freeling.h"
#include "freeling/morfo/traces.h"

using namespace freeling;

adaboost *classif;

// ----------------------------------------------------

int main(int argc, char* argv[]) {

  util::init_locale();

  if (argc!=3) {
    wcerr<<L"Usage:  test nerFile.dat step <encoded_text"<<endl;
    exit(1);
  }

  //traces::TraceLevel=4;
  //traces::TraceModule=OMLET_TRACE;

  wstring nerFile = util::string2wstring(argv[1]);
  int step = util::wstring2int(util::string2wstring(argv[2]));
  wstring path=nerFile.substr(0,nerFile.find_last_of(L"/\\")+1);
  wstring classnames;
    
  // read configuration file and store information   
  wifstream file;
  util::open_utf8_file(file,nerFile);
  if (file.fail()) { wcerr<<L"Error opening file "<<nerFile<<endl; exit(1);}
  
  int reading=0;
  wstring line;
  wstring lexFile,abmFile;
  while (getline(file,line)) {
    wistringstream sin;
    sin.str(line);
    if (line == L"<Lexicon>") reading=1;
    else if (line == L"</Lexicon>") reading=0;
    else if (line == L"<ModelFile>") reading=2;
    else if (line == L"</ModelFile>") reading=0;
    else if (line == L"<Classes>") reading=3;
    else if (line == L"</Classes>") reading=0;
    else if (reading == 1) {
      // Reading lexicon file name
      sin>>lexFile;
      lexFile= util::absolute(lexFile,path); 
    }
    else if (reading == 2) {
      // Reading AdaBoost model file name
      sin>>abmFile;
      abmFile= util::absolute(abmFile,path); 
    }
    else if (reading == 3) {
      // reading class names and codes e.g. "0 B 1 I 2 O"
      classnames=line;
    }
  }

  // create AdaBoost classifier
  classif = new adaboost(abmFile,classnames);      

  // load feature lexicon
  fex_lexicon lex(lexFile);
  
  int nlabels=classif->get_nlabels();
  wstring defclas=classif->default_class();
  bool use_default= (not defclas.empty());
  
  // create one empty vector for each step.
  vector<int > preds;
  for (int r=0; r<classif->n_rules(); r+=step) {
    preds.push_back(0);
  }
  
  wstring text;
  while (getline(wcin,text)) {
    
    // each line contains an encoded example.
    wistringstream sin; 
    sin.str(text);
      
    // build an example 
    example exmp(nlabels);
      
    wstring BIOtag;
    sin>>BIOtag;  // first field is the right tag, ignore (evaluation is outside)
      
    // remanining fields are feature names. Convert to codes and add to example.
    wstring feat;
    while (sin>>feat){
      int x = lex.get_code(feat);
      if (x>0)
	exmp.add_feature(x);
    }
    
    double *p = new double[nlabels];
    for (int i=0; i<nlabels; i++) p[i]=0;
    classif->pcl_ini_pointer();

    // classify word, keeping results each 'step' rules.
    for (int r=0; r<classif->n_rules()/step; r++) {
      classif->pcl_classify(exmp, p, step);
      classif->pcl_advance_pointer(step);
      
      // Remember selection at this step
      int max=0;
      for (int i=0; i<nlabels; i++) if (p[i]>=p[max]) max=i;

      if (use_default and p[max]<=0) wcout<<defclas<<L" ";
      else wcout<<classif->get_label(max)<<L" ";
    }

    wcout<<endl;
    delete [] p;
  }  

  delete(classif);
}


