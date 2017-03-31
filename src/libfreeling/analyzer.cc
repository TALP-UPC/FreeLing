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

#include <sstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/analyzer.h"


using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#define MOD_TRACENAME L"ANALYZER"


//---------------------------------------------
//  Classes to hold configuration options
//---------------------------------------------

/// config options constructor, default values (except file names, initialized to "")
analyzer::analyzer_config_options::analyzer_config_options() {
  MACO_ProbabilityThreshold = 0.001;
  TAGGER_RelaxMaxIter = 500;
  TAGGER_RelaxScaleFactor = 67;
  TAGGER_RelaxEpsilon = 0.001;
  TAGGER_Retokenize = true;
  TAGGER_kbest = 1;
  TAGGER_ForceSelect=TAGGER;
};
  
/// destructor
analyzer::analyzer_config_options::~analyzer_config_options() {};

/// invoke options constructor, default values 
analyzer::analyzer_invoke_options::analyzer_invoke_options() {
  InputLevel = TEXT;  OutputLevel = TAGGED;
  
  MACO_UserMap=false;            MACO_AffixAnalysis=true;        MACO_MultiwordsDetection=true;
  MACO_NumbersDetection=true;    MACO_PunctuationDetection=true; MACO_DatesDetection=true;
  MACO_QuantitiesDetection=true; MACO_DictionarySearch=true;     MACO_ProbabilityAssignment=true;
  MACO_CompoundAnalysis=true;    MACO_NERecognition=true;        MACO_RetokContractions=true;
  
  PHON_Phonetics=false;
  NEC_NEClassification=false;
  
  SENSE_WSD_which=NO_WSD;
  TAGGER_which=HMM;
  DEP_which=NO_DEP;    
};

/// destructor
analyzer::analyzer_invoke_options::~analyzer_invoke_options() {};


//---------------------------------------------
//  analyzer class
//---------------------------------------------

///---------------------------------------------
/// Create analyzers defined by config_options.
///---------------------------------------------

