

#include "freeling/morfo/util.h"
#include "freeling/morfo/analyzer.h"
#include "freeling/morfo/summarizer.h"

using namespace std;
using namespace freeling;

/// predeclarations
analyzer::config_options fill_config(const wstring &path, const wstring &lang);
analyzer::invoke_options fill_invoke();


//////////////   MAIN PROGRAM  /////////////////////

int main (int argc, char **argv) {

  //// set locale to an UTF8 compatible locale
  util::init_locale(L"default");
  
  if (argc < 3) {
    wcerr<<L"Usage:  ./summary lang num_words [FreeLing_install_path]  < doc-to-summarize.txt" << endl;
    exit(1);
  }
  wstring lang = util::string2wstring(argv[1]);
  int num_words = util::wstring2int(util::string2wstring(argv[2]));
  
  /// read FreeLing installation path if given, use default otherwise
  wstring ipath;
  if (argc < 4) ipath = L"/usr/local";
  else ipath = util::string2wstring(argv[3]);
  
  wcerr << L"Loading analyzers..." << endl;
  /// set config options (which modules to create, with which configuration)
  analyzer::config_options cfg = fill_config(ipath + L"/share/freeling/", lang);
  /// create analyzer
  analyzer anlz(cfg);

  /// set invoke options (which modules to use)
  analyzer::invoke_options ivk = fill_invoke();
  if (lang==L"en" or lang==L"es") ivk.OutputLevel = COREF;
  anlz.set_current_invoke_options(ivk);

  // create summarizer
  wcerr << L"Loading summarizer..." << endl;
  summarizer sum(ipath + L"/share/freeling/" + lang + L"/summarizer.dat");

  /// load document to analyze
  wcerr << L"Reading input text..." << endl;
  wstring text, line;
  while (getline(wcin, line))
    text = text + line + L"\n";

  /// analyze text, leave result in doc
  wcerr << L"Analyzing..." << endl;
  document doc;
  anlz.analyze(text, doc, true);

  // summarize document
  wcerr << L"Summarizing..." << endl;
  list<pair<const sentence*,double> > selected_sentences = sum.summarize(doc, num_words);
  
  // output results
  for (list<pair<const sentence*,double> >::const_iterator s=selected_sentences.begin(); s!=selected_sentences.end(); s++) {
     size_t beg=(*s->first).front().get_span_start();
     size_t end=(*s->first).back().get_span_finish();
     wcout << s->second << L" [" << s->first->get_sentence_id() << L"] "<< text.substr(beg, end-beg) << endl;
  }

}


///////////////////////////////////////////////////
/// Load an ad-hoc set of configuration options

analyzer::config_options fill_config(const wstring &path, const wstring &lang) {

  analyzer::config_options cfg;

  /// Language of text to process
  cfg.Lang = lang;

  // path to language specific data
  wstring lpath = path + L"/" + cfg.Lang + L"/";

  /// Tokenizer configuration file
  cfg.TOK_TokenizerFile = lpath + L"tokenizer.dat";
  /// Splitter configuration file
  cfg.SPLIT_SplitterFile = lpath + L"splitter.dat";
  /// Morphological analyzer options
  cfg.MACO_Decimal = L".";
  cfg.MACO_Thousand = L",";
  cfg.MACO_LocutionsFile = lpath + L"locucions.dat";
  cfg.MACO_QuantitiesFile = lpath + L"quantities.dat";
  cfg.MACO_AffixFile = lpath + L"afixos.dat";
  cfg.MACO_ProbabilityFile = lpath + L"probabilitats.dat";
  cfg.MACO_DictionaryFile = lpath + L"dicc.src";
  cfg.MACO_NPDataFile = lpath + L"np.dat";
  cfg.MACO_PunctuationFile = path + L"common/punct.dat";
  cfg.MACO_ProbabilityThreshold = 0.001;

  /// NEC config file
  cfg.NEC_NECFile = lpath + L"nerc/nec/nec-ab-poor1.dat";
  /// Sense annotator and WSD config files
  cfg.SENSE_ConfigFile = lpath + L"senses.dat";
  cfg.UKB_ConfigFile = lpath + L"ukb.dat";
  /// Tagger options
  cfg.TAGGER_HMMFile = lpath + L"tagger.dat";
  cfg.TAGGER_ForceSelect = RETOK;
  /// Chart parser config file
  cfg.PARSER_GrammarFile = lpath + L"chunker/grammar-chunk.dat";
  /// Dependency parsers config files
  cfg.DEP_TxalaFile = lpath + L"dep_txala/dependences.dat";
  cfg.DEP_TreelerFile = lpath + L"dep_treeler/dependences.dat";
  /// Coreference resolution config file
  if (lang==L"en" or lang==L"es") cfg.COREF_CorefFile = lpath + L"coref/relaxcor/relaxcor.dat";

  return cfg;
}


///////////////////////////////////////////////////
/// Load an ad-hoc set of invoke options

analyzer::invoke_options fill_invoke() {

  analyzer::invoke_options ivk;

  /// Level of analysis in input and output
  ivk.InputLevel = TEXT;
  ivk.OutputLevel = SENSES;

  /// activate/deactivate morphological analyzer modules
  ivk.MACO_UserMap = false;
  ivk.MACO_AffixAnalysis = true;
  ivk.MACO_MultiwordsDetection = true;
  ivk.MACO_NumbersDetection = true;
  ivk.MACO_PunctuationDetection = true;
  ivk.MACO_DatesDetection = true;
  ivk.MACO_QuantitiesDetection  = true;
  ivk.MACO_DictionarySearch = true;
  ivk.MACO_ProbabilityAssignment = true;
  ivk.MACO_CompoundAnalysis = false;
  ivk.MACO_NERecognition = true;
  ivk.MACO_RetokContractions = true;

  ivk.PHON_Phonetics = false;
  ivk.NEC_NEClassification = true;

  ivk.SENSE_WSD_which = UKB;
  ivk.TAGGER_which = HMM;
  ivk.DEP_which = TREELER;

  return ivk;
}
