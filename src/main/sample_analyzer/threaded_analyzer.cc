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
//  This file contains a simple main program to illustrate 
//  usage of FreeLing analyzers library.
//
//  This sample main program may be used straightforwardly as 
//  a basic front-end for the analyzers (e.g. to analyze corpora)
//
//  Neverthless, if you want embed the FreeLing libraries inside
//  a larger application, or you want to deal with other 
//  input/output formats (e.g. XML), the efficient and elegant 
//  way to do so is consider this file as a mere example, and call 
//  the library from your your own main code.
//
//------------------------------------------------------------------//


#include "freeling/output/output_freeling.h"
#include "threaded_processor.h"
#include "config.h"

using namespace std;

// we use pointers to the analyzers, so we
// can create only those strictly necessary.
tokenizer *tk=NULL;
splitter *sp=NULL;
maco *morfo=NULL;
nec *neclass=NULL;
senses *sens=NULL;
ukb *dsb=NULL;
POS_tagger *tagger=NULL;
phonetics *phon;
chart_parser *parser=NULL;
dependency_parser *dep=NULL;

// read configuration file and command-line options
config *cfg;

// pipes to communicate each thread module with the next.
vector<FL_pipe*> pipes;



/// ----------------------------------------------
/// Create analyzers using data files from given path
/// ----------------------------------------------

