//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////

#include <fstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/foma_FSM.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"FOMA_FSM"
#define MOD_TRACECODE ALTERNATIVES_TRACE



wstring print_sigma(struct sigma *sigma) {
  int size;
  wstring sgm = L"Sigma:";
  for (size = 0; sigma != NULL; sigma = sigma->next) {
      if (sigma->number > 2) {
          sgm += L" " + util::string2wstring(string(sigma->symbol));
          size++;
      }
      if (sigma->number == IDENTITY) {
          sgm += L" IDENT@";
      }
      if (sigma->number == UNKNOWN) {
          sgm += L" UNKNW?";
      }
  }
  sgm += L" (Size: "+util::int2wstring(size)+L")";
  return sgm;
}

  ///////////////////////////////////////////////////////////////
  /// Create regular foma FSM from given text file
  ///////////////////////////////////////////////////////////////

  foma_FSM::foma_FSM(const wstring &fname, const wstring &mcost, const list<wstring> &joins) {

    // check that file type and cost matrix are consistent.
    // (Cost matrix is not allowed with binary files, it should
    //  be compiled into the binary file)
    if (not mcost.empty() and fname.rfind(L".bin")==fname.rfind(L".")) 
      ERROR_CRASH(L"Unexpected cost matrix given with binary FSA file.");
    
    // load dictionary into a FSA
    fsa = load_dictionary_file(fname);
    TRACE(3,L"loaded FSA from dictionary. "+print_sigma(fsa->sigma));
    
    // is join character list provided, convert to compound FSA
    if (not joins.empty()) {
      create_compound_FSA(joins);
      TRACE(3,L"created compound FSA. "+print_sigma(fsa->sigma));
    }

    // set SED operations cost
    set_basic_operation_cost(1000);
    // If a cost matrix is provided, load it
    if (not mcost.empty()) 
      load_cost_matrix(mcost);

    // if compound FSA, set costs for declared join characters
    if (not joins.empty()) {
      set_operation_cost(L"_", L"", 1);
      for (list<wstring>::const_iterator s=joins.begin(); s!=joins.end(); s++) 
        set_operation_cost(L"_", *s, 0);
    }

    // initialize for minimum edit distance searches
    init_MED();
  }

  ///////////////////////////////////////////////////////////////
  /// Create regular foma FSM from given text buffer
  ///////////////////////////////////////////////////////////////

  foma_FSM::foma_FSM(wistream &buff, const wstring &mcost, const list<wstring> &joins) {

    // load dictionary into a FSA
    fsa = load_dictionary_buffer(buff);
    TRACE(3,L"loaded FSA from dictionary. "+print_sigma(fsa->sigma));
    
    // is join character list provided, convert to compound FSA
    if (not joins.empty()) {
      create_compound_FSA(joins);
      TRACE(3,L"created compound FSA. "+print_sigma(fsa->sigma));
    }

    // set SED operations cost
    set_basic_operation_cost(1000);
    // If a cost matrix is provided, load it
    if (not mcost.empty())
      load_cost_matrix(mcost);

    // if compound FSA, set costs for declared join characters
    if (not joins.empty()) {
      set_operation_cost(L"_", L"", 1);
      for (list<wstring>::const_iterator s=joins.begin(); s!=joins.end(); s++) 
        set_operation_cost(L"_", *s, 0);
    }

    // initialize for minimum edit distance searches
    init_MED();
  }

  ///////////////////////////////////////////////////////////////
  /// Destructor, free foma structs
  ///////////////////////////////////////////////////////////////

  foma_FSM::~foma_FSM() {
    // free med lookup and matrix
    apply_med_clear(h_fsa);
    fsm_destroy(fsa);
  }


  ///////////////////////////////////////////////////////////////
  /// Set maximum edit distance to retrieve
  ///////////////////////////////////////////////////////////////

  void foma_FSM::set_cutoff_threshold(int thr) {
    apply_med_set_med_cutoff(h_fsa, thr); // max distance for matches
  }

  ///////////////////////////////////////////////////////////////
  /// Set maximum number of matches to retrieve
  ///////////////////////////////////////////////////////////////

  void foma_FSM::set_num_matches(int max) {
    apply_med_set_med_limit(h_fsa, max);      // max of matches to get
  }

  ///////////////////////////////////////////////////////////////
  /// Set cost for basic SED operations to given value
  ///////////////////////////////////////////////////////////////

  void foma_FSM::set_basic_operation_cost(int cost) {
    cmatrix_init(fsa);
    cmatrix_default_insert(fsa,cost);
    cmatrix_default_delete(fsa,cost);
    cmatrix_default_substitute(fsa,cost);
  }

  ///////////////////////////////////////////////////////////////
  /// Set cost for a particular SED operation
  ///////////////////////////////////////////////////////////////

  void foma_FSM::set_operation_cost(const wstring &in, const wstring &out, int cost) {
    char *pin = NULL;
    if (not in.empty()) {
      pin = new char[8];
      strcpy(pin,util::wstring2string(in).c_str());
    }
    char *pout = NULL;
    if (not out.empty()) {
      pout = new char[8];
      strcpy(pout,util::wstring2string(out).c_str());
    }
    cmatrix_set_cost(fsa, pin, pout, cost);

    delete [] pin;
    delete [] pout;
  }


  ////////////////////////////////////////////////////////////////////////
  /// Use automata to obtain closest matches to given form, 
  /// adding them (and the distance) to given list.
  ////////////////////////////////////////////////////////////////////////

  void foma_FSM::get_similar_words(const wstring &form, list<freeling::alternative> &alts) const {

    TRACE(3,L"Copying to char buffer");
    // convert input const wstring to non-const char* to satisfy foma API
    char* search = new char[form.size()*sizeof(wchar_t)+1];
    strcpy(search,util::wstring2string(form).c_str());

    set<wstring> seen;  // avoid duplicates, keep only first occurrence (lowest cost)

    // get closest match for search form
    TRACE(3,L"Search closest match");
    char *result = apply_med(h_fsa, search);
    while (result) {
      wstring alt = util::string2wstring(string(result));
      // if result is new, add to list      
      if (seen.find(alt)==seen.end()) {  
        // it is no longer new
        seen.insert(alt);
        // get distance
        int c = apply_med_get_cost(h_fsa);
        // store alternative in list
        alts.push_back(alternative(alt,c));
      }
    
      // Call with NULL on subsequent calls to get next alternatives
      result = apply_med(h_fsa, NULL);
    }

    free(result);
    delete[] search;
  }


  /// ----------------- Private methods ----------------------------

  ////////////////////////////////////////////////////////////////////////
  /// Auxiliary for constructors:  create a FSM loading a file
  ////////////////////////////////////////////////////////////////////////

  struct fsm* foma_FSM::load_dictionary_file(const wstring &fname) const {

    TRACE(3,L"Loading dictionary file");
    struct fsm *aut=NULL;

    // build FSA from text dictionary
    if (fname.rfind(L".src")==fname.rfind(L".")) {

      stringstream forms;      
      // Read text dictionary and dump keys into temp buffer.
      wifstream fabr;
      util::open_utf8_file(fabr,fname);
      if (fabr.fail()) ERROR_CRASH(L"Error opening file "+fname);    
      wstring line; 
      while (getline(fabr,line)) {
        // get key
        wistringstream sin; sin.str(line);
        wstring key; sin>>key;
        // add key to temp buffer for FOMA
        forms << util::wstring2string(key) << endl;
      }
      fabr.close();
     
      aut = fsm_read_text_file(forms.str().c_str());     // create FSA
      aut = fsm_minimize(aut);       
    }

    // Binary file with previously compiled FSA
    else if (fname.rfind(L".bin")==fname.rfind(L".")) { 
      char saux[1024];  // auxiliar for string conversion
      strcpy(saux,util::wstring2string(fname).c_str());
      aut = fsm_read_binary_file(saux);
    }

    // wrong file name
    else 
      ERROR_CRASH(L"Unknown file extension for '"+fname+L". Expected '.src' or '.bin'");

    // return built automata
    return aut;
  }

  ////////////////////////////////////////////////////////////////////////
  /// Auxiliary for constructors:  create a FSM from a text buffer
  ////////////////////////////////////////////////////////////////////////

  struct fsm* foma_FSM::load_dictionary_buffer(wistream &buff) const {

    TRACE(3,L"Loading dictionary buffer");
    struct fsm *aut=NULL;

    std::wstringstream text;
    text << buff.rdbuf();
    aut = fsm_read_text_file(util::wstring2string(text.str()).c_str());     // create FSA
    aut = fsm_minimize(aut);       
    
    // return built automata
    return aut;
  }

  ///////////////////////////////////////////////////////////////
  /// Auxiliary for constructors: Load cost matrix, completing alphabet if necessary
  ///////////////////////////////////////////////////////////////

  void foma_FSM::load_cost_matrix(const wstring &mcost) {

    TRACE(3,L"Loading cost matrix: "+mcost);

    // complete FSM alphabet with symbols from cost matrix.
    complete_alphabet(mcost);
    
    // read cost matrix file into a string buffer.
    std::ifstream mc(util::wstring2string(mcost).c_str());
    std::stringstream buffer; buffer << mc.rdbuf();
    char *cms = new char[buffer.str().size()+1];
    strcpy(cms,buffer.str().c_str());
    
    // load cost matrix and associate it to the automata
    my_cmatrixparse(fsa,cms);
    
    delete[] cms;    // clean up

    TRACE(3,L"Loaded cost matrix. "+print_sigma(fsa->sigma));
  }


  ////////////////////////////////////////////////////////////////////////
  /// Auxiliary for constructors:  create a compound-detector FSM 
  ////////////////////////////////////////////////////////////////////////

  void foma_FSM::create_compound_FSA(const list<wstring> &joins) {

    // build L(_L)+ from given dictionary L
    struct fsm *fsaorig = fsm_copy(fsa);
    char undersc[8];
    strcpy(undersc,"[\"_\"]");
    fsa = fsm_concat(fsm_parse_regex(undersc,NULL,NULL),fsa);
    fsa = fsm_kleene_plus(fsa);
    fsa = fsm_concat(fsaorig,fsa);
    // determinize and minimize
    fsa = fsm_minimize(fsa); 

    // add "join" symbols to alphabet
    set<wstring> alf = get_alphabet();
    alf.insert(joins.begin(),joins.end());
    update_FSM_alphabet(alf);
  }


  ///////////////////////////////////////////////////////////////
  // Auxiliary for constructors: initialize FSM for minimum edit
  // distance searches
  ///////////////////////////////////////////////////////////////

  void foma_FSM::init_MED() {
    // obtain med handle
    h_fsa = apply_med_init(fsa);

    // set search parameters
    apply_med_set_heap_max(h_fsa, 4194304);  // max heap to use (4MB)
    apply_med_set_med_limit(h_fsa, 20);      // max of matches to get
  }


  ////////////////////////////////////////////////////////////////////////
  /// Auxiliary for constructor: Complete FSM alphabet with any symbol 
  /// in cost matrix it may be missing. 
  ////////////////////////////////////////////////////////////////////////
 
  void foma_FSM::complete_alphabet(const wstring &mcost) {

    TRACE(3,L"Completing alphabet with cost matrix symbols. "+print_sigma(fsa->sigma) );

    // remember size of the FSA alphabet 
    set<wstring> alph = get_alphabet();
    size_t alphsz = alph.size();
    
    // read cost matrix, and add any missing symbols to alphabet
    wifstream fabr;
    util::open_utf8_file(fabr,mcost);
    if (fabr.fail()) ERROR_CRASH(L"Error opening file "+mcost);
    wstring line;
    while (getline(fabr,line)) {
      if (line[0]==L' ') {
        list<pair<wstring,wstring> > p = freeling::util::wstring2pairlist<wstring,wstring>(line.substr(1),L":",L" ");
        for (list<pair<wstring,wstring> >::iterator k=p.begin(); k!=p.end(); k++) {
          alph.insert(k->first);
          alph.insert(k->second);
        }
      }
    }
    fabr.close();
    
    // if alphabet gained symbols from matrix, compose the automata with Sigma*,
    // to make sure the automata has all matrix symbols (even if it doesn't 
    // actually use them)
    if (alph.size() > alphsz) {
      TRACE(3,L"Alphabet was extended with cost matrix symbols (size=" + util::int2wstring(alph.size()) + L") "+util::set2wstring(alph,L" "));
      update_FSM_alphabet(alph);
      TRACE(3,L"FSA alphabet updated " + print_sigma(fsa->sigma));
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // Update FSM alphabet by composing it with Sigma* to make sure the 
  // automata has all needed symbols (even if it doesn't actually use them)
  ////////////////////////////////////////////////////////////////////////
  
  void foma_FSM::update_FSM_alphabet(const set<wstring> &alph) {
 
    wstring alph_RE = L"[\"" + freeling::util::set2wstring(alph,L"\"|\"") + L"\"]*";
    char saux[1024];  
    strcpy(saux,util::wstring2string(alph_RE).c_str());
    struct fsm *alf = fsm_parse_regex(saux,NULL,NULL);
    fsa = fsm_compose(fsa,alf);    
  }

  ////////////////////////////////////////////////////////////////////////
  // Get FSM alphabet
  ////////////////////////////////////////////////////////////////////////
  
  set<wstring> foma_FSM::get_alphabet() {
    set<wstring> alph;
    struct sigma *sigma;
    for (sigma=fsa->sigma; sigma != NULL; sigma = sigma->next) {
      if (sigma->number > 2) {
        alph.insert(util::string2wstring(string(sigma->symbol)));
      }
      if (sigma->number == IDENTITY) {
        alph.insert(L"IDENT@");
      }
      if (sigma->number == UNKNOWN) {
        alph.insert(L"UNKNW?");
      }
    }
    return alph;
  }



} // namespace
