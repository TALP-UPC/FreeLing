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

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>

#include "freeling/morfo/srl_treeler.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"

#include "treeler/base/sentence.h"
#include "treeler/util/options.h"
#include "treeler/control/factory-base.h"
#include "treeler/control/factory-scores.h"
#include "treeler/control/factory-dep.h"
#include "treeler/control/factory-ioconll.h"
#include "treeler/control/models.h"
#include "treeler/util/options.h"

#include "treeler/srl/srl_parser.h"

/*
#include "treeler/io/conllstream.h"
#include "treeler/srl/srl.h"
#include "treeler/srl/sentence.h"
#include "treeler/srl/io.h"
*/

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"SRL_TREELER"
#define MOD_TRACECODE SRL_TRACE


//---------- Class srl_treeler ----------------------------------

/////////////////////////////////////////////////////////////
//// Constructor. Read config file and load models

srl_treeler::srl_treeler(const wstring &config)  {

  TRACE(1,L"Loading module");

  wstring path=config.substr(0,config.find_last_of(L"/\\")+1);

  // default values
  string srl_cfg;
  wstring tagsetFile;
  map<wstring,wstring> prefiles;

  // configuration file
  enum sections {DEPENDENCIES, SRL};
  config_file cfg;  
  cfg.add_section(L"SRL",SRL);
  
  if (not cfg.open(config))
    ERROR_CRASH(L"Error opening file "+config);

  // read configuration file 
  wstring line; 
  while (cfg.get_content_line(line)) {
    
    switch (cfg.get_section()) {
      
    case SRL: { // reading default POS for predicates;
      wistringstream sin;
      sin.str(line);
      wstring key,name,pos;
      sin>>key;
      if (key==L"SRLTreeler") {
        sin>>name;
        srl_cfg = freeling::util::wstring2string(freeling::util::absolute(name,path));
      }
      else if (key==L"DefaultArgs") {
        while (sin>>name) 
          default_args.push_back(make_pair(freeling::util::wstring2string(name),""));
      }
      else if (key==L"Predicates") {
        sin>>pos>>name;
        prefiles.insert(make_pair(pos,freeling::util::absolute(name,path)));
        wstring all;
        if (sin>>all) {
          if (all==L"*") always_predicate.insert(freeling::util::wstring2string(pos));
          else WARNING(L"Warning: Ignoring unexpected token "+all+L" in Predicate definition in SRL section of file "+config+L"."); 
        }
      }
      else if (key==L"PredicateException") {
        sin>>name>>pos;
        pred_exceptions.insert(make_pair(freeling::util::wstring2string(name),freeling::util::wstring2string(pos)));
      }
      else
        WARNING(L"Error: Unknown parameter "+key+L" in SRL section of file "+config+L"."); 
    
      break;
    }

    default: break;
    }
  }
  cfg.close();

  // create required tagset handler 
  if (tagsetFile.empty()) {
      ERROR_CRASH(L"No Tagset specified configuration file "+config+L".");
  }
  tags = new freeling::tagset(tagsetFile);
  TRACE(3,L"Tagset loaded");
  
  // Create semantic role labeller, if required
  if (prefiles.empty()) {
      ERROR_CRASH(L"No Predicates file specified in "+config+L".");
  }
  if (srl_cfg.empty()) {
      ERROR_CRASH(L"No SRLTreeler configuration file specified in "+config+L".");
  }
  load_predicate_files(prefiles);
  treeler::Options srl_options;
  load_options(srl_cfg,srl_options);
  srl = new treeler::srl_parser(srl_options);
  
}

///////////////////////////////////////////////////////////////
/// Destructor
///////////////////////////////////////////////////////////////

srl_treeler::~srl_treeler() {
  delete srl;
}


///////////////////////////////////////////////////////////////
/// Enrich given sentence with a depenceny tree.
///////////////////////////////////////////////////////////////

void srl_treeler::analyze(freeling::sentence &s) const {

  TRACE(3,L"Analyzing sentence");

  // convert FL sentence to Treeler sentence
  treeler::dependency_parser::sentence tl_sentence;
  treeler::dependency_parser::dep_vector deptree;
  FL2Treeler(s, tl_sentence, deptree);

  // call SRL
  treeler::srl_parser::pred_arg_set semroles;
  treeler::srl::PossiblePreds preds;
  compute_predicates(tl_sentence, s, deptree, preds);
  srl->parse(tl_sentence, deptree, preds, semroles);
  TRACE(3,L"Sentence SRLed");

  // Add SRL produced by treeler to FL sentence
  Treeler2FL(s, semroles);

  // debugging
  // dep_tree::PrintDepTree(s.get_dep_tree().begin(), 0);
   
  TRACE(3,L"Sentence analyzed");
}



// -------------  Private methods -----------------------//

