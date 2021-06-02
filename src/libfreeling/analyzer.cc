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
//  analyzer class
//---------------------------------------------

///---------------------------------------------
/// Create analyzers with a full set of options (creation and invocation)
///---------------------------------------------

analyzer::analyzer(const analyzer_config &opts) {

  // store options used at creation time.
  initial_options = opts;
  // init current options with creation values.
  current_invoke_options = opts.invoke_opt;

  // load required modules
  create_analyzers();
}


///---------------------------------------------
/// Create analyzers with a set of creation options.
/// Default invokation options are left empty, so the user application
/// should set them with set_current_invoke_options
///---------------------------------------------
  
analyzer::analyzer(const analyzer_config::config_options &cfg) {  

  // store options used at creation time.
  initial_options.config_opt = cfg;

  // init invoke options to do nothing, until the caller sets them properly
  initial_options.invoke_opt.InputLevel = initial_options.invoke_opt.OutputLevel = TEXT;
  
  initial_options.invoke_opt.MACO_UserMap = initial_options.invoke_opt.MACO_AffixAnalysis
    = initial_options.invoke_opt.MACO_MultiwordsDetection =  initial_options.invoke_opt.MACO_NumbersDetection
    = initial_options.invoke_opt.MACO_PunctuationDetection = initial_options.invoke_opt.MACO_DatesDetection
    = initial_options.invoke_opt.MACO_QuantitiesDetection = initial_options.invoke_opt.MACO_DictionarySearch 
    = initial_options.invoke_opt.MACO_ProbabilityAssignment = initial_options.invoke_opt.MACO_CompoundAnalysis
    = initial_options.invoke_opt.MACO_NERecognition = initial_options.invoke_opt.MACO_RetokContractions 
    = false;
  
  initial_options.invoke_opt.PHON_Phonetics 
    = initial_options.invoke_opt.NEC_NEClassification 
    = false;

  initial_options.invoke_opt.SENSE_WSD_which = NO_WSD;
  initial_options.invoke_opt.TAGGER_which = NO_TAGGER;
  initial_options.invoke_opt.DEP_which = NO_DEP;
  initial_options.invoke_opt.SRL_which = NO_SRL;

  current_invoke_options = initial_options.invoke_opt;

  // load required modules
  create_analyzers();  
}

//---------------------------------------------
// create analyzers (auxiliary for constructors)
//---------------------------------------------

