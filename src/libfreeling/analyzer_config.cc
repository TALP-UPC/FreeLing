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
  
  analyzer_config::analyzer_config() : config_opt(), invoke_opt() {

    cl_opts.add_options()
      ("lang",po::wvalue<std::wstring>(&config_opt.Lang),"language of the input text")
      ("inplv",po::wvalue<freeling::AnalysisLevel>(&invoke_opt.InputLevel)->default_value(freeling::TEXT),"Input analysis level (text,token,splitted,morfo,tagged,shallow,dep,srl,coref)")
      ("outlv",po::wvalue<freeling::AnalysisLevel>(&invoke_opt.OutputLevel)->default_value(freeling::TAGGED),"Output analysis level (token,splitted,morfo,tagged,shallow,parsed,dep,srl,coref,semgraph)")
      ("ftok",po::wvalue<std::wstring>(&config_opt.TOK_TokenizerFile),"Tokenizer rules file")
      ("fsplit",po::wvalue<std::wstring>(&config_opt.SPLIT_SplitterFile),"Splitter option file")
      ("afx","Perform affix analysis")
      ("noafx","Do not perform affix analysis")
      ("usr","Apply user mapping file")
      ("nousr","Do not apply user mapping file")
      ("loc","Perform multiword detection")
      ("noloc","Do not perform multiword detection")
      ("numb","Perform number detection")
      ("nonumb","Do not perform number detection")
      ("punt","Perform punctuation detection")
      ("nopunt","Do not perform punctuation detection")
      ("date","Perform date/time expression detection")
      ("nodate","Do not perform date/time expression detection")
      ("quant","Perform magnitude/ratio detection")
      ("noquant","Do not perform magnitude/ratio detection")
      ("dict","Perform dictionary search")
      ("nodict","Do not perform dictionary search")
      ("prob","Perform probability assignment")
      ("noprob","Do not perform probability assignment")
      ("rtkcon","Dictionary retokenizes contractions regardless of --nortk option")
      ("nortkcon","Dictionary leaves contraction retokenization to --rtk/nortk option")
      ("comp","Perform compound analysis")
      ("nocomp","Do not perform compound analysis")
      ("ner","Perform NE recognition")
      ("noner","Do not perform NE recognition")
      ("dec",po::wvalue<std::wstring>(&config_opt.MACO_Decimal),"Decimal point character")
      ("thou",po::wvalue<std::wstring>(&config_opt.MACO_Thousand),"Thousand point character")
      ("fmap,M",po::wvalue<std::wstring>(&config_opt.MACO_UserMapFile),"User-map file")
      ("floc,L",po::wvalue<std::wstring>(&config_opt.MACO_LocutionsFile),"Multiwords file")
      ("fqty,Q",po::wvalue<std::wstring>(&config_opt.MACO_QuantitiesFile),"Quantities file")
      ("fafx,S",po::wvalue<std::wstring>(&config_opt.MACO_AffixFile),"Affix rules file")
      ("fprob,P",po::wvalue<std::wstring>(&config_opt.MACO_ProbabilityFile),"Probabilities file")
      ("thres,e",po::wvalue<double>(&config_opt.MACO_ProbabilityThreshold),"Probability threshold for unknown word tags")
      ("fdict,D",po::wvalue<std::wstring>(&config_opt.MACO_DictionaryFile),"Form dictionary")
      ("fnp,N",po::wvalue<std::wstring>(&config_opt.MACO_NPDataFile),"NE recognizer data file")
      ("fcomp,K",po::wvalue<std::wstring>(&config_opt.MACO_CompoundFile),"Compound detector configuration file")
      ("fpunct,F",po::wvalue<std::wstring>(&config_opt.MACO_PunctuationFile),"Punctuation symbol file")
      ("phon","Perform phonetic encoding of words")
      ("nophon","Do not perform phonetic encoding of words")
      ("fphon",po::wvalue<std::wstring>(&config_opt.PHON_PhoneticsFile),"Phonetic encoding configuration file")
      ("nec","Perform NE classification")
      ("nonec","Do not perform NE classification")
      ("fnec",po::wvalue<std::wstring>(&config_opt.NEC_NECFile),"NEC configuration file")
      ("sense,s",po::wvalue<freeling::WSDAlgorithm>(&invoke_opt.SENSE_WSD_which),"Type of sense annotation (no|none,all,mfs,ukb)")
      ("fsense,W",po::wvalue<std::wstring>(&config_opt.SENSE_ConfigFile),"Configuration file for sense annotation module")
      ("fukb,U",po::wvalue<std::wstring>(&config_opt.UKB_ConfigFile),"Configuration file for UKB word sense disambiguator")
      ("hmm,H",po::wvalue<std::wstring>(&config_opt.TAGGER_HMMFile),"Data file for HMM tagger")
      ("rlx,R",po::wvalue<std::wstring>(&config_opt.TAGGER_RelaxFile),"Data file for RELAX tagger")
      ("tag,t",po::wvalue<freeling::TaggerAlgorithm>(&invoke_opt.TAGGER_which),"Tagging alogrithm to use (hmm, relax)")
      ("iter,i",po::wvalue<int>(&config_opt.TAGGER_RelaxMaxIter),"Maximum number of iterations allowed for RELAX tagger")
      ("sf,r",po::wvalue<double>(&config_opt.TAGGER_RelaxScaleFactor),"Support scale factor for RELAX tagger (affects step size)")
      ("eps",po::wvalue<double>(&config_opt.TAGGER_RelaxEpsilon),"Convergence epsilon value for RELAX tagger")
      ("rtk","Perform retokenization after PoS tagging")
      ("nortk","Do not perform retokenization after PoS tagging")
      ("force",po::wvalue<freeling::ForceSelectStrategy>(&config_opt.TAGGER_ForceSelect),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("grammar,G",po::wvalue<std::wstring>(&config_opt.PARSER_GrammarFile),"Grammar file for chart parser")
      ("dep,d",po::wvalue<freeling::DependencyParser>(&invoke_opt.DEP_which),"Dependency parser to use (txala,treeler,lstm)")
      ("srl",po::wvalue<freeling::SRLParser>(&invoke_opt.SRL_which),"SRL parser to use (treeler)")
      ("txala,T",po::wvalue<std::wstring>(&config_opt.DEP_TxalaFile),"Rule file for Txala dependency parser")
      ("treeler,E",po::wvalue<std::wstring>(&config_opt.DEP_TreelerFile),"Configuration file for Treeler dependency parser")
      ("lstm",po::wvalue<std::wstring>(&config_opt.DEP_LSTMFile),"Configuration file for LSTM dependency parser")
      ("SRLtreeler",po::wvalue<std::wstring>(&config_opt.SRL_TreelerFile),"Configuration file for SRL treeler parser")
      ("fcorf,C",po::wvalue<std::wstring>(&config_opt.COREF_CorefFile),"Coreference solver data file")
      ("fsge,g",po::wvalue<std::wstring>(&config_opt.SEMGRAPH_SemGraphFile),"Semantic graph extractor config file")
      ;

    cf_opts.add_options()
      ("Lang",po::wvalue<std::wstring>(&config_opt.Lang),"Language of the input text")
      ("InputLevel",po::wvalue<freeling::AnalysisLevel>(&invoke_opt.InputLevel)->default_value(freeling::TEXT),"Input analysis level (text,token,splitted,morfo,tagged,shallow,dep,srl,coref)")
      ("OutputLevel",po::wvalue<freeling::AnalysisLevel>(&invoke_opt.OutputLevel)->default_value(freeling::TAGGED),"Output analysis level (token,splitted,morfo,tagged,shallow,parsed,dep,srl,coref,semgraph)")
      ("TokenizerFile",po::wvalue<std::wstring>(&config_opt.TOK_TokenizerFile),"Tokenizer rules file")
      ("SplitterFile",po::wvalue<std::wstring>(&config_opt.SPLIT_SplitterFile),"Splitter option file")
      ("AffixAnalysis",po::wvalue<bool>(&invoke_opt.MACO_AffixAnalysis)->default_value(false),"Perform affix analysis")
      ("UserMap",po::wvalue<bool>(&invoke_opt.MACO_UserMap)->default_value(false),"Apply user mapping file")
      ("MultiwordsDetection",po::wvalue<bool>(&invoke_opt.MACO_MultiwordsDetection)->default_value(false),"Perform multiword detection")
      ("NumbersDetection",po::wvalue<bool>(&invoke_opt.MACO_NumbersDetection)->default_value(false),"Perform number detection")
      ("PunctuationDetection",po::wvalue<bool>(&invoke_opt.MACO_PunctuationDetection)->default_value(false),"Perform punctuation detection")
      ("DatesDetection",po::wvalue<bool>(&invoke_opt.MACO_DatesDetection)->default_value(false),"Perform date/time expression detection")
      ("QuantitiesDetection",po::wvalue<bool>(&invoke_opt.MACO_QuantitiesDetection)->default_value(false),"Perform magnitude/ratio detection")
      ("DictionarySearch",po::wvalue<bool>(&invoke_opt.MACO_DictionarySearch)->default_value(false),"Perform dictionary search")
      ("RetokContractions",po::wvalue<bool>(&invoke_opt.MACO_RetokContractions)->default_value(true),"Dictionary retokenizes contractions regardless of --nortk option")
      ("ProbabilityAssignment",po::wvalue<bool>(&invoke_opt.MACO_ProbabilityAssignment)->default_value(false),"Perform probability assignment")
      ("CompoundAnalysis",po::wvalue<bool>(&invoke_opt.MACO_CompoundAnalysis)->default_value(false),"Perform compound analysis")
      ("NERecognition",po::wvalue<bool>(&invoke_opt.MACO_NERecognition)->default_value(false),"Perform NE recognition")
      ("DecimalPoint",po::wvalue<std::wstring>(&config_opt.MACO_Decimal),"Decimal point character")
      ("ThousandPoint",po::wvalue<std::wstring>(&config_opt.MACO_Thousand),"Thousand point character")
      ("UserMapFile",po::wvalue<std::wstring>(&config_opt.MACO_UserMapFile),"User mapping file")
      ("LocutionsFile",po::wvalue<std::wstring>(&config_opt.MACO_LocutionsFile),"Multiwords file")
      ("QuantitiesFile",po::wvalue<std::wstring>(&config_opt.MACO_QuantitiesFile),"Quantities file")
      ("AffixFile",po::wvalue<std::wstring>(&config_opt.MACO_AffixFile),"Affix rules file")    
      ("ProbabilityFile",po::wvalue<std::wstring>(&config_opt.MACO_ProbabilityFile),"Probabilities file")
      ("ProbabilityThreshold",po::wvalue<double>(&config_opt.MACO_ProbabilityThreshold),"Probability threshold for unknown word tags")
      ("DictionaryFile",po::wvalue<std::wstring>(&config_opt.MACO_DictionaryFile),"Form dictionary")
      ("NPDataFile",po::wvalue<std::wstring>(&config_opt.MACO_NPDataFile),"NP recognizer data file")
      ("CompoundFile",po::wvalue<std::wstring>(&config_opt.MACO_CompoundFile),"Compound detector configuration file")
      ("PunctuationFile",po::wvalue<std::wstring>(&config_opt.MACO_PunctuationFile),"Punctuation symbol file")
      ("Phonetics",po::wvalue<bool>(&invoke_opt.PHON_Phonetics)->default_value(false),"Perform phonetic encoding of words")
      ("PhoneticsFile",po::wvalue<std::wstring>(&config_opt.PHON_PhoneticsFile),"Phonetic encoding configuration file")
      ("NEClassification",po::wvalue<bool>(&invoke_opt.NEC_NEClassification)->default_value(false),"Perform NE classification")
      ("NECFile",po::wvalue<std::wstring>(&config_opt.NEC_NECFile),"NEC configuration file")
      ("SenseAnnotation",po::wvalue<freeling::WSDAlgorithm>(&invoke_opt.SENSE_WSD_which)->default_value(freeling::NO_WSD),"Type of sense annotation (no|none,all,mfs,ukb)")
      ("SenseConfigFile",po::wvalue<std::wstring>(&config_opt.SENSE_ConfigFile),"Configuration file for sense annotation module")
      ("UKBConfigFile",po::wvalue<std::wstring>(&config_opt.UKB_ConfigFile),"Configuration file for UKB word sense disambiguator")
      ("TaggerHMMFile",po::wvalue<std::wstring>(&config_opt.TAGGER_HMMFile),"Data file for HMM tagger")
      ("TaggerRelaxFile",po::wvalue<std::wstring>(&config_opt.TAGGER_RelaxFile),"Data file for RELAX tagger")
      ("Tagger",po::wvalue<freeling::TaggerAlgorithm>(&invoke_opt.TAGGER_which)->default_value(freeling::HMM),"Tagging alogrithm to use (hmm, relax)")
      ("TaggerRelaxMaxIter",po::wvalue<int>(&config_opt.TAGGER_RelaxMaxIter),"Maximum number of iterations allowed for RELAX tagger")
      ("TaggerRelaxScaleFactor",po::wvalue<double>(&config_opt.TAGGER_RelaxScaleFactor),"Support scale factor for RELAX tagger (affects step size)")
      ("TaggerRelaxEpsilon",po::wvalue<double>(&config_opt.TAGGER_RelaxEpsilon),"Convergence epsilon value for RELAX tagger")
      ("TaggerRetokenize",po::wvalue<bool>(&config_opt.TAGGER_Retokenize)->default_value(false),"Perform retokenization after PoS tagging")
      ("TaggerForceSelect",po::wvalue<freeling::ForceSelectStrategy>(&config_opt.TAGGER_ForceSelect)->default_value(freeling::RETOK),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("GrammarFile",po::wvalue<std::wstring>(&config_opt.PARSER_GrammarFile),"Grammar file for chart parser")
      ("DependencyParser",po::wvalue<freeling::DependencyParser>(&invoke_opt.DEP_which)->default_value(freeling::TXALA),"Dependency parser to use (txala,treeler,lstm)")
      ("DepTxalaFile",po::wvalue<std::wstring>(&config_opt.DEP_TxalaFile),"Rule file for Txala dependency parser")
      ("DepTreelerFile",po::wvalue<std::wstring>(&config_opt.DEP_TreelerFile),"Configuration file for Treeler dependency parser")
      ("DepLSTMFile",po::wvalue<std::wstring>(&config_opt.DEP_LSTMFile),"Configuration file for LSTM dependency parser")
      ("SRLParser",po::wvalue<freeling::SRLParser>(&invoke_opt.SRL_which)->default_value(freeling::SRL_TREELER),"SRL parser to use (treeler)")
      ("SRLTreelerFile",po::wvalue<std::wstring>(&config_opt.SRL_TreelerFile),"Configuration file for Treeler SRL parser")
      ("CorefFile",po::wvalue<std::wstring>(&config_opt.COREF_CorefFile),"Coreference solver data file")
      ("SemGraphExtractorFile",po::wvalue<std::wstring>(&config_opt.SEMGRAPH_SemGraphFile),"Semantic graph extractor config file")
      ;

  }

  /// destructor
  
  analyzer_config::~analyzer_config() {}

  /// acces to option descriptions, in case user want to add options

  po::options_description& analyzer_config::command_line_options() { return cl_opts; }

  /// acces to option descriptions, in case user want to add options

  po::options_description& analyzer_config::config_file_options() { return cf_opts; }
 

  /// load options from a config file, update variables map
  
  void analyzer_config::parse_options(const wstring &cfgFile, po::variables_map &vm) {
    std::wifstream fcfg;
    freeling::util::open_utf8_file(fcfg,cfgFile);
    if (fcfg.fail()) ERROR_CRASH(L"Can not open config file '"<<cfgFile<<"'");

    try {
      po::store(po::parse_config_file(fcfg, cf_opts), vm);
      po::notify(vm);
    }
    catch (exception &e) {
      ERROR_CRASH(L"Error while parsing configuration file: "<<util::string2wstring(e.what()));
    }    

    expand_options(vm);
  }

  
  /// load options from a config file, ignore variables map

  void analyzer_config::parse_options(const wstring &cfgFile) {
    po::variables_map vm;
    parse_options(cfgFile, vm);
  }
  
  /// load options from command line, update variables map
  
  void analyzer_config::parse_options(int ac, char *av[], po::variables_map &vm) {
    try {
      po::store(po::parse_command_line(ac, av, cl_opts), vm);
      po::notify(vm);    
    } 
    catch (exception &e) {
      ERROR_CRASH("Error while parsing command line: "<<e.what());
    }
    expand_options(vm);
  }

  /// load options from command line, ignore variables map
  
  void analyzer_config::parse_options(int ac, char *av[]) {
    po::variables_map vm;
    parse_options(ac, av, vm);
  }
  
  /// load options from a config file + command line, update variables map
  void analyzer_config::parse_options(const wstring &cfgFile, int ac, char *av[], po::variables_map &vm) {
    parse_options(ac, av, vm);
    parse_options(cfgFile, vm);
  }

  /// load options from a config file + command line, ignore variables map
  void analyzer_config::parse_options(const wstring &cfgFile, int ac, char *av[]) {
    po::variables_map vm;
    parse_options(cfgFile, ac, av, vm);
  }

  
  /// expand paths in filenames, and handle boolean command line options --xx/--noxx

  void analyzer_config::expand_options(const po::variables_map &vm) {
    // expand environment variables in filenames
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

  }
  
  analyzer_config::status analyzer_config::check_invoke_options(const analyzer_config::invoke_options &opt) const {
    // Check if given options make sense. Issue warnings/errors if not

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