///////////////////////////////////////////////////////////////
/// Convert FreeLing sentence to Treeler example
///////////////////////////////////////////////////////////////

  void srl_treeler::FL2Treeler(const freeling::sentence& fl_sentence,
			       treeler::dependency_parser::sentence &tl_sentence,
			       treeler::dependency_parser::dep_vector &tl_tree) const {

  // best tagger sequence (first one by default, unless user selects another)
  int best = fl_sentence.get_best_seq();
  freeling::dep_tree const & dt = fl_sentence.get_dep_tree();
    
  // Create the treeler sentence
  // copy each word in FL sentence to treeler sentence, with appropriate changes
  size_t i=0;
  for (freeling::sentence::const_iterator wd = fl_sentence.begin(); wd!=fl_sentence.end(); ++wd) {

    // obtain token information
    string word = freeling::util::wstring2string(wd->get_form());
    string lemma = freeling::util::wstring2string(wd->get_lemma(best));

    wstring fpos = wd->get_tag(best);
    string fine_pos = freeling::util::wstring2string(fpos);
    string coarse_pos =  freeling::util::wstring2string(tags->get_short_tag(fpos));

    // build the treeler token
    treeler::dependency_parser::token tk(word, lemma, coarse_pos, fine_pos);

    // add msd features to the token
    list<wstring> msd = freeling::util::wstring2list(tags->get_msd_string(fpos),L"|");
    for (list<wstring>::iterator f=msd.begin(); f!=msd.end(); f++) 
      tk.morpho_push(freeling::util::wstring2string(*f));

    // add token to sentence
    tl_sentence.add_token(tk);

    // add dependency to dep vector
    freeling::dep_tree::const_iterator p = dt.get_node_by_pos(i);
    tl_tree.push_back(treeler::HeadLabelPair<string>(p.get_parent()->get_word().get_position()+1,
						     freeling::util::wstring2string(p->get_label())) );
    
    ++i;
  }

  
}


///////////////////////////////////////////////////////////////
/// Add treeler output tree to FreeLing sentence
///////////////////////////////////////////////////////////////

void srl_treeler::Treeler2FL(freeling::sentence &fl_sentence,
			     const treeler::srl_parser::pred_arg_set &tl_roles) const {

  TRACE(3, L"Converting SRL result to FreeLing predicates");

  // add sem roles to FL sentence   //fl_sentence.pred_args = tl_roles;
  TRACE(3, L"adding SRL output to sentence");
  for (treeler::srl::PredArgSet::const_iterator p=tl_roles.begin(); p!=tl_roles.end(); p++) {
    // create new predicate
    predicate pred(p->first, freeling::util::string2wstring(p->second.sense));
    // add arguments
    for (treeler::srl::PredArgStructure::const_iterator a=p->second.begin(); a!=p->second.end(); a++) {
      if (a->second!="_") 
        pred.add_argument(a->first,freeling::util::string2wstring(a->second));
    }

    // add predicate to the sentence
    fl_sentence.add_predicate(pred);
  }

  TRACE(3, L"dep_tree conversion finished");
}




///////////////////////////////////////////////////////////////
/// Load files containint predicate argument structures
///////////////////////////////////////////////////////////////

void srl_treeler::load_predicate_files(const map<wstring,wstring> &files) {

  for (map<wstring,wstring>::const_iterator f=files.begin(); f!=files.end(); f++) {

    // add a predicate list for current PoS
    map<string,map<wstring,treeler::srl::PossiblePredArgs> >::iterator pred;
    pred = predicates.insert(predicates.end(), make_pair(freeling::util::wstring2string(f->first),map<wstring,treeler::srl::PossiblePredArgs>()));

      // open file to load
    wifstream fnom;
    freeling::util::open_utf8_file(fnom, f->second);
    if (fnom.fail()) ERROR_CRASH(L"Error opening nominal predicate file "+f->second);

    // load file into predicate list
    wstring line;
    while (getline(fnom,line)) {
      wistringstream sin; sin.str(line);
      // read synset, and create list entry
      wstring syns,pr,arg;  
      sin >> syns;
      map<wstring,treeler::srl::PossiblePredArgs>::iterator pd = pred->second.insert(pred->second.end(),make_pair(syns,treeler::srl::PossiblePredArgs()));
      // read predicate PB code into the entry
      sin >> pr;
      list<pair<wstring,wstring> > pds = util::wstring2pairlist<wstring,wstring>(pr,L".",L"|");      
      for (list<pair<wstring,wstring> >::const_iterator p = pds.begin(); p!=pds.end(); p++) {
        pd->second.predsense.push_back(make_pair(util::wstring2string(p->first),
                                                util::wstring2string(p->second)));
      }

      // load arguments into the entry
      while (sin>>arg) {
        vector<wstring> v=freeling::util::wstring2vector(arg,L":");
        if (v.size()<2) v.push_back(L"");  
        pd->second.possible_args.push_back(make_pair(freeling::util::wstring2string(v[0]),
                                                    freeling::util::wstring2string(v[1])));
      }
    }
  }
}

///////////////////////////////////////////////////////////////
/// Returns true if a word should be a predicate but is not (e.g auxiliary verbs)
///////////////////////////////////////////////////////////////

