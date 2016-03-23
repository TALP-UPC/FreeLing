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


#include <fstream>
#include <sstream>
#include <math.h>

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/omlet/viterbi.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"VITERBI"
#define MOD_TRACECODE OMLET_TRACE


  //---------- Visible Viterbi Class  ----------------------------------

  ////////////////////////////////////////////////////////////////
  /// Constructor: Create dynammic storage for the best path
  ////////////////////////////////////////////////////////////////

  vis_viterbi::vis_viterbi(const wstring &nerFile) {
    
    map <wstring, int> class_num;

    // read probabilities file and store information   
    enum sections {CLASSES,INITIAL,TRANSITION,SOFTMAX};
    config_file cfg(true);  
    cfg.add_section(L"Classes",CLASSES);
    cfg.add_section(L"InitialProb",INITIAL);
    cfg.add_section(L"TransitionProb",TRANSITION);
    cfg.add_section(L"UseSoftMax",SOFTMAX);

    if (not cfg.open(nerFile))
      ERROR_CRASH(L"Error opening file "+nerFile);

    ZERO_logprob = -numeric_limits<float>::infinity();

    use_softmax=false;  
    N=0;  // init global number of classes.
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {

      case CLASSES: {
        // Reading class name and numbers: e.g. 0 B 1 I 2 O
        int num;
        wstring name;
        while(sin>>num>>name)
          class_num.insert(make_pair(name,num));   // to be used when reading probabilities
        N=class_num.size();       // now we know number of classes, set N
        break;
      }

      case INITIAL: {
        if (cfg.at_section_start()) { 
          // check we have read <Classes> section
          if (N==0) ERROR_CRASH(L"Classes name/numbers must appear before probability values in "+nerFile);
          // initialize initial probs vector 
          p_ini=vector<double>(N,99.0);
        }

        // Reading initial probabilities for each class (one class per line)
        wstring name,prob;
        sin>>name>>prob;
        map<wstring, int>:: iterator c=class_num.find(name);
        if (c==class_num.end())
          ERROR_CRASH(L"Class name for NER: \""+name+L"\" not found in class/int key");
        
        p_ini[c->second] = log(util::wstring2double(prob));  // store logprob   
        break;
      }

      case TRANSITION : {
        if (cfg.at_section_start()) { 
          // check we have read <Classes> section
          if (N==0) ERROR_CRASH(L"Classes name/numbers must appear before probability values in "+nerFile);
          
          // initialize transition matrix (with impossible values, to detect holes)
          p_trans=vector<vector<double> >(N,vector<double>(N,99.0));
        }
        // Reading transition probabilities for each class 
        //  (one transition per line: "B I prob(B->I)")
        wstring name,name1,prob;
        sin>>name>>name1>>prob;
        map<wstring, int>::iterator c=class_num.find(name);
        map<wstring, int>::iterator c1=class_num.find(name1);
        if (c==class_num.end() or c1==class_num.end())
          ERROR_CRASH(L"Class name for NER \""+name+L"\" or \""+name1+L"\", not found in class/int key");
        
        p_trans[c->second][c1->second] = log(util::wstring2double(prob)); // store logprob
        break;
      }

      case SOFTMAX : {
        use_softmax=(line==L"yes");
        break;
      }

      default: break;
      }
    }

    // Once file is readed, check that all p_ini and p_trans elements have 
    // received correct values (between 0 and 1).
    for (int i=0; i<N; i++){
      if (p_ini[i]>0)
        ERROR_CRASH(L"Invalid or missing probability value for some initial probability");
      for (int j=0; j<N; j++) {
        if(p_trans[i][j]>0)
          ERROR_CRASH(L"Invalid or missing probability value for some transition probability");
      }
    }
  
    TRACE(3,L"Viterbi created, from file "+nerFile);
  }
  

  ////////////////////////////////////////////////////////////////
  /// SoftMax function to convert weights in probabilities 
  /// p(i)=exp(w(i))/sum(exp(w(j)))
  ////////////////////////////////////////////////////////////////

  void vis_viterbi::softmax(double* p) const {
    // compute normalization factor
    double sum=0;
    for (int i=0; i<N; i++) sum+=exp(p[i]);
    // compute softmax for each component
    for (int i=0; i<N; i++) p[i] = exp(p[i])/sum;
  }


  ////////////////////////////////////////////////////////////////
  /// find_best_path: perform viterbi algortihm given the weights matrix
  ////////////////////////////////////////////////////////////////

  vector<int> vis_viterbi::find_best_path(vector<double*> &predictions) const {

    int sent_size=predictions.size();
    double p, max;
    int argmax=0;

    TRACE(3,L"  Viterbi: processing sentence of size: "+util::int2wstring(sent_size));
  
    // array of 2xN vector<int> containing the most likely path for 
    // each class at current (and next) word.
    vector<vector<vector<int> > > bestp=vector<vector<vector<int> > >(2,vector<vector<int> >(N,vector<int>()));
    // array of 2xN doubles containing best path probability of reaching 
    // current (and next) word with each possible class.
    vector<vector<double> > paths=vector<vector<double> >(2,vector<double>(N,0.0));

    // who is holding path for current/next word
    int current=0;
    int newp=1;
 
    // convert weights to probabilities, if needed. 
    if (use_softmax)
      for (int w=0; w<sent_size; w++) softmax(predictions[w]);

    // convert probabilities (either given or softmaxed) to logprobs
    for (int w=0; w<sent_size; w++) {
      for (int i=0; i<N; i++) {
        if (predictions[w][i]>1 or predictions[w][i]<0)
          ERROR_CRASH(L"Invalid class probability prediction. Did you forgot UseSoftMax=yes?");
        predictions[w][i] = log(predictions[w][i]);
      }
    }

    // initialize array with the probabilities for the first word
    // multiplied by initial probability (logprob sum)
    for (int i=0; i<N; i++) {
      paths[current][i]=p_ini[i]+predictions[0][i];
      TRACE(4,L"   initial prob for class "+util::int2wstring(i)+L": "+util::double2wstring(paths[current][i]));
      TRACE(4,L"         p_ini["+util::int2wstring(i)+L"]: "+util::double2wstring(p_ini[i]));
      TRACE(4,L"         p_pred: "+util::double2wstring(predictions[0][i]));
    }

    // predictions contains the weights for each class and each word,
    //  use these weights  and transition probabilities to compute 
    //  most likely path.
    for (int w=1; w<sent_size; w++){ // for each word starting in the second one
      TRACE(4,L" studying word in position "+util::int2wstring(w));
    
      for (int i=0; i<N; i++){ // for each class, store the best probability
        TRACE(5,L"   store best probability for class "+util::int2wstring(i));
        max=ZERO_logprob; 
        for (int j=0; j<N; j++) { 
          // accumulate path probability (logprob sum)
          p=paths[current][j] + predictions[w][i] + p_trans[j][i];
          TRACE(6,L"       with class "+util::int2wstring(j)+L" p: "+util::double2wstring(p)+L" (ptrans= "+util::double2wstring(p_trans[j][i])+L" pred: "+util::double2wstring(predictions[w][i])+L")");

          if(max<p) {
            max=p;
            argmax=j;
          }
        }

        TRACE(5,L"     best prob with class: "+util::int2wstring(argmax)+L" p: "+util::double2wstring(max));
        // store most likely path for this class.      
        bestp[newp][i]=bestp[current][argmax]; // Choose the path that led to best prob
        // as new best path for this class.
        bestp[newp][i].push_back(argmax);  // Add curent class to best path.  
        paths[newp][i]=max;                // Remember probability for new best path.
      }
    
      // swap current and newp for next iteration
      newp = 1-newp;
      current = 1-current;
    }
  
    // once the last word is reached, choose the best path
    max=ZERO_logprob;
    for (int i=0; i<N; i++) {
      if (paths[current][i] > max) {
        max=paths[current][i];
        argmax=i;
      }
    }
  
    TRACE(5,L"   best path for this sentence ends with "+util::int2wstring(argmax)+L" global p: "+util::double2wstring(paths[current][argmax]));

    // store best class for last word in best_path
    bestp[current][argmax].push_back(argmax);
  
    TRACE(3,L"Best path found");

    return (bestp[current][argmax]);
  }

} // namespace
