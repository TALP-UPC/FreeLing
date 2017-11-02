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


//////////////////////////////////////////////////////////
///  Auxiliary functions to print several analysis results
//////////////////////////////////////////////////////////

#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/output/output_freeling.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"OUTPUT_FREELING"
#define MOD_TRACECODE OUTPUT_TRACE

//---------------------------------------------
// Constructor
//---------------------------------------------

output_freeling::output_freeling() {
  // default values
  OutputSenses=true;
  AllSenses=false;
  OutputPhonetics=false;
  OutputDepTree=true;
  OutputCorefs=false;
}

//---------------------------------------------
// Constructor from cfg file
//---------------------------------------------

output_freeling::output_freeling(const wstring &cfgFile) {

  enum sections {OUTPUT_TYPE,OPTIONS};
  config_file cfg(true);
  cfg.add_section(L"Type",OUTPUT_TYPE);
  cfg.add_section(L"Options",OPTIONS);

  if (not cfg.open(cfgFile))
    ERROR_CRASH(L"Error opening file "+cfgFile);

  // default values
  OutputSenses=true;
  AllSenses=false;
  OutputPhonetics=false;
  OutputDepTree=true;
  OutputCorefs=false;
  
  wstring line; 
  while (cfg.get_content_line(line)) {
    
    // process each content line according to the section where it is found
    switch (cfg.get_section()) {
      
    case OUTPUT_TYPE: {
      if (util::lowercase(line)!=L"freeling")
        ERROR_CRASH(L"Invalid configuration file for 'freeling' output handler, "+cfgFile);
      break;
    }
      
    case OPTIONS: {  // reading Function words
      wistringstream sin; sin.str(line);
      wstring key,val;
      sin >> key >> val;
      val = util::lowercase(val);
      bool b = (val==L"true" or val==L"yes" or val==L"y" or val==L"on");
      if (key == L"OutputSenses") OutputSenses=b;
      else if (key == L"AllSenses") AllSenses=b;
      else if (key == L"OutputPhonetics") OutputPhonetics=b;
      else if (key == L"OutputDepTree") OutputDepTree=b;
      else if (key == L"OutputCorefs") OutputCorefs=b;
      else WARNING(L"Invalid option '"+key+L" in output_freeling config file '"+cfgFile+L"'");
      break;
    }
      
    default: break;
    }
  }
  
  cfg.close(); 

}

//---------------------------------------------
// Destructor
//---------------------------------------------

output_freeling::~output_freeling() {}


//---------------------------------------------
// Print senses information for an analysis
//---------------------------------------------

wstring output_freeling::outputSenses (const analysis & a) const {

  const list<pair<wstring,double> > & ls = a.get_senses ();

  // no sense list, return dash
  if (ls.empty()) return L" -";

  // output all senses, with weights    
  if (AllSenses) return L" " + util::pairlist2wstring (ls, L":", L"/");
  // output only first sense
  else return L" " + ls.begin()->first;
}

//---------------------------------------------
// print parse tree
//--------------------------------------------

void output_freeling::PrintTree (wostream &sout, parse_tree::const_iterator n, int depth, int best) const {

  parse_tree::const_sibling_iterator d;

  sout << wstring (depth * 2, ' ');  
  if (n.num_children() == 0) {
    if (n->is_head()) sout << L"+";
    const word & w = n->get_word();
    sout << L"(" << w.get_form() << L" " << w.get_lemma(best) << L" " << w.get_tag(best);
    sout << outputSenses ((*w.selected_begin(best)));
    sout << L")" << endl;
  }
  else {
    if (n->is_head()) sout << L"+";

    sout<<n->get_label();

    /*
    if (cfg->COREF_CoreferenceResolution) {
      // Print coreference group, if needed.
      int ref = doc.get_coref_group(n->get_node_id());
      if (ref != -1 and n->get_label() == L"sn") sout<<L"(REF:" << ref <<L")";
    }
    */
    sout << L"_[" << endl;

    for (d = n.sibling_begin (); d != n.sibling_end (); ++d) 
      PrintTree (sout, d, depth + 1, best);
    sout << wstring (depth * 2, ' ') << L"]" << endl;
  }
}



//---------------------------------------------
// print dependency tree
//---------------------------------------------

void output_freeling::PrintDepTree (wostream &sout, dep_tree::const_iterator n, int depth, int best) const {

  sout << wstring (depth*2, ' ');

  parse_tree::const_iterator pn = n->get_link();
  if (pn.is_defined()) {
    sout<<pn->get_label(); 

    /*
    int ref = (cfg->COREF_CoreferenceResolution ? doc.get_coref_group(pn->get_node_id()) : -1);
    if (ref != -1 and pn->get_label() == L"sn") {
      sout<<L"(REF:" << ref <<L")";
    }
    */
    sout<<L"/";
  }

  sout<< n->get_label() << L"/";  
  if (n->get_label()!=L"VIRTUAL_ROOT") {
    const word & w = n->get_word();
    sout << L"(" << w.get_form() << L" " << w.get_lemma(best) << L" " << w.get_tag(best);
    sout << outputSenses ((*w.selected_begin(best)));
    sout << L")";
  }
  
  if (n.num_children () > 0) {
    sout << L" [" << endl;
    
    // Sort children. Non-chunks first, then by their word position.
    // (this is just for aesthetic reasons)
    list<dep_tree::const_sibling_iterator> children;
    for (dep_tree::const_sibling_iterator d = n.sibling_begin (); d != n.sibling_end (); ++d)
      children.push_back(d);
    children.sort(ascending_position);


    // print children in the right order
    for (list<dep_tree::const_sibling_iterator>::const_iterator ch=children.begin();
         ch != children.end(); 
         ch++) 
      PrintDepTree (sout, (*ch), depth + 1, best);
       
    sout << wstring (depth * 2, ' ') << L"]";
  }
  sout << endl;
}


