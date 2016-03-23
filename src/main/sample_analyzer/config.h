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

#include "freeling/version.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/analyzer.h"

#define MOD_TRACENAME L"CONFIG_OPTIONS"

// Default server parameters
#define DEFAULT_MAX_WORKERS 5   // maximum number of workers simultaneously active.
#define DEFAULT_QUEUE_SIZE 32   // maximum number of waiting clients

// codes for InputMode
typedef enum {MODE_CORPUS,MODE_DOC} InputModes;
// codes for OutputFormat
typedef enum {OUT_FREELING,OUT_TRAIN,OUT_CONLL,OUT_XML,OUT_JSON,OUT_NAF} OutputFormats;
// codes for InputFormat
typedef enum {INP_TEXT, INP_FREELING, INP_CONLL} InputFormats;

namespace po = boost::program_options;
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

  /// Locale of text to process
  std::wstring Locale;

  /// Configuration file for language identifier
  std::wstring IDENT_identFile;

  /// Mode used to process input: 
  ///   DOC: load a document, then process it. 
  ///   CORPUS: infinite sentence-by-sentnce processing
  InputModes InputMode;
  /// Selected input and output format
  OutputFormats OutputFormat;
  InputFormats InputFormat;
  /// whether splitter buffer must be flushed at each line
  bool AlwaysFlush;

  /// Tagset to use for shortening tags in output
  std::wstring TAGSET_TagsetFile;

  analyzer::config_options analyzer_config_options;
  analyzer::invoke_options analyzer_invoke_options;

  /// constructor
  config(int ac, char **av) {

    // Auxiliary variables to store options read as strings before they are converted
    // to their final enumerate/integer values 
    std::string InputLv, OutputLv, InputM, OutputF, InputF, Tagger, SenseAnot, Force, Dep;
    std::string tracemod;
    std::string language, locale, identFile, tagsetFile, tokFile, splitFile,
      macoDecimal, macoThousand, usermapFile, locutionsFile, quantitiesFile, 
      affixFile, probabilityFile, dictionaryFile, npDataFile, punctuationFile,
      compoundFile; 
    std::string phonFile, necFile, senseFile, ukbFile;
    std::string hmmFile,relaxFile,grammarFile,txalaFile,treelerFile,corefFile,semgraphFile;

    Port=0;

    po::options_description vis_cl("Available command-line options");
    vis_cl.add_options()
      ("help,h", "Help about command-line options.")
      ("help-cf", "Help about configuration file options.")
#ifndef WIN32
      ("version,v", "Print installed FreeLing version.")
#endif
      ("fcfg,f", po::value<std::string>(&ConfigFile)->default_value(""), "Configuration file to use")
      ("lang",po::value<std::string>(&language),"language of the input text")
      ("locale",po::value<std::string>(&locale),"locale encoding of input text (\"default\"=en_US.UTF-8, \"system\"=current system locale, [other]=any valid locale string installed in the system (e.g. ca_ES.UTF-8,it_IT.UTF-8,...)")
      ("server","Activate server mode (default: off)")
      ("port,p",po::value<int>(&Port),"Port where server is to be started")
      ("workers,w",po::value<int>(&MaxWorkers)->default_value(DEFAULT_MAX_WORKERS),"Maximum number of workers to fork in server mode")
      ("queue,q",po::value<int>(&QueueSize)->default_value(DEFAULT_QUEUE_SIZE),"Maximum number of waiting clients.")
      ("flush","Consider each newline as a sentence end")
      ("noflush","Do not consider each newline as a sentence end")
      ("inplv",po::value<std::string>(&InputLv),"Input analysis level (text,token,splitted,morfo,sense,tagged)")
      ("outlv",po::value<std::string>(&OutputLv),"Output analysis level (ident,token,splitted,morfo,tagged,shallow,parsed,dep)")
      ("mode",po::value<std::string>(&InputM),"Input mode (doc,corpus)")
      ("output",po::value<std::string>(&OutputF),"Output format (freeling,conll,train,xml,json,naf)")
      ("input",po::value<std::string>(&InputF),"Input format (text,freeling,conll)")
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
      ("thres,e",po::value<double>(&analyzer_config_options.MACO_ProbabilityThreshold),"Probability threshold for unknown word tags")
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
      ("iter,i",po::value<int>(&analyzer_config_options.TAGGER_RelaxMaxIter),"Maximum number of iterations allowed for RELAX tagger")
      ("sf,r",po::value<double>(&analyzer_config_options.TAGGER_RelaxScaleFactor),"Support scale factor for RELAX tagger (affects step size)")
      ("eps",po::value<double>(&analyzer_config_options.TAGGER_RelaxEpsilon),"Convergence epsilon value for RELAX tagger")
      ("rtk","Perform retokenization after PoS tagging")
      ("nortk","Do not perform retokenization after PoS tagging")
      ("force",po::value<std::string>(&Force),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("grammar,G",po::value<std::string>(&grammarFile),"Grammar file for chart parser")
      ("dep,d",po::value<std::string>(&Dep),"Dependency parser to use (txala,treeler)")
      ("txala,T",po::value<std::string>(&txalaFile),"Rule file for Txala dependency parser")
      ("treeler,E",po::value<std::string>(&treelerFile),"Configuration file for Treeler dependency parser")
      ("fcorf,C",po::value<std::string>(&corefFile),"Coreference solver data file")
      ("fsge,g",po::value<std::string>(&semgraphFile),"Semantic graph extractor config file")
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
      ("InputLevel",po::value<std::string>(&InputLv)->default_value("text"),"Input analysis level (text,token,splitted,morfo,tagged,shallow,dep,coref)")
      ("OutputLevel",po::value<std::string>(&OutputLv)->default_value("tagged"),"Output analysis level (token,splitted,morfo,tagged,shallow,parsed,dep,coref,semgraph)")
      ("InputMode",po::value<std::string>(&InputM)->default_value("corpus"),"Input mode (corpus,doc)")
      ("OutputFormat",po::value<std::string>(&OutputF)->default_value("freeling"),"Output format (freeling,conll,train,xml,json,naf)")
      ("InputFormat",po::value<std::string>(&InputF)->default_value("text"),"Input format (text,freeling,conll)")
      ("LangIdentFile",po::value<std::string>(&identFile),"Language identifier file")
      ("TokenizerFile",po::value<std::string>(&tokFile),"Tokenizer rules file")
      ("TagsetFile",po::value<std::string>(&tagsetFile),"Tagset description file")
      ("SplitterFile",po::value<std::string>(&splitFile),"Splitter option file")
      ("AffixAnalysis",po::value<bool>(&analyzer_invoke_options.MACO_AffixAnalysis)->default_value(false),"Perform affix analysis")
      ("UserMap",po::value<bool>(&analyzer_invoke_options.MACO_UserMap)->default_value(false),"Apply user mapping file")
      ("MultiwordsDetection",po::value<bool>(&analyzer_invoke_options.MACO_MultiwordsDetection)->default_value(false),"Perform multiword detection")
      ("NumbersDetection",po::value<bool>(&analyzer_invoke_options.MACO_NumbersDetection)->default_value(false),"Perform number detection")
      ("PunctuationDetection",po::value<bool>(&analyzer_invoke_options.MACO_PunctuationDetection)->default_value(false),"Perform punctuation detection")
      ("DatesDetection",po::value<bool>(&analyzer_invoke_options.MACO_DatesDetection)->default_value(false),"Perform date/time expression detection")
      ("QuantitiesDetection",po::value<bool>(&analyzer_invoke_options.MACO_QuantitiesDetection)->default_value(false),"Perform magnitude/ratio detection")
      ("DictionarySearch",po::value<bool>(&analyzer_invoke_options.MACO_DictionarySearch)->default_value(false),"Perform dictionary search")
      ("RetokContractions",po::value<bool>(&analyzer_invoke_options.MACO_RetokContractions)->default_value(true),"Dictionary retokenizes contractions regardless of --nortk option")
      ("ProbabilityAssignment",po::value<bool>(&analyzer_invoke_options.MACO_ProbabilityAssignment)->default_value(false),"Perform probability assignment")
      ("CompoundAnalysis",po::value<bool>(&analyzer_invoke_options.MACO_CompoundAnalysis)->default_value(false),"Perform compound analysis")
      ("NERecognition",po::value<bool>(&analyzer_invoke_options.MACO_NERecognition)->default_value(false),"Perform NE recognition")
      ("DecimalPoint",po::value<std::string>(&macoDecimal),"Decimal point character")
      ("ThousandPoint",po::value<std::string>(&macoThousand),"Thousand point character")
      ("UserMapFile",po::value<std::string>(&usermapFile),"User mapping file")
      ("LocutionsFile",po::value<std::string>(&locutionsFile),"Multiwords file")
      ("QuantitiesFile",po::value<std::string>(&quantitiesFile),"Quantities file")
      ("AffixFile",po::value<std::string>(&affixFile),"Affix rules file")

      ("ProbabilityFile",po::value<std::string>(&probabilityFile),"Probabilities file")
      ("ProbabilityThreshold",po::value<double>(&analyzer_config_options.MACO_ProbabilityThreshold),"Probability threshold for unknown word tags")
      ("DictionaryFile",po::value<std::string>(&dictionaryFile),"Form dictionary")
      ("NPDataFile",po::value<std::string>(&npDataFile),"NP recognizer data file")
      ("CompoundFile",po::value<std::string>(&compoundFile),"Compound detector configuration file")
      ("PunctuationFile",po::value<std::string>(&punctuationFile),"Punctuation symbol file")
      ("Phonetics",po::value<bool>(&analyzer_invoke_options.PHON_Phonetics)->default_value(false),"Perform phonetic encoding of words")
      ("PhoneticsFile",po::value<std::string>(&phonFile),"Phonetic encoding configuration file")
      ("NEClassification",po::value<bool>(&analyzer_invoke_options.NEC_NEClassification)->default_value(false),"Perform NE classification")
      ("NECFile",po::value<std::string>(&necFile),"NEC configuration file")
      ("SenseAnnotation",po::value<std::string>(&SenseAnot)->default_value("none"),"Type of sense annotation (no|none,all,mfs,ukb)")
      ("SenseConfigFile",po::value<std::string>(&senseFile),"Configuration file for sense annotation module")
      ("UKBConfigFile",po::value<std::string>(&ukbFile),"Configuration file for UKB word sense disambiguator")
      ("TaggerHMMFile",po::value<std::string>(&hmmFile),"Data file for HMM tagger")
      ("TaggerRelaxFile",po::value<std::string>(&relaxFile),"Data file for RELAX tagger")
      ("Tagger",po::value<std::string>(&Tagger)->default_value("hmm"),"Tagging alogrithm to use (hmm, relax)")
      ("TaggerRelaxMaxIter",po::value<int>(&analyzer_config_options.TAGGER_RelaxMaxIter),"Maximum number of iterations allowed for RELAX tagger")
      ("TaggerRelaxScaleFactor",po::value<double>(&analyzer_config_options.TAGGER_RelaxScaleFactor),"Support scale factor for RELAX tagger (affects step size)")
      ("TaggerRelaxEpsilon",po::value<double>(&analyzer_config_options.TAGGER_RelaxEpsilon),"Convergence epsilon value for RELAX tagger")
      ("TaggerRetokenize",po::value<bool>(&analyzer_config_options.TAGGER_Retokenize)->default_value(false),"Perform retokenization after PoS tagging")
      ("TaggerForceSelect",po::value<std::string>(&Force)->default_value("retok"),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("GrammarFile",po::value<std::string>(&grammarFile),"Grammar file for chart parser")
      ("DependencyParser",po::value<std::string>(&Dep)->default_value("txala"),"Dependency parser to use (txala,treeler)")
      ("DepTxalaFile",po::value<std::string>(&txalaFile),"Rule file for Txala dependency parser")
      ("DepTreelerFile",po::value<std::string>(&treelerFile),"Configuration file for Treeler dependency parser")
      ("CorefFile",po::value<std::string>(&corefFile),"Coreference solver data file")
      ("SemGraphExtractorFile",po::value<std::string>(&semgraphFile),"Semantic graph extractor config file")
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
    if (ConfigFile.empty()) {
      std::cerr<<"Configuration file not specified. Please use option -f to provide a configuration file."<<std::endl;
      exit(1);
    }
      
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
    semgraphFile = util::expand_filename(semgraphFile); 

    // translate string options (including expanded filenames) to wstrings
    Locale = util::string2wstring(locale);
    IDENT_identFile = util::string2wstring(identFile);
    TAGSET_TagsetFile = util::string2wstring(tagsetFile);

    analyzer_config_options.Lang = util::string2wstring(language);
    analyzer_config_options.TOK_TokenizerFile = util::string2wstring(tokFile);
    analyzer_config_options.SPLIT_SplitterFile = util::string2wstring(splitFile);
    analyzer_config_options.MACO_Decimal = util::string2wstring(macoDecimal);
    analyzer_config_options.MACO_Thousand = util::string2wstring(macoThousand);
    analyzer_config_options.MACO_UserMapFile = util::string2wstring(usermapFile);
    analyzer_config_options.MACO_LocutionsFile = util::string2wstring(locutionsFile);
    analyzer_config_options.MACO_QuantitiesFile = util::string2wstring(quantitiesFile);
    analyzer_config_options.MACO_AffixFile = util::string2wstring(affixFile);
    analyzer_config_options.MACO_ProbabilityFile = util::string2wstring(probabilityFile);
    analyzer_config_options.MACO_DictionaryFile = util::string2wstring(dictionaryFile);
    analyzer_config_options.MACO_NPDataFile = util::string2wstring(npDataFile);
    analyzer_config_options.MACO_PunctuationFile = util::string2wstring(punctuationFile);
    analyzer_config_options.MACO_CompoundFile = util::string2wstring(compoundFile);
    analyzer_config_options.PHON_PhoneticsFile = util::string2wstring(phonFile);
    analyzer_config_options.NEC_NECFile = util::string2wstring(necFile);
    analyzer_config_options.SENSE_ConfigFile = util::string2wstring(senseFile);
    analyzer_config_options.UKB_ConfigFile = util::string2wstring(ukbFile);
    analyzer_config_options.TAGGER_HMMFile = util::string2wstring(hmmFile);
    analyzer_config_options.TAGGER_RelaxFile = util::string2wstring(relaxFile);
    analyzer_config_options.PARSER_GrammarFile = util::string2wstring(grammarFile);
    analyzer_config_options.DEP_TxalaFile = util::string2wstring(txalaFile);
    analyzer_config_options.DEP_TreelerFile = util::string2wstring(treelerFile);
    analyzer_config_options.COREF_CorefFile = util::string2wstring(corefFile);
    analyzer_config_options.SEMGRAPH_SemGraphFile = util::string2wstring(semgraphFile);

    // Handle boolean options expressed with --myopt or --nomyopt in command line
    SetBooleanOptionCL(vm.count("server"),!vm.count("server"),Server,"server");
    SetBooleanOptionCL(vm.count("flush"),vm.count("noflush"),AlwaysFlush,"flush");

    SetBooleanOptionCL(vm.count("rtk"),vm.count("nortk"),analyzer_config_options.TAGGER_Retokenize,"rtk");

    SetBooleanOptionCL(vm.count("afx"),vm.count("noafx"),analyzer_invoke_options.MACO_AffixAnalysis,"afx");
    SetBooleanOptionCL(vm.count("usr"),vm.count("nousr"),analyzer_invoke_options.MACO_UserMap,"usr");
    SetBooleanOptionCL(vm.count("loc"),vm.count("noloc"),analyzer_invoke_options.MACO_MultiwordsDetection,"loc");
    SetBooleanOptionCL(vm.count("numb"),vm.count("nonumb"),analyzer_invoke_options.MACO_NumbersDetection,"numb");
    SetBooleanOptionCL(vm.count("punt"),vm.count("nopunt"),analyzer_invoke_options.MACO_PunctuationDetection,"punt");
    SetBooleanOptionCL(vm.count("date"),vm.count("nodate"),analyzer_invoke_options.MACO_DatesDetection,"date");
    SetBooleanOptionCL(vm.count("ner"),vm.count("noner"),analyzer_invoke_options.MACO_NERecognition,"ner");
    SetBooleanOptionCL(vm.count("quant"),vm.count("noquant"),analyzer_invoke_options.MACO_QuantitiesDetection,"quant");
    SetBooleanOptionCL(vm.count("dict"),vm.count("nodict"),analyzer_invoke_options.MACO_DictionarySearch,"dict");
    SetBooleanOptionCL(vm.count("rtkcon"),vm.count("nortkcon"),analyzer_invoke_options.MACO_RetokContractions,"rtkcon");
    SetBooleanOptionCL(vm.count("prob"),vm.count("noprob"),analyzer_invoke_options.MACO_ProbabilityAssignment,"prob");
    SetBooleanOptionCL(vm.count("comp"),vm.count("nocomp"),analyzer_invoke_options.MACO_CompoundAnalysis,"comp");
    SetBooleanOptionCL(vm.count("phon"),vm.count("nophon"),analyzer_invoke_options.PHON_Phonetics,"phon");
    SetBooleanOptionCL(vm.count("nec"),vm.count("nonec"),analyzer_invoke_options.NEC_NEClassification,"nec");
    
    // translate InputLv strings to appropriate enum values.
    if (InputLv=="text") analyzer_invoke_options.InputLevel = TEXT;
    else if (InputLv=="token") analyzer_invoke_options.InputLevel = TOKEN;
    else if (InputLv=="splitted") analyzer_invoke_options.InputLevel = SPLITTED;
    else if (InputLv=="morfo") analyzer_invoke_options.InputLevel = MORFO;
    else if (InputLv=="tagged") analyzer_invoke_options.InputLevel = TAGGED;
    else if (InputLv=="sense") analyzer_invoke_options.InputLevel = SENSES;
    else if (InputLv=="shallow") analyzer_invoke_options.InputLevel = SHALLOW;
    else if (InputLv=="parsed") analyzer_invoke_options.InputLevel = PARSED;
    else if (InputLv=="dep") analyzer_invoke_options.InputLevel = DEP;
    else if (InputLv=="coref") analyzer_invoke_options.InputLevel = COREF;
    else { ERROR_CRASH(L"Unknown or invalid input analysis level: "+util::string2wstring(InputLv));}

    // translate OutputLv strings appropriate enum values.
    if (OutputLv=="ident") analyzer_invoke_options.OutputLevel = IDENT;
    else if (OutputLv=="token") analyzer_invoke_options.OutputLevel = TOKEN;
    else if (OutputLv=="splitted") analyzer_invoke_options.OutputLevel = SPLITTED;
    else if (OutputLv=="morfo") analyzer_invoke_options.OutputLevel = MORFO;
    else if (OutputLv=="tagged") analyzer_invoke_options.OutputLevel = TAGGED;
    else if (OutputLv=="shallow") analyzer_invoke_options.OutputLevel = SHALLOW;
    else if (OutputLv=="parsed") analyzer_invoke_options.OutputLevel = PARSED;
    else if (OutputLv=="dep") analyzer_invoke_options.OutputLevel = DEP;
    else if (OutputLv=="coref") analyzer_invoke_options.OutputLevel = COREF;
    else if (OutputLv=="semgraph") analyzer_invoke_options.OutputLevel = SEMGRAPH;
    else { ERROR_CRASH(L"Unknown or invalid output analysis level: "+util::string2wstring(OutputLv));}

    // translate InputM strings to appropriate enum values.
    if (InputM=="corpus") InputMode = MODE_CORPUS;
    else if (InputM=="doc") InputMode = MODE_DOC;
    else { ERROR_CRASH(L"Unknown or invalid input mode: "+util::string2wstring(InputM));}

    // translate OutputF strings appropriate enum values.
    if (OutputF=="freeling") OutputFormat = OUT_FREELING;
    else if (OutputF=="conll") OutputFormat = OUT_CONLL;
    else if (OutputF=="train") OutputFormat = OUT_TRAIN;
    else if (OutputF=="xml") OutputFormat = OUT_XML;
    else if (OutputF=="json") OutputFormat = OUT_JSON;
    else if (OutputF=="naf") OutputFormat = OUT_NAF;
    else { ERROR_CRASH(L"Unknown or invalid output format: "+util::string2wstring(OutputF));}

    // translate InputF strings appropriate enum values.
    if (InputF=="text") InputFormat = INP_TEXT;
    else if (InputF=="freeling") InputFormat = INP_FREELING;
    else if (InputF=="conll") InputFormat = INP_CONLL;
    else { ERROR_CRASH(L"Unknown or invalid input format: "+util::string2wstring(InputF));}

    // translate Tagger string to appropriate enum values.
    if (Tagger=="hmm") analyzer_invoke_options.TAGGER_which = HMM;
    else if (Tagger=="relax") analyzer_invoke_options.TAGGER_which = RELAX;
    else {
      analyzer_invoke_options.TAGGER_which = HMM;
      WARNING(L"Invalid tagger algorithm '"+util::string2wstring(Tagger)+L"'. Using default.");
    }
    
    // Translate ForceSelect string to appropriate enum values.
    if (Force=="none" || Force=="no") analyzer_config_options.TAGGER_ForceSelect = NO_FORCE;
    else if (Force=="tagger") analyzer_config_options.TAGGER_ForceSelect = TAGGER;
    else if (Force=="retok") analyzer_config_options.TAGGER_ForceSelect = RETOK;
    else {
      analyzer_config_options.TAGGER_ForceSelect = RETOK;
      WARNING(L"Invalid ForceSelect value '"+util::string2wstring(Force)+L"'. Using default.");
    }    
    
    // translate SenseAnot string to more useful integer values.
    if (SenseAnot=="none" || SenseAnot=="no") analyzer_invoke_options.SENSE_WSD_which = NO_WSD;
    else if (SenseAnot=="all") analyzer_invoke_options.SENSE_WSD_which = ALL;
    else if (SenseAnot=="mfs") analyzer_invoke_options.SENSE_WSD_which = MFS;
    else if (SenseAnot=="ukb") analyzer_invoke_options.SENSE_WSD_which = UKB;
    else {
      analyzer_invoke_options.SENSE_WSD_which = NO_WSD;
      WARNING(L"Invalid sense annotation option '"+util::string2wstring(SenseAnot)+L"'. Using default.");
    }

    // translate Dep string to appropriate enum values.
    if (Dep=="txala") analyzer_invoke_options.DEP_which = TXALA;
    else if (Dep=="treeler") analyzer_invoke_options.DEP_which = TREELER;
    else {
      analyzer_invoke_options.DEP_which = TXALA;
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