analyzer::analyzer(const config_options &cfg) {

  tk=NULL; sp=NULL; morfo=NULL; neclass=NULL; sens=NULL;
  dsb=NULL; hmm=NULL; relax=NULL; phon=NULL; parser=NULL;
  deptxala=NULL; deptreeler=NULL; corfc=NULL; sge=NULL;

  offs = 0;
  nsentence = 1;
  
  //--- create needed analyzers, depending on given options ---//

  // tokenizer requested
  if (not cfg.TOK_TokenizerFile.empty()) tk = new tokenizer(cfg.TOK_TokenizerFile);
  // splitter requested
  if (not cfg.SPLIT_SplitterFile.empty()) {
    sp = new splitter(cfg.SPLIT_SplitterFile);
    sp_id = sp->open_session();
  }

  // some morfological analysis requested
  if (not cfg.MACO_UserMapFile.empty() or not cfg.MACO_PunctuationFile.empty() or
      not cfg.MACO_DictionaryFile.empty() or not cfg.MACO_AffixFile.empty() or 
      not cfg.MACO_CompoundFile.empty() or not cfg.MACO_LocutionsFile.empty() or  
      not cfg.MACO_NPDataFile.empty() or not cfg.MACO_QuantitiesFile.empty() or 
      not cfg.MACO_ProbabilityFile.empty()) {

    // the morfo class requires several options at creation time.
    // they are passed packed in a maco_options object.
    maco_options mopt(cfg.Lang);
    // decimal/thousand separators used by number detection
    mopt.set_nummerical_points (cfg.MACO_Decimal, cfg.MACO_Thousand);
    // Minimum probability for a tag for an unkown word
    mopt.set_threshold (cfg.MACO_ProbabilityThreshold);
    // Whether the dictionary offers inverse acces (lemma#pos -> form). 
    // Only needed if your application is going to do such an access.
    mopt.set_inverse_dict(false);

    // Data files for morphological submodules. by default set to ""
    // Only files for active modules have to be specified 
    mopt.set_data_files (cfg.MACO_UserMapFile,
                        cfg.MACO_PunctuationFile, cfg.MACO_DictionaryFile,
                        cfg.MACO_AffixFile, cfg.MACO_CompoundFile,
                        cfg.MACO_LocutionsFile, cfg.MACO_NPDataFile,
                        cfg.MACO_QuantitiesFile, cfg.MACO_ProbabilityFile);
    // create analyzer with desired options
    morfo = new maco(mopt);
  }

  // sense annotation requested/needed
  if (not cfg.SENSE_ConfigFile.empty()) 
    sens = new senses(cfg.SENSE_ConfigFile);

  // sense disambiguation requested
  if (not cfg.UKB_ConfigFile.empty()) 
    dsb = new ukb(cfg.UKB_ConfigFile);

  // taggers requested
  if (not cfg.TAGGER_HMMFile.empty())
    hmm = new hmm_tagger(cfg.TAGGER_HMMFile, 
                         cfg.TAGGER_Retokenize,
                         cfg.TAGGER_ForceSelect,
                         cfg.TAGGER_kbest);

  if (not cfg.TAGGER_RelaxFile.empty())
    relax = new relax_tagger(cfg.TAGGER_RelaxFile, 
                             cfg.TAGGER_RelaxMaxIter,
                             cfg.TAGGER_RelaxScaleFactor,
                             cfg.TAGGER_RelaxEpsilon, 
                             cfg.TAGGER_Retokenize,
                             cfg.TAGGER_ForceSelect);

  // phonetics requested
  if (not cfg.PHON_PhoneticsFile.empty()) 
    phon = new phonetics(cfg.PHON_PhoneticsFile);
  
  // NEC requested
  if (not cfg.NEC_NECFile.empty())
    neclass = new nec(cfg.NEC_NECFile);
  
  // Chunking requested
  if (not cfg.PARSER_GrammarFile.empty()) 
    parser = new chart_parser (cfg.PARSER_GrammarFile);

  // rule-based dep parser
  if (not cfg.DEP_TxalaFile.empty() and not cfg.PARSER_GrammarFile.empty()) 
    deptxala = new dep_txala(cfg.DEP_TxalaFile, parser->get_start_symbol ());

  // statistical dep-parser and SRL
  if (not cfg.DEP_TreelerFile.empty()) 
    deptreeler = new dep_treeler(cfg.DEP_TreelerFile);

  // coreference resolution
  if (not cfg.COREF_CorefFile.empty()) 
    corfc = new relaxcor(cfg.COREF_CorefFile);

  if (not cfg.SEMGRAPH_SemGraphFile.empty())
    sge = new semgraph_extract(cfg.SEMGRAPH_SemGraphFile);

  // set invoke options to do nothing, until the caller sets them properly
  current_invoke_options.InputLevel = current_invoke_options.OutputLevel = TEXT;
  
  current_invoke_options.MACO_UserMap = current_invoke_options.MACO_AffixAnalysis
    = current_invoke_options.MACO_MultiwordsDetection =  current_invoke_options.MACO_NumbersDetection
    = current_invoke_options.MACO_PunctuationDetection = current_invoke_options.MACO_DatesDetection
    = current_invoke_options.MACO_QuantitiesDetection = current_invoke_options.MACO_DictionarySearch 
    = current_invoke_options.MACO_ProbabilityAssignment = current_invoke_options.MACO_CompoundAnalysis
    = current_invoke_options.MACO_NERecognition = current_invoke_options.MACO_RetokContractions 
    = false;
  
  current_invoke_options.PHON_Phonetics 
    = current_invoke_options.NEC_NEClassification 
    = false;

  current_invoke_options.SENSE_WSD_which = NO_WSD;
  current_invoke_options.TAGGER_which = NO_TAGGER;
  current_invoke_options.DEP_which = NO_DEP;
}


//---------------------------------------------
// Destroy analyzers 
//---------------------------------------------

analyzer::~analyzer() {
  // clean up. Note that deleting a null pointer is a safe operation
  delete tk;
  if (sp!=NULL) sp->close_session(sp_id); 
  delete sp;
  delete morfo;
  delete phon;
  delete hmm;
  delete relax;
  delete neclass;
  delete sens;
  delete dsb;
  delete parser;
  delete deptxala;
  delete deptreeler;
  delete corfc;
  delete sge;
}


//---------------------------------------------  
// analyze further levels on a partially analyzed document or sentence list
//---------------------------------------------

