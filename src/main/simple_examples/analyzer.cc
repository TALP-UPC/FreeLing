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


//------------------------------------------------------------------//
//
//                    IMPORTANT NOTICE
//
//  This file contains a sample main program to illustrate 
//  usage of FreeLing analyzers library.
//
//  This sample main program may be used straightforwardly as 
//  a basic front-end for the analyzers (e.g. to analyze corpora)
//
//  Neverthless, if you want embed the FreeLing libraries inside
//  a larger application, or you want to deal with other 
//  input/output formats (e.g. XML,HTML,...), the efficient and elegant 
//  way to do so is consider this file as a mere example, and call 
//  the library from your your own main code.
//
//  See README file to find out how to compile and execute this sample
//
//------------------------------------------------------------------//


#include <iostream>

#include "freeling/morfo/analyzer.h"
#include "freeling/output/output_freeling.h"

#include "config.h"

using namespace std;
using namespace freeling;

/// predeclarations
analyzer::config_options fill_config(const wstring &path);
analyzer::invoke_options fill_invoke();


//////////////   MAIN PROGRAM  /////////////////////

int main (int argc, char **argv) {

  //// set locale to an UTF8 compatible locale 
  util::init_locale(L"default");

  /// read FreeLing installation path if given, use default otherwise
  wstring ipath;
  if (argc < 2) ipath = L"/usr/local";
  else ipath = util::string2wstring(argv[1]);

  /// set config options (which modules to create, with which configuration)
  analyzer::config_options cfg = fill_config(ipath+L"/share/freeling/");
  /// create analyzer
  analyzer anlz(cfg);

  /// set invoke options (which modules to use)
  analyzer::invoke_options ivk = fill_invoke();
  /// load invoke options into analyzer
  anlz.set_current_invoke_options(ivk);

  /// load document to analyze
  wstring text;  
  wstring line;
  while (getline(wcin,line)) 
    text = text + line + L"\n";

  /// analyze text, leave result in doc
  document doc;
  anlz.analyze(text,doc);

  /// Create output handler and select desired output
  io::output_freeling *out = new io::output_freeling();
  out->output_senses(true);
  out->output_dep_tree(true);
  out->output_corefs(true);

  /// print analysis results stored in doc
  out->PrintResults(wcout,doc);

  delete out;
}


///////////////////////////////////////////////////
/// Load an ad-hoc set of configuration options

analyzer::config_options fill_config(const wstring &path) {

  analyzer::config_options cfg;

  /// Language of text to process
  cfg.Lang = L"en";
 
  // path to language specific data
  wstring lpath = path + L"/" + cfg.Lang + L"/";

  /// Tokenizer configuration file
  cfg.TOK_TokenizerFile = lpath + L"tokenizer.dat";
  /// Splitter configuration file
  cfg.SPLIT_SplitterFile = lpath + L"splitter.dat";
  /// Morphological analyzer options
  cfg.MACO_Decimal = L".";
  cfg.MACO_Thousand = L",";
  cfg.MACO_LocutionsFile = lpath + L"locucions.dat";
  cfg.MACO_QuantitiesFile = lpath + L"quantities.dat";
  cfg.MACO_AffixFile = lpath + L"afixos.dat";
  cfg.MACO_ProbabilityFile = lpath + L"probabilitats.dat";
  cfg.MACO_DictionaryFile = lpath + L"dicc.src";
  cfg.MACO_NPDataFile = lpath + L"np.dat";
  cfg.MACO_PunctuationFile = path + L"common/punct.dat";
  cfg.MACO_ProbabilityThreshold = 0.001;

  /// NEC config file
  cfg.NEC_NECFile = lpath + L"nerc/nec/nec-ab-poor1.dat";
  /// Sense annotator and WSD config files
  cfg.SENSE_ConfigFile = lpath + L"senses.dat";
  cfg.UKB_ConfigFile = lpath + L"ukb.dat";
  /// Tagger options
  cfg.TAGGER_HMMFile = lpath + L"tagger.dat";
  cfg.TAGGER_ForceSelect=RETOK;
  /// Chart parser config file
  cfg.PARSER_GrammarFile = lpath + L"chunker/grammar-chunk.dat";
  /// Dependency parsers config files
  cfg.DEP_TxalaFile = lpath + L"dep_txala/dependences.dat";   
  cfg.DEP_TreelerFile = lpath + L"dep_treeler/labeled/dependences.dat";   
  /// Coreference resolution config file
  cfg.COREF_CorefFile = lpath + L"coref/relaxcor/relaxcor.dat";

  return cfg;
}


///////////////////////////////////////////////////
/// Load an ad-hoc set of invoke options

analyzer::invoke_options fill_invoke() {

  analyzer::invoke_options ivk;

  /// Level of analysis in input and output
  ivk.InputLevel = TEXT;
  ivk.OutputLevel = COREF;
  
  /// activate/deactivate morphological analyzer modules
  ivk.MACO_UserMap = false;
  ivk.MACO_AffixAnalysis = true;
  ivk.MACO_MultiwordsDetection = true;
  ivk.MACO_NumbersDetection = true;
  ivk.MACO_PunctuationDetection = true;
  ivk.MACO_DatesDetection = true;
  ivk.MACO_QuantitiesDetection  = true;
  ivk.MACO_DictionarySearch = true;
  ivk.MACO_ProbabilityAssignment = true;
  ivk.MACO_CompoundAnalysis = false;
  ivk.MACO_NERecognition = true;
  ivk.MACO_RetokContractions = false;
  
  ivk.NEC_NEClassification = true;
  
  ivk.SENSE_WSD_which = UKB;
  ivk.TAGGER_which = HMM;
  ivk.DEP_which = TREELER;

  return ivk;
}