void analyzer::create_analyzers() {
  tk=NULL; sp=NULL; morfo=NULL; neclass=NULL; sens=NULL;
  dsb=NULL; hmm=NULL; relax=NULL; phon=NULL; parser=NULL;
  deptxala=NULL; deptreeler=NULL; deplstm=NULL; srltreeler=NULL;
  corfc=NULL; sge=NULL;
  
  offs = 0;
  nsentence = 1;
  
  //--- create needed analyzers, depending on given options ---//
  
  // tokenizer requested
  if (not initial_options.config_opt.TOK_TokenizerFile.empty())
    tk = new tokenizer(initial_options.config_opt.TOK_TokenizerFile);
  // splitter requested
  if (not initial_options.config_opt.SPLIT_SplitterFile.empty()) {
    sp = new splitter(initial_options.config_opt.SPLIT_SplitterFile);
    sp_id = sp->open_session();
  }

  // some morfological analysis requested
  if (not initial_options.config_opt.MACO_UserMapFile.empty() or not initial_options.config_opt.MACO_PunctuationFile.empty() or
      not initial_options.config_opt.MACO_DictionaryFile.empty() or not initial_options.config_opt.MACO_AffixFile.empty() or 
      not initial_options.config_opt.MACO_CompoundFile.empty() or not initial_options.config_opt.MACO_LocutionsFile.empty() or  
      not initial_options.config_opt.MACO_NPDataFile.empty() or not initial_options.config_opt.MACO_QuantitiesFile.empty() or 
      not initial_options.config_opt.MACO_ProbabilityFile.empty()) {

    // the morfo class requires several options at creation time.
    // they are passed packed in a maco_options object.
    maco_options mopt(initial_options.config_opt.Lang);
    // decimal/thousand separators used by number detection
    mopt.set_nummerical_points (initial_options.config_opt.MACO_Decimal, initial_options.config_opt.MACO_Thousand);
    // Minimum probability for a tag for an unkown word
    mopt.set_threshold (initial_options.config_opt.MACO_ProbabilityThreshold);
    // Whether the dictionary offers inverse acces (lemma#pos -> form). 
    // Only needed if your application is going to do such an access.
    mopt.set_inverse_dict(false);

    // Data files for morphological submodules. by default set to ""
    // Only files for active modules have to be specified 
    mopt.set_data_files (initial_options.config_opt.MACO_UserMapFile,
                        initial_options.config_opt.MACO_PunctuationFile, initial_options.config_opt.MACO_DictionaryFile,
                        initial_options.config_opt.MACO_AffixFile, initial_options.config_opt.MACO_CompoundFile,
                        initial_options.config_opt.MACO_LocutionsFile, initial_options.config_opt.MACO_NPDataFile,
                        initial_options.config_opt.MACO_QuantitiesFile, initial_options.config_opt.MACO_ProbabilityFile);
    // create analyzer with desired options
    morfo = new maco(mopt);
  }

  // sense annotation requested/needed
  if (not initial_options.config_opt.SENSE_ConfigFile.empty()) 
    sens = new senses(initial_options.config_opt.SENSE_ConfigFile);

  // sense disambiguation requested
  if (not initial_options.config_opt.UKB_ConfigFile.empty()) 
    dsb = new ukb(initial_options.config_opt.UKB_ConfigFile);

  // taggers requested
  if (not initial_options.config_opt.TAGGER_HMMFile.empty())
    hmm = new hmm_tagger(initial_options.config_opt.TAGGER_HMMFile, 
                         initial_options.config_opt.TAGGER_Retokenize,
                         initial_options.config_opt.TAGGER_ForceSelect,
                         initial_options.config_opt.TAGGER_kbest);

  if (not initial_options.config_opt.TAGGER_RelaxFile.empty())
    relax = new relax_tagger(initial_options.config_opt.TAGGER_RelaxFile, 
                             initial_options.config_opt.TAGGER_RelaxMaxIter,
                             initial_options.config_opt.TAGGER_RelaxScaleFactor,
                             initial_options.config_opt.TAGGER_RelaxEpsilon, 
                             initial_options.config_opt.TAGGER_Retokenize,
                             initial_options.config_opt.TAGGER_ForceSelect);

  // phonetics requested
  if (not initial_options.config_opt.PHON_PhoneticsFile.empty()) 
    phon = new phonetics(initial_options.config_opt.PHON_PhoneticsFile);
  
  // NEC requested
  if (not initial_options.config_opt.NEC_NECFile.empty())
    neclass = new nec(initial_options.config_opt.NEC_NECFile);
  
  // Chunking requested
  if (not initial_options.config_opt.PARSER_GrammarFile.empty()) 
    parser = new chart_parser (initial_options.config_opt.PARSER_GrammarFile);

  // rule-based dep parser
  if (not initial_options.config_opt.DEP_TxalaFile.empty() and not initial_options.config_opt.PARSER_GrammarFile.empty()) 
    deptxala = new dep_txala(initial_options.config_opt.DEP_TxalaFile, parser->get_start_symbol ());

  // statistical dep-parser 
  if (not initial_options.config_opt.DEP_TreelerFile.empty()) 
    deptreeler = new dep_treeler(initial_options.config_opt.DEP_TreelerFile);

  // LSTM based statistical parser
  if (not initial_options.config_opt.DEP_LSTMFile.empty()) 
    deplstm = new dep_lstm(initial_options.config_opt.DEP_LSTMFile);

  // statistical SRL
  if (not initial_options.config_opt.SRL_TreelerFile.empty()) 
    srltreeler = new srl_treeler(initial_options.config_opt.SRL_TreelerFile);

  // coreference resolution
  if (not initial_options.config_opt.COREF_CorefFile.empty()) 
    corfc = new relaxcor(initial_options.config_opt.COREF_CorefFile);

  // semantic graph extractor
  if (not initial_options.config_opt.SEMGRAPH_SemGraphFile.empty())
    sge = new semgraph_extract(initial_options.config_opt.SEMGRAPH_SemGraphFile);
   
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
  delete deplstm;
  delete srltreeler;
  delete corfc;
  delete sge;
}

  
//---------------------------------------------
// Modify options currently set for the analyzer
//---------------------------------------------