template<class T> void analyzer::do_analysis(T &doc) const {

  // apply requested levels of analysis

  if (doc.empty()) return;

  // --------- MORFO
  // apply morfo if needed
  if (current_invoke_options.InputLevel < MORFO && current_invoke_options.OutputLevel >= MORFO) {
    morfo->analyze(doc);
    // apply sense tagging (w/o disambiguation) if needed
    if (current_invoke_options.SENSE_WSD_which != NO_WSD) 
      sens->analyze(doc);
  }

  // add phonetic encoding if needed 
  if (current_invoke_options.PHON_Phonetics) 
    phon->analyze(doc);

  // if expected output was MORFO or less, we are done
  if (current_invoke_options.OutputLevel<=MORFO) return;

  // --------- TAGGER
  // apply tagger if needed
  if (current_invoke_options.InputLevel < TAGGED && current_invoke_options.OutputLevel >= TAGGED) {

    if (current_invoke_options.TAGGER_which==HMM) hmm->analyze(doc);
    else if (current_invoke_options.TAGGER_which==RELAX) relax->analyze(doc);
      
    if (current_invoke_options.OutputLevel >= TAGGED and current_invoke_options.SENSE_WSD_which == UKB and dsb != NULL) 
      dsb->analyze(doc);

    if (current_invoke_options.OutputLevel >= TAGGED and current_invoke_options.NEC_NEClassification and neclass != NULL) 
      neclass->analyze(doc);
  }
  // if expected output was TAGGED, we are done
  if (current_invoke_options.OutputLevel==TAGGED) return;

  // --------- CHART PARSER
  // apply chart parser if needed
  if (parser != NULL and
      ((current_invoke_options.OutputLevel>=COREF and current_invoke_options.InputLevel < SHALLOW)
       or (current_invoke_options.InputLevel < SHALLOW
	   and (current_invoke_options.OutputLevel == SHALLOW or 
		current_invoke_options.OutputLevel == PARSED or 
		(current_invoke_options.OutputLevel > SHALLOW and current_invoke_options.DEP_which==TXALA)))))
    parser->analyze(doc);
  
 // if expected output was SHALLOW, we are done
  if (current_invoke_options.OutputLevel==SHALLOW) return;

  if (deptxala != NULL and 
      ((current_invoke_options.OutputLevel>=COREF 
	and current_invoke_options.InputLevel < PARSED
	and corfc!=NULL)
       or (current_invoke_options.InputLevel < PARSED
	   and (current_invoke_options.OutputLevel == PARSED or 
		(current_invoke_options.OutputLevel > PARSED and current_invoke_options.DEP_which==TXALA)))) )
    deptxala->complete_parse_tree(doc);

  // if expected output was PARSED, we are done
  if (current_invoke_options.OutputLevel==PARSED) return;

  // --------- DEP PARSER (+SRL where available).
  // apply dep parser if needed
  if (deptreeler!=NULL and 
      current_invoke_options.InputLevel<DEP and
      ((current_invoke_options.OutputLevel>=COREF and corfc!=NULL)
       or (current_invoke_options.OutputLevel >= DEP and current_invoke_options.DEP_which==TREELER))) {

    deptreeler->analyze(doc);
  }
  else if (deptxala != NULL and current_invoke_options.InputLevel < DEP and
           current_invoke_options.OutputLevel >= DEP and current_invoke_options.DEP_which==TXALA) 
    deptxala->analyze(doc);
}


//---------------------------------------------  
// analyze further levels on a partially analyzed document
//---------------------------------------------

void analyzer::analyze(document &doc) const {

  // analyze document
  do_analysis<document>(doc);

  // solve coreference if needed 
  if (current_invoke_options.InputLevel<COREF and current_invoke_options.OutputLevel>=COREF and corfc!=NULL)
    corfc->analyze(doc);  

  // extract semantic graph if needed
  if (current_invoke_options.OutputLevel>=SEMGRAPH) 
    sge->extract(doc);  
}


//---------------------------------------------  
// analyze further levels on a list of  partially analyzed sentences
//---------------------------------------------

void analyzer::analyze(list<sentence> &ls) const {
  do_analysis<list<sentence> >(ls);
}


//---------------------------------------------
// Apply analyzer cascade to sentences in 'text',
// return results as a single document.
// This will consider the input as a complete document
// No buffer will be accumulated for next calls
//---------------------------------------------

void analyzer::analyze(const wstring &text, document &doc, bool parag) const {

  doc.clear();

  // tokenize and split using local values for offset, nsent, sp_id
  list<word> av;
  unsigned long offs=0;
  unsigned long nsent=1;
  splitter::session_id sp_ses = sp->open_session();

  if (parag) {
    wistringstream tin(text);
    wstring line;
    wstring paragraph;
    while (getline(tin,line)) {
      if (line.empty()) {
        doc.push_back(list<sentence>());
        tokenize_split(paragraph, doc.back(), offs, av, nsent, true, sp_ses);
        paragraph.clear();
      }
      else {
        paragraph += line+L"\n";
      }
    }
    // process last paragraph, if any pending
    if (not paragraph.empty()) {
      doc.push_back(list<sentence>());
      tokenize_split(paragraph, doc.back(), offs, av, nsent, true, sp_ses);
    }
  }

  else {
    doc.push_back(list<sentence>());
    tokenize_split(text, doc.back(), offs, av, nsent, true, sp_ses);
  }

  sp->close_session(sp_ses);

  // process the splitted document for the rest of required levels
  analyze(doc);
}