//---------------------------------------------
// print predicate-argument structure of the sentence
//---------------------------------------------

void output_freeling::PrintPredArgs(wostream &sout, const sentence &s) const {

  sout << L"Predicates {" << endl;

  const dep_tree & dt = s.get_dep_tree(s.get_best_seq());
  wstring sid = s.get_sentence_id();

  /// see which entities appear as arguments of predicates
  map<int,wstring> arg_words;
  map<int,pair<int,int> > arg_span;
  for (sentence::predicates::const_iterator pred=s.get_predicates().begin(); pred!=s.get_predicates().end(); pred++) {
    for (predicate::const_iterator arg=pred->begin(); arg!=pred->end(); arg++) {
      // if argument already computed in another predicate, skip it
      if (arg_words.find(arg->get_position())!=arg_words.end()) continue;
      
      // compute words and span for this argument
      wstring wds=L"";
      size_t wf = dep_tree::get_first_word(dt.get_node_by_pos(arg->get_position())); 
      size_t wl = dep_tree::get_last_word(dt.get_node_by_pos(arg->get_position())); 
      for (size_t w=wf; w<=wl; w++) {
        if (w!=wf) wds += L" ";
        wds += s[w].get_form();
      }
      arg_words.insert(make_pair(arg->get_position(), wds));  
      arg_span.insert(make_pair(arg->get_position(), make_pair(wf,wl)));        
    }
  }

  // print all predicates, referring to entities (or other predicates) as their arguments
  int npred=1;
  for (sentence::predicates::const_iterator pred=s.get_predicates().begin(); pred!=s.get_predicates().end(); pred++) {
    sout << L"   " << get_token_id(sid, npred, L"Pred") << L": " << pred->get_sense() 
         << L" " << get_token_id(sid,pred->get_position()+1) 
         << L" (" << s[pred->get_position()].get_form() << L")"
         << L" [" << endl;

    for (predicate::const_iterator arg=pred->begin(); arg!=pred->end(); arg++) {
      sout << L"              " << arg->get_role() << L" "
           << get_token_id(sid,arg->get_position()+1)
           << L" (" << arg_words[arg->get_position()] << L")"
           << L" [" << get_token_id(sid,arg_span[arg->get_position()].first+1) 
           << L" .. " << get_token_id(sid,arg_span[arg->get_position()].second+1) << L"]"
           << endl;
    }
    sout << L"            ]" << endl;
    npred++;
  }

  sout<<"}"<<endl;
}


//---------------------------------------------
// print analysis for a word
//---------------------------------------------

void output_freeling::PrintWord (wostream &sout, const word &w, bool only_sel, bool probs, int best) const {

  sout << w.get_form();
  if (OutputPhonetics) sout<<L" "<<w.get_ph_form();	  

  word::const_iterator a_beg,a_end;
  if (only_sel) {
    a_beg = w.selected_begin(best);
    a_end = w.selected_end(best);
  }
  else {
    a_beg = w.analysis_begin();
    a_end = w.analysis_end();
  }

  for (word::const_iterator ait = a_beg; ait != a_end; ait++) {
    if (ait->is_retokenizable ()) {
      const list <word> & rtk = ait->get_retokenizable();
      list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
      for (list<analysis>::iterator x=la.begin(); x!=la.end(); x++) {
        sout << L" " << x->get_lemma() << L" " << x->get_tag();
        if (probs) sout << L" " << ait->get_prob()/la.size();
      }
    }
    else {
      sout << L" " << ait->get_lemma() << L" " << ait->get_tag ();
      if (probs) sout << L" " << ait->get_prob ();
    }

    if (OutputSenses)
      sout << outputSenses (*ait);
  }
}

//---------------------------------------------
// print coreference groups of the document
//---------------------------------------------