void analyzer::set_current_invoke_options(const analyzer_config::invoke_options &opt) { 

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
}

//---------------------------------------------  
// analyze further levels on a partially analyzed document or sentence list
//---------------------------------------------

template<class T> void analyzer::do_analysis(T &doc, const analyzer_config::invoke_options &ivk) const {

  // apply requested levels of analysis

  if (doc.empty()) return;

  // --------- MORFO
  // apply morfo if needed
  if (ivk.InputLevel < MORFO && ivk.OutputLevel >= MORFO) {
    morfo->analyze(doc);
  }

  // apply sense tagging (without WSD) if requested at morfo level
  if (ivk.SENSE_WSD_which != NO_WSD and ivk.OutputLevel <= MORFO) 
    sens->analyze(doc);
  
  // add phonetic encoding if needed 
  if (ivk.PHON_Phonetics) 
    phon->analyze(doc);

  // if expected output was MORFO or less, we are done
  if (ivk.OutputLevel <= MORFO) return;

  // --------- TAGGER
  // apply tagger if needed
  if (ivk.InputLevel < TAGGED && ivk.OutputLevel >= TAGGED) {
    if (ivk.TAGGER_which==HMM) hmm->analyze(doc);
    else if (ivk.TAGGER_which==RELAX) relax->analyze(doc);
  }
  
  // --------- WSD
  if (ivk.OutputLevel >= TAGGED) {
    // apply sense tagging if needed
    if (ivk.SENSE_WSD_which != NO_WSD) {
      if (sens->get_duplicate_analysis()) {
	sens->set_duplicate_analysis(false);
	WARNING(L"Deactivated DuplicateAnalysis option for 'senses' module due to selected OutputLevel>=TAGGED.")
      }
      sens->analyze(doc);

      // apply WSD if requested
      if (ivk.SENSE_WSD_which == UKB and dsb != NULL) 
	dsb->analyze(doc);
    }
  }

  // -- NEC
  if (ivk.OutputLevel >= TAGGED and ivk.NEC_NEClassification and neclass != NULL) 
    neclass->analyze(doc);
  
  // if expected output was TAGGED, we are done
  if (ivk.OutputLevel==TAGGED) return;

  // --------- CHART PARSER
  // apply chart parser if needed
  if (parser != NULL and                                    // chart parser is loaded
      ivk.InputLevel < SHALLOW and       // input is not parsed
        (ivk.OutputLevel == SHALLOW or     // requested output is shallow or parsed
         ivk.OutputLevel == PARSED or      // 
         (ivk.OutputLevel >= DEP and       // or any later stage, but dep_txala
          ivk.DEP_which==TXALA)))          // was explicitly requested
      parser->analyze(doc);
  
  // if expected output was SHALLOW, we are done
  if (ivk.OutputLevel==SHALLOW) return;

  // --------  Check if "PARSED" level needs to be computed
  if (deptxala != NULL                                        // dep_txala is loaded
      and ivk.InputLevel < PARSED          // input is at most chunked
      and (ivk.OutputLevel == PARSED       // and requested output is parsed
           or (ivk.OutputLevel > PARSED    // or any later stage, but dep_txala 
               and ivk.DEP_which==TXALA))) // was explicitly requested  
    deptxala->complete_parse_tree(doc);
  
  // if expected output was PARSED, we are done
  if (ivk.OutputLevel==PARSED) return;
  
  // --------- DEP PARSER (+SRL in treeler-> to detach).
  // apply dep parser if needed
  if (deptreeler!=NULL and 
      ivk.InputLevel<DEP and
      ((ivk.OutputLevel>=COREF and corfc!=NULL)
       or (ivk.OutputLevel >= DEP and ivk.DEP_which==TREELER))) {

    deptreeler->analyze(doc);
  }
  // apply lstm dep parser if needed
  else if (deplstm!=NULL and 
      ivk.InputLevel<DEP and
      ((ivk.OutputLevel>=COREF and corfc!=NULL)
       or (ivk.OutputLevel >= DEP and ivk.DEP_which==LSTM))) {

    deplstm->analyze(doc);
  }
  // default to rule based dep parser
  else if (deptxala != NULL and ivk.InputLevel < DEP and
           ivk.OutputLevel >= DEP and ivk.DEP_which==TXALA) {

    deptxala->analyze(doc);
  }
  
  // if expected output was DEP, we are done
  if (ivk.OutputLevel==DEP) return;

  // --------- SRL PARSER
  if (srltreeler!=NULL and 
      ivk.InputLevel<SRL and
      ((ivk.OutputLevel>=COREF and corfc!=NULL)
       or (ivk.OutputLevel >= SRL and ivk.SRL_which==SRL_TREELER))) {
    srltreeler->analyze(doc);
  }
  
}


