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

// ####  Test a svm model using an encoded  corpus
// ####
// ####  Input (stdin) is encoded training, one example per line:
// ####     class feat1 feat2 feat3...
// ####  Output (stdout) is the answer classes, one per line

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>

#include "freeling.h"

using namespace freeling;

// ----------------------------------------------------

int main(int argc, char* argv[]) {

  util::init_locale();

  if (argc!=2) {
    wcerr<<L"Usage:  test-svm necFile.dat <encoded_text"<<endl;
    exit(1);
  }

  wstring nerFile = util::string2wstring(argv[1]);
  wstring path=nerFile.substr(0,nerFile.find_last_of(L"/\\")+1);
      
  // read configuration file and store information   
  wifstream file;
  util::open_utf8_file(file,nerFile);
  if (file.fail()) { wcerr<<L"Error opening file "<<nerFile<<endl; exit(1);}
  
  int reading=0;
  wstring line;
  wstring lexFile,modelFile;

  wstring classnames;

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
      // Reading SVM model file name
      sin>>modelFile;
      modelFile= util::absolute(modelFile,path); 
    }
    else if (reading == 3) 
      // class names e.g "0 B 1 I 2 O"
      classnames=line;
  }   

  // load feature lexicon
  fex_lexicon lex(lexFile);

  // create classifier
  svm classifier(modelFile,classnames);
  int nlabels=classifier.get_nlabels();  // num. of classes
  //  wstring defclas=classifier->default_class();
  //bool use_default= (not defclas.empty());
  wstring defclas=L"???";
  bool use_default=false;

  wstring text;
  while (getline(wcin,text)) {

    if (text != L"") { // got a word line
      wistringstream sin; 
      sin.str(text);
      
      wstring BIOtag;
      sin>>BIOtag;  // first field is the B-I-O tag, ignore (evaluation is outside)
      
      // remanining fields are feature names. Convert to codes and add to example.
      wstring feat;
      example exm(nlabels);
      while (sin>>feat){
	int x=lex.get_code(feat);
        if (x>0) exm.add_feature(x);
      }

      // classify example
      double *p = new double[nlabels];
      classifier.classify(exm, p);

      // Select best label
      int max=0;
      for (int i=0; i<nlabels; i++) { 
	if (p[i]>=p[max]) max=i;
      }

      if (use_default and p[max]<=0) wcout<<defclas<<L" ";
      else wcout<<classifier.get_label(max)<<L" ";
      
      for (int i=0; i<nlabels; i++) wcout<<" "<<p[i];      
    }

    wcout<<endl;
  }
  
}


