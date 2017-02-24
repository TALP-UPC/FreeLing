#include <iostream>
#include <fstream>

#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/corrector.h"
#include "include/remove_repeated.h"

using namespace std;

/// Do whatever is needed with corrected sentences
void ProcessResults(const list<freeling::sentence> &ls) {
  // print normalized text
  wcout << L"Normalized text: " << endl;
  for (list<freeling::sentence>::iterator s = ls.begin(); s != ls.end(); s++) {
    for (freeling::sentence::iterator w = s->begin(); w != s->end(); w++) {
      if (w->has_alternatives()) {
        for (list<alternative>::iterator a = w->alternatives_begin(); a != w->alternatives_end(); a++) {
          if (a->is_selected()) { // you can also use a->is_selected(i) to get the i-nth best selection (starting from 1)
            wcout << a->get_form() << " "; 
          }
        }
      } else {
        wcout << w->get_form() << " ";
      }
    }
  }
  wcout << endl << endl;
}

///////////////////////////////////////////////////
/// main program
///////////////////////////////////////////////////

int main(int argc, char* argv[]){

  // set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  // print usage if config-file missing
  if (argc > 3) {
    wcerr << L"Usage:  corrector lang freelingdir" << endl; 
    exit(1);
  }

  // language
  wstring lang = L"en";
  if (argc>=2) lang = freeling::util::string2wstring(argv[1]);
  /// path to data files
  wstring path = L"/usr/local/share/freeling";
  if (argc>=3) path = freeling::util::string2wstring(argv[2])
  path = path + L"/";                 
  
  // create a noisy text normalization module with the given config file
  wstring cfgFile = freeling::util::string2wstring(argv[1]);
  std::wcout << L"Initializing corrector module..." << std::flush;
  freeling::corrector corrector(path+lang+L"/corrector.dat");
  std::wcout << L" DONE" << std::endl;
  
  // accept twitter text (this detects @tags, #hashtags, :D smilies, url's, etc.)
  // set to false to use the default tokenizer and usermap
  bool is_twitter = true;
  
  // create modules and analyzer
  std::wcout << L"Initializing analyzer and other modules"<< L"..." << std::flush;

  wstring tokenizer_file = path + lang + L"/tokenizer.dat";
  wstring mapfile = L"";
  if (is_twitter) {
    std::wcout << L" (using twitter tokenizer and usermap)";
    tokenizer_file = path + lang + L"/twitter/tokenizer.dat";
    mapfile = path+lang+L"/twitter/usermap.dat";
  }
  freeling::tokenizer tk(tokenizer_file); 
  freeling::splitter sp(path + lang + L"/splitter.dat");
  freeling::splitter::session_id sid = sp.open_session();
  
  // morphological analysis module and options
  maco_options opt(lang);
  opt.UserMapFile=mapfile;
  opt.LocutionsFile=path+lang+L"/locucions.dat"; opt.AffixFile=path+lang+L"/afixos.dat";
  opt.ProbabilityFile=path+lang+L"/probabilitats.dat"; opt.DictionaryFile=path+lang+L"/dicc.src";
  opt.NPdataFile=path+lang+L"/np.dat"; opt.PunctuationFile=path+L"/common/punct.dat"; 
  maco morfo(opt);
  morfo.set_active_options (is_twitter, // UserMap
                            true,    // NumbersDetection,
                            true,    //  PunctuationDetection,
                            true,    //  DatesDetection,
                            true,    //  DictionarySearch,
                            true,    //  AffixAnalysis,
                            false,   //  CompoundAnalysis,
                            true,    //  RetokContractions,
                            true,    //  MultiwordsDetection,
                            true,    //  NERecognition,
                            false,   //  QuantitiesDetection,
                            false);  //  ProbabilityAssignment
  
  // create alternative porposers
  freeling::alternatives alts_ort(path + lang + L"/alternatives-ort.dat");
  freeling::alternatives alts_phon(path + lang + L"/alternatives-phon.dat");
  std::wcout << L" DONE" << endl;
  
  // process input
  wstring line;
  list<freeling::word> lw;
  list<freeling::sentence> ls;
  std::wcout << L"Write text to normalize: " << endl;
  while (getline(wcin, line)) {
    // get input as list of sentences
    lw = tk.tokenize(line);
    ls = sp.split(sid, lw, true);    
    // perform morphosyntactic analysis
    morfo.analyze(ls);
    
    // propose alternative forms
    alts_ort.analyze(ls);
    alts_phon.analyze(ls);
        
    // normalize text with corrector
    corrector.normalize(ls);

    // process corrected sentences
    ProcessResults(ls);
    
    // clear temporary lists
    lw.clear(); ls.clear();
  }
  
  sp.close_session(sid);
}
