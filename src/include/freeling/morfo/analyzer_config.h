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

#ifndef _ANALYZER_CONFIG
#define _ANALYZER_CONFIG

#include <iostream> 
#include <sstream> 
#include <list>

#include "freeling/windll.h"
#include <boost/program_options.hpp>
namespace po=boost::program_options;

namespace freeling {

  // codes for input-output formats
  typedef enum {TEXT,IDENT,TOKEN,SPLITTED,MORFO,TAGGED,SENSES,SHALLOW,PARSED,DEP,SRL,COREF,SEMGRAPH} AnalysisLevel;
  // codes for tagging algorithms
  typedef enum {NO_TAGGER,HMM,RELAX} TaggerAlgorithm;
  // codes for dependency parsers
  typedef enum {NO_DEP,TXALA,TREELER,LSTM} DependencyParser;
  // codes for SRL parsers
  typedef enum {NO_SRL,SRL_TREELER} SRLParser;
  // codes for sense annotation
  typedef enum {NO_WSD,ALL,MFS,UKB} WSDAlgorithm;
  // codes for ForceSelect
  typedef enum {NO_FORCE,TAGGER,RETOK} ForceSelectStrategy;

  std::wistream& operator>>(std::wistream& in, freeling::ForceSelectStrategy& val);  
  std::wistream& operator>>(std::wistream& in, freeling::TaggerAlgorithm& val);
  std::wistream& operator>>(std::wistream& in, freeling::WSDAlgorithm& val);
  std::wistream& operator>>(std::wistream& in, freeling::DependencyParser& val);
  std::wistream& operator>>(std::wistream& in, freeling::SRLParser& val);
  std::wistream& operator>>(std::wistream& in, freeling::AnalysisLevel& val);
  bool read_bool(std::wistream& in);


  
////////////////////////////////////////////////////////////////
/// 
///  Class analyzer is a meta class that just calls all modules in
///  FreeLing in the right order.
///  Its construction options allow to instantiate different kinds of
///  analysis pipelines, and or different languages.
///  Also, invocation options may be altered at each call, 
///  tuning the analysis to each particular sentence or document needs.
///  For a finer control, underlying modules should be called directly.
///
////////////////////////////////////////////////////////////////

class WINDLL analyzer_config {

 private:

   ////////////////////////////////////////////////////////////////
   /// 
   ///  Class analyzer::config_options contains the configuration options
   ///  that define which modules are active and which configuration files
   ///  are loaded for each of them at construction time.
   ///  Options in this set can not be altered once the analyzer is created.  
   ///
   ////////////////////////////////////////////////////////////////

   class WINDLL analyzer_config_options {
     public:
       /// Language of text to process
       std::wstring Lang;
       /// Tokenizer configuration file
       std::wstring TOK_TokenizerFile;
       /// Splitter configuration file
       std::wstring SPLIT_SplitterFile;
       /// Morphological analyzer options
       std::wstring MACO_Decimal, MACO_Thousand;
       std::wstring MACO_UserMapFile, MACO_LocutionsFile,   MACO_QuantitiesFile,
         MACO_AffixFile,   MACO_ProbabilityFile, MACO_DictionaryFile, 
         MACO_NPDataFile,  MACO_PunctuationFile, MACO_CompoundFile;
       double MACO_ProbabilityThreshold;
       /// Phonetics config file
       std::wstring PHON_PhoneticsFile;
       /// NEC config file
       std::wstring NEC_NECFile;
       /// Sense annotator and WSD config files
       std::wstring SENSE_ConfigFile;
       std::wstring UKB_ConfigFile;
       /// Tagger options
       std::wstring TAGGER_HMMFile;
       std::wstring TAGGER_RelaxFile;
       int TAGGER_RelaxMaxIter;
       double TAGGER_RelaxScaleFactor;
       double TAGGER_RelaxEpsilon;
       bool TAGGER_Retokenize;
       int TAGGER_kbest;
       ForceSelectStrategy TAGGER_ForceSelect;
       /// Chart parser config file
       std::wstring PARSER_GrammarFile;
       /// Dependency parsers config files
       std::wstring DEP_TxalaFile;   
       std::wstring DEP_TreelerFile;   
       std::wstring DEP_LSTMFile;
       // SRL parsers config files
       std::wstring SRL_TreelerFile;   
       /// Coreference resolution config file
       std::wstring COREF_CorefFile;
       /// semantic graph extractor config file
       std::wstring SEMGRAPH_SemGraphFile;

       /// constructor
       analyzer_config_options();
       /// destructor
       ~analyzer_config_options();

       // debug help
       std::wstring dump() const;
   };

   ////////////////////////////////////////////////////////////////
   /// 
   ///  Class analyzer::invoke_options contains the options
   ///  that define the behaviour of each module in the analyze 
   ///  on the next analysis.
   ///  Options in this set can be altered after construction
   ///  (e.g. to activate/deactivate certain modules)
   ///
   ////////////////////////////////////////////////////////////////
   
   class WINDLL analyzer_invoke_options {
     public:
       /// Level of analysis in input and output
       AnalysisLevel InputLevel, OutputLevel;

       /// activate/deactivate morphological analyzer modules
       bool MACO_UserMap, MACO_AffixAnalysis, MACO_MultiwordsDetection, 
         MACO_NumbersDetection, MACO_PunctuationDetection, 
         MACO_DatesDetection, MACO_QuantitiesDetection, 
         MACO_DictionarySearch, MACO_ProbabilityAssignment, MACO_CompoundAnalysis,
         MACO_NERecognition, MACO_RetokContractions;

       /// activate/deactivate phonetics and NEC
       bool PHON_Phonetics;
       bool NEC_NEClassification;

       /// Select which tagger, parser, or sense annotator to use
       WSDAlgorithm SENSE_WSD_which;
       TaggerAlgorithm TAGGER_which;
       DependencyParser DEP_which;    
       SRLParser SRL_which;    

       /// constructor
       analyzer_invoke_options();
       /// destructor
       ~analyzer_invoke_options();

       // debug help
       std::wstring dump() const;
   };

   
 protected:
   po::options_description opts;
   
 public:
   ////////////////////////////////////////////////////////////////
   ///  class to handle configuration error states
   typedef enum {CFG_OK,CFG_WARNING, CFG_ERROR} CFG_status;
   class WINDLL status {
   public:
     CFG_status stat;
     std::wstring description;
   };

   typedef analyzer_config::analyzer_config_options config_options;
   typedef analyzer_config::analyzer_invoke_options invoke_options;

   config_options config;
   invoke_options invoke;

   /// constructor
   analyzer_config();
   /// destructor
   ~analyzer_config();

   /// load options from a config file
   void parse_options(const std::wstring &cfgFile);
   /// load options from a config file + command line   
   void parse_options(const std::wstring &cfgFile, int ac, char *av[]);   
   /// load options from a stream (auxiliary for the other constructors)
   void parse_options(std::wistream &cfg, analyzer_config::config_options &config, analyzer_config::invoke_options &invoke) const;

   // check invoke options
   status check_invoke_options(const analyzer_config::invoke_options &opt) const; 

};
 
} // namespace

#endif

