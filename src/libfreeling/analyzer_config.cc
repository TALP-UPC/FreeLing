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

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/analyzer_config.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#define MOD_TRACENAME L"ANALYZER_CONFIG"

  // READ OPERATORS FOR ENUM TYPES, USED BY boost::program_options
  
  std::wistream& operator>>(std::wistream& in, freeling::ForceSelectStrategy& val) {
    std::wstring token;
    in >> token;
    if (token==L"no" or token==L"none") val = freeling::NO_FORCE;
    else if (token==L"tagger") val = freeling::TAGGER;
    else if (token==L"retok") val = freeling::RETOK;
    else {
      val = freeling::RETOK; // default value
      WARNING(L"Unknown or invalid force strategy: "<<token<<L". Using default.");
    }
    return in;
  }
  
  std::wistream& operator>>(std::wistream& in, freeling::TaggerAlgorithm& val) {
    std::wstring token;
    in >> token;
    if (token==L"no" or token==L"none") val = freeling::NO_TAGGER;
    else if (token==L"hmm") val = freeling::HMM;
    else if (token==L"relax") val = freeling::RELAX;
    else {
      val = freeling::NO_TAGGER; // default value
      WARNING(L"Unknown or invalid tagger: "<<token<<L". Using default.");
    }
    return in;
  }
  
  std::wistream& operator>>(std::wistream& in, freeling::WSDAlgorithm& val) {
    std::wstring token;
    in >> token;
    if (token==L"no" or token==L"none") val = freeling::NO_WSD;
    else if (token==L"all") val = freeling::ALL;
    else if (token==L"mfs") val = freeling::MFS;
    else if (token==L"ukb") val = freeling::UKB;
    else {
      val = freeling::NO_WSD; // default value
      WARNING(L"Unknown or invalid WSD: "<<token<<L". Using default.");
    }
    return in;
  }
  
  std::wistream& operator>>(std::wistream& in, freeling::DependencyParser& val) {
    std::wstring token;
    in >> token;
    if (token==L"no" or token==L"none") val = freeling::NO_DEP;
    else if (token==L"txala") val = freeling::TXALA;
    else if (token==L"treeler") val = freeling::TREELER;
    else if (token==L"lstm") val = freeling::LSTM;
    else {
      val = freeling::NO_DEP; // default value
      WARNING(L"Unknown or invalid dependency parser: "<<token<<L". Using default.");
    }
    return in;
  }
  
  std::wistream& operator>>(std::wistream& in, freeling::SRLParser& val) {
    std::wstring token;
    in >> token;
    if (token==L"no" or token==L"none") val = freeling::NO_SRL;
    else if (token==L"treeler") val = freeling::SRL_TREELER;
    else {
      val = freeling::NO_SRL; // default value
      WARNING(L"Unknown or invalid SRL parser: "<<token<<L". Using default.");
    }
    return in;
  }
  
  std::wistream& operator>>(std::wistream& in, freeling::AnalysisLevel& val) {
    std::wstring token;
    in >> token;
    if (token==L"text") val = freeling::TEXT;
    else if (token==L"token") val = freeling::TOKEN;
    else if (token==L"splitted") val = freeling::SPLITTED;
    else if (token==L"morfo") val = freeling::MORFO;
    else if (token==L"tagged") val = freeling::TAGGED;
    else if (token==L"sense") val = freeling::SENSES;
    else if (token==L"shallow") val = freeling::SHALLOW;
    else if (token==L"parsed") val = freeling::PARSED;
    else if (token==L"dep") val = freeling::DEP;
    else if (token==L"srl") val = freeling::SRL;
    else if (token==L"coref") val = freeling::COREF;
    else if (token==L"semgraph") val = freeling::SEMGRAPH;
    else {
      val = freeling::TEXT; // default value
      WARNING(L"Unknown or invalid analysis level: "<<token<<L". Using default.");
    }
    return in;
  }
    
  bool read_bool(std::wistream& in) {
    std::wstring token;
    in >> token;
    return (token[0]==L'y' or token[0]==L'Y' or token==L"true" or token==L"TRUE" or token==L"1");
  }
  

  //---------------------------------------------
  //  Classes to hold configuration options
  //---------------------------------------------
  
  
  /// config options constructor, default values (except file names, initialized to "")
  
  analyzer_config::analyzer_config_options::analyzer_config_options() {
    MACO_InverseDictionary = false;
    MACO_ProbabilityThreshold = 0.001;
    TAGGER_RelaxMaxIter = 500;
    TAGGER_RelaxScaleFactor = 67;
    TAGGER_RelaxEpsilon = 0.001;
    TAGGER_Retokenize = true;
    TAGGER_kbest = 1;
    TAGGER_ForceSelect = TAGGER;
  }
  
  /// destructor
  
  analyzer_config::analyzer_config_options::~analyzer_config_options() {}

  /// dumper

  wstring analyzer_config::analyzer_config_options::dump() const {
    wostringstream sout;
    sout << L"Lang: " << Lang << endl;
    sout << L"TOK_TokenizerFile: " << TOK_TokenizerFile << endl;
    sout << L"SPLIT_SplitterFile: " << SPLIT_SplitterFile << endl;
    sout << L"MACO_Decimal: " << MACO_Decimal << endl;
    sout << L"MACO_Thousand: " << MACO_Thousand << endl;
    sout << L"MACO_UserMapFile: " << MACO_UserMapFile << endl;
    sout << L"MACO_LocutionsFile: " << MACO_LocutionsFile  << endl;
    sout << L"MACO_QuantitiesFile: " << MACO_QuantitiesFile << endl;
    sout << L"MACO_AffixFile: " << MACO_AffixFile << endl;
    sout << L"MACO_ProbabilityFile: " << MACO_ProbabilityFile << endl;
    sout << L"MACO_DictionaryFile: " << MACO_DictionaryFile << endl;
    sout << L"MACO_NPDataFile: " << MACO_NPDataFile << endl;
    sout << L"MACO_PunctuationFile: " << MACO_PunctuationFile << endl;
    sout << L"MACO_CompoundFile: " << MACO_CompoundFile << endl;
    sout << L"MACO_ProbabilityThreshold: " << MACO_ProbabilityThreshold << endl;
    sout << L"PHON_PhoneticsFile: " << PHON_PhoneticsFile << endl;
    sout << L"NEC_NECFile: " << NEC_NECFile << endl;
    sout << L"SENSE_ConfigFile: " << SENSE_ConfigFile << endl;
    sout << L"UKB_ConfigFile: " << UKB_ConfigFile << endl;
    sout << L"TAGGER_HMMFile: " << TAGGER_HMMFile << endl;
    sout << L"TAGGER_RelaxFile: " << TAGGER_RelaxFile << endl;
    sout << L"TAGGER_RelaxMaxIter: " << TAGGER_RelaxMaxIter << endl;
    sout << L"TAGGER_RelaxScaleFactor: " << TAGGER_RelaxScaleFactor << endl;
    sout << L"TAGGER_RelaxEpsilon: " << TAGGER_RelaxEpsilon << endl;
    sout << L"TAGGER_Retokenize: " << TAGGER_Retokenize << endl;
    sout << L"TAGGER_kbest: " << TAGGER_kbest << endl;
    sout << L"TAGGER_ForceSelect: " << TAGGER_ForceSelect << endl;
    sout << L"PARSER_GrammarFile: " << PARSER_GrammarFile << endl;
    sout << L"DEP_TxalaFile: " << DEP_TxalaFile << endl;
    sout << L"DEP_TreelerFile: " << DEP_TreelerFile << endl;
    sout << L"DEP_LSTMFile: " << DEP_LSTMFile << endl;
    sout << L"SRL_TreelerFile: " << SRL_TreelerFile << endl;
    sout << L"COREF_CorefFile: " << COREF_CorefFile << endl;
    sout << L"SEMGRAPH_SemGraphFile: " << SEMGRAPH_SemGraphFile << endl;
    return sout.str();
  }

  
  /// invoke options constructor, default values 
  
  analyzer_config::analyzer_invoke_options::analyzer_invoke_options() {
    InputLevel = TEXT;  OutputLevel = TAGGED;
    
    MACO_UserMap=false;            MACO_AffixAnalysis=true;        MACO_MultiwordsDetection=true;
    MACO_NumbersDetection=true;    MACO_PunctuationDetection=true; MACO_DatesDetection=true;
    MACO_QuantitiesDetection=true; MACO_DictionarySearch=true;     MACO_ProbabilityAssignment=true;
    MACO_CompoundAnalysis=true;    MACO_NERecognition=true;        MACO_RetokContractions=true;
    
    PHON_Phonetics=false;
    NEC_NEClassification=false;
    
    SENSE_WSD_which = NO_WSD;
    TAGGER_which = HMM;
    DEP_which = NO_DEP;    
    SRL_which = NO_SRL;
  }
  
  /// destructor
  
  analyzer_config::analyzer_invoke_options::~analyzer_invoke_options() {}
  
  /// dumper

  wstring analyzer_config::analyzer_invoke_options::dump() const {
    wostringstream sout;
    sout << L"InputLevel: " << InputLevel << endl;
    sout << L"OutputLevel: " << OutputLevel << endl;
    sout << L"MACO_UserMap: " << MACO_UserMap << endl;
    sout << L"MACO_AffixAnalysis: " << MACO_AffixAnalysis << endl;
    sout << L"MACO_MultiwordsDetection: " << MACO_MultiwordsDetection << endl;
    sout << L"MACO_NumbersDetection: " << MACO_NumbersDetection << endl;
    sout << L"MACO_PunctuationDetection: " << MACO_PunctuationDetection << endl;
    sout << L"MACO_DatesDetection: " << MACO_DatesDetection << endl;
    sout << L"MACO_QuantitiesDetection: " << MACO_QuantitiesDetection << endl;
    sout << L"MACO_DictionarySearch: " << MACO_DictionarySearch << endl;
    sout << L"MACO_ProbabilityAssignment: " << MACO_ProbabilityAssignment << endl;
    sout << L"MACO_CompoundAnalysis: " << MACO_CompoundAnalysis << endl;
    sout << L"MACO_NERecognition: " << MACO_NERecognition << endl;
    sout << L"MACO_RetokContractions: " << MACO_RetokContractions << endl;
    sout << L"PHON_Phonetics: " << PHON_Phonetics << endl;
    sout << L"NEC_NEClassification: " << NEC_NEClassification << endl;
    sout << L"SENSE_WSD_which: " << SENSE_WSD_which << endl;
    sout << L"TAGGER_which: " << TAGGER_which << endl;
    sout << L"DEP_which: " << DEP_which << endl;
    sout << L"SRL_which: " << SRL_which << endl;
    return sout.str();
  }
  
  ///////////////////////////////////////////////////////////////////////  
  /// Class holding all configuration options for "analyzer" class

  /// default constructor
  
  analyzer_config::analyzer_config() : config_opt(), invoke_opt() {}

  /// destructor
  
  analyzer_config::~analyzer_config() {}

  /// Extract values from variables map into analyzer_config members.
  /// Also expand filenames and boolean values in options map
  void analyzer_config::extract_options(const po::variables_map &vm) {

    // first, process config file options
    for (auto v : vm) {
      string name = v.first;

      // config options
      if (name == "Lang") config_opt.Lang = v.second.as<wstring>();
      else if (name == "TokenizerFile") config_opt.TOK_TokenizerFile = v.second.as<wstring>();
      else if (name == "SplitterFile") config_opt.SPLIT_SplitterFile = v.second.as<wstring>();
      else if (name == "DecimalPoint") config_opt.MACO_Decimal = v.second.as<wstring>();
      else if (name == "ThousandPoint") config_opt.MACO_Thousand = v.second.as<wstring>();
      else if (name == "UserMapFile") config_opt.MACO_UserMapFile = v.second.as<wstring>();
      else if (name == "LocutionsFile") config_opt.MACO_LocutionsFile = v.second.as<wstring>();
      else if (name == "QuantitiesFile") config_opt.MACO_QuantitiesFile = v.second.as<wstring>();
      else if (name == "AffixFile") config_opt.MACO_AffixFile = v.second.as<wstring>();
      else if (name == "ProbabilityFile") config_opt.MACO_ProbabilityFile = v.second.as<wstring>();
      else if (name == "ProbabilityThreshold") config_opt.MACO_ProbabilityThreshold = v.second.as<double>();
      else if (name == "DictionaryFile") config_opt.MACO_DictionaryFile = v.second.as<wstring>();
      else if (name == "NPDataFile") config_opt.MACO_NPDataFile = v.second.as<wstring>();
      else if (name == "CompoundFile") config_opt.MACO_CompoundFile = v.second.as<wstring>();
      else if (name == "PunctuationFile") config_opt.MACO_PunctuationFile = v.second.as<wstring>();
      else if (name == "PhoneticsFile") config_opt.PHON_PhoneticsFile = v.second.as<wstring>();
      else if (name == "NECFile") config_opt.NEC_NECFile = v.second.as<wstring>();
      else if (name == "SenseConfigFile") config_opt.SENSE_ConfigFile = v.second.as<wstring>();
      else if (name == "UKBConfigFile") config_opt.UKB_ConfigFile = v.second.as<wstring>();
      else if (name == "TaggerHMMFile") config_opt.TAGGER_HMMFile = v.second.as<wstring>();
      else if (name == "TaggerRelaxFile") config_opt.TAGGER_RelaxFile = v.second.as<wstring>();
      else if (name == "TaggerRelaxMaxIter") config_opt.TAGGER_RelaxMaxIter = v.second.as<int>();
      else if (name == "TaggerRelaxScaleFactor") config_opt.TAGGER_RelaxScaleFactor = v.second.as<double>();
      else if (name == "TaggerRelaxEpsilon") config_opt.TAGGER_RelaxEpsilon = v.second.as<double>();
      else if (name == "TaggerRetokenize") config_opt.TAGGER_Retokenize = v.second.as<bool>();
      else if (name == "TaggerForceSelect") config_opt.TAGGER_ForceSelect = v.second.as<freeling::ForceSelectStrategy>();
      else if (name == "TaggerKBest") config_opt.TAGGER_kbest = v.second.as<int>();
      else if (name == "GrammarFile") config_opt.PARSER_GrammarFile = v.second.as<wstring>();
      else if (name == "DepTxalaFile") config_opt.DEP_TxalaFile = v.second.as<wstring>();
      else if (name == "DepTreelerFile") config_opt.DEP_TreelerFile = v.second.as<wstring>();
      else if (name == "DepLSTMFile") config_opt.DEP_LSTMFile = v.second.as<wstring>();
      else if (name == "SRLTreelerFile") config_opt.SRL_TreelerFile = v.second.as<wstring>();
      else if (name == "CorefFile") config_opt.COREF_CorefFile = v.second.as<wstring>();
      else if (name == "SemGraphExtractorFile") config_opt.SEMGRAPH_SemGraphFile = v.second.as<wstring>();      

      // invoke options
      else if (name == "InputLevel") invoke_opt.InputLevel = v.second.as<freeling::AnalysisLevel>();
      else if (name == "OutputLevel") invoke_opt.OutputLevel = v.second.as<freeling::AnalysisLevel>();
      else if (name == "AffixAnalysis") invoke_opt.MACO_AffixAnalysis = v.second.as<bool>();
      else if (name == "UserMap") invoke_opt.MACO_UserMap = v.second.as<bool>();
      else if (name == "MultiwordsDetection") invoke_opt.MACO_MultiwordsDetection = v.second.as<bool>();
      else if (name == "NumbersDetection") invoke_opt.MACO_NumbersDetection = v.second.as<bool>();
      else if (name == "PunctuationDetection") invoke_opt.MACO_PunctuationDetection = v.second.as<bool>();
      else if (name == "DatesDetection") invoke_opt.MACO_DatesDetection = v.second.as<bool>();
      else if (name == "DatesDetection") invoke_opt.MACO_DatesDetection = v.second.as<bool>();
      else if (name == "QuantitiesDetection") invoke_opt.MACO_QuantitiesDetection = v.second.as<bool>();
      else if (name == "DictionarySearch") invoke_opt.MACO_DictionarySearch = v.second.as<bool>();
      else if (name == "RetokContractions") invoke_opt.MACO_RetokContractions = v.second.as<bool>();
      else if (name == "ProbabilityAssignment") invoke_opt.MACO_ProbabilityAssignment = v.second.as<bool>();
      else if (name == "CompoundAnalysis") invoke_opt.MACO_CompoundAnalysis = v.second.as<bool>();
      else if (name == "NERecognition") invoke_opt.MACO_NERecognition = v.second.as<bool>();
      else if (name == "Phonetics") invoke_opt.PHON_Phonetics = v.second.as<bool>();
      else if (name == "NEClassification") invoke_opt.NEC_NEClassification = v.second.as<bool>();
      else if (name == "SenseAnnotation") invoke_opt.SENSE_WSD_which = v.second.as<freeling::WSDAlgorithm>();
      else if (name == "Tagger") invoke_opt.TAGGER_which = v.second.as<freeling::TaggerAlgorithm>();
      else if (name == "DependencyParser") invoke_opt.DEP_which = v.second.as<freeling::DependencyParser>();
      else if (name == "SRLParser") invoke_opt.SRL_which = v.second.as<freeling::SRLParser>();
    }

    // second, process command line options, overwriting config file 
    for (auto v : vm) {
      string name = v.first;
      // /////////////


      //  TO DO
      // TO DO

      
      // if (name=="") ...
    }
    // Handle boolean options expressed with --myopt or --nomyopt in command line
    SetBooleanOptionCL(vm.count("rtk"),vm.count("nortk"),config_opt.TAGGER_Retokenize,"rtk");
    SetBooleanOptionCL(vm.count("afx"),vm.count("noafx"),invoke_opt.MACO_AffixAnalysis,"afx");
    SetBooleanOptionCL(vm.count("usr"),vm.count("nousr"),invoke_opt.MACO_UserMap,"usr");
    SetBooleanOptionCL(vm.count("loc"),vm.count("noloc"),invoke_opt.MACO_MultiwordsDetection,"loc");
    SetBooleanOptionCL(vm.count("numb"),vm.count("nonumb"),invoke_opt.MACO_NumbersDetection,"numb");
    SetBooleanOptionCL(vm.count("punt"),vm.count("nopunt"),invoke_opt.MACO_PunctuationDetection,"punt");
    SetBooleanOptionCL(vm.count("date"),vm.count("nodate"),invoke_opt.MACO_DatesDetection,"date");
    SetBooleanOptionCL(vm.count("ner"),vm.count("noner"),invoke_opt.MACO_NERecognition,"ner");
    SetBooleanOptionCL(vm.count("quant"),vm.count("noquant"),invoke_opt.MACO_QuantitiesDetection,"quant");
    SetBooleanOptionCL(vm.count("dict"),vm.count("nodict"),invoke_opt.MACO_DictionarySearch,"dict");
    SetBooleanOptionCL(vm.count("rtkcon"),vm.count("nortkcon"),invoke_opt.MACO_RetokContractions,"rtkcon");
    SetBooleanOptionCL(vm.count("prob"),vm.count("noprob"),invoke_opt.MACO_ProbabilityAssignment,"prob");
    SetBooleanOptionCL(vm.count("comp"),vm.count("nocomp"),invoke_opt.MACO_CompoundAnalysis,"comp");
    SetBooleanOptionCL(vm.count("phon"),vm.count("nophon"),invoke_opt.PHON_Phonetics,"phon");
    SetBooleanOptionCL(vm.count("nec"),vm.count("nonec"),invoke_opt.NEC_NEClassification,"nec");

    
    // expand environment variables in file names, if any    
    config_opt.TOK_TokenizerFile = freeling::util::expand_filename(config_opt.TOK_TokenizerFile);
    config_opt.SPLIT_SplitterFile = freeling::util::expand_filename(config_opt.SPLIT_SplitterFile);
    config_opt.MACO_UserMapFile = freeling::util::expand_filename(config_opt.MACO_UserMapFile);
    config_opt.MACO_LocutionsFile = freeling::util::expand_filename(config_opt.MACO_LocutionsFile);
    config_opt.MACO_QuantitiesFile = freeling::util::expand_filename(config_opt.MACO_QuantitiesFile);
    config_opt.MACO_AffixFile = freeling::util::expand_filename(config_opt.MACO_AffixFile);
    config_opt.MACO_ProbabilityFile = freeling::util::expand_filename(config_opt.MACO_ProbabilityFile);
    config_opt.MACO_DictionaryFile = freeling::util::expand_filename(config_opt.MACO_DictionaryFile);
    config_opt.MACO_NPDataFile = freeling::util::expand_filename(config_opt.MACO_NPDataFile);
    config_opt.MACO_CompoundFile = freeling::util::expand_filename(config_opt.MACO_CompoundFile);
    config_opt.MACO_PunctuationFile = freeling::util::expand_filename(config_opt.MACO_PunctuationFile);
    config_opt.PHON_PhoneticsFile = freeling::util::expand_filename(config_opt.PHON_PhoneticsFile);
    config_opt.NEC_NECFile = freeling::util::expand_filename(config_opt.NEC_NECFile);
    config_opt.SENSE_ConfigFile = freeling::util::expand_filename(config_opt.SENSE_ConfigFile);
    config_opt.UKB_ConfigFile = freeling::util::expand_filename(config_opt.UKB_ConfigFile);
    config_opt.TAGGER_HMMFile = freeling::util::expand_filename(config_opt.TAGGER_HMMFile);
    config_opt.TAGGER_RelaxFile = freeling::util::expand_filename(config_opt.TAGGER_RelaxFile);
    config_opt.PARSER_GrammarFile = freeling::util::expand_filename(config_opt.PARSER_GrammarFile);
    config_opt.DEP_TxalaFile = freeling::util::expand_filename(config_opt.DEP_TxalaFile);
    config_opt.DEP_TreelerFile = freeling::util::expand_filename(config_opt.DEP_TreelerFile);
    config_opt.DEP_LSTMFile = freeling::util::expand_filename(config_opt.DEP_LSTMFile);
    config_opt.SRL_TreelerFile = freeling::util::expand_filename(config_opt.SRL_TreelerFile);
    config_opt.COREF_CorefFile = freeling::util::expand_filename(config_opt.COREF_CorefFile);
    config_opt.SEMGRAPH_SemGraphFile = freeling::util::expand_filename(config_opt.SEMGRAPH_SemGraphFile);

  }
  
  /// Check if given options make sense. Issue warnings/errors if not

  analyzer_config::status analyzer_config::check_invoke_options(const analyzer_config::invoke_options &opt) const {

    analyzer_config::status st;

    ////  check for errrors
    st.stat = CFG_ERROR;
    if (config_opt.TOK_TokenizerFile.empty() and opt.InputLevel == TEXT and opt.OutputLevel >= TOKEN)
      st.description = L"Tokenizer requested, but it was not instantiated in config options.";
    else if (config_opt.SPLIT_SplitterFile.empty() and opt.InputLevel < SPLITTED and opt.OutputLevel >= SPLITTED)
      st.description = L"Splitter requested, but it was not instantiated in config options.";
    else if (config_opt.MACO_UserMapFile.empty() and config_opt.MACO_PunctuationFile.empty() and
             config_opt.MACO_DictionaryFile.empty() and config_opt.MACO_AffixFile.empty() and
             config_opt.MACO_CompoundFile.empty() and config_opt.MACO_LocutionsFile.empty() and
             config_opt.MACO_NPDataFile.empty() and config_opt.MACO_QuantitiesFile.empty() and
             config_opt.MACO_ProbabilityFile.empty()
             and opt.InputLevel < MORFO and opt.OutputLevel >= MORFO)
      st.description = L"Morphological analysis requested, but it was not instantiated in config options.";

    else if (config_opt.TAGGER_HMMFile.empty() and opt.TAGGER_which==HMM
        and opt.InputLevel < TAGGED and opt.OutputLevel >= TAGGED)
      st.description = L"HMM tagger requested, but it was not instantiated in config options.";
    else if (config_opt.TAGGER_RelaxFile.empty() and opt.TAGGER_which==RELAX
        and opt.InputLevel < TAGGED and opt.OutputLevel >= TAGGED)
      st.description = L"Relax tagger requested, but it was not instantiated in config options.";
    else if (opt.TAGGER_which==NO_TAGGER and opt.InputLevel < TAGGED and opt.OutputLevel >= TAGGED)
      st.description = L"Tagger deactivated, but it is needed for required output analysis level.";
    else if (opt.DEP_which==NO_DEP and opt.InputLevel < DEP and opt.OutputLevel >= DEP)
      st.description = L"Dependency parser deactivated, but it is needed for required output analysis level.";
    else if (config_opt.DEP_TxalaFile.empty() and opt.OutputLevel == PARSED)
      st.description = L"depTxala parser deactivated, but it is needed for required output analysis level.";
    
    else if (config_opt.NEC_NECFile.empty() and opt.NEC_NEClassification)
      st.description = L"NE classification requested, but it was not instantiated in config options.";
    else if (config_opt.PHON_PhoneticsFile.empty() and opt.PHON_Phonetics)
      st.description = L"Phonetic transcription requested, but it was not instantiated in config options.";
    else if (config_opt.SENSE_ConfigFile.empty() and opt.SENSE_WSD_which != NO_WSD) 
      st.description = L"Sense annotation requested, but it was not instantiated in config options.";
    else if (config_opt.UKB_ConfigFile.empty() and opt.SENSE_WSD_which == UKB)
      st.description = L"UKB word sense disambiguation requested, but it was not instantiated in config options.";
    
    else if (config_opt.PARSER_GrammarFile.empty() and opt.InputLevel < SHALLOW and (opt.OutputLevel == PARSED or opt.OutputLevel == SHALLOW))
      st.description = L"Required analysis level requires chart parser, but it was not instantiated in config options";    
    else if (config_opt.PARSER_GrammarFile.empty() and opt.DEP_which==TXALA and opt.InputLevel < SHALLOW  and opt.OutputLevel >= DEP) 
      st.description = L"Required analysis level requires chart parser, but it was not instantiated in config options";
    else if (config_opt.DEP_TxalaFile.empty() and opt.DEP_which==TXALA and opt.InputLevel < DEP and opt.OutputLevel >= DEP)
      st.description = L"Required analysis level requires depTxala parser, but it was not instantiated in config options";
    else if (config_opt.DEP_TreelerFile.empty() and opt.DEP_which==TREELER and opt.InputLevel < DEP and opt.OutputLevel >= DEP)
      st.description = L"Required analysis level requires depTreeler parser, but it was not instantiated in config options";
    else if (config_opt.DEP_LSTMFile.empty() and opt.DEP_which==LSTM and opt.InputLevel < DEP and opt.OutputLevel >= DEP)
      st.description = L"Required analysis level requires depLSTM parser, but it was not instantiated in config options";
    else if (config_opt.DEP_TxalaFile.empty() and config_opt.DEP_TreelerFile.empty() and config_opt.DEP_LSTMFile.empty() and opt.OutputLevel >= DEP) 
      st.description = L"Required analysis level requires a Dependency Parser, but none was instantiated in config options";
    else if (config_opt.SRL_TreelerFile.empty() and opt.SRL_which==SRL_TREELER and opt.InputLevel < SRL and opt.OutputLevel >= SRL)
      st.description = L"Required analysis level requires depTreeler parser, but it was not instantiated in config options";
    else if (config_opt.COREF_CorefFile.empty() and opt.OutputLevel == COREF and opt.InputLevel < COREF) 
      st.description = L"Required analysis level requires Coreference solver, but it was not instantiated in config options";
    else if (config_opt.SEMGRAPH_SemGraphFile.empty() and opt.OutputLevel == SEMGRAPH and opt.InputLevel < SEMGRAPH) 
      st.description = L"Required analysis level requires semantic graph extractor, but it was not instantiated in config options";

    else {
      //// no errors, check for warnings
      st.stat = CFG_WARNING;
      if (opt.InputLevel >= opt.OutputLevel)
        st.description = L"Input and output analysis levels are the same. No analysis will be performed.";
      else if (opt.PHON_Phonetics and opt.OutputLevel < SPLITTED)
        st.description = L"Phonetics requires at least 'splitted' output analysis level.";
      else if (opt.SENSE_WSD_which!=NO_WSD and opt.OutputLevel < MORFO)
        st.description = L"Sense annotation requires at least 'morfo' output analysis level.";
      else if (opt.SENSE_WSD_which==UKB and opt.OutputLevel < TAGGED)
        st.description = L"UKB word sense disambiguation requires at least 'tagged' output analysis level.";
      else if (opt.NEC_NEClassification and opt.OutputLevel < TAGGED)
        st.description = L"NE classification requires at least 'tagged' output analysis level.";
      else if (opt.NEC_NEClassification and not opt.MACO_NERecognition)
        st.description = L"NE classification requires NE recognition.";

      else {        
        /// no warnings, we are good
        st.stat = CFG_OK;
        st.description = L"";
      }
    }

    return st;
  }

  void analyzer_config::SetBooleanOptionCL (const int pos, const int neg, bool &opt, const std::string &name) {
    if (pos && neg) {
      WARNING(L"Ambiguous specification for option --"+util::string2wstring(name)+L" in command line. Using default value.");
    }
    else if (pos)
      opt=true;
    else if (neg)
      opt=false;
    //else: nothing specified, leave things as they are.
  }

  
} // namespace
