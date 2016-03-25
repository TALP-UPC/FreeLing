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
#include "freeling/morfo/traces.h"

using namespace std;
using namespace freeling;


void process_sentence(vector<double*> &preds);

vis_viterbi *vit;
svm * classif;

// ----------------------------------------------------

int main(int argc, char* argv[]) {

  util::init_locale();

  if (argc!=2) {
    wcerr<<L"Usage:  test-svm nerFile.dat <encoded_text"<<endl;
    exit(1);
  }

  //traces::TraceLevel=6;
  //traces::TraceModule=OMLET_TRACE;

  wstring nerFile = util::string2wstring(argv[1]);
  wstring path=nerFile.substr(0,nerFile.find_last_of(L"/\\")+1);
  wstring classnames;
      
  // read configuration file and store information   
  wifstream file;
  util::open_utf8_file(file,nerFile);
  if (file.fail()) { wcerr<<L"Error opening file "<<nerFile<<endl; exit(1);}
  
  int reading=0;
  wstring line;
  wstring lexFile,modelFile;

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
    else if (reading == 3) { // class names
      classnames=line;
    }
  }   

  // load feature lexicon
  fex_lexicon lex(lexFile);

  // create classifier
  classif = new svm(modelFile,classnames);
  int nlabels=classif->get_nlabels();  // num. of classes

  // create viterbi solver
  vit = new vis_viterbi(nerFile);

  vector<double*> preds;
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
      classif->classify(exm, p);

      preds.push_back(p);
    }
    else  // blank line, sentence end.
      process_sentence(preds); // do viterbi and print results
  }
  
  if (not preds.empty())   // no blank line after last sentence
    process_sentence(preds); // do viterbi and print results                            
  
  delete vit;
}



// ----------------------------------------------------
void process_sentence(vector<double*> &preds) {
  
  int nw=preds.size();  
  
  // Once all sentence has been encoded, use Viterbi algorithm to 
  // determine which is the most likely class combination
  vector<int> best = vit->find_best_path(preds);
  
  // print best path 
  for (int i=0; i<nw; i++) {
    wcout << classif->get_label(best[i]) << endl;
  }
  // sentence separator
  wcout<<endl;

  // clean vector, ready for next sentence.
  for (int i=0; i<nw; i++) delete(preds[i]);
  preds.clear();

}

