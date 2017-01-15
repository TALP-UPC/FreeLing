#include <iostream>
#include "freeling.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;
using namespace freeling;

#define MOD_TRACECODE CORRECTOR_TRACE
#define MOD_TRACENAME L"NORMALITZADOR"

///---------------------------------------------
/// Analyzers
///---------------------------------------------

/////////////////////////////////////////////////////////////////////
/////////  
/////////                 MAIN PROGRAM 
/////////  
/////////////////////////////////////////////////////////////////////

int main (int argc, char **argv) {

  /// set locale to an UTF8 compatible locale
  freeling::util::init_locale(L"default");

  /// path to freeling installation to be used
  wstring ipath;
  if (argc < 2) ipath=L"/usr/local";
  else ipath=freeling::util::string2wstring(argv[1]);

  /// path to data files
  wstring path=ipath+L"/share/freeling/";

  // set the language to your desired target
  const wstring lang=L"es";


  // if FreeLing was compiled with --enable-traces, you can activate
  // the required trace verbosity for the desired modules.
  //freeling::traces::TraceLevel=6;
  //freeling::traces::TraceModule=0x0E800000;
  
  // create modules
  // create analyzers
  freeling::tokenizer tk(path+lang+L"/tokenizer.dat"); 
  freeling::splitter sp(path+lang+L"/splitter.dat");
  freeling::splitter::session_id sid=sp.open_session();

  // morphological analysis has a lot of options, and for simplicity they are packed up
  // in a maco_options object. First, create the maco_options object with default values.
  maco_options opt(L"es");  
  // alternatively, you can set active modules in a single call:
  //     opt.set_active_modules(false, true, true, true, true, true, false, true, true, 0);

  // and provide files for morphological submodules. Note that it is not necessary
  // to set opt.QuantitiesFile, since Quantities module was deactivated.
  opt.UserMapFile=L"";
  opt.LocutionsFile=path+lang+L"/locucions.dat"; opt.AffixFile=path+lang+L"/afixos.dat";
  opt.ProbabilityFile=path+lang+L"/probabilitats.dat"; opt.DictionaryFile=path+lang+L"/dicc.src";
  opt.NPdataFile=path+lang+L"/np.dat"; opt.PunctuationFile=path+L"/common/punct.dat"; 
  // alternatively, you can set the files in a single call:
  // opt.set_data_files("", path+"locucions.dat", "", path+"afixos.dat",
  //                   path+"probabilitats.dat", opt.DictionaryFile=path+"maco.db",
  //                   path+"np.dat", path+"../common/punct.dat");

  // create the analyzer with the just build set of maco_options
  maco morfo(opt); 
  // then, set required options on/off  
  morfo.set_active_options (false,// UserMap
                             true, // NumbersDetection,
                             true, //  PunctuationDetection,
                             true, //  DatesDetection,
                             true, //  DictionarySearch,
                             true, //  AffixAnalysis,
                             false, //  CompoundAnalysis,
                             true, //  RetokContractions,
                             true, //  MultiwordsDetection,
                             true, //  NERecognition,
                             false, //  QuantitiesDetection,
                             false);  //  ProbabilityAssignment

  // create alternative porposers
  freeling::alternatives alts_ort(path+lang+L"/alternatives-ort.dat");

  // IMPORTANT: comment this out if there is no phonetic encoder for target language
  //freeling::alternatives alts_phon(path+lang+L"/alternatives-phon.dat");
  //freeling::alternatives alts_key(path+lang+L"/alternatives-key.dat");

  // get plain text input lines while not EOF.
  wstring text;
  list<freeling::word> lw;
  list<freeling::sentence> ls;
  while (getline(wcin,text)) {
    
    // tokenize input line into a list of words
    lw=tk.tokenize(text);
    // split into sentences, flushing buffer at each line
    ls=sp.split(sid, lw, true);    
    // perform morphosyntactic analysis 
    morfo.analyze(ls);

    // propose alternative forms
    alts_ort.analyze(ls);

    // IMPORTANT: comment this out if there is no phonetic encoder for your language    
    //alts_phon.analyze(ls);

    // print results.
    for (list<freeling::sentence>::iterator s=ls.begin(); s!=ls.end(); s++) {
      for (freeling::sentence::iterator w=s->begin(); w!=s->end(); w++) {
        wcout<<L"FORM: "<<w->get_form()<<endl; 
        wcout<<L"   ANALYSIS:";
        for (freeling::word::iterator a=w->begin(); a!=w->end(); a++) 
          wcout<<L" ["<<a->get_lemma()<<L","<<a->get_tag()<<L"]";
        wcout<<endl;
        wcout<<L"   ALTERNATIVE FORMS:";
        for (list<freeling::alternative>::iterator a=w->alternatives_begin(); a!=w->alternatives_end(); a++) 
           wcout<<L" ["<< a->get_form() <<L","<< a->get_distance() <<L"]";
        wcout<<endl; 
      }
      wcout<<endl;
    }

    // clear temporary lists;
    lw.clear(); ls.clear();    
  }
  
  sp.close_session(sid);
}

