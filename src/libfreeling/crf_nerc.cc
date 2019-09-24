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
#include "freeling/morfo/crf_nerc.h"
#include "freeling/morfo/configfile.h"


using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"CRF_NERC"
#define MOD_TRACECODE NP_TRACE

  ///////////////////////////////////////////////////////////////
  /// Perform named entity recognition and classification using CRF
  ///////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  /// Create a named entity recognition module, loading
  /// appropriate files.
  ///////////////////////////////////////////////////////////////

  crf_nerc::crf_nerc(const std::wstring &nerFile) : ner_module(nerFile) {

    wstring lexFile,rgfFile,modelFile;
    wstring clastype, classnames;
    wstring path=nerFile.substr(0,nerFile.find_last_of(L"/\\")+1);
  
    TRACE(3,L"Loading CRF ner options from "+nerFile);

    // read configuration file and store information   
    enum sections {NER_TYPE,RGF,MODELFILE,CLASSES};
    config_file cfg(true);  
    cfg.add_section(L"Type",NER_TYPE);
    cfg.add_section(L"RGF",RGF);
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
        if (util::lowercase(line)!=L"crf")
          ERROR_CRASH(L"Invalid configuration file for 'crf' NER, "+nerFile);
        break;
      }

      case RGF: {
        // Reading RGF file name
        sin>>rgfFile;
        rgfFile= util::absolute(rgfFile,path); 
        break;
      }

      case MODELFILE: {
        // Reading classifier model file name
        sin>>modelFile;
        modelFile= util::absolute(modelFile,path); 
        break;
      }

      case CLASSES: {
        // Reading class name and numbers: e.g. "PER NP00SP0 LOC NP00G00 ORG NP00O00 MISC NP00V00"
        wstring type,tag;
        while (sin >> type >> tag) NE_Tag.insert(make_pair(type, tag));
        break;
      } 

      default: break;
      }
    }
    cfg.close();

    // create feature extractor with appropriate rules and lexicon
    TRACE(3,L" Creating extractor with "+rgfFile);
    extractor = new fex(rgfFile,L"",nerc_features::functions);

    // create CRF tagger with given model
    crf = new CRFSuite::Tagger();
    crf->open(util::wstring2string(modelFile));

    TRACE(2,L"analyzer succesfully created");
  }

  /////////////////////////////////////////////////////////////////////////////
  /// Destructor: deletes created pointers

  crf_nerc::~crf_nerc() {
    delete extractor;
    delete crf;
  }
   
  /////////////////////////////////////////////////////////////////////////////
  /// Recognize NEs in given sentence
  /////////////////////////////////////////////////////////////////////////////

  void crf_nerc::analyze(sentence &se) const {

    TRACE(2,L"CRF nerc annotating sentence.");
  
    // extract sentence features
    vector<set<wstring> > features;
    extractor->encode_name(se,features);
    TRACE(2,L"Sentence encoded.");
  
    // Create CRF input  sequence process each word
    CRFSuite::ItemSequence xseq;
    for (vector<set<wstring>>::const_iterator w=features.begin(); w!=features.end(); ++w) {
      // for each sentence word, create an CRF seqence item and fill it with the word features
      CRFSuite::Item x;
      for (set<wstring>::const_iterator f=w->begin(); f!=w->end(); ++f) {
        x.push_back(CRFSuite::Attribute(util::wstring2string(*f)));
      }
      // add item to CRF input sequence
      xseq.push_back(x);
    }

    // call CRF to find optimal tagging
    CRFSuite::StringList yseq = crf->tag(xseq);

    // process obtained best_path and join detected NEs, syncronize it with sentence
    bool inNE = false;
    sentence::iterator beg;
    bool changes = false;
    wstring NE_type;
    
    // for each word
    int i=0;
    for (sentence::iterator w=se.begin(); w!=se.end(); ++w,++i) { 
      // look for the BIOtag choosen for this word
      wstring tag = util::string2wstring(yseq[i]);
      if (tag[0]==L'B')
        NE_type = tag.substr(2); // tags are like B-ORG, B-PER... get the part after "B-"
      
      TRACE(3, L"Word "+w->get_form()+L" has BIO tag "+tag);
      // if we were inside NE, and the chosen class for this word is "B" or "O", 
      //  previous NE is finished: build multiword.       
      if ((inNE && tag[0]==L'B') || (inNE && tag[0]==L'O')) {        
        sentence::iterator w1=w; w1--;
        w = BuildMultiword(se, beg, w1, NE_type);
        changes = true;
        w++; // add one because w points to last word of multiword, which is previous word
        inNE=false;
        TRACE(5,L"  multiword built. Current word: "+w->get_form());
      }
      // if we found "B", start new NE (previous if statment joins possible previous NE that finishes here)
      if (tag[0]==L'B') {
        inNE=true;
        beg=w;
      }   
    }

    // If last words were an NE, build it
    if (inNE) {
      sentence::iterator w1 = se.end(); w1--;
      sentence::iterator w = BuildMultiword(se, beg, w1, NE_type);
      changes = true;
      TRACE(5,L"  multiword built. Current word: "+w->get_form());
    }
  
    // rebuild word index if numer of words was changed
    if (changes) se.rebuild_word_index();

    TRACE_SENTENCE(1,se);  
  }


  /////////////////////////////////////////////////////////////////////////////
  /// Modyfy sentence, replacing group of words with the new multiword
  /////////////////////////////////////////////////////////////////////////////

  sentence::iterator crf_nerc::BuildMultiword(sentence &se, sentence::iterator start, sentence::iterator end, const wstring &type) const {

    TRACE(3,L"  Building multiword");
    list<word> mw;
    wstring form;
    sentence::iterator i;
    for (i=start; i!=end; i++){
      mw.push_back(*i);           
      form += i->get_form()+L"_";
      TRACE(3,L"   added next ["+form+L"]");
    }
    // don't forget last word
    mw.push_back(*i);
    form += i->get_form();
    TRACE(3,L"   added last ["+form+L"]");
    
    // build new word with the mw list
    word w(form,mw);
    // replace old words with new multiword
    end++;
    i = se.erase(start, end);
    i = se.insert(i,w); 
    TRACE(3,L"New word inserted");

      // Add an NP analysis, with the compound form as lemma.
    TRACE(3,L"   adding NP analysis");
    // CALCULAR QUIN ES EL TAG a partir de CLASSES i el resultat del CRF
    i->add_analysis(analysis(i->get_lc_form(), NE_Tag.find(type)->second));

    // record this word was analyzed by this module
    i->set_analyzed_by(word::NERC);    
    // prevent guesser from adding more tags.
    i->lock_analysis();

    return i;
  }

  
} // namespace
