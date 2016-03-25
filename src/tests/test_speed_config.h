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

#ifndef _CONFIG
#define _CONFIG

#include <exception>
#include <fstream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "freeling/version.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"CONFIG_OPTIONS"
#define MOD_TRACECODE OPTIONS_TRACE

#define DefaultConfigFile "analyzer.cfg" // default ConfigFile

// Default server parameters
#define DEFAULT_MAX_WORKERS 5   // maximum number of workers simultaneously active.
#define DEFAULT_QUEUE_SIZE 32   // maximum number of waiting clients

// codes for input-output formats
#define PLAIN    0
#define IDENT    1
#define TOKEN    2
#define SPLITTED 3
#define MORFO    4
#define TAGGED   5
#define SENSES   6
#define SHALLOW  7
#define PARSED   8
#define DEP      9

// codes for tagging algorithms
#define HMM   0
#define RELAX 1

// codes for dependency parsers
#define TXALA	0
#define TREELER	1

// codes for sense annotation
#define NONE  0
#define ALL   1
#define MFS   2
#define UKB   3

// codes for ForceSelect
#define FORCE_NONE   0
#define FORCE_TAGGER 1
#define FORCE_RETOK  2

using namespace freeling;

////////////////////////////////////////////////////////////////
///  Class config implements a set of specific options
/// for the NLP analyzer, providing a C++ wrapper to 
/// libcfg+ library.
////////////////////////////////////////////////////////////////

class config {

 public:
  std::string ConfigFile;

  /// Server mode on/off
  bool Server;
  /// port number for server mode  
  int Port; 
  /// Maximum number of workers to fork (i.e. number of simultaneously atended clients)
  int MaxWorkers;
  /// Size of socket queue (number of clients waiting to be atended without being rejected)
  int QueueSize;

  /// Language of text to process
  std::wstring Lang;
  /// Locale of text to process
  std::wstring Locale;
  /// Level of analysis in input and output
  int InputFormat, OutputFormat;
  /// Flush splitter at each line
  bool AlwaysFlush;
  /// produce output in a format suitable to train the tagger.
  bool TrainingOutput;

  /// Tokenizer options
  std::wstring TOK_TokenizerFile;

  /// Splitter options
  std::wstring SPLIT_SplitterFile;

  /// Morphological analyzer options
  bool MACO_UserMap, MACO_AffixAnalysis, MACO_MultiwordsDetection, 
    MACO_NumbersDetection, MACO_PunctuationDetection, 
    MACO_DatesDetection, MACO_QuantitiesDetection, 
    MACO_DictionarySearch, MACO_ProbabilityAssignment, MACO_CompoundAnalysis,
    MACO_NERecognition;

  /// Morphological analyzer options
  std::wstring MACO_Decimal, MACO_Thousand;

  /// Language identifier options
  std::wstring IDENT_identFile;

  /// Tagset to use
  std::wstring TAGSET_TagsetFile;

  /// Morphological analyzer options
  std::wstring MACO_UserMapFile, MACO_LocutionsFile,   MACO_QuantitiesFile,
    MACO_AffixFile,   MACO_ProbabilityFile, MACO_DictionaryFile, 
    MACO_NPDataFile,  MACO_PunctuationFile, MACO_CompoundFile; 
  	 
  double MACO_ProbabilityThreshold;
  bool MACO_RetokContractions;

  // Phonetics options
  bool PHON_Phonetics;
  std::wstring PHON_PhoneticsFile;

  // NEC options
  bool NEC_NEClassification;
  std::wstring NEC_NECFile;

  // Sense annotator options
  int SENSE_WSD_which;
  std::wstring SENSE_ConfigFile;
  std::wstring UKB_ConfigFile;

  /// Tagger options
  std::wstring TAGGER_HMMFile;
  std::wstring TAGGER_RelaxFile;
  int TAGGER_which;
  int TAGGER_RelaxMaxIter;
  double TAGGER_RelaxScaleFactor;
  double TAGGER_RelaxEpsilon;
  bool TAGGER_Retokenize;
  int TAGGER_ForceSelect;

  /// Parser options
  std::wstring PARSER_GrammarFile;