void output_freeling::PrintCorefs(wostream &sout, const document &doc) const {

  map<mention::mentionType,wstring> mtypes = { {mention::COMPOSITE, L"Composite"}, {mention::NOUN_PHRASE, L"Noun_phrase"},
                                               {mention::PROPER_NOUN, L"Proper_noun"}, {mention::PRONOUN, L"Pronoun"} };

  sout << L"Coreferences {" << endl;
  for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {
    sout << L"   Group " << *g+1 << L" {" << endl;      
    list<int> mentions = doc.get_coref_id_mentions(*g);
    int nm=1;
    for (list<int>::iterator m=mentions.begin(); m!=mentions.end(); m++) {
      const mention & ment = doc.get_mention(*m);
      const sentence & sent = *(ment.get_sentence());
      wstring sid = util::int2wstring(ment.get_n_sentence()+1);
      
      int j = ment.get_pos_begin();
      wstring words = sent[j].get_form();
      for (j=ment.get_pos_begin()+1; j<=ment.get_pos_end(); j++) 
        words = words + L" " + sent[j].get_form();
      
      sout << L"      mention " << *g+1 << L"." << nm 
           << L" (" << words << L")"
           << L" [" << get_token_id(sid,ment.get_pos_begin()+1) << " .. " << get_token_id(sid,ment.get_pos_end()+1) << "]"
           << L" " << mtypes[ment.get_type()]
           << endl;
      nm++;
    }      
    sout << L"   }" << endl;
  }  
  sout << L"}" << endl;
}



//---------------------------------------------
// print Semantic graph  of the document
//---------------------------------------------

void output_freeling::PrintSemgraph(wostream &sout, const document &doc) const {

  sout << L"SemanticGraph {" << endl;
  for (vector<semgraph::SG_entity>::const_iterator e=doc.get_semantic_graph().get_entities().begin();
       e!=doc.get_semantic_graph().get_entities().end();
       e++) {
    // skip non NE entities that are not argument to any predicate
    if (not doc.get_semantic_graph().is_argument(e->get_id())) continue;

    sout << L"   Entity " << e->get_id() << L" " 
         << e->get_lemma() << L" " 
         << e->get_semclass() << L" "
         << e->get_sense() << L" {" << endl;      
    
    for (vector<semgraph::SG_mention>::const_iterator m=e->get_mentions().begin(); m!=e->get_mentions().end(); m++) {
      sout << L"      mention " << get_token_id(m->get_sentence_id(),util::wstring2int(m->get_id()))
           << L" (" << util::list2wstring(m->get_words(),L" ") << L")"
           << endl;
    }      
    sout << L"   }" << endl;
  }  

  for (vector<semgraph::SG_frame>::const_iterator f=doc.get_semantic_graph().get_frames().begin();
       f!=doc.get_semantic_graph().get_frames().end();
       f++) {
    // skip argless nominal predicates
    if (not doc.get_semantic_graph().has_arguments(f->get_id())) continue;

    sout << L"   Frame " << f->get_id() << L" " 
         << get_token_id(f->get_sentence_id(),util::wstring2int(f->get_token_id())) << L" "
         << f->get_lemma() << L" " 
         << f->get_sense() <<L" {" << endl; 

    for (vector<semgraph::SG_argument>::const_iterator a=f->get_arguments().begin(); 
         a!=f->get_arguments().end();
         a++) {
      sout << L"      " << a->get_role() << L" " << a->get_entity() <<endl; 
    }
    sout << L"   }" << endl;
  }

  sout << L"}" << endl;
}


//---------------------------------------------
// print obtained analysis in classical FreeLing format
//---------------------------------------------

void output_freeling::PrintResults (wostream &sout, const list<sentence > &ls) const {

    for (list<sentence>::const_iterator is = ls.begin (); is != ls.end (); is++) {

      int best = is->get_best_seq();
      // sentence is dep parsed, print dep tree
      if (OutputDepTree and is->is_dep_parsed()) {
        PrintDepTree (sout, is->get_dep_tree(best).begin(), 0, best);
	// print SRL if any
        if (not is->get_predicates().empty()) PrintPredArgs(sout,*is);       
      }

      // no dep parsing, but constituents are found, print constituent tree
      else if (is->is_parsed()) {
        const parse_tree & tr = is->get_parse_tree(best);
        PrintTree (sout, tr.begin (), 0, best);
        sout << endl;
      }

      // sentence is not parsed, print morphological information
      else {
	for (sentence::const_iterator w = is->begin (); w != is->end (); w++) {
          /// Normal output: print selected analysis (with probs)
          PrintWord(sout, *w, true, true, best);  
	  sout << endl;   
	}
      }

      // sentence separator: blank line.
      sout << endl;
    }
}

//----------------------------------------------------------
// print the mentions of the document with coreferences 
//----------------------------------------------------------

void output_freeling::PrintResults(wostream &sout, const document &doc) const {

  // print tagging and parsing results for each sentence
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) 
    PrintResults(sout,*p);

  // print coreference results, if needed
  if (OutputCorefs and (doc.get_num_groups() > 0))  PrintCorefs (sout, doc);

  if (OutputSemgraph and not doc.get_semantic_graph().empty()) PrintSemgraph (sout, doc);
  
}

/// activate/deactivate printing levels
void output_freeling::output_senses(bool b) {OutputSenses=b;}
void output_freeling::output_all_senses(bool b) {AllSenses=b;}
void output_freeling::output_phonetics(bool b) {OutputPhonetics=b;}
void output_freeling::output_dep_tree(bool b) {OutputDepTree=b;}
void output_freeling::output_corefs(bool b) {OutputCorefs=b;}  
void output_freeling::output_semgraph(bool b) {OutputSemgraph=b;}  
