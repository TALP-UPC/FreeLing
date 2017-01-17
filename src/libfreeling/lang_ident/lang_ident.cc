
// /////////////////////// Class classificador ///////////////////
//
//  Class that manage a probability calculator for each language
//  and test them all to determine the most likely language
//  from a specified text
//
// ////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/lang_ident.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"LANG_IDENT"
#define MOD_TRACECODE LANGIDENT_TRACE

  ///////////////////////////////////////////////////////////////////
  /// Empty constructor (e.g. for using in training)
  /////////////////////////////////////////////////////////////////

  lang_ident::lang_ident() {}

  ///////////////////////////////////////////////////////////////////
  /// constructor, given a config file
  /////////////////////////////////////////////////////////////////

  lang_ident::lang_ident(const wstring &idnFile) {

    wstring path=idnFile.substr(0,idnFile.find_last_of(L"/\\")+1);

    // config file
    enum sections {LANGUAGES};
    config_file cfg;  
    cfg.add_section(L"Languages",LANGUAGES);

    if (not cfg.open(idnFile)) {
      ERROR_CRASH(L"Error opening file "+idnFile);
    }

    // load list of languages and their model files
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {
  
      case LANGUAGES: {
        wistringstream sin;
        sin.str(line);
        wstring file;
        sin>>file; 
        // locate model file relative to main idnFile config file
        add_language(util::absolute(file,path));
        break;
      }

      default: break;
      }
    }
    cfg.close(); 

    TRACE(1,L"Module sucessfully loaded");
  }

  ///////////////////////////////////////////////////////////////////
  /// destructor
  ///////////////////////////////////////////////////////////////////

  lang_ident::~lang_ident() {
    for (map<wstring,idioma*>::iterator p=idiomes.begin(); p!=idiomes.end(); p++)
      delete p->second;
  }


  ///////////////////////////////////////////////////////////////////
  /// load a model for a new language, and add it to the known
  /// languages list
  ///////////////////////////////////////////////////////////////////

  void lang_ident::add_language(const wstring &modelFile) {
    idioma *id = new idioma(modelFile);
    idiomes.insert(make_pair(id->get_language_code(),id));
    all_known_languages.insert(id->get_language_code());
  }

  ///////////////////////////////////////////////////////////////////
  /// train a model for a language, store in modelFile, and add 
  /// it to the known languages list.
  ///////////////////////////////////////////////////////////////////

  void lang_ident::train_language(const wstring &textFile, const wstring &modelFile, const wstring &code, size_t order) {
    wifstream fin;
    util::open_utf8_file(fin, textFile);
    if (fin.fail()) {
      ERROR_CRASH(L"Error opening language model training text file '"+textFile+L"'.");
    }

    idioma::create_model(modelFile, fin, code, order, L'#');    
    add_language(modelFile);    
  }

  /////////////////////////////////////////////////////////////////////////
  /// Identify language of given text, considering only languages in
  /// given list (empty list--> all languages)
  ////////////////////////////////////////////////////////////////////////

  wstring lang_ident::identify_language (const wstring &text, const set<wstring>& ls) const {
    /// get probabilities for all languages
    vector<pair<double,wstring> > result;
    language_perplexities(result, text, ls);

    /// return language above the threshold with best probabiltiy
    wstring best_l = L"none";
    double best_p = 99999999;

    vector<pair<double,wstring> >::iterator k;
    for (k = result.begin(); k!=result.end(); k++) {
      TRACE(4,L"  Perplexity for language "+k->second+L" p="+util::double2wstring(k->first));
      if (k->first<best_p) {
        best_p = k->first;
        best_l = k->second;
        TRACE(4,L"  ...is new best with p="+util::double2wstring(k->first));
      }
    }    

    TRACE(2,L"  Best language is: "+best_l);
    if (best_l == L"none" or best_p <= idiomes.find(best_l)->second->get_threshold())
      return best_l;
    else {
      TRACE(2,L"  Perplexity is over threshold, rejected");
      return L"none";
    }
  }

  /////////////////////////////////////////////////////////////////////////
  /// Identify language of given text, considering only languages in
  /// given list (empty list--> all languages)
  ////////////////////////////////////////////////////////////////////////

  void lang_ident::rank_languages (vector<pair<double,wstring> > &result, const wstring &text, const set<wstring>& ls) const {

    /// get probabilities for all languages
    language_perplexities(result, text, ls);
    sort(result.begin(), result.end(), util::ascending_first<double,wstring>);
  }

  /////////////////////////////////////////////////////////////////////////
  /// Identify language of given text, considering only languages in
  /// given list (empty list--> all languages). Return copy, for python API
  ////////////////////////////////////////////////////////////////////////

  vector<pair<double,wstring> > lang_ident::rank_languages (const wstring &text,
                                                            const set<wstring> &ls) const {
    vector<pair<double,wstring> > result;
    rank_languages(result, text, ls);
    return result;
  }

  /////////////////////////////////////////////////////////////////////////
  /// Get probabilities for each language, considering only languages in
  /// given list (empty list--> all languages)
  ////////////////////////////////////////////////////////////////////////

  void lang_ident::language_perplexities (vector<pair<double,wstring> > &result, const wstring &text, const set<wstring>& ls) const {

    const set<wstring> *langs = &ls;
  
    if (ls.empty()) langs = &all_known_languages;

    result.clear();
    result.reserve(ls.size());

    /// check each candidate language
    set<wstring>::const_iterator li;
    for (li = langs->begin(); li!=langs->end(); li++) {
      map<wstring,idioma*>::const_iterator k = idiomes.find(*li);
      if (k==idiomes.end()) {
        WARNING(L"Unknown language "+(*li)+L" given as identification candidate.");
      }
      else {
        TRACE(3,L"Analyzing sequence for language "+k->first);
        // compute probability, store result.
        double prob = k->second->compute_perplexity(text);
        result.push_back(make_pair(prob,k->first));
      }    
    }
  }

} // namespace