//---------------------------------------------
// same than above, but for python API
//---------------------------------------------

document analyzer::analyze_as_document(const wstring &text, bool parag) const {
  document doc;
  analyze(text, doc, parag);
  return doc;
}

//---------------------------------------------
// Apply analyzer cascade to sentences in 'text',
// return results as list of sentences
// This will consider the input as partial text,
// and incomplete sentence will we kept in
// buffer for completion at the next call
//---------------------------------------------

void analyzer::analyze(const wstring &text, list<sentence> &ls, bool flush) {

  // tokenize and split using global values for offset, nsent, sp_id
  tokenize_split(text, ls, offs, tokens, nsentence, flush, sp_id);
  // perform rest of required analysis levels, if any.
  analyze(ls);
}


//---------------------------------------------
// same than above, but for python API
//---------------------------------------------

list<sentence> analyzer::analyze(const wstring &text, bool flush) {
  list<sentence> ls;
  analyze(text, ls, flush);
  return ls;
}


//---------------------------------------------
// Tokenize and split. Valid for both corpus and document mode
//---------------------------------------------

void analyzer::tokenize_split(const wstring &text, 
                              list<sentence> &ls, 
                              unsigned long &offs, 
                              list<word> &av, 
                              unsigned long &nsent, 
                              bool flush, 
                              splitter::session_id sp_ses) const {
  ls.clear();

  // --------- TOKENIZER
  tk->tokenize (text, offs, av);
  
  // if expected output was TOKEN, we are done
  if (current_invoke_options.OutputLevel==TOKEN) {
    ls.push_back(sentence(av));
    return;
  }

  // --------- SPLITTER
  // if splitter is needed, apply it
  if (current_invoke_options.InputLevel<SPLITTED and 
      current_invoke_options.OutputLevel>=SPLITTED) {
    // split text into a list of sentences (ls)
    sp->split(sp_ses, av, flush, ls);
    av.clear();
    // assign an id to each sentence.
    for (list<sentence>::iterator s=ls.begin(); s!=ls.end(); s++) {
      s->set_sentence_id(util::int2wstring(nsent));
      nsent++;
    }
  }

}


//---------------------------------------------
// flush buffers and return analysis for pending text, if any.
//---------------------------------------------

void analyzer::flush_buffer(list<sentence> &ls) { 
  analyze(L"",ls,true); 
}


//---------------------------------------------
void analyzer::reset_offset() {
  offs=0;
  nsentence=1;
}


//---------------------------------------------
// return options currently set for the analyzer
//---------------------------------------------

const analyzer::invoke_options& analyzer::get_current_invoke_options() const { 
  return current_invoke_options; 
}


//---------------------------------------------
// Modify options currently set for the analyzer
//---------------------------------------------