void CreateAnalyzers(int argc, char **argv) {

  cfg = new config(argc, argv);

  /// set the locale to UTF to properly handle special characters.
  util::init_locale(cfg->Locale);

  // check option coherence
  if (cfg->analyzer_invoke_options.OutputLevel==IDENT) {
    wcerr <<L"Error - Language identification not available in threaded mode."<<endl;
    exit (1);
  }

  if (cfg->analyzer_invoke_options.OutputLevel==COREF) {
    wcerr <<L"Error - Coreference resolution not available in threaded mode" <<endl;
    exit (1);
  }

  if (!((cfg->analyzer_invoke_options.InputLevel < cfg->analyzer_invoke_options.OutputLevel) or
        (cfg->analyzer_invoke_options.InputLevel == cfg->analyzer_invoke_options.OutputLevel 
         and cfg->analyzer_invoke_options.InputLevel == TAGGED
         and cfg->analyzer_invoke_options.NEC_NEClassification)))
    {
      wcerr <<L"Error - Input format cannot be more complex than desired output."<<endl;
      exit (1);
    }

  if (cfg->analyzer_invoke_options.OutputLevel < TAGGED and cfg->analyzer_invoke_options.SENSE_WSD_which == UKB)   {
    wcerr <<L"Error - UKB word sense disambiguation requires PoS tagging. Specify 'tagged', 'parsed' or 'dep' output format." <<endl;
    exit (1);
  }

  if (cfg->analyzer_invoke_options.OutputLevel != TAGGED and cfg->OutputFormat==OUT_TRAIN) {
    wcerr <<L"Warning - OutputLevel changed to 'tagged' since option --train was specified." <<endl;
    cfg->analyzer_invoke_options.OutputLevel = TAGGED;
  }
  
  //--- create needed threaded analyzers, depending on given options ---//

  // number of created modules
  int nmodules=0;
  // input pipe to first module
  FL_pipe *p = new FL_pipe(); pipes.push_back(p); nmodules++;

  // tokenizer requested
  if (cfg->analyzer_invoke_options.InputLevel < TOKEN and cfg->analyzer_invoke_options.OutputLevel >= TOKEN) {
    // create tokenizer
    tk = new tokenizer (cfg->analyzer_config_options.TOK_TokenizerFile);
    // create tokenizer output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch tokenizer thread
    threaded_processor<tokenizer> Th_tk(tk,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }
  // splitter requested
  if (cfg->analyzer_invoke_options.InputLevel < SPLITTED and cfg->analyzer_invoke_options.OutputLevel >= SPLITTED) {
    // create splitter
    sp = new splitter (cfg->analyzer_config_options.SPLIT_SplitterFile);
    // create splitter output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch splitter thread
    threaded_processor<splitter> Th_sp(sp,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }

  // morfological analysis requested
  if (cfg->analyzer_invoke_options.InputLevel < MORFO and cfg->analyzer_invoke_options.OutputLevel >= MORFO) {
    // the morfo class requires several options at creation time.
    // they are passed packed in a maco_options object.
    maco_options opt (cfg->analyzer_config_options.Lang);

    // decimal/thousand separators used by number detection
    opt.set_nummerical_points (cfg->analyzer_config_options.MACO_Decimal, cfg->analyzer_config_options.MACO_Thousand);
    // Minimum probability for a tag for an unkown word
    opt.set_threshold (cfg->analyzer_config_options.MACO_ProbabilityThreshold);
    // Whether the dictionary offers inverse acces (lemma#pos -> form). 
    // Only needed if your application is going to do such an access.
    opt.set_inverse_dict(false);
    // Whether contractions are splitted by the dictionary right away,
    // or left for later "retok" option to decide.
    opt.set_retok_contractions(cfg->analyzer_invoke_options.MACO_RetokContractions);

    // Data files for morphological submodules. by default set to ""
    // Only files for active modules have to be specified 
    opt.set_data_files (cfg->analyzer_config_options.MACO_UserMapFile,
                        cfg->analyzer_config_options.MACO_PunctuationFile, 
                        cfg->analyzer_config_options.MACO_DictionaryFile,
                        cfg->analyzer_config_options.MACO_AffixFile,
                        cfg->analyzer_config_options.MACO_CompoundFile,
                        cfg->analyzer_config_options.MACO_LocutionsFile, 
                        cfg->analyzer_config_options.MACO_NPDataFile,
                        cfg->analyzer_config_options.MACO_QuantitiesFile, 
                        cfg->analyzer_config_options.MACO_ProbabilityFile);

    // create analyzer with desired options
    morfo = new maco (opt);

    // boolean options to activate/desactivate modules
    morfo->set_active_options (cfg->analyzer_invoke_options.MACO_UserMap,
                               cfg->analyzer_invoke_options.MACO_NumbersDetection,
                               cfg->analyzer_invoke_options.MACO_PunctuationDetection,
                               cfg->analyzer_invoke_options.MACO_DatesDetection,
                               cfg->analyzer_invoke_options.MACO_DictionarySearch,
                               cfg->analyzer_invoke_options.MACO_AffixAnalysis,
                               cfg->analyzer_invoke_options.MACO_CompoundAnalysis,
                               cfg->analyzer_invoke_options.MACO_RetokContractions,
                               cfg->analyzer_invoke_options.MACO_MultiwordsDetection,
                               cfg->analyzer_invoke_options.MACO_NERecognition,
                               cfg->analyzer_invoke_options.MACO_QuantitiesDetection,
                               cfg->analyzer_invoke_options.MACO_ProbabilityAssignment);

    // create morfo output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch morfo thread
    threaded_processor<processor> Th_mf(morfo,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }

  // sense annotation requested
  if (cfg->analyzer_invoke_options.InputLevel < SENSES 
      and cfg->analyzer_invoke_options.OutputLevel >= MORFO 
      and cfg->analyzer_invoke_options.SENSE_WSD_which != NO_WSD)  {
    sens = new senses (cfg->analyzer_config_options.SENSE_ConfigFile);
    // create sense output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch sense thread
    threaded_processor<processor> Th_sens(sens,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }

  // tagger requested, see which method
  if (cfg->analyzer_invoke_options.InputLevel < TAGGED and cfg->analyzer_invoke_options.OutputLevel >= TAGGED) {
    if (cfg->analyzer_invoke_options.TAGGER_which == HMM) {
      tagger =
        new hmm_tagger (cfg->analyzer_config_options.TAGGER_HMMFile, 
                        cfg->analyzer_config_options.TAGGER_Retokenize,
                        cfg->analyzer_config_options.TAGGER_ForceSelect);
    }
    else if (cfg->analyzer_invoke_options.TAGGER_which == RELAX) {
      tagger =
        new relax_tagger (cfg->analyzer_config_options.TAGGER_RelaxFile, 
                          cfg->analyzer_config_options.TAGGER_RelaxMaxIter,
                          cfg->analyzer_config_options.TAGGER_RelaxScaleFactor,
                          cfg->analyzer_config_options.TAGGER_RelaxEpsilon, 
                          cfg->analyzer_config_options.TAGGER_Retokenize,
                          cfg->analyzer_config_options.TAGGER_ForceSelect);
    }
    // create tagger output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch tagger thread
    threaded_processor<processor> Th_tg(tagger,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }

  // phonetics requested
  if (cfg->analyzer_invoke_options.PHON_Phonetics) {
    phon = new phonetics (cfg->analyzer_config_options.PHON_PhoneticsFile);
    // create phonetics output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch phonetics thread
    threaded_processor<processor> Th_tg(phon,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }
  
  // sense disambiguation requested
  if ((cfg->analyzer_invoke_options.InputLevel < SENSES 
       and cfg->analyzer_invoke_options.OutputLevel >= TAGGED
       and cfg->analyzer_invoke_options.SENSE_WSD_which==UKB) 
      or cfg->analyzer_invoke_options.OutputLevel == COREF) {
    dsb = new ukb(cfg->analyzer_config_options.UKB_ConfigFile);
    // create ukb output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch ukb thread
    threaded_processor<processor> Th_ukb(dsb,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }

  // NEC requested
  if (cfg->analyzer_invoke_options.InputLevel <= TAGGED 
      and cfg->analyzer_invoke_options.OutputLevel >= TAGGED 
      and (cfg->analyzer_invoke_options.NEC_NEClassification 
           or cfg->analyzer_invoke_options.OutputLevel == COREF)) {
    neclass = new nec (cfg->analyzer_config_options.NEC_NECFile);
    // create nec output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch nec thread
    threaded_processor<processor> Th_nec(neclass,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }
  
  // Chunking requested
  if (cfg->analyzer_invoke_options.InputLevel < SHALLOW 
      and (cfg->analyzer_invoke_options.OutputLevel >= SHALLOW 
           or cfg->analyzer_invoke_options.OutputLevel == COREF)) {
    parser = new chart_parser (cfg->analyzer_config_options.PARSER_GrammarFile);
    // create chunker output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch chunker thread
    threaded_processor<processor> Th_chk(parser,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }

  // Dependency parsing requested
  if (cfg->analyzer_invoke_options.InputLevel < SHALLOW 
      and cfg->analyzer_invoke_options.OutputLevel >= PARSED) {
    dep = new dep_txala (cfg->analyzer_config_options.DEP_TxalaFile, parser->get_start_symbol ());
    // create dep output pipe
    FL_pipe *p = new FL_pipe(); pipes.push_back(p); 
    // launch dep thread
    threaded_processor<processor> Th_dep(dep,*pipes[nmodules-1],*pipes[nmodules]);
    nmodules++;
  }
}


//---------------------------------------------
// Destroy analyzers 
//---------------------------------------------

void cleanup() {
  // clean up. Note that deleting a null pointer is a safe (yet useless) operation
  delete tk;
  delete sp;
  delete morfo;
  delete tagger;
  delete phon;
  delete neclass;
  delete sens;
  delete dsb;
  delete parser;
  delete dep;

  delete cfg;
}

/// ----------------------------------------------
/// Threaded function executed by get_input thread.
/// Reads from stdin and sends data to the tokenizer pipe.
//---------------------------------------------

void read_text(FL_pipe &o) {
  wstring *line = new wstring;
  while (getline(wcin,*line)) {
    o.send((void*)line);
    line = new wstring;
  }
  o.close_write();
}

///////////////////////////////////////////////////////////////////
///
///        MAIN PROGRAM
///
///////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

  /// Create language processors in threads
  CreateAnalyzers(argc,argv);
  config *cfg = new config(argc,argv);

  io::output_freeling out;
  out.output_senses(cfg->analyzer_invoke_options.SENSE_WSD_which!=NO_WSD);
  out.output_all_senses(cfg->analyzer_invoke_options.SENSE_WSD_which!=MFS);
  out.output_phonetics(cfg->analyzer_invoke_options.PHON_Phonetics);
  out.output_dep_tree(cfg->analyzer_invoke_options.OutputLevel==DEP);
  out.output_corefs(cfg->analyzer_invoke_options.OutputLevel==COREF);

  // launch a thread that reads wcin and sends data to the first module in chain
  boost::thread get_input(read_text,*pipes[0]);
  
  // wait for output to come out from last module output pipe, and print it.
  int lm = pipes.size()-1;   
  list<sentence> *ls;
  while ( (ls = (list<sentence>*) pipes[lm]->receive()) ) {
    // process results
    out.PrintResults(wcout,*ls);
    // free memory
    delete ls;
  }

  // clean up, delete pipes
  for (size_t i=0; i<pipes.size(); i++)
    delete pipes[i];

  // delete analyzers
  cleanup();
}