//---------------------------------------------  
// analyze further levels on a partially analyzed document
//---------------------------------------------

void analyzer::analyze(document &doc, const analyzer_config::invoke_options& ivk) const {

  // analyze document
    do_analysis<document>(doc, ivk);

  // solve coreference if needed 
  if (ivk.InputLevel<COREF and ivk.OutputLevel>=COREF and corfc!=NULL and not doc.empty())
    corfc->analyze(doc);  

  // extract semantic graph if needed
  if (ivk.OutputLevel>=SEMGRAPH and sge!=NULL and not doc.empty()) 
    sge->extract(doc);  
}

//---------------------------------------------  
// analyze further levels on a partially analyzed document, using current default options
//---------------------------------------------
  
void analyzer::analyze(document &doc) const { analyze(doc, current_invoke_options); }


//---------------------------------------------  
// analyze further levels on a list of  partially analyzed sentences
//---------------------------------------------

void analyzer::analyze(list<sentence> &ls, const analyzer_config::invoke_options& ivk) const {
  do_analysis<list<sentence> >(ls, ivk);
}

//---------------------------------------------  
// analyze further levels on a list of  partially analyzed sentences, using current default options
//---------------------------------------------

void analyzer::analyze(list<sentence> &ls) const { analyze(ls, current_invoke_options); }

//---------------------------------------------  
// auxiliary to ensure safe threading
//---------------------------------------------  
  
wistream& analyzer::safe_getline(wistream& is, wstring& t)  {
  t.clear();
  
  // The characters in the stream are read one-by-one using a std::streambuf.
  // That is faster than reading them one-by-one using the std::istream.
  // Code that uses streambuf this way must be guarded by a sentry object.
  // The sentry object performs various tasks,
  // such as thread synchronization and updating the stream state.
  
  std::wistream::sentry se(is, true);
  std::wstreambuf* sb = is.rdbuf();
  
  for(;;) {
    wchar_t c = sb->sbumpc();
    switch (c) {
    case L'\n':
      return is;
    case L'\r':
      if(sb->sgetc() == '\n') sb->sbumpc();
      return is;
    case L'\uFFFF':
    case EOF:
      // Also handle the case when the last line has no line ending
      if (t.empty()) is.setstate(std::ios::eofbit);
      return is;
    default:
      t += (wchar_t) c;
    }
  }
}
  
  
//---------------------------------------------
// Apply analyzer cascade to sentences in 'text',
// return results as a single document.
// This will consider the input as a complete document
// No buffer will be accumulated for next calls
//---------------------------------------------

  void analyzer::analyze(const wstring &text, document &doc, const analyzer_config::invoke_options& ivk, bool parag) const {

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
    while (safe_getline(tin,line)) {
      if (line.empty()) {
        doc.push_back(list<sentence>());
        tokenize_split(paragraph, doc.back(), offs, av, nsent, true, sp_ses, ivk);
        paragraph.clear();
      }
      else {
        paragraph += line+L"\n";
      }
    }
    // process last paragraph, if any pending
    if (not paragraph.empty()) {
      doc.push_back(list<sentence>());
      tokenize_split(paragraph, doc.back(), offs, av, nsent, true, sp_ses, ivk);
    }
  }

  else {
    doc.push_back(list<sentence>());
    tokenize_split(text, doc.back(), offs, av, nsent, true, sp_ses, ivk);
  }

  sp->close_session(sp_ses);

  // purge document from any empty paragraph (may happen if there are blank lines at the
  // beggining or end of the file, or if there are several blank lines toghether)
  document::const_iterator p = doc.begin();
  while (p != doc.end()) {
    if (p->empty()) p = doc.erase(p);
    else ++p;
  }
  
  // process the splitted document for the rest of required levels
  analyze(doc, ivk);
}