void analyzer::set_current_invoke_options(const invoke_options &opt, bool check) { 

  if (morfo!=NULL) {
    // morfo class will take care of setting and validating its own options
    morfo->set_active_options (opt.MACO_UserMap,
                               opt.MACO_NumbersDetection,
                               opt.MACO_PunctuationDetection,
                               opt.MACO_DatesDetection,
                               opt.MACO_DictionarySearch,
                               opt.MACO_AffixAnalysis,
                               opt.MACO_CompoundAnalysis,
                               opt.MACO_RetokContractions,
                               opt.MACO_MultiwordsDetection,
                               opt.MACO_NERecognition,
                               opt.MACO_QuantitiesDetection,
                               opt.MACO_ProbabilityAssignment);
  }

  // store given options as current
  current_invoke_options = opt; 
  
  if (check) {
    // Check if given options make sense. Issue warnings/errors if not
    if (opt.InputLevel >= opt.OutputLevel)
      WARNING(L"Input and output analysis levels are the same. No analysis will be performed.");
    if (opt.PHON_Phonetics and opt.OutputLevel < SPLITTED)
      WARNING(L"Phonetics requires at least 'splitted' output analysis level.");
    if (opt.SENSE_WSD_which!=NO_WSD and opt.OutputLevel < MORFO)
      WARNING(L"Sense annotation requires at least 'morfo' output analysis level.");
    if (opt.SENSE_WSD_which==UKB and opt.OutputLevel < TAGGED)
      WARNING(L"UKB word sense disambiguation requires at least 'tagged' output analysis level.");
    if (opt.NEC_NEClassification and opt.OutputLevel < TAGGED)
      WARNING(L"NE classification requires at least 'tagged' output analysis level.");
    if (opt.NEC_NEClassification and not opt.MACO_NERecognition)
      WARNING(L"NE classification requires NE recognition.");
    
    if (tk==NULL and opt.InputLevel == TEXT and opt.OutputLevel >= TOKEN)
      ERROR_CRASH(L"Tokenizer requested, but it was not instantiated in config options.");
    if (sp==NULL and opt.InputLevel < SPLITTED and opt.OutputLevel >= SPLITTED)
      ERROR_CRASH(L"Splitter requested, but it was not instantiated in config options.");
    if (morfo==NULL and opt.InputLevel < MORFO and opt.OutputLevel >= MORFO)
      ERROR_CRASH(L"Morphological analysis requested, but it was not instantiated in config options.");
    if (hmm==NULL and opt.TAGGER_which==HMM
        and opt.InputLevel < TAGGED and opt.OutputLevel >= TAGGED)
      ERROR_CRASH(L"HMM tagger requested, but it was not instantiated in config options.");
    if (relax==NULL and opt.TAGGER_which==RELAX
        and opt.InputLevel < TAGGED and opt.OutputLevel >= TAGGED)
      ERROR_CRASH(L"Relax tagger requested, but it was not instantiated in config options.");
    if (opt.TAGGER_which==NO_TAGGER and opt.InputLevel < TAGGED and opt.OutputLevel >= TAGGED)
      ERROR_CRASH(L"Tagger deactivated, but it is needed for required output analysis level.");
    if (opt.DEP_which==NO_DEP and opt.InputLevel < DEP and opt.OutputLevel >= DEP)
      ERROR_CRASH(L"Dependency parser deactivated, but it is needed for required output analysis level.");
    if (deptxala==NULL and opt.OutputLevel == PARSED)
      ERROR_CRASH(L"depTxala parser deactivated, but it is needed for required output analysis level.");

    if (opt.NEC_NEClassification and neclass==NULL)
      ERROR_CRASH(L"NE classification requested, but it was not instantiated in config options.");
    if (opt.PHON_Phonetics and phon==NULL)
      ERROR_CRASH(L"Phonetic transcription requested, but it was not instantiated in config options.");
    if (opt.SENSE_WSD_which != NO_WSD and sens==NULL) 
      ERROR_CRASH(L"Sense annotation requested, but it was not instantiated in config options.");
    if (opt.SENSE_WSD_which == UKB and dsb==NULL)
      ERROR_CRASH(L"UKB word sense disambiguation requested, but it was not instantiated in config options.");
    
    if (parser==NULL 
        and ((opt.OutputLevel == COREF and opt.InputLevel < PARSED) 
             or (opt.InputLevel < SHALLOW 
                 and (opt.OutputLevel == PARSED or opt.OutputLevel == SHALLOW))))
      ERROR_CRASH(L"Required analysis level requires chart parser, but it was not instantiated in config options");
    
    if (deptxala==NULL 
        and ((opt.OutputLevel == COREF and opt.InputLevel < DEP) 
             or (opt.DEP_which==TXALA
                 and opt.InputLevel < DEP and opt.OutputLevel > DEP)))
      ERROR_CRASH(L"Required analysis level requires depTxala parser, but it was not instantiated in config options");
    
    if (deptreeler==NULL 
        and ((opt.OutputLevel >= COREF and opt.InputLevel < DEP) 
             or (opt.DEP_which==TREELER
                 and opt.InputLevel < DEP and opt.OutputLevel > DEP)))
      ERROR_CRASH(L"Required analysis level requires depTreeler parser, but it was not instantiated in config options");

    if (corfc==NULL and opt.OutputLevel == COREF and opt.InputLevel < COREF) 
      ERROR_CRASH(L"Required analysis level requires Coreference solver, but it was not instantiated in config options");

    if (sge==NULL and opt.OutputLevel == SEMGRAPH and opt.InputLevel < SEMGRAPH) 
      ERROR_CRASH(L"Required analysis level requires semantic graph extractor, but it was not instantiated in config options");
  }
}


} // namespace