  /// Dependency options
  std::wstring DEP_TxalaFile;   
  std::wstring DEP_TreelerFile;   
  int DEP_which;    
 
  bool COREF_CoreferenceResolution;
  std::wstring COREF_CorefFile;

  /// constructor
  config(int ac, char **av) {

    // Auxiliary variables to store options read as strings before they are converted
    // to their final enumerate/integer values 
    std::string InputF, OutputF, Tagger, SenseAnot, Force, Dep;
    std::string tracemod;
    std::string language, locale, identFile, tagsetFile, tokFile, splitFile,
      macoDecimal, macoThousand, usermapFile, locutionsFile, quantitiesFile, 
      affixFile, probabilityFile, dictionaryFile, npDataFile, punctuationFile,
      compoundFile; 
    std::string phonFile, necFile, senseFile, ukbFile;
    std::string hmmFile,relaxFile,grammarFile,txalaFile,treelerFile,corefFile;

    Port=0;

    po::options_description vis_cl("Available command-line options");
    vis_cl.add_options()
      ("help,h", "Help about command-line options.")
      ("help-cf", "Help about configuration file options.")
#ifndef WIN32
      ("version,v", "Print installed FreeLing version.")
#endif
      ("fcfg,f", po::value<std::string>(&ConfigFile)->default_value(DefaultConfigFile), "Configuration file to use")
      ("lang",po::value<std::string>(&language),"language of the input text")
      ("locale",po::value<std::string>(&locale),"locale encoding of input text (\"default\"=en_US.UTF-8, \"system\"=current system locale, [other]=any valid locale string installed in the system (e.g. ca_ES.UTF-8,it_IT.UTF-8,...)")
      ("server","Activate server mode (default: off)")
      ("port,p",po::value<int>(&Port),"Port where server is to be started")
      ("workers,w",po::value<int>(&MaxWorkers)->default_value(DEFAULT_MAX_WORKERS),"Maximum number of workers to fork in server mode")
      ("queue,q",po::value<int>(&QueueSize)->default_value(DEFAULT_QUEUE_SIZE),"Maximum number of waiting clients.")
      ("flush","Consider each newline as a sentence end")
      ("noflush","Do not consider each newline as a sentence end")
      ("inpf",po::value<std::string>(&InputF),"Input format (plain,token,splitted,morfo,sense,tagged)")
      ("outf",po::value<std::string>(&OutputF),"Output format (ident,token,splitted,morfo,tagged,shallow,parsed,dep)")
      ("train","Produce output format suitable for train scripts")
      ("fidn,I",po::value<std::string>(&identFile),"Language identifier file")
      ("ftok",po::value<std::string>(&tokFile),"Tokenizer rules file")
      ("ftags",po::value<std::string>(&tagsetFile),"Tagset description file")
      ("fsplit",po::value<std::string>(&splitFile),"Splitter option file")
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
      ("dec",po::value<std::string>(&macoDecimal),"Decimal point character")
      ("thou",po::value<std::string>(&macoThousand),"Thousand point character")
      ("fmap,M",po::value<std::string>(&usermapFile),"User-map file")
      ("floc,L",po::value<std::string>(&locutionsFile),"Multiwords file")
      ("fqty,Q",po::value<std::string>(&quantitiesFile),"Quantities file")
      ("fafx,S",po::value<std::string>(&affixFile),"Affix rules file")
      ("fprob,P",po::value<std::string>(&probabilityFile),"Probabilities file")
      ("thres,e",po::value<double>(&MACO_ProbabilityThreshold),"Probability threshold for unknown word tags")
      ("fdict,D",po::value<std::string>(&dictionaryFile),"Form dictionary")
      ("fnp,N",po::value<std::string>(&npDataFile),"NE recognizer data file")
      ("fcomp,K",po::value<std::string>(&compoundFile),"Compound detector configuration file")
      ("fpunct,F",po::value<std::string>(&punctuationFile),"Punctuation symbol file")
      ("nec","Perform NE classification")
      ("nonec","Do not perform NE classification")
      ("fnec",po::value<std::string>(&necFile),"NEC configuration file")
      ("phon","Perform phonetic encoding of words")
      ("nophon","Do not perform phonetic encoding of words")
      ("fphon",po::value<std::string>(&phonFile),"Phonetic encoding configuration file")
      ("sense,s",po::value<std::string>(&SenseAnot),"Type of sense annotation (no|none,all,mfs,ukb)")
      ("fsense,W",po::value<std::string>(&senseFile),"Configuration file for sense annotation module")
      ("fukb,U",po::value<std::string>(&ukbFile),"Configuration file for UKB word sense disambiguator")
      ("hmm,H",po::value<std::string>(&hmmFile),"Data file for HMM tagger")
      ("rlx,R",po::value<std::string>(&relaxFile),"Data file for RELAX tagger")
      ("tag,t",po::value<std::string>(&Tagger),"Tagging alogrithm to use (hmm, relax)")
      ("iter,i",po::value<int>(&TAGGER_RelaxMaxIter),"Maximum number of iterations allowed for RELAX tagger")
      ("sf,r",po::value<double>(&TAGGER_RelaxScaleFactor),"Support scale factor for RELAX tagger (affects step size)")
      ("eps",po::value<double>(&TAGGER_RelaxEpsilon),"Convergence epsilon value for RELAX tagger")
      ("rtk","Perform retokenization after PoS tagging")
      ("nortk","Do not perform retokenization after PoS tagging")
      ("force",po::value<std::string>(&Force),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("grammar,G",po::value<std::string>(&grammarFile),"Grammar file for chart parser")
      ("dep,d",po::value<std::string>(&Dep),"Dependency parser to use (txala,treeler)")
      ("txala,T",po::value<std::string>(&txalaFile),"Rule file for Txala dependency parser")
      ("treeler,E",po::value<std::string>(&treelerFile),"Configuration file for Treeler dependency parser")
      ("coref","Perform coreference resolution")
      ("nocoref","Do not perform coreference resolution")
      ("fcorf,C",po::value<std::string>(&corefFile),"Coreference solver data file")
      ;
 
    po::options_description vis_cf("Available configuration file options");
    vis_cf.add_options()
      ("Lang",po::value<std::string>(&language),"Language of the input text")
      ("Locale",po::value<std::string>(&locale)->default_value("default"),"locale encoding of input text (\"default\"=en_US.UTF-8, \"system\"=current system locale, [other]=any valid locale string installed in the system (e.g. ca_ES.UTF-8,it_IT.UTF-8,...)")
      ("ServerMode",po::value<bool>(&Server)->default_value(false),"Activate server mode (default: off)")
      ("ServerPort",po::value<int>(&Port),"Port where server is to be started")
      ("ServerMaxWorkers",po::value<int>(&MaxWorkers)->default_value(DEFAULT_MAX_WORKERS),"Maximum number of workers to fork in server mode")
      ("ServerQueueSize",po::value<int>(&QueueSize)->default_value(DEFAULT_QUEUE_SIZE),"Maximum number of waiting requests in server mode")
      ("AlwaysFlush",po::value<bool>(&AlwaysFlush)->default_value(false),"Consider each newline as a sentence end")
      ("InputFormat",po::value<std::string>(&InputF)->default_value("plain"),"Input format (plain,token,splitted,morfo,sense,tagged)")
      ("OutputFormat",po::value<std::string>(&OutputF)->default_value("tagged"),"Output format (token,splitted,morfo,tagged,shallow,parsed,dep)")
      ("LangIdentFile",po::value<std::string>(&identFile),"Language identifier file")
      ("TokenizerFile",po::value<std::string>(&tokFile),"Tokenizer rules file")
      ("TagsetFile",po::value<std::string>(&tagsetFile),"Tagset description file")
      ("SplitterFile",po::value<std::string>(&splitFile),"Splitter option file")
      ("AffixAnalysis",po::value<bool>(&MACO_AffixAnalysis)->default_value(false),"Perform affix analysis")
      ("UserMap",po::value<bool>(&MACO_UserMap)->default_value(false),"Apply user mapping file")
      ("MultiwordsDetection",po::value<bool>(&MACO_MultiwordsDetection)->default_value(false),"Perform multiword detection")
      ("NumbersDetection",po::value<bool>(&MACO_NumbersDetection)->default_value(false),"Perform number detection")
      ("PunctuationDetection",po::value<bool>(&MACO_PunctuationDetection)->default_value(false),"Perform punctuation detection")
      ("DatesDetection",po::value<bool>(&MACO_DatesDetection)->default_value(false),"Perform date/time expression detection")
      ("QuantitiesDetection",po::value<bool>(&MACO_QuantitiesDetection)->default_value(false),"Perform magnitude/ratio detection")
      ("DictionarySearch",po::value<bool>(&MACO_DictionarySearch)->default_value(false),"Perform dictionary search")
      ("RetokContractions",po::value<bool>(&MACO_RetokContractions)->default_value(true),"Dictionary retokenizes contractions regardless of --nortk option")
      ("ProbabilityAssignment",po::value<bool>(&MACO_ProbabilityAssignment)->default_value(false),"Perform probability assignment")
      ("CompoundAnalysis",po::value<bool>(&MACO_CompoundAnalysis)->default_value(false),"Perform compound analysis")
      ("NERecognition",po::value<bool>(&MACO_NERecognition)->default_value(false),"Perform NE recognition")
      ("DecimalPoint",po::value<std::string>(&macoDecimal),"Decimal point character")
      ("ThousandPoint",po::value<std::string>(&macoThousand),"Thousand point character")
      ("UserMapFile",po::value<std::string>(&usermapFile),"User mapping file")
      ("LocutionsFile",po::value<std::string>(&locutionsFile),"Multiwords file")
      ("QuantitiesFile",po::value<std::string>(&quantitiesFile),"Quantities file")
      ("AffixFile",po::value<std::string>(&affixFile),"Affix rules file")

      ("ProbabilityFile",po::value<std::string>(&probabilityFile),"Probabilities file")
      ("ProbabilityThreshold",po::value<double>(&MACO_ProbabilityThreshold),"Probability threshold for unknown word tags")
      ("DictionaryFile",po::value<std::string>(&dictionaryFile),"Form dictionary")
      ("NPDataFile",po::value<std::string>(&npDataFile),"NP recognizer data file")
      ("CompoundFile",po::value<std::string>(&compoundFile),"Compound detector configuration file")
      ("PunctuationFile",po::value<std::string>(&punctuationFile),"Punctuation symbol file")
      ("Phonetics",po::value<bool>(&PHON_Phonetics)->default_value(false),"Perform phonetic encoding of words")
      ("PhoneticsFile",po::value<std::string>(&phonFile),"Phonetic encoding configuration file")
      ("NEClassification",po::value<bool>(&NEC_NEClassification)->default_value(false),"Perform NE classification")
      ("NECFile",po::value<std::string>(&necFile),"NEC configuration file")
      ("SenseAnnotation",po::value<std::string>(&SenseAnot)->default_value("none"),"Type of sense annotation (no|none,all,mfs,ukb)")
      ("SenseConfigFile",po::value<std::string>(&senseFile),"Configuration file for sense annotation module")
      ("UKBConfigFile",po::value<std::string>(&ukbFile),"Configuration file for UKB word sense disambiguator")
      ("TaggerHMMFile",po::value<std::string>(&hmmFile),"Data file for HMM tagger")
      ("TaggerRelaxFile",po::value<std::string>(&relaxFile),"Data file for RELAX tagger")
      ("Tagger",po::value<std::string>(&Tagger)->default_value("hmm"),"Tagging alogrithm to use (hmm, relax)")
      ("TaggerRelaxMaxIter",po::value<int>(&TAGGER_RelaxMaxIter),"Maximum number of iterations allowed for RELAX tagger")
      ("TaggerRelaxScaleFactor",po::value<double>(&TAGGER_RelaxScaleFactor),"Support scale factor for RELAX tagger (affects step size)")
      ("TaggerRelaxEpsilon",po::value<double>(&TAGGER_RelaxEpsilon),"Convergence epsilon value for RELAX tagger")
      ("TaggerRetokenize",po::value<bool>(&TAGGER_Retokenize)->default_value(false),"Perform retokenization after PoS tagging")
      ("TaggerForceSelect",po::value<std::string>(&Force)->default_value("retok"),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("GrammarFile",po::value<std::string>(&grammarFile),"Grammar file for chart parser")
      ("DependencyParser",po::value<std::string>(&Dep),"Dependency parser to use (txala,treeler)")
      ("DepTxalaFile",po::value<std::string>(&txalaFile),"Rule file for Txala dependency parser")
      ("DepTreelerFile",po::value<std::string>(&treelerFile),"Configuration file for Treeler dependency parser")
      ("CoreferenceResolution",po::value<bool>(&COREF_CoreferenceResolution)->default_value(false),"Perform coreference resolution")
      ("CorefFile",po::value<std::string>(&corefFile),"Coreference solver data file")
      ;

    po::options_description hid_cl("Hidden CL options");
    hid_cl.add_options()
      ("tlevel,l",po::value<int>(&traces::TraceLevel),"Debug traces verbosity")
      ("tmod,m",po::value<std::string>(&tracemod),"Mask indicating which modules to trace")
      ;

    po::options_description hid_cf("Hidden CF options");
    hid_cf.add_options()
      ("TraceLevel",po::value<int>(&traces::TraceLevel)->default_value(0),"Debug traces verbosity")
      ("TraceModule",po::value<std::string>(&tracemod)->default_value("0x0"),"Mask indicating which modules to trace")
      ;

    po::options_description cl_op("All command line options");
    cl_op.add(vis_cl).add(hid_cl);

    po::options_description cf_op("All configuration file options");
    cf_op.add(vis_cf).add(hid_cf);

    po::variables_map vm;
    try {
      po::store(po::parse_command_line(ac, av, cl_op), vm);
      po::notify(vm);    
    } 
    catch (exception &e) {
      std::cerr<<"Error while parsing command line: "<<e.what()<<std::endl;
      exit(1);
    }

    // Version required
    if (vm.count("version")) {
      #ifndef FREELING_VERSION
        std::cerr<<"Option '--version' not available in this executable."<<std::endl;
      #else
        std::cerr<<FREELING_VERSION<<std::endl;
      #endif
      exit(0); // return to system
    }

    // Help screen required
    if (vm.count("help")) {
      std::cerr<<vis_cl<<std::endl;
      exit(0); // return to system
    }

    // Help screen required
    if (vm.count("help-cf")) {
      std::cerr<<vis_cf<<std::endl;
      exit(0); // return to system
    }

    // Load config file.
    std::wifstream fcfg;
    util::open_utf8_file(fcfg,util::string2wstring(ConfigFile));
    if (fcfg.fail()) {
      // if the file is not found, report the error, giving the absolute path to the file.
      string cwd=ConfigFile;
      if (not util::is_absolute(cwd)) cwd = util::absolute(cwd,util::get_current_path());
      std::cerr<<"Can not open config file '"+cwd+"'."<<std::endl;
      exit(1);
    }
    try {
      po::store(po::parse_config_file(fcfg, cf_op), vm);
      po::notify(vm);    
    }
    catch (exception &e) {
      std::cerr<<"Error while parsing configuration file: "<<e.what()<<std::endl;
      exit(1);
    }
    
    // check options involving Filenames for environment vars expansion.
    identFile = util::expand_filename(identFile);
    tokFile = util::expand_filename(tokFile);
    tagsetFile = util::expand_filename(tagsetFile);
    splitFile = util::expand_filename(splitFile);
    usermapFile = util::expand_filename(usermapFile);
    locutionsFile = util::expand_filename(locutionsFile);
    quantitiesFile = util::expand_filename(quantitiesFile);
    affixFile = util::expand_filename(affixFile);
    probabilityFile = util::expand_filename(probabilityFile);
    dictionaryFile = util::expand_filename(dictionaryFile); 
    npDataFile = util::expand_filename(npDataFile);
    punctuationFile = util::expand_filename(punctuationFile);
    compoundFile = util::expand_filename(compoundFile);
    phonFile = util::expand_filename(phonFile); 
    necFile = util::expand_filename(necFile); 
    senseFile = util::expand_filename(senseFile); 
    ukbFile = util::expand_filename(ukbFile); 
    hmmFile = util::expand_filename(hmmFile);
    relaxFile = util::expand_filename(relaxFile); 
    grammarFile = util::expand_filename(grammarFile); 
    txalaFile = util::expand_filename(txalaFile);
    treelerFile = util::expand_filename(treelerFile);
    corefFile = util::expand_filename(corefFile); 

    // translate string options (including expanded filenames) to wstrings
    Lang = util::string2wstring(language);
    Locale = util::string2wstring(locale);
    IDENT_identFile = util::string2wstring(identFile);
    TOK_TokenizerFile = util::string2wstring(tokFile);
    TAGSET_TagsetFile = util::string2wstring(tagsetFile);
    SPLIT_SplitterFile = util::string2wstring(splitFile);
    MACO_Decimal = util::string2wstring(macoDecimal);
    MACO_Thousand = util::string2wstring(macoThousand);
    MACO_UserMapFile = util::string2wstring(usermapFile);
    MACO_LocutionsFile = util::string2wstring(locutionsFile);
    MACO_QuantitiesFile = util::string2wstring(quantitiesFile);
    MACO_AffixFile = util::string2wstring(affixFile);
    MACO_ProbabilityFile = util::string2wstring(probabilityFile);
    MACO_DictionaryFile = util::string2wstring(dictionaryFile);
    MACO_NPDataFile = util::string2wstring(npDataFile);
    MACO_PunctuationFile = util::string2wstring(punctuationFile);
    MACO_CompoundFile = util::string2wstring(compoundFile);
    PHON_PhoneticsFile = util::string2wstring(phonFile);
    NEC_NECFile = util::string2wstring(necFile);
    SENSE_ConfigFile = util::string2wstring(senseFile);
    UKB_ConfigFile = util::string2wstring(ukbFile);
    TAGGER_HMMFile = util::string2wstring(hmmFile);
    TAGGER_RelaxFile = util::string2wstring(relaxFile);
    PARSER_GrammarFile = util::string2wstring(grammarFile);
    DEP_TxalaFile = util::string2wstring(txalaFile);
    DEP_TreelerFile = util::string2wstring(treelerFile);
    COREF_CorefFile = util::string2wstring(corefFile);
    
    // Handle boolean options expressed with --myopt or --nomyopt in command line
    SetBooleanOptionCL(vm.count("server"),!vm.count("server"),Server,"server");
    SetBooleanOptionCL(vm.count("train"),!vm.count("train"),TrainingOutput,"train");
    SetBooleanOptionCL(vm.count("flush"),vm.count("noflush"),AlwaysFlush,"flush");
    SetBooleanOptionCL(vm.count("afx"),vm.count("noafx"),MACO_AffixAnalysis,"afx");
    SetBooleanOptionCL(vm.count("usr"),vm.count("nousr"), MACO_UserMap,"usr");
    SetBooleanOptionCL(vm.count("loc"),vm.count("noloc"), MACO_MultiwordsDetection,"loc");
    SetBooleanOptionCL(vm.count("numb"),vm.count("nonumb"),MACO_NumbersDetection,"numb");
    SetBooleanOptionCL(vm.count("punt"),vm.count("nopunt"),MACO_PunctuationDetection,"punt");
    SetBooleanOptionCL(vm.count("date"),vm.count("nodate"),MACO_DatesDetection,"date");
    SetBooleanOptionCL(vm.count("ner"),vm.count("noner"),MACO_NERecognition,"ner");
    SetBooleanOptionCL(vm.count("quant"),vm.count("noquant"),MACO_QuantitiesDetection,"quant");
    SetBooleanOptionCL(vm.count("dict"),vm.count("nodict"),MACO_DictionarySearch,"dict");
    SetBooleanOptionCL(vm.count("rtkcon"),vm.count("nortkcon"),MACO_RetokContractions,"rtkcon");
    SetBooleanOptionCL(vm.count("prob"),vm.count("noprob"),MACO_ProbabilityAssignment,"prob");
    SetBooleanOptionCL(vm.count("comp"),vm.count("nocomp"),MACO_CompoundAnalysis,"comp");
    SetBooleanOptionCL(vm.count("phon"),vm.count("nophon"),PHON_Phonetics,"phon");
    SetBooleanOptionCL(vm.count("nec"),vm.count("nonec"),NEC_NEClassification,"nec");
    SetBooleanOptionCL(vm.count("rtk"),vm.count("nortk"),TAGGER_Retokenize,"rtk");
    SetBooleanOptionCL(vm.count("coref"),vm.count("nocoref"),COREF_CoreferenceResolution,"coref");
    
    // translate InputF and OutputF strings to more useful integer values.
    if (InputF=="plain") InputFormat = PLAIN;
    else if (InputF=="token") InputFormat = TOKEN;
    else if (InputF=="splitted") InputFormat = SPLITTED;
    else if (InputF=="morfo") InputFormat = MORFO;
    else if (InputF=="tagged") InputFormat = TAGGED;
    else if (InputF=="sense") InputFormat = SENSES;
    else { ERROR_CRASH(L"Unknown or invalid input format: "+util::string2wstring(InputF));}
    
    if (OutputF=="ident") OutputFormat = IDENT;
    else if (OutputF=="token") OutputFormat = TOKEN;
    else if (OutputF=="splitted") OutputFormat = SPLITTED;
    else if (OutputF=="morfo") OutputFormat = MORFO;
    else if (OutputF=="tagged") OutputFormat = TAGGED;
    else if (OutputF=="shallow") OutputFormat = SHALLOW;
    else if (OutputF=="parsed") OutputFormat = PARSED;
    else if (OutputF=="dep") OutputFormat = DEP;
    else { ERROR_CRASH(L"Unknown or invalid output format: "+util::string2wstring(OutputF));}
    
    // translate Tagger string to more useful integer values.
    if (Tagger=="hmm") TAGGER_which = HMM;
    else if (Tagger=="relax") TAGGER_which = RELAX;
    else {
      TAGGER_which = HMM;
      WARNING(L"Invalid tagger algorithm '"+util::string2wstring(Tagger)+L"'. Using default.");
    }
    
    // Translate ForceSelect string to more useful integer values.
    if (Force=="none" || Force=="no") TAGGER_ForceSelect = FORCE_NONE;
    else if (Force=="tagger") TAGGER_ForceSelect = FORCE_TAGGER;
    else if (Force=="retok") TAGGER_ForceSelect = FORCE_RETOK;
    else {
      TAGGER_ForceSelect = FORCE_RETOK;
      WARNING(L"Invalid ForceSelect value '"+util::string2wstring(Force)+L"'. Using default.");
    }    
    
    // translate SenseAnot string to more useful integer values.
    if (SenseAnot=="none" || SenseAnot=="no") SENSE_WSD_which = NONE;
    else if (SenseAnot=="all") SENSE_WSD_which = ALL;
    else if (SenseAnot=="mfs") SENSE_WSD_which = MFS;
    else if (SenseAnot=="ukb") SENSE_WSD_which = UKB;
    else {
      SENSE_WSD_which = NONE;
      WARNING(L"Invalid sense annotation option '"+util::string2wstring(SenseAnot)+L"'. Using default.");
    }

    // translate Dep string to more useful integer values.
    if (Dep=="txala") DEP_which = TXALA;
    else if (Dep=="treeler") DEP_which = TREELER;
    else {
      DEP_which = TXALA;
      WARNING(L"Invalid dependency parser '"+util::string2wstring(Dep)+L"'. Using default.");
    }
    
    // translate tracmod string (hex) into traces::TraceModule unsinged long
    std::stringstream sin;
    sin << std::hex << tracemod;
    sin >> traces::TraceModule;
  }
  
 private:
    
  void SetBooleanOptionCL (const int pos, const int neg, bool &opt, const std::string &name) {
    if (pos && neg) {
      WARNING(L"Ambiguous specification for option --"+util::string2wstring(name)+L" in command line. Using default value.");
    }
    else if (pos)
      opt=true;
    else if (neg)
      opt=false;
    //else: nothing specified, leave things as they are.
  }

};


#endif

