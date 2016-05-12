
#include <ctime>
#include <iostream>
#include "freeling.h"
#include "test_speed_config.h"

using namespace std;

double seconds(clock_t tmp) {
  return (double)tmp/(double)CLOCKS_PER_SEC;
}

int main(int argc, char* argv[]) {
  
  clock_t b0,bini,bend;

  if (argc<2) {
    wcerr<<endl;
    wcerr<<L"Usage: test-speed -f file.cfg  < mytext.txt "<<endl;
    wcerr<<endl;
    wcerr<<L"  Where:  "<<endl;
    wcerr<<L"     - file.cfg is the absolute path to a FreeLing configuration file"<<endl;
    wcerr<<L"     - mytext.txt is a file containing plain text. The file should contain at"<<endl;
    wcerr<<L"       least a few thousands words to obtain reliable measurements."<<endl;
    wcerr<<endl;
    wcerr<<L"  You will need to define $FREELINGSHARE to point to the share/freeling directory"<<endl;
    wcerr<<L"  of your FreeLing installation."<<endl;
    wcerr<<endl;
    exit(1);
  }

  bini=clock();

  config *cfg = new config(argc,argv);

  b0=clock();
  freeling::util::init_locale(cfg->Locale);
  freeling::tokenizer tk(cfg->analyzer_config_options.TOK_TokenizerFile); 

  freeling::splitter sp(cfg->analyzer_config_options.SPLIT_SplitterFile); 
  freeling::splitter::session_id sp_ses = sp.open_session();

  freeling::maco_options opt(cfg->analyzer_config_options.Lang);
  opt.set_nummerical_points (cfg->analyzer_config_options.MACO_Decimal, cfg->analyzer_config_options.MACO_Thousand);
  opt.set_threshold (cfg->analyzer_config_options.MACO_ProbabilityThreshold);
  opt.set_inverse_dict(false);
  opt.set_retok_contractions(cfg->analyzer_invoke_options.MACO_RetokContractions);
  opt.set_data_files (cfg->analyzer_config_options.MACO_UserMapFile,
                      cfg->analyzer_config_options.MACO_PunctuationFile, cfg->analyzer_config_options.MACO_DictionaryFile,
                      cfg->analyzer_config_options.MACO_AffixFile, cfg->analyzer_config_options.MACO_CompoundFile,
                      cfg->analyzer_config_options.MACO_LocutionsFile, cfg->analyzer_config_options.MACO_NPDataFile,
                      cfg->analyzer_config_options.MACO_QuantitiesFile, cfg->analyzer_config_options.MACO_ProbabilityFile);
  freeling::maco morfo(opt);
  morfo.set_active_options (cfg->analyzer_invoke_options.MACO_UserMap, cfg->analyzer_invoke_options.MACO_NumbersDetection,
                            cfg->analyzer_invoke_options.MACO_PunctuationDetection, cfg->analyzer_invoke_options.MACO_DatesDetection,
                            cfg->analyzer_invoke_options.MACO_DictionarySearch, cfg->analyzer_invoke_options.MACO_AffixAnalysis,
                            cfg->analyzer_invoke_options.MACO_CompoundAnalysis, cfg->analyzer_invoke_options.MACO_RetokContractions,
                            cfg->analyzer_invoke_options.MACO_MultiwordsDetection, cfg->analyzer_invoke_options.MACO_NERecognition,
                            cfg->analyzer_invoke_options.MACO_QuantitiesDetection, cfg->analyzer_invoke_options.MACO_ProbabilityAssignment);

  freeling::hmm_tagger tagger(cfg->analyzer_config_options.TAGGER_HMMFile, cfg->analyzer_config_options.TAGGER_Retokenize, cfg->analyzer_config_options.TAGGER_ForceSelect);

  freeling::senses *sens=NULL;
  freeling::ukb *wsd=NULL;
  if (cfg->analyzer_invoke_options.SENSE_WSD_which == UKB) {
    sens = new freeling::senses(cfg->analyzer_config_options.SENSE_ConfigFile);
    wsd = new freeling::ukb(cfg->analyzer_config_options.UKB_ConfigFile);
  }

  freeling::chart_parser parser(cfg->analyzer_config_options.PARSER_GrammarFile);
  freeling::dep_txala dep1(cfg->analyzer_config_options.DEP_TxalaFile, parser.get_start_symbol ());

  freeling::dep_treeler *dep2=NULL;
  if (not cfg->analyzer_config_options.DEP_TreelerFile.empty())
    dep2 = new freeling::dep_treeler(cfg->analyzer_config_options.DEP_TreelerFile);


  double timeinit = clock()-b0;

  int nw=0,ns=0;
  wstring text;
  double timetok=0, timesplit=0, timemorfo=0, timetag=0, timesenses=0, timeukb=0, timeparser=0, timedep1=0, timedep2=0;
  while (getline(wcin,text)) {
    b0=clock();
    list<freeling::word> lw;
    tk.tokenize(text,lw);
    timetok += clock()-b0;

    b0=clock();
    list<freeling::sentence> ls;
    sp.split(sp_ses, lw, false, ls);
    timesplit += clock()-b0;

    b0=clock();
    morfo.analyze(ls);
    timemorfo += clock()-b0;
	
    b0=clock();
    tagger.analyze(ls);
    timetag += clock()-b0;

    if (cfg->analyzer_invoke_options.SENSE_WSD_which == UKB) {
      b0=clock();
      sens->analyze(ls);
      timesenses += clock()-b0;
      
      b0=clock();
      wsd->analyze(ls);
      timeukb += clock()-b0;
    }

    b0=clock();
    parser.analyze(ls);
    timeparser += clock()-b0;

    if (dep2!=NULL) {
      b0=clock();
      dep2->analyze(ls);
      timedep2 += clock()-b0;
    }

    b0=clock();
    dep1.analyze(ls);
    timedep1 += clock()-b0;

    for (list<sentence>::iterator s=ls.begin(); s!=ls.end(); s++) {
      ns++;
      nw+=s->size();
    }
  }

  bend=clock();

  wcerr<<L"Analyzed "<<nw<<" words in "<<ns<<" sentences."<<endl;
  wcerr<<L"Time (seconds) spent in: "<<endl;
  wcerr<<L"  Creating modules: "<<seconds(timeinit)<<endl;
  wcerr<<L"  Tokenizing = "<<seconds(timetok)<<" ("<<nw/seconds(timetok)<<" w/s)"<<endl;
  wcerr<<L"  Splitting = "<<seconds(timesplit)<<" ("<<nw/seconds(timesplit)<<" w/s)"<<endl;
  wcerr<<L"  Morfing = "<<seconds(timemorfo)<<" ("<<nw/seconds(timemorfo)<<" w/s)"<<endl;
  wcerr<<L"  Tagging = "<<seconds(timetag)<<" ("<<nw/seconds(timetag)<<" w/s)"<<endl;
  if (cfg->analyzer_invoke_options.SENSE_WSD_which == UKB) {
    wcerr<<L"  Annotating senses = "<<seconds(timesenses)<<" ("<<nw/seconds(timesenses)<<" w/s)"<<endl;
    wcerr<<L"  Disambiguating senses = "<<seconds(timeukb)<<" ("<<nw/seconds(timeukb)<<" w/s)"<<endl;
  }
  wcerr<<L"  Chunking = "<<seconds(timeparser)<<" ("<<nw/seconds(timeparser)<<" w/s)"<<endl;
  wcerr<<L"  Dep-parsing txala = "<<seconds(timedep1)<<" ("<<nw/seconds(timedep1)<<" w/s)"<<endl;
  wcerr<<L"  Dep-parsing treeler = "<<seconds(timedep2)<<" ("<<nw/seconds(timedep2)<<" w/s)"<<endl;

  double fl_time;
  fl_time=timetok+timesplit+timemorfo+timetag+timesenses+timeukb+timeparser+timedep1;
  wcerr<<L"  TOTAL FreeLing services (with txala) = "<<seconds(fl_time)<<" ("<<nw/seconds(fl_time)<<" w/s)"<<endl;
  if (dep2!=NULL) {
    fl_time=timetok+timesplit+timemorfo+timetag+timesenses+timeukb+timeparser+timedep2;
    wcerr<<L"  TOTAL FreeLing services (with treeler) = "<<seconds(fl_time)<<" ("<<nw/seconds(fl_time)<<" w/s)"<<endl;
  }

  wcerr<<L"TOTAL execution= "<<seconds(bend-bini)<<endl;

  sp.close_session(sp_ses);
  delete cfg; delete sens; delete wsd; delete dep2;
}

