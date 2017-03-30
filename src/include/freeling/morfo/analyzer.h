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

#ifndef _ANALYZER
#define _ANALYZER

#include <iostream> 
#include <list>

#include "freeling.h"

namespace freeling {

// codes for input-output formats
typedef enum {TEXT,IDENT,TOKEN,SPLITTED,MORFO,TAGGED,SENSES,SHALLOW,PARSED,DEP,COREF,SEMGRAPH} AnalysisLevel;
// codes for tagging algorithms
typedef enum {NO_TAGGER,HMM,RELAX} TaggerAlgorithm;
// codes for dependency parsers
typedef enum {NO_DEP,TXALA,TREELER} DependencyParser;
// codes for sense annotation
typedef enum {NO_WSD,ALL,MFS,UKB} WSDAlgorithm;
// codes for ForceSelect
typedef enum {NO_FORCE,TAGGER,RETOK} ForceSelectStrategy;


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

class WINDLL analyzer {

 private:

   ////////////////////////////////////////////////////////////////
   /// 
   ///  Class analyzer::config_options contains the configuration options
   ///  that define which modules are active and which configuration files
   ///  are loaded for each of them at construction time.
   ///  Options in this set can not be altered once the analyzer is created.  
   ///
   ////////////////////////////////////////////////////////////////

   class analyzer_config_options {
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
       /// Coreference resolution config file
       std::wstring COREF_CorefFile;
       /// semantic graph extractor config file
       std::wstring SEMGRAPH_SemGraphFile;

       /// constructor
       analyzer_config_options();
       /// destructor
       ~analyzer_config_options();
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
   
   class analyzer_invoke_options {
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

       /// constructor
       analyzer_invoke_options();
       /// destructor
       ~analyzer_invoke_options();
   };
   

   // we use pointers to the analyzers, so we
   // can create only those strictly necessary.
   tokenizer *tk;
   splitter *sp;
   maco *morfo;
   nec *neclass;
   senses *sens;
   ukb *dsb;
   POS_tagger *hmm;
   POS_tagger *relax;
   phonetics *phon;
   chart_parser *parser;
   dep_txala *deptxala;
   dep_treeler *deptreeler;
   relaxcor *corfc;
   semgraph_extract *sge;

   // store configuration options
   //   config *cfg;
   analyzer_invoke_options current_invoke_options;

   // remember splitter session
   splitter::session_id sp_id;

   // remember token offsets in plain text input
   unsigned long offs;
   // number of sentences processed (used to generate sentence id's)
   unsigned long nsentence;
   // words pending of being splitted in InputMode==CORPUS
   std::list<word> tokens; 
  
   /// analyze further levels on a partially analyzed document
   template<class T> void do_analysis(T &doc) const;
   // tokenize and split text.
   void tokenize_split(const std::wstring &text, 
                       std::list<sentence> &ls, 
                       unsigned long &offs, 
                       std::list<word> &av, 
                       unsigned long &nsent, 
                       bool flush, 
                       splitter::session_id sp_ses) const;

 public:
   typedef analyzer_config_options config_options;
   typedef analyzer_invoke_options invoke_options;

   analyzer(const config_options &cfg);
   void set_current_invoke_options(const invoke_options &opt, bool check=true);
   const invoke_options& get_current_invoke_options() const;

   ~analyzer();
   /// analyze further levels on a partially analyzed document
   void analyze(document &doc) const;
   /// analyze further levels on partially analyzed sentences
   void analyze(std::list<sentence> &ls) const;
   /// analyze text as a whole document
   void analyze(const std::wstring &text, document &doc, bool parag=false) const;
  /// Analyze text as a partial document. Retain incomplete sentences in buffer   
   /// in case next call completes them (except if flush==true)
   void analyze(const std::wstring &text, std::list<sentence> &ls, bool flush=false);

   // for python API
   std::list<sentence> analyze(const std::wstring &text, bool flush=false) ;
   document analyze_as_document(const std::wstring &text, bool parag=false) const;

   // flush splitter buffer and analyze any pending text. 
   void flush_buffer(std::list<sentence> &ls);
   // reset tokenizer offset counter
   void reset_offset();
};



} // namespace

#endif

