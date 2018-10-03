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

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/nec.h"
#include "freeling/morfo/nerc_features.h"
#include "freeling/omlet/svm.h"
#include "freeling/omlet/adaboost.h"

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"NEC"
#define MOD_TRACECODE NEC_TRACE

  ///////////////////////////////////////////////////////////////
  /// Create a named entity classification module, loading
  /// appropriate files.
  ///////////////////////////////////////////////////////////////

  nec::nec(const std::wstring &necFile) 
  {
    wstring lexFile,rgfFile,modelFile;
    wstring clastype, classnames;
    wstring path=necFile.substr(0,necFile.find_last_of(L"/\\")+1);
     
    // read configuration file and store information   
    enum sections {LEXICON,RGF,CLASSIFIER,MODELFILE,CLASSES,NE_TAG};
    config_file cfg;  
    cfg.add_section(L"Lexicon",LEXICON);
    cfg.add_section(L"RGF",RGF);
    cfg.add_section(L"Classifier",CLASSIFIER);
    cfg.add_section(L"ModelFile",MODELFILE);
    cfg.add_section(L"Classes",CLASSES);
    cfg.add_section(L"NE_Tag",NE_TAG);

    if (not cfg.open(necFile))
      ERROR_CRASH(L"Error opening file "+necFile);

    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);
    
      switch (cfg.get_section()) {
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
        // Reading class names and numbers: e.g. "0 NP00SP0 1 NP00G00 2 NP00O00 3 NP00V00"
        classnames=line;
        break;
      } 

      case NE_TAG: {
        // tag that identifies detected NEs to be classified
        NPtag=line;
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
  /// Destructor: delete pointers
  /////////////////////////////////////////////////////////////////////////////
  nec::~nec(){
    delete extractor;
    delete classif;
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Classify NEs in given sentence
  /////////////////////////////////////////////////////////////////////////////

  void nec::analyze(sentence &se) const {

    // check if the sentence has any word with NEtag.  
    bool hasNE=false;
    for (sentence::iterator w=se.begin(); w!=se.end() and not hasNE; w++)
      for (word::iterator a=w->selected_begin(); a!=w->selected_end() and not hasNE; a++)
        hasNE = (a->get_tag()==NPtag);
    // If no NEs in the sentence, let's avoid useless work. We are done.
    if (not hasNE) return;
    
    // allocate prediction array 
    double *pred = new double[classif->get_nlabels()];
    
    // extract sentence features
    vector<set<int> > features;
    features.clear();
    extractor->encode_int(se,features);
    TRACE(1,L"Sentence encoded.");
  
    // process each word
    int i;
    sentence::iterator w;
    for (w=se.begin(),i=0; w!=se.end(); w++,i++) {
      // for any analysis (selected by the tagger) that has NEtag, create and classify new example
      for (word::iterator a=w->selected_begin(); a!=w->selected_end(); a++) {
        if (a->get_tag()==NPtag) {
        
          TRACE(2,L"NP found ("+w->get_form()+L"). building example");
          example exmp(classif->get_nlabels());
          // add all extracted features to example
          for (set<int>::iterator f=features[i].begin(); f!=features[i].end(); f++) exmp.add_feature(*f);
          TRACE(3,L"   example build, with "+util::int2wstring(exmp.size())+L" features");
        
          // classify example
          classif->classify(exmp,pred);
          TRACE(2,L"Example classified");
        
          // find out which class has highest weight,
          double max=pred[0]; 
          wstring tag=classif->get_label(0);
          TRACE(3,L"   label:"+classif->get_label(0)+L" weight:"+util::double2wstring(pred[0]));
          for (int j=1; j<classif->get_nlabels(); j++) {
            TRACE(3,L"   label:"+classif->get_label(j)+L" weight:"+util::double2wstring(pred[j]));
            if (pred[j]>max) {
              max=pred[j];
              tag=classif->get_label(j);
            }
          } 
        
          // if no label has a positive prediction and <others> is defined, select <others> label.
          wstring def = classif->default_class();
          if (max<0 && def!=L"") tag = def;
        
          // tag NE appropriately: modify NP tag to be the right one... 
          a->set_tag(tag);
        }
      } 
    }
  
    TRACE_SENTENCE(1,se);
  
    delete [] pred;
  }

} // namespace