//---------------------------------------------
// same than above, but returning result (useful for python API)
//---------------------------------------------

document analyzer::analyze_as_document(const wstring &text, const analyzer_config::invoke_options& ivk, bool parag) const {
  document doc;
  analyze(text, doc, ivk, parag);
  return doc;
}

//---------------------------------------------
// same than above, with current default options
//---------------------------------------------

void analyzer::analyze(const wstring &text, document &doc, bool parag) const {
  analyze(text, doc, current_invoke_options, parag);
}

//---------------------------------------------
// same than above, but returning result (useful for python API), with current default options
//---------------------------------------------

document analyzer::analyze_as_document(const wstring &text, bool parag) const {
  return analyze_as_document(text, current_invoke_options, parag);
}

//---------------------------------------------
// Apply analyzer cascade to sentences in 'text',
// return results as list of sentences
// This will consider the input as partial text,
// and incomplete sentence will we kept in
// buffer for completion at the next call
//---------------------------------------------

void analyzer::analyze(const wstring &text, list<sentence> &ls, const analyzer_config::invoke_options& ivk, bool flush) {
  // tokenize and split using global values for offset, nsent, sp_id
  tokenize_split(text, ls, offs, tokens, nsentence, flush, sp_id, ivk);
  // perform rest of required analysis levels, if any.
  analyze(ls, ivk);
}

//---------------------------------------------
// same than above, but returning result (useful for python API)
//---------------------------------------------

list<sentence> analyzer::analyze(const wstring &text, const analyzer_config::invoke_options& ivk, bool flush) {
  list<sentence> ls;
  analyze(text, ls, ivk, flush);
  return ls;
}

//---------------------------------------------
// same than above, with current default options
//---------------------------------------------

void analyzer::analyze(const wstring &text, list<sentence> &ls, bool flush) {
  analyze(text, ls, current_invoke_options, flush);
}

//---------------------------------------------
// same than above, but returning result (useful for python API), with current default options
//---------------------------------------------

list<sentence> analyzer::analyze(const wstring &text, bool flush) {
  return analyze(text, current_invoke_options, flush);
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
                              splitter::session_id sp_ses,
			      const analyzer_config::invoke_options &ivk) const {
  ls.clear();

  // --------- TOKENIZER
  tk->tokenize (text, offs, av);
  
  // if expected output was TOKEN, we are done
  if (ivk.OutputLevel==TOKEN) {
    ls.push_back(sentence(av));
    return;
  }

  // --------- SPLITTER
  // if splitter is needed, apply it
  if (ivk.InputLevel<SPLITTED and 
      ivk.OutputLevel>=SPLITTED) {
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

void analyzer::flush_buffer(list<sentence> &ls, const analyzer_config::invoke_options &ivk) { 
  analyze(L"", ls, ivk, true); 
}

//---------------------------------------------
// flush buffers, with current default options
//---------------------------------------------

void analyzer::flush_buffer(list<sentence> &ls) { flush_buffer(ls, current_invoke_options); }


//---------------------------------------------
void analyzer::reset_offset() {
  offs=0;
  nsentence=1;
}


//---------------------------------------------
// return options currently set for the analyzer
//---------------------------------------------

const analyzer_config::invoke_options& analyzer::get_current_invoke_options() const { return current_invoke_options; }

//---------------------------------------------
// return options used to create the analyzer
//---------------------------------------------

const analyzer_config& analyzer::get_initial_options() const { return initial_options; }


} // namespace
