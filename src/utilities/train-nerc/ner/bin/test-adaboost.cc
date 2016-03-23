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

void process_sentence(vector<vector<double *> > &preds);
vector<int> find_best_path(const vector<double*> &preds);

adaboost *classf;
vis_viterbi *vit;

// ----------------------------------------------------

int main(int argc, char* argv[]) {

  util::init_locale();

  // traces::TraceLevel=4;
  // traces::TraceModule=OMLET_TRACE;

  if (argc!=3) {
    wcerr<<L"Usage:  test nerFile.dat step <encoded_text"<<endl;
    exit(1);
  }

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
  file.close();
   
  // create AdaBoost classf
  classf = new adaboost(abmFile,classnames);      
  // load feature lexicon
  fex_lexicon lex(lexFile);
  // create viterbi solver
  vit = new vis_viterbi(nerFile);
  
  int nlabels=classf->get_nlabels();
  
  // create one empty vector for each step.
  vector<vector<double*> > preds;
  for (int r=0; r<classf->n_rules(); r+=step) {
    vector<double *> v;
    preds.push_back(v);
  }
  
  wstring text;
  while (getline(wcin,text)) {
    
    if (text != L"") { // got a word line
      wistringstream sin; 
      sin.str(text);
      
      // build an example 
      example exmp(nlabels);
      
      wstring BIOtag;
      sin>>BIOtag;  // first field is the B-I-O tag, ignore (evaluation is outside)
      
      // remanining fields are feature names. Convert to codes and add to example.
      wstring feat;
      while (sin>>feat){
	int x = lex.get_code(feat);
	if (x>0)
	  exmp.add_feature(x);
      }
      
      double *p = new double[nlabels];
      classf->pcl_ini_pointer();
      // classify word, keeping results each 'step' rules.
      for (int r=0; r<classf->n_rules()/step; r++) {
	classf->pcl_classify(exmp, p, step);
	classf->pcl_advance_pointer(step);
	
	// copy partial results before continuing to next step
	double *vp = new double[nlabels];
	for (int i=0; i<nlabels; i++) vp[i]=p[i];
	preds[r].push_back(vp);
      }

      delete [] p;
    }
    else  // blank line, sentence end.
      process_sentence(preds); //--> viterbi, imprimir resultats
  }
  
  if (not preds.empty())   // no blank line after last sentence
    process_sentence(preds); //--> viterbi, imprimir resultats                            
  
  delete(classf);
  delete(vit);
}


#define MAX_STEPS 50

// ----------------------------------------------------
void process_sentence(vector<vector<double*> > &preds) {
  
  int nw=preds[0].size();
  int ns=preds.size();
  
  wstring* results[MAX_STEPS];

  for (int r=0; r<ns; r++) {
    results[r] = new wstring[nw];

    // Once all sentence has been encoded, use Viterbi algorithm to 
    // determine which is the most likely class combination
    vector<int> best;
    best = vit->find_best_path(preds[r]);
      
    // store best path at each step  
    for (int i=0; i<nw; i++) 
      results[r][i] = classf->get_label(best[i]);
    
    // clean step vector, ready for next sentence.
    for (int i=0; i<(int)preds[r].size(); i++) 
      delete(preds[r][i]);
    preds[r].clear();
  }

  // print best path at each step	 
  for (int i=0; i<nw; i++) {
    for (int r=0; r<ns; r++) 
       wcout << results[r][i] << L" ";
    wcout<<endl;  // end word line
  }
  wcout<<endl;    // sentence separator

}