bool srl_treeler::is_pred_exception(const treeler::srl_parser::basic_sentence &ts, size_t i, const treeler::srl_parser::dep_vector &dv) const {

  // look up pos:lemma in exceptions list
  map<string,string>::const_iterator exc;
  exc = pred_exceptions.find(ts.get_token(i).fine_pos().substr(0,1)+":"+ts.get_token(i).lemma());
  
  // if not found, it is not an exception
  if (exc==pred_exceptions.end()) return false;
  
  // if found with no child constraints, it is an exception
  if (exc->second=="*") return true;
  
  // if found with child constraints, check it has a child with required PoS
  for (size_t j=0; j<dv.size(); j++)
    if (dv[j].h==(int)i and ts.get_token(j).fine_pos().find(exc->second)==0)
      return true;
  
  // no matching child, not an exception
  return false;
}

///////////////////////////////////////////////////////////////
/// Identifies predicates
///////////////////////////////////////////////////////////////

void srl_treeler::compute_predicates(const treeler::srl_parser::basic_sentence &ts, 
                                     const freeling::sentence &fls,
                                     const treeler::srl_parser::dep_vector &dv, 
                                     treeler::srl::PossiblePreds &preds) const {  
  preds.clear();
  for (int i=0; i<ts.size(); i++) {
    const string &lemma = ts.get_token(i).lemma();
    const string &fpos = ts.get_token(i).fine_pos().substr(0,1);

    // look up PoS in predicates definitions
    map<string,map<wstring, treeler::srl::PossiblePredArgs> >::const_iterator pred = predicates.find(fpos);

    // if not a predicate. skip to next word
    if (pred==predicates.end()) continue;

    // PoS is a possible predicate. Look up synset
    bool found=false;
    const list<pair<wstring,double> > &senses = fls[i].get_senses(fls.get_best_seq());
    if (not senses.empty()) {
      map<wstring,treeler::srl::PossiblePredArgs>::const_iterator syn = pred->second.find(senses.begin()->first);
      if (syn!=pred->second.end()) {  // predicate found, use information in predicates list
        // insert new possible predicate in the list
        preds.insert(make_pair(i,syn->second));
        found=true;
      } 
    }
    
    if (not found and always_predicate.find(fpos)!=always_predicate.end() and not is_pred_exception(ts,i,dv) ) {
      // not in list, but the PoS is always_predicate and not an exception
      
      // insert new possible predicate in the list
      treeler::srl::PossiblePreds::iterator pd = preds.insert(preds.end(),make_pair(i,treeler::srl::PossiblePredArgs()));
      pd->second.predsense.push_back(make_pair(lemma,"00"));
      pd->second.possible_args = default_args;
    }
  }
}
  

///////////////////////////////////////////////////////////////
/// Load a treeler configuration file into a treeler::Options object
///////////////////////////////////////////////////////////////

void srl_treeler::load_options(const string &cfg, treeler::Options &opts) {

  string path = cfg.substr(0,cfg.find_last_of("/\\")+1);
  string name;
  
  // load options file
  TRACE(2,L"Loading treeler options from "+freeling::util::string2wstring(cfg));
  opts.read(cfg, true);
  
  // --- model parameters options ---
  
  // if wavg not set, set default value
  int wavg;
  if (not opts.get("wavg",wavg)) opts.set("wavg", 1, true);
  
  if (opts.get("wdir", name))
    opts.set("wdir", freeling::util::absolute(freeling::util::expand_filename(name),path), true);
  
  // --- dictionaries options ---  
  if (opts.get("dict-words", name))
    opts.set("dict-words", freeling::util::absolute(freeling::util::expand_filename(name),path), true);
  
  if (opts.get("dict-lemmas", name))
    opts.set("dict-lemmas", freeling::util::absolute(freeling::util::expand_filename(name),path), true);
  
  if (opts.get("dict-cpos", name))
    opts.set("dict-cpos", freeling::util::absolute(freeling::util::expand_filename(name),path), true);
  
  if (opts.get("dict-fpos", name)) {
    name = freeling::util::absolute(freeling::util::expand_filename(name),path);
    opts.set("dict-fpos", name, true);
    opts.set("fpos", name, true);
  }

  if (opts.get("dict-morphos", name))
    opts.set("dict-morphos", freeling::util::absolute(freeling::util::expand_filename(name),path), true);
  
  if (opts.get("dict-dependencies", name))
    opts.set("dict-dependencies", freeling::util::absolute(freeling::util::expand_filename(name),path), true);

  //load the role map dicts, if any
  if (opts.get("dict-roles", name))
    opts.set("dict-roles", freeling::util::absolute(freeling::util::expand_filename(name),path), true);

  #ifdef VERBOSE 
    if ((freeling::traces::TraceLevel > 3) and (freeling::traces::TraceModule & MOD_TRACECODE))
      opts.set("verbose", 1, true);
  #endif 
  
  TRACE(2,L"options loaded");
}


} // namespace
