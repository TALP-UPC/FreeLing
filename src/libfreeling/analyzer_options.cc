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
#include "freeling/morfo/analyzer_options.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#define MOD_TRACENAME L"ANALYZER_OPTIONS"

  /// default constructor
  
  analyzer_options::analyzer_options() {}

  /// destructor
  
  analyzer_options::~analyzer_options() {}

  /// Create descriptions for valid config file options
  
  po::options_description analyzer_options::config_file_opts() {
    po::options_description cfo;
    cfo.add_options()
      ("Lang",po::wvalue<std::wstring>(),"Language of the input text")
      ("InputLevel",po::wvalue<freeling::AnalysisLevel>()->default_value(freeling::TEXT),"Input analysis level (text,token,splitted,morfo,tagged,shallow,dep,srl,coref)")
      ("OutputLevel",po::wvalue<freeling::AnalysisLevel>()->default_value(freeling::TAGGED),"Output analysis level (token,splitted,morfo,tagged,shallow,parsed,dep,srl,coref,semgraph)")
      ("TokenizerFile",po::wvalue<std::wstring>(),"Tokenizer rules file")
      ("SplitterFile",po::wvalue<std::wstring>(),"Splitter option file")
      ("AffixAnalysis",po::wvalue<bool>()->default_value(false),"Perform affix analysis")
      ("UserMap",po::wvalue<bool>()->default_value(false),"Apply user mapping file")
      ("MultiwordsDetection",po::wvalue<bool>()->default_value(false),"Perform multiword detection")
      ("NumbersDetection",po::wvalue<bool>()->default_value(false),"Perform number detection")
      ("PunctuationDetection",po::wvalue<bool>()->default_value(false),"Perform punctuation detection")
      ("DatesDetection",po::wvalue<bool>()->default_value(false),"Perform date/time expression detection")
      ("QuantitiesDetection",po::wvalue<bool>()->default_value(false),"Perform magnitude/ratio detection")
      ("DictionarySearch",po::wvalue<bool>()->default_value(false),"Perform dictionary search")
      ("RetokContractions",po::wvalue<bool>()->default_value(true),"Dictionary retokenizes contractions regardless of --nortk option")
      ("ProbabilityAssignment",po::wvalue<bool>()->default_value(false),"Perform probability assignment")
      ("CompoundAnalysis",po::wvalue<bool>()->default_value(false),"Perform compound analysis")
      ("NERecognition",po::wvalue<bool>()->default_value(false),"Perform NE recognition")
      ("DecimalPoint",po::wvalue<std::wstring>(),"Decimal point character")
      ("ThousandPoint",po::wvalue<std::wstring>(),"Thousand point character")
      ("UserMapFile",po::wvalue<std::wstring>(),"User mapping file")
      ("LocutionsFile",po::wvalue<std::wstring>(),"Multiwords file")
      ("QuantitiesFile",po::wvalue<std::wstring>(),"Quantities file")
      ("AffixFile",po::wvalue<std::wstring>(),"Affix rules file")    
      ("ProbabilityFile",po::wvalue<std::wstring>(),"Probabilities file")
      ("ProbabilityThreshold",po::wvalue<double>(),"Probability threshold for unknown word tags")
      ("DictionaryFile",po::wvalue<std::wstring>(),"Form dictionary")
      ("NPDataFile",po::wvalue<std::wstring>(),"NP recognizer data file")
      ("CompoundFile",po::wvalue<std::wstring>(),"Compound detector configuration file")
      ("PunctuationFile",po::wvalue<std::wstring>(),"Punctuation symbol file")
      ("Phonetics",po::wvalue<bool>()->default_value(false),"Perform phonetic encoding of words")
      ("PhoneticsFile",po::wvalue<std::wstring>(),"Phonetic encoding configuration file")
      ("NEClassification",po::wvalue<bool>()->default_value(false),"Perform NE classification")
      ("NECFile",po::wvalue<std::wstring>(),"NEC configuration file")
      ("SenseAnnotation",po::wvalue<freeling::WSDAlgorithm>()->default_value(freeling::NO_WSD),"Type of sense annotation (no|none,all,mfs,ukb)")
      ("SenseConfigFile",po::wvalue<std::wstring>(),"Configuration file for sense annotation module")
      ("UKBConfigFile",po::wvalue<std::wstring>(),"Configuration file for UKB word sense disambiguator")
      ("TaggerHMMFile",po::wvalue<std::wstring>(),"Data file for HMM tagger")
      ("TaggerRelaxFile",po::wvalue<std::wstring>(),"Data file for RELAX tagger")
      ("Tagger",po::wvalue<freeling::TaggerAlgorithm>()->default_value(freeling::HMM),"Tagging alogrithm to use (hmm, relax)")
      ("TaggerRelaxMaxIter",po::wvalue<int>(),"Maximum number of iterations allowed for RELAX tagger")
      ("TaggerRelaxScaleFactor",po::wvalue<double>(),"Support scale factor for RELAX tagger (affects step size)")
      ("TaggerRelaxEpsilon",po::wvalue<double>(),"Convergence epsilon value for RELAX tagger")
      ("TaggerRetokenize",po::wvalue<bool>()->default_value(false),"Perform retokenization after PoS tagging")
      ("TaggerForceSelect",po::wvalue<freeling::ForceSelectStrategy>()->default_value(freeling::RETOK),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("TaggerKBest",po::wvalue<int>(),"Number of sequences produced by HMM tagger")
      ("GrammarFile",po::wvalue<std::wstring>(),"Grammar file for chart parser")
      ("DependencyParser",po::wvalue<freeling::DependencyParser>()->default_value(freeling::TXALA),"Dependency parser to use (txala,treeler,lstm)")
      ("DepTxalaFile",po::wvalue<std::wstring>(),"Rule file for Txala dependency parser")
      ("DepTreelerFile",po::wvalue<std::wstring>(),"Configuration file for Treeler dependency parser")
      ("DepLSTMFile",po::wvalue<std::wstring>(),"Configuration file for LSTM dependency parser")
      ("SRLParser",po::wvalue<freeling::SRLParser>()->default_value(freeling::SRL_TREELER),"SRL parser to use (treeler)")
      ("SRLTreelerFile",po::wvalue<std::wstring>(),"Configuration file for Treeler SRL parser")
      ("CorefFile",po::wvalue<std::wstring>(),"Coreference solver data file")
      ("SemGraphExtractorFile",po::wvalue<std::wstring>(),"Semantic graph extractor config file");

    return cfo;
  }

  
  /// Create descriptions for valid command line options
  
  po::options_description analyzer_options::command_line_opts() {
    po::options_description clo;
    clo.add_options()
      ("lang",po::wvalue<std::wstring>(),"language of the input text")
      ("inplv",po::wvalue<freeling::AnalysisLevel>()->default_value(freeling::TEXT),"Input analysis level (text,token,splitted,morfo,tagged,shallow,dep,srl,coref)")
      ("outlv",po::wvalue<freeling::AnalysisLevel>()->default_value(freeling::TAGGED),"Output analysis level (token,splitted,morfo,tagged,shallow,parsed,dep,srl,coref,semgraph)")
      ("ftok",po::wvalue<std::wstring>(),"Tokenizer rules file")
      ("fsplit",po::wvalue<std::wstring>(),"Splitter option file")
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
      ("dec",po::wvalue<std::wstring>(),"Decimal point character")
      ("thou",po::wvalue<std::wstring>(),"Thousand point character")
      ("fmap,M",po::wvalue<std::wstring>(),"User-map file")
      ("floc,L",po::wvalue<std::wstring>(),"Multiwords file")
      ("fqty,Q",po::wvalue<std::wstring>(),"Quantities file")
      ("fafx,S",po::wvalue<std::wstring>(),"Affix rules file")
      ("fprob,P",po::wvalue<std::wstring>(),"Probabilities file")
      ("thres,e",po::wvalue<double>(),"Probability threshold for unknown word tags")
      ("fdict,D",po::wvalue<std::wstring>(),"Form dictionary")
      ("fnp,N",po::wvalue<std::wstring>(),"NE recognizer data file")
      ("fcomp,K",po::wvalue<std::wstring>(),"Compound detector configuration file")
      ("fpunct,F",po::wvalue<std::wstring>(),"Punctuation symbol file")
      ("phon","Perform phonetic encoding of words")
      ("nophon","Do not perform phonetic encoding of words")
      ("fphon",po::wvalue<std::wstring>(),"Phonetic encoding configuration file")
      ("nec","Perform NE classification")
      ("nonec","Do not perform NE classification")
      ("fnec",po::wvalue<std::wstring>(),"NEC configuration file")
      ("sense,s",po::wvalue<freeling::WSDAlgorithm>(),"Type of sense annotation (no|none,all,mfs,ukb)")
      ("fsense,W",po::wvalue<std::wstring>(),"Configuration file for sense annotation module")
      ("fukb,U",po::wvalue<std::wstring>(),"Configuration file for UKB word sense disambiguator")
      ("hmm,H",po::wvalue<std::wstring>(),"Data file for HMM tagger")
      ("rlx,R",po::wvalue<std::wstring>(),"Data file for RELAX tagger")
      ("tag,t",po::wvalue<freeling::TaggerAlgorithm>(),"Tagging alogrithm to use (hmm, relax)")
      ("iter,i",po::wvalue<int>(),"Maximum number of iterations allowed for RELAX tagger")
      ("sf,r",po::wvalue<double>(),"Support scale factor for RELAX tagger (affects step size)")
      ("eps",po::wvalue<double>(),"Convergence epsilon value for RELAX tagger")
      ("rtk","Perform retokenization after PoS tagging")
      ("nortk","Do not perform retokenization after PoS tagging")
      ("force",po::wvalue<freeling::ForceSelectStrategy>(),"When the tagger must be forced to select only one tag per word (no|none,tagger,retok)")
      ("grammar,G",po::wvalue<std::wstring>(),"Grammar file for chart parser")
      ("dep,d",po::wvalue<freeling::DependencyParser>(),"Dependency parser to use (txala,treeler,lstm)")
      ("srl",po::wvalue<freeling::SRLParser>(),"SRL parser to use (treeler)")
      ("txala,T",po::wvalue<std::wstring>(),"Rule file for Txala dependency parser")
      ("treeler,E",po::wvalue<std::wstring>(),"Configuration file for Treeler dependency parser")
      ("lstm",po::wvalue<std::wstring>(),"Configuration file for LSTM dependency parser")
      ("SRLtreeler",po::wvalue<std::wstring>(),"Configuration file for SRL treeler parser")
      ("fcorf,C",po::wvalue<std::wstring>(),"Coreference solver data file")
      ("fsge,g",po::wvalue<std::wstring>(),"Semantic graph extractor config file");

    return clo;
  }


  /// load provided options from a config file, update variables map
  
  void analyzer_options::parse_options(const wstring &cfgFile,
                                       const po::options_description &cf_opts,
                                       po::variables_map &vm) {
    std::wifstream fcfg;
    freeling::util::open_utf8_file(fcfg,cfgFile);
    if (fcfg.fail()) ERROR_CRASH(L"Can not open config file '" << cfgFile << "'");

    try {
      po::store(po::parse_config_file(fcfg, cf_opts), vm);
      po::notify(vm);
    }
    catch (exception &e) {
      ERROR_CRASH(L"Error while parsing configuration file: " << util::string2wstring(e.what()));
    }
  }

  /// load provided options from a config file, return variables map

  po::variables_map analyzer_options::parse_options(const wstring &cfgFile,
                                                    const po::options_description &cf_opts) {
    po::variables_map vm;    
    parse_options(cfgFile, cf_opts, vm);
    return vm;
  }

  /// load basic options from a config file, return variables map

  po::variables_map analyzer_options::parse_options(const wstring &cfgFile) {
    po::options_description cf_opts = config_file_opts(); 
    return parse_options(cfgFile, cf_opts);
  }

  /// load provided options from command line, update variables map
  
  void analyzer_options::parse_options(int ac, char *av[], const po::options_description &cl_opts, po::variables_map &vm) {
    try {
      po::store(po::parse_command_line(ac, av, cl_opts), vm);
      po::notify(vm);    
    } 
    catch (exception &e) {
      ERROR_CRASH("Error while parsing command line: "<<e.what());
    }
  }

  /// load provided options from command line, return variables map
  
  po::variables_map analyzer_options::parse_options(int ac, char *av[], const po::options_description &cl_opts) {
    po::variables_map vm;
    parse_options(ac, av, cl_opts, vm);
    return vm;
  }
  
  /// load default options from command line 
  
  po::variables_map analyzer_options::parse_options(int ac, char *av[]) {
    po::options_description cl_opts = command_line_opts(); 
    return parse_options(ac, av, cl_opts);
  }

  /// load provided options from config file and command line
  
  po::variables_map analyzer_options::parse_options(const wstring &cfgFile, int ac, char *av[],
				      const po::options_description &cf_opts,
				      const po::options_description &cl_opts) {
    po::variables_map vm;
    parse_options(ac, av, cl_opts, vm);
    parse_options(cfgFile, cf_opts, vm);
    return vm;
  }

  /// load default options from config file and command line
  
  po::variables_map analyzer_options::parse_options(const wstring &cfgFile, int ac, char *av[]) {
    po::options_description cl_opts = command_line_opts();
    po::options_description cf_opts = config_file_opts();
    return parse_options(cfgFile, ac, av, cf_opts, cl_opts);
  }


  
} //namespace
