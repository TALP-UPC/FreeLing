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

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/nerc_features.h"
#include "freeling/morfo/bioner.h"
#include "freeling/morfo/configfile.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NP"
#define MOD_TRACECODE NP_TRACE

  ///////////////////////////////////////////////////////////////
  /// Perform named entity recognition using AdaBoost
  ///////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  /// Create a named entity recognition module, loading
  /// appropriate files.
  ///////////////////////////////////////////////////////////////

  bioner::bioner(const std::wstring &nerFile) : ner_module(nerFile), vit(nerFile) {

    wstring lexFile,rgfFile,modelFile;
    wstring clastype, classnames;
    wstring path=nerFile.substr(0,nerFile.find_last_of(L"/\\")+1);
  
    TRACE(3,L"Loading BIO ner options from "+nerFile);

    // read configuration file and store information   
    enum sections {NER_TYPE,LEXICON,RGF,CLASSIFIER,MODELFILE,CLASSES};
    config_file cfg(true);  
    cfg.add_section(L"Type",NER_TYPE);
    cfg.add_section(L"Lexicon",LEXICON);
    cfg.add_section(L"RGF",RGF);
    cfg.add_section(L"Classifier",CLASSIFIER);
    cfg.add_section(L"ModelFile",MODELFILE);
    cfg.add_section(L"Classes",CLASSES);

    if (not cfg.open(nerFile))
      ERROR_CRASH(L"Error opening file "+nerFile);

    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      // process each content line according to the section where it is found
      switch (cfg.get_section()) {

      case NER_TYPE: {
        if (util::lowercase(line)!=L"bio")
          ERROR_CRASH(L"Invalid configuration file for 'bio' NER, "+nerFile);
        break;
      }

      case LEXICON: {
        // Reading lexicon file name
        sin>>lexFile;
        lexFile= util::absolute(lexFile,path); 
        break;
      }

      case RGF: {
        // Reading RGF file name
        sin>>rgfFile;
        rgfFile= util::absolute(rgfFile,path); 
        break;
      }

      case CLASSIFIER: {
        // Reading classifier type
        sin>>clastype;
        break;
      }

      case MODELFILE: {
        // Reading classifier model file name
        sin>>modelFile;
        modelFile= util::absolute(modelFile,path); 
        break;
      }

      case CLASSES: {
        // Reading class name and numbers: e.g. 0 B 1 I 2 O
        classnames=line;
        break;
      } 

      default: break;
      }
    }
    cfg.close();

    // create feature extractor with appropriate rules and lexicon
    TRACE(3,L" Creating extractor with "+rgfFile+L" and "+lexFile);
    extractor = new fex(rgfFile,lexFile,nerc_features::functions);

    // create appropriate classifier
    if (clastype==L"AdaBoost") {
      TRACE(3,L" Loading adaboost model "+modelFile);
      classif = new adaboost(modelFile,classnames);      
    }
    else if (clastype==L"SVM") {
      TRACE(3,L" Creating SVM with model "+modelFile);
      classif = new svm(modelFile,classnames);
    }
    else 
      ERROR_CRASH (L"Unspecifed or invalid <Classifier> section in file "+modelFile);

    TRACE(2,L"analyzer succesfully created");
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Destructor: deletes created pointers

  bioner::~bioner(){
    delete classif;
    delete extractor;
  }
   
  /////////////////////////////////////////////////////////////////////////////
  /// Recognize NEs in given sentence
  /////////////////////////////////////////////////////////////////////////////

  void bioner::analyze(sentence &se) const {

    ner_status *st = (ner_status*)NULL;

    TRACE(2,L"BIO ner annotating sentence.");
    // remember sentence size, we'll need it a lot of times
    int nw=se.size();
  
    // Prediction array for whole sentece
    // All predictions of the sentence will be stored in an array.
    // For each word, predictions for each class are stored,
    // that is:   all_pred[i][j] = prediction for word i to have class j.
    vector<double*> all_pred = vector<double*>(nw,(double*)NULL);
    for (int i = 0; i<nw; ++i) 
      all_pred[i] = new double[classif->get_nlabels()];
      
    // extract sentence features
    vector<set<int> > features;
    features.clear();
    extractor->encode_int(se,features);
    TRACE(2,L"Sentence encoded.");
  
    // process each word
    int i=0;
    for (sentence::iterator w=se.begin(); w!=se.end(); w++,i++) {

      if (w->is_locked_multiwords()) {
        TRACE(3,L"Word is locked. BIO tag set to 'O'.");
        for (int j=0; j<classif->get_nlabels(); j++) all_pred[i][j] = 0.0;
        all_pred[i][classif->get_index(L"O")]=1.0;
      }
      else {
        example exmp(classif->get_nlabels());
      
        // add all extracted features to example
        for (set<int>::iterator f=features[i].begin(); f!=features[i].end(); f++) 
          exmp.add_feature(*f);
        TRACE(4,L"   example build, with "+util::int2wstring(exmp.size())+L" features");
      
        // classify example
        classif->classify(exmp,all_pred[i]);
        TRACE(3,L"Example classified");
      }
    }
  
    // Once all sentence has been encoded, use Viterbi algorithm to 
    // determine which is the most likely class combination
    vector<int> best;
    best = vit.find_best_path(all_pred);
  
    // process obtained best_path and join detected NEs, syncronize it with sentence
    bool inNE=false;
    sentence::iterator beg;
    bool changes=false;
    
    // for each word
    i=0;
    for (sentence::iterator w=se.begin(); w!=se.end(); w++,i++) { 
      // look for the BIOtag choosen for this word
      wstring tag=classif->get_label(best[i]); 
      TRACE(3, L"Word "+w->get_form()+L" has BIO tag "+tag);
      // if we were inside NE, and the chosen class (best[i]) for this word is "B" or "O", 
      //  previous NE is finished: build multiword.       
      if ((inNE && tag==L"B") || (inNE && tag==L"O")) {
        sentence::iterator w1=w; w1--;
        bool built;
        w=BuildMultiword(se, beg, w1, 0, built, st);
        if (built) changes=true;
        w++; // add one because w points to last word of multiword, which is previous word
        inNE=false;
        TRACE(5,L"  multiword built. Current word: "+w->get_form());
      }
      // if we found "B", start new NE (previous if statment joins possible previous NE that finishes here)
      if (tag==L"B") {
        inNE=true;
        beg=w;
      }   
    }

    // If last words were an NE, build it
    if (inNE) {
      sentence::iterator w1=se.end(); w1--;
      bool built;
      sentence::iterator w=BuildMultiword(se, beg, w1, 0, built, st);
      if (built) changes=true;
      TRACE(5,L"  multiword built. Current word: "+w->get_form());
    }
  
    // free memory 
    for (int i = 0; i < nw; ++i) delete(all_pred[i]);
    // rebuild word index if numer of words was changed
    if (changes) se.rebuild_word_index();

    TRACE_SENTENCE(1,se);  
  }



} // namespace
