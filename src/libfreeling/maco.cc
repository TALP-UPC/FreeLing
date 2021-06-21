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

  maco::maco(const analyzer_config &opts) : initial_options(opts) {

    // init current options with creation values.
    current_invoke_options = opts.invoke_opt;

    // create required modules according to given configuration    
    loc=NULL; dico=NULL; numb=NULL; date=NULL; quant=NULL;
    punt=NULL; user=NULL; prob=NULL; npm=NULL;

    if (not opts.config_opt.MACO_UserMapFile.empty()) 
      user = new RE_map(opts.config_opt.MACO_UserMapFile);

    numb = new numbers(opts.config_opt.Lang,
                       opts.config_opt.MACO_Decimal, opts.config_opt.MACO_Thousand);

    if (not opts.config_opt.MACO_PunctuationFile.empty())
      punt = new punts(opts.config_opt.MACO_PunctuationFile);

    date = new dates(opts.config_opt.Lang);

    if (not opts.config_opt.MACO_DictionaryFile.empty()) 
      dico = new dictionary(opts);
    
    if (not opts.config_opt.MACO_LocutionsFile.empty()) 
      loc = new locutions(opts.config_opt.MACO_LocutionsFile);

    if (not opts.config_opt.MACO_NPDataFile.empty()) 
      npm = new ner(opts.config_opt.MACO_NPDataFile);

    if (not opts.config_opt.MACO_QuantitiesFile.empty()) 
      quant = new quantities(opts.config_opt.Lang, opts.config_opt.MACO_QuantitiesFile);

    if (not opts.config_opt.MACO_ProbabilityFile.empty()) 
      prob = new probabilities(opts.config_opt.MACO_ProbabilityFile, opts.config_opt.MACO_ProbabilityThreshold);

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
  ///  convenience:  retrieve options used at creation time (e.g. to reset current config)
  ///////////////////////////////////////////////////////////////  

  const analyzer_config& maco::get_initial_options() const { return initial_options; }

  ///////////////////////////////////////////////////////////////
  /// set configuration to be used by default
  ///////////////////////////////////////////////////////////////

  void maco::set_current_invoke_options(const analyzer_invoke_options &opt) { current_invoke_options = opt; }

  ///////////////////////////////////////////////////////////////
  /// get configuration being used by default
  ///////////////////////////////////////////////////////////////

  const analyzer_invoke_options& maco::get_current_invoke_options() const { return current_invoke_options; }


  ///////////////////////////////////////////////////////////////
  ///  set active modules for further analysis.
  ///  Alternative mehod for set_current_invoke_options
  ///////////////////////////////////////////////////////////////  

  void maco::set_active_options(bool umap, bool num, bool pun, bool dat,
                                bool dic, bool aff, bool comp, bool rtk,
                                bool mw, bool ner, bool qt, bool prb) {

    wstring msg=L"option can't be activated because it was not loaded at instantiation time.";

    if (umap and user==NULL) { WARNING(L"UserMap "+msg); }
    else current_invoke_options.MACO_UserMap = umap;

    if (mw and loc==NULL) { WARNING(L"Multiwords "+msg); }
    else current_invoke_options.MACO_MultiwordsDetection = mw;

    if (num and numb==NULL) { WARNING(L"Numbers "+msg); }
    else current_invoke_options.MACO_NumbersDetection = num; 

    if (pun and punt==NULL) { WARNING(L"Punctuation "+msg) }
    else current_invoke_options.MACO_PunctuationDetection = pun;

    if (dat and date==NULL) { WARNING(L"Dates "+msg); }
    else current_invoke_options.MACO_DatesDetection = dat;   

    if (qt and quant==NULL) { WARNING(L"Quantities "+msg); }
    else current_invoke_options.MACO_QuantitiesDetection = qt;

    if (prb and prob==NULL) { WARNING(L"Probabilities "+msg); }
    else current_invoke_options.MACO_ProbabilityAssignment = prb;

    if (ner and npm==NULL) { WARNING(L"NE Recognition "+msg); }
    else current_invoke_options.MACO_NERecognition = ner;    

    if (dic and dico==NULL) { WARNING(L"Dictionary "+msg); }
    else current_invoke_options.MACO_DictionarySearch = dic;

    if (dic and dico==NULL) { WARNING(L"Retokenize contractions "+msg); }
    else current_invoke_options.MACO_RetokContractions = rtk;

    if (aff and (dico==NULL or initial_options.config_opt.MACO_AffixFile.empty())) {
      WARNING(L"Affixation "+msg);
    }
    else current_invoke_options.MACO_AffixAnalysis = aff;

    if (comp and (dico==NULL or initial_options.config_opt.MACO_CompoundFile.empty())) { WARNING(L"Compound "+msg); }
    else current_invoke_options.MACO_CompoundAnalysis = comp;
  }

  ///////////////////////////////////////////////////////////////
  ///  Apply cascade of analyzers to given sentence, according to given options
  ///////////////////////////////////////////////////////////////  

  void maco::analyze(sentence &s, const analyzer_invoke_options &opt) const {
  
    if (opt.MACO_UserMap and user!=NULL) { 
      user->analyze(s);
      TRACE(2,L"Sentence annotated by the user-map module.");
    }

    if (opt.MACO_NumbersDetection and numb!=NULL) { 
      // (Skipping number detection will affect dates and quantities modules)
      numb->analyze(s);    
      TRACE(2,L"Sentence annotated by the numbers module.");
    }

    if (opt.MACO_PunctuationDetection and punt!=NULL) { 
      punt->analyze(s);    
      TRACE(2,L"Sentence annotated by the punts module.");
    }
     
    if (opt.MACO_DatesDetection and date!=NULL) { 
      date->analyze(s);    
      TRACE(2,L"Sentence annotated by the dates module.");
    }

    if (opt.MACO_DictionarySearch and dico!=NULL) {
      // (Skipping dictionary search will also skip suffix analysis and compound analysis)
      dico->analyze(s, opt);      
      TRACE(2,L"Sentence annotated by the dictionary searcher.");
    }

    // annotate list of sentences with required modules
    if (opt.MACO_MultiwordsDetection and loc!=NULL) { 
      loc->analyze(s);
      TRACE(2,L"Sentence annotated by the locutions module.");
    }

    if (opt.MACO_NERecognition and npm!=NULL) { 
      npm->analyze(s);
      TRACE(2,L"Sentence annotated by the np module.");
    }

    if (opt.MACO_QuantitiesDetection and quant!=NULL) {
      quant->analyze(s);    
      TRACE(2,L"Sentence annotated by the quantities module.");
    }

    if (opt.MACO_ProbabilityAssignment and prob!=NULL) {
      prob->analyze(s);    
      TRACE(2,L"Sentences annotated by the probabilities module.");
    }

    // mark all analysis of each word as selected (taggers assume it's this way)
    for (sentence::iterator w=s.begin(); w!=s.end(); w++)
      w->select_all_analysis();
  }

  ///////////////////////////////////////////////////////////////
  ///  Apply cascade of analyzers to given sentence, using current default options
  ///////////////////////////////////////////////////////////////

  void maco::analyze(sentence &s) const {
    analyze(s, current_invoke_options);
  } 
  
} // namespace
