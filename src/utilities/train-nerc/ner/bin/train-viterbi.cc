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

// ####  Given a corpus of B-I-O sequences, estimate the parameters 
// ####  of a markov model
// ####
// ####  usage:
// ####     ./train-viterbi codes <corpus
// ####
// ####   "codes" argument is a string with numeric codes for each class: 
// ####  e.g.: "0 B 1 I 2 O"
// ####
// ####  Input (stdin) is encoded training, one example per line. 
// ####     class feat1 feat2 feat3...
// ####  Sentences must be separated by blank lines.


#include <map>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>

#include "freeling/morfo/util.h"

using namespace std;
using namespace freeling;

// ----------------------------------------------------
void count(map<wstring,unsigned int> &m, wstring s) {
  map<wstring,unsigned int>::iterator p=m.find(s);
  if (p==m.end()) {
    wcerr<<L"BIO tag entry ["<<s<<L"] not found in count tables. Check training corpus."<<endl;
    exit(1);
  }

  p->second++;
}

// ----------------------------------------------------
int main(int argc, char* argv[]) {


  if (argc!=2) {
    wcerr<<L"Usage:  train-viterbi \"codes\" <encoded_text"<<endl;
    exit(1);
  }

  util::init_locale();

  map<wstring,unsigned int> ntrans,ninit,nocc;

  // get and analyze class codes string
  wstring cod=util::string2wstring(argv[1]);
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
  
  // init counts for b-i-o HMM
  for (map<wstring,wstring>::iterator c=codes.begin(); c!=codes.end(); c++) {
    ninit.insert(make_pair(c->first,0));
    nocc.insert(make_pair(c->first,0));
    for (map<wstring,wstring>::iterator d=codes.begin(); d!=codes.end(); d++)
      ntrans.insert(make_pair(c->first+L" "+d->first,0));
  }

  // load input into train data set, in the form of feature vectors.
  unsigned int ft=0,fu=0;
  unsigned int tinit=1;
  wstring text, BIOtag;
  wstring ant=L"";
  bool first=true;
  while (getline(wcin,text)) {
    
    if (text.empty()) { // sentence end
      first=true;
      tinit++;
      ant=L"";
    }
    else {
      wistringstream sin; 
      sin.str(text);
      sin>>BIOtag;  // first field is the B-I-O tag

      // collect statistics for viterbi algorithm.
      if (first) { 
	count(ninit,BIOtag);
	first=false; 
      }
      else {
	count(nocc,ant);
	count(ntrans,ant+L" "+BIOtag);
      }
      ant=BIOtag; 
    }
  }


  wcout<<L"<InitialProb>"<<endl;
  for (map<wstring,unsigned int>::iterator p=ninit.begin(); p!=ninit.end(); p++)
    wcout<<p->first<<L" "<<(double)p->second/(double)tinit<<endl;
  wcout<<L"</InitialProb>"<<endl;

  wcout<<L"<TransitionProb>"<<endl;
  for (map<wstring,unsigned int>::iterator p=ntrans.begin(); p!=ntrans.end(); p++) {
    wstring c=p->first.substr(0,p->first.find(L" "));
    wcout<<p->first<<L" "<<(double)p->second/(double)nocc[c]<<endl;
  }
  wcout<<L"</TransitionProb>"<<endl;

}


