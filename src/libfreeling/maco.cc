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

#include "freeling/morfo/traces.h"
#include "freeling/morfo/maco.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"MACO"
#define MOD_TRACECODE MACO_TRACE

  ///////////////////////////////////////////////////////////////
  ///  Create the morphological analyzer, and all required 
  /// recognizers and modules.
  ///////////////////////////////////////////////////////////////

  maco::maco(const maco_options &opts) {

    MultiwordsDetection = PunctuationDetection = false;
    QuantitiesDetection = DictionarySearch = ProbabilityAssignment = false;
    UserMap = NERecognition = false;

    loc=NULL; dico=NULL; numb=NULL; date=NULL; quant=NULL;
    punt=NULL; user=NULL; prob=NULL; npm=NULL;

    if (not opts.UserMapFile.empty()) {
      user = new RE_map(opts.UserMapFile);
      UserMap = true;
    }

    numb = new numbers(opts.Lang,opts.Decimal,opts.Thousand);
    NumbersDetection = true;

    if (not opts.PunctuationFile.empty()) {
      punt = new punts(opts.PunctuationFile);
      PunctuationDetection = true;
    }

    date = new dates(opts.Lang);
    DatesDetection = true;

    if (not opts.DictionaryFile.empty()) {
      dico = new dictionary(opts.Lang, opts.DictionaryFile, 
                            opts.AffixFile, opts.CompoundFile,
                            opts.InverseDict, opts.RetokContractions);
      DictionarySearch = true;
    }

    if (not opts.LocutionsFile.empty()) {
      loc = new locutions(opts.LocutionsFile);
      MultiwordsDetection = true; 
    }

    if (not opts.NPdataFile.empty()) {
      npm = new ner(opts.NPdataFile);
      NERecognition = true;
    }

    if (not opts.QuantitiesFile.empty()) {
      quant = new quantities(opts.Lang, opts.QuantitiesFile);
      QuantitiesDetection = true;      
    }

    if (not opts.ProbabilityFile.empty()) {
      prob = new probabilities(opts.ProbabilityFile, opts.ProbabilityThreshold);
      ProbabilityAssignment = true;
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Destroy morphological analyzer, and all required 
  /// recognizers and modules.
  ///////////////////////////////////////////////////////////////

  maco::~maco() {
    delete user;
    delete numb;
    delete punt;
    delete date;
    delete dico;
    delete loc;
    delete npm;
    delete quant;
    delete prob;
  }

  ///////////////////////////////////////////////////////////////
  ///  set active modules for further analysis
  ///////////////////////////////////////////////////////////////  

  void maco::set_active_options(bool umap, bool num, bool pun, bool dat,
                                bool dic, bool aff, bool comp, bool rtk,
                                bool mw, bool ner, bool qt, bool prb) {

    wstring msg=L"option can't be activated because it was not loaded at instantiation time.";

    if (umap and user==NULL) { WARNING(L"UserMap "+msg); }
    else UserMap=umap;

    if (mw and loc==NULL) { WARNING(L"Multiwords "+msg); }
    else MultiwordsDetection=mw;

    if (num and numb==NULL) { WARNING(L"Numbers "+msg); }
    else NumbersDetection=num; 

    if (pun and punt==NULL) { WARNING(L"Punctuation "+msg) }
    else PunctuationDetection=pun;

    if (dat and date==NULL) { WARNING(L"Dates "+msg); }
    else DatesDetection=dat;   

    if (qt and quant==NULL) { WARNING(L"Quantities "+msg); }
    else QuantitiesDetection=qt;

    if (prb and prob==NULL) { WARNING(L"Probabilities "+msg); }
    else ProbabilityAssignment=prb;

    if (ner and npm==NULL) { WARNING(L"NE Recognition "+msg); }
    else NERecognition=ner;    

    if (dic and dico==NULL) { WARNING(L"Dictionary "+msg); }
    else DictionarySearch=dic;

    if (dic and dico==NULL) { WARNING(L"Retokenize contractions "+msg); }
    else dico->set_retokenize_contractions(rtk);

    if (aff and (dico==NULL or not dico->has_affixes())) { WARNING(L"Affixation "+msg); }
    else dico->set_affix_analysis(aff);

    if (comp and (dico==NULL or not dico->has_compounds())) { WARNING(L"Compound "+msg); }
    else dico->set_compound_analysis(comp);
  }


  ///////////////////////////////////////////////////////////////
  ///  Apply cascade of analyzers to given sentence.
  ///////////////////////////////////////////////////////////////  

  void maco::analyze(sentence &s) const {
  
    if (UserMap and user!=NULL) { 
      user->analyze(s);
      TRACE(2,L"Sentence annotated by the user-map module.");
    }

    if (NumbersDetection and numb!=NULL) { 
      // (Skipping number detection will affect dates and quantities modules)
      numb->analyze(s);    
      TRACE(2,L"Sentence annotated by the numbers module.");
    }

    if (PunctuationDetection and punt!=NULL) { 
      punt->analyze(s);    
      TRACE(2,L"Sentence annotated by the punts module.");
    }
     
    if (DatesDetection and date!=NULL) { 
      date->analyze(s);    
      TRACE(2,L"Sentence annotated by the dates module.");
    }

    if (DictionarySearch and dico!=NULL) {
      // (Skipping dictionary search will also skip suffix analysis)
      dico->analyze(s);
      TRACE(2,L"Sentence annotated by the dictionary searcher.");
    }

    // annotate list of sentences with required modules
    if (MultiwordsDetection and loc!=NULL) { 
      loc->analyze(s);
      TRACE(2,L"Sentence annotated by the locutions module.");
    }

    if (NERecognition and npm!=NULL) { 
      npm->analyze(s);
      TRACE(2,L"Sentence annotated by the np module.");
    }

    if (QuantitiesDetection and quant!=NULL) {
      quant->analyze(s);    
      TRACE(2,L"Sentence annotated by the quantities module.");
    }

    if (ProbabilityAssignment and prob!=NULL) {
      prob->analyze(s);    
      TRACE(2,L"Sentences annotated by the probabilities module.");
    }

    // mark all analysis of each word as selected (taggers assume it's this way)
    for (sentence::iterator w=s.begin(); w!=s.end(); w++)
      w->select_all_analysis();
  }

} // namespace
