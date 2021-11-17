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
#include "freeling/output/output_conll.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"OUTPUT_CONLL"
#define MOD_TRACECODE OUTPUT_TRACE

//---------------------------------------------
// empty constructor
//---------------------------------------------

output_conll::output_conll() : output_handler(), conll_handler() {}

//---------------------------------------------
// Constructor from config file
//---------------------------------------------

output_conll::output_conll(const wstring &cfgFile) : output_handler(), conll_handler(cfgFile) {

  enum sections {OUTPUT_TYPE,TAGSET};

  config_file cfg(true);
  cfg.add_section(L"Type",OUTPUT_TYPE);
  cfg.add_section(L"TagsetFile",TAGSET);
  
  if (not cfg.open(cfgFile))
    ERROR_CRASH(L"Error opening file "+cfgFile);
  
  wstring line; 
  while (cfg.get_content_line(line)) {
    
    // process each content line according to the section where it is found
    switch (cfg.get_section()) {
      
    case OUTPUT_TYPE: {
      if (util::lowercase(line)!=L"conll")
        ERROR_CRASH(L"Invalid configuration file for 'conll' output handler, "+cfgFile);
      break;
    }

    case TAGSET: { 
      wstring path = cfgFile.substr(0,cfgFile.find_last_of(L"/\\")+1);
      load_tagset(util::absolute(line,path));
      break;
    }
      
    default: break;
    }
  }
  
  cfg.close(); 

  if (FieldPos.find(L"SRL")!=FieldPos.end() and FieldPos.find(L"SRL")->second!=FieldPos.size()-1) {
    ERROR_CRASH(L"SRL column in CoNLL output must be in the rightmost position.");
  }
}



//---------------------------------------------
// Destructor
//---------------------------------------------

output_conll::~output_conll() {}

//----------------------------------------------------------
// print list of sentences in conll format
//---------------------------------------------------------

void output_conll::PrintResults (wostream &sout, const list<sentence > &ls) const {

  for (list<sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) {
    if (s->empty()) continue;
    
    conll_sentence cs;
    freeling2conll(*s,cs);
    cs.print_conll_sentence(sout);
  }

}

//----------------------------------------------------------
// print document in conll format
//----------------------------------------------------------

void output_conll::PrintResults(wostream &sout, const document &doc) const {

  // Precompute coreference column if requested and available
  map<wstring,wstring> openmention, closemention;
  if (doc.get_num_groups()>0 and FieldPos.find(L"COREF")!=FieldPos.end())
    openclosementions(doc, openmention, closemention);
  
  // convert and print each sentence in the document
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    if (p->empty()) continue;

    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) {      
      if (s->empty()) continue;

      conll_sentence cs;
      freeling2conll(*s,cs,openmention,closemention);

      cs.print_conll_sentence(sout);
    }
  }
}

//---------------------------------------------
// Fill conll_sentence from freeling::sentence
//---------------------------------------------

void output_conll::freeling2conll(const sentence &s, 
                                  conll_sentence &cs, 
                                  const map<wstring,wstring> &openmention,
                                  const map<wstring,wstring> &closemention) const {

  cs.clear();

  // if constituency parsing is available (and needed), precompute column
  vector<wstring> openchunk(s.size(),L"");
  vector<wstring> closechunk(s.size(),L"");
  if (s.is_parsed() and FieldPos.find(L"SYNTAX")!=FieldPos.end() )
    openclosechunks(s.get_parse_tree(s.get_best_seq()).begin(), openchunk, closechunk);

  for (sentence::const_iterator w=s.begin(); w!=s.end(); w++) {

    // fill a vector with available fields for current word
    vector<wstring> token;

    for (size_t i=0; i<FieldName.size(); ++i) {

      if (FieldName[i]==L"SRL") {
        // SRL is a multicolumn field, special treatment
        add_srl(token, s, w->get_position());
      }

      else if (FieldName[i].find(UserPrefix)==0) {
        // user fields (one column, but name is variable 'USERxx')
        size_t k = util::wstring2int(FieldName[i].substr(UserPrefix.size()));
        wstring usr = L"-";
        if (k < w->user.size()) 
          usr = w->user[k];
        else 
          WARNING(L"User field "<<UserPrefix<<k<<L" is not initialized.");

        token.push_back(usr);
      }

      else { 
        // normal one-column field
        wstring val = compute_value(s, (*w), FieldName[i], openchunk, closechunk, openmention, closemention);
        token.push_back(val);
      }
    }

    // add line to sentence
    cs.add_token(token);
  }
}

//---------------------------------------------
// Auxiliary for freeling2conll, to compute value to add to 'field' column
//---------------------------------------------

wstring output_conll::compute_value(const sentence &s,
                                    const word &w, 
                                    const wstring &field, 
                                    const vector<wstring> &openchunk, 
                                    const vector<wstring> &closechunk,
                                    const map<wstring,wstring> &openmention,
                                    const map<wstring,wstring> &closemention) const {

  int id = w.get_position();
  int best = s.get_best_seq();
  
  switch (field_code(field)) {
    
  case ID: {
    // token ID
    return util::int2wstring(id+1);
    break;
  } 
     
  case SPAN_BEGIN: {
    // Span begin
    return util::int2wstring(w.get_span_start());
    break;
  }

  case SPAN_END: {
    // Span end
    return util::int2wstring(w.get_span_finish());
    break;
  }

  case FORM: {
    // Form
    return w.get_form();
    break;
  }

  case LEMMA: {
    // lemma 
    wstring lemma;
    if (w.empty()) 
      lemma = L"-";
    else if (w.selected_begin(best)->is_retokenizable()) {
      const list <word> &rtk = w.selected_begin(best)->get_retokenizable();
      list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
      lemma = la.begin()->get_lemma();
    }
    else // w not empty, not retokenizable
      lemma = w.get_lemma(best);
    
    return lemma;
    break;
  }

  case TAG: {
    // Full PoS Tag
    wstring tag;
    if (w.empty()) 
      tag = L"-";
    else if (w.selected_begin(best)->is_retokenizable()) {
      const list <word> &rtk = w.selected_begin(best)->get_retokenizable();
      list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
      tag = la.begin()->get_tag();
    }
    else // w not empty, not retokenizable
      tag = w.get_tag(best);
    
    return tag;
    break;
  }

  case SHORT_TAG: {
    // Short PoS tag
    wstring shtag = L"-";
    if (not w.empty() and Tags!=NULL) {
      list<wstring> tgs=util::wstring2list(w.get_tag(best),L"+");
      for (list<wstring>::const_iterator t=tgs.begin(); t!=tgs.end(); t++) 
        shtag += L"+" + Tags->get_short_tag(*t);
      shtag = shtag.substr(1);
    }    
    return shtag;
    break;
  }
    
  case MSD: {
    // Morphosyntactic description
    wstring msd = L"-";
    if (not w.empty() and Tags!=NULL) {
      wstring tag;
      if (w.selected_begin(best)->is_retokenizable()) {
        const list <word> &rtk = w.selected_begin(best)->get_retokenizable();
        list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
        tag = la.begin()->get_tag();
      }
      else 
        tag = w.get_tag(best);

      list<wstring> tgs=util::wstring2list(tag,L"+");
      for (list<wstring>::const_iterator t=tgs.begin(); t!=tgs.end(); t++) 
        msd += L"+" + Tags->get_msd_string(*t);
      msd = msd.substr(1);
    }

    return msd;
    break;
  }

  case NEC: {
    // NEC
    wstring nec=L"-";
    if (not w.empty()) {
      if (w.get_tag(best)==L"NP00SP0") nec=L"B-PER";
      else if (w.get_tag(best)==L"NP00G00") nec=L"B-LOC";
      else if (w.get_tag(best)==L"NP00O00") nec=L"B-ORG";
      else if (w.get_tag(best)==L"NP00V00") nec=L"B-MISC";    
    }
    return nec;
    break;
  }

  case SENSE: {
    // WSD
    wstring wsd = L"-";
    if (not w.empty() and not w.get_senses(best).empty())
      wsd = w.get_senses(best).begin()->first;
    return wsd;
    break;
  } 

  case ALL_SENSES: {
    // All senses, with ranking if available
    wstring wsd = L"-";
    if (not w.empty() and not w.get_senses(best).empty())
      wsd = util::pairlist2wstring(w.get_senses(best),L":",L"/");
    return wsd;
    break;
  } 

  case SYNTAX: {
    // parse tree
    wstring cst=L"-";
    if (s.is_parsed()) {
      cst = openchunk[id] + closechunk[id];
      if (cst.empty()) cst=L"-";
    }
    return cst;
    break;
  }

  case DEPHEAD: {
    // dependency head
    wstring dhead = L"-";
    if (s.is_dep_parsed()) {
      dep_tree::const_iterator n = s.get_dep_tree(s.get_best_seq()).get_node_by_pos(id);
      if (n.is_root() or n.get_parent()->get_label()==L"VIRTUAL_ROOT") dhead = L"0";
      else dhead = util::int2wstring(n.get_parent()->get_word().get_position()+1); 
    }
    return dhead;
    break;
  }
    
  case DEPREL: {
    // dependency function   
    wstring drel = L"-";
    if (s.is_dep_parsed()) {
      dep_tree::const_iterator n = s.get_dep_tree(s.get_best_seq()).get_node_by_pos(id);
      drel = n->get_label();
    }
    return drel;
    break;
  }

  case COREF: {
    // coreference
    wstring coref=L"-";
    if (not openmention.empty()) {
      wstring sid = s.get_sentence_id();
      int wpos = w.get_position();
      wstring wid = sid + L"." + util::int2wstring(wpos+1);
      wstring ocoref = L""; 
      wstring ccoref = L"";
      
      map<wstring, wstring>::const_iterator p;
      p = openmention.find(wid); if (p!=openmention.end()) ocoref = p->second;
      p = closemention.find(wid); if (p!=closemention.end()) ccoref = p->second;
      
      if (not ocoref.empty() and not ccoref.empty()) coref = ocoref+L"|"+ccoref;
      else if (not ocoref.empty() or not ccoref.empty()) coref = ocoref+ccoref;
    }
    return coref;
    break;
  }

  default: {
    return L"-";
    break;
  }
  }

}


//---------------------------------------------
// Auxiliary for freeling2conll, to add SRL columns
//---------------------------------------------

void output_conll::add_srl(vector<wstring> &token, const sentence &s, int id) {
  // predicate column
  wstring prd=L"-";
  if (not s.get_predicates().empty() and s.is_predicate(id)) 
    prd = s.get_predicate_by_pos(id).get_sense();
  token.push_back(prd);
  // arguments columns
  for (size_t np=0; np < s.get_predicates().size(); np++) {
    wstring a = L"-";
    const predicate &pred = s.get_predicate_by_number(np);
    if (pred.has_argument(id)) a = pred.get_argument_by_pos(id).get_role();
    token.push_back(a);
  }
}


//---------------------------------------------
// Private auxiliary to handle parenthesis open/close in constituency columns
//---------------------------------------------

void output_conll::openclosechunks(parse_tree::const_iterator n, vector<wstring> &open, vector<wstring> &close) {

  if (n.num_children() == 0) return;

  int w1 = parse_tree::get_leftmost_leaf(n)->get_word().get_position();
  open[w1] += L"("+n->get_label() + L":" + util::int2wstring(parse_tree::get_head_position(n)+1);

  int w2 = parse_tree::get_rightmost_leaf(n)->get_word().get_position();
  close[w2] += L")";
  
  parse_tree::const_sibling_iterator d;
  for (d = n.sibling_begin (); d != n.sibling_end (); ++d) 
    openclosechunks(d, open, close);

}

//---------------------------------------------
// Private auxiliary to handle parenthesis open/close in coreference columns
//---------------------------------------------

void output_conll::openclosementions(const freeling::document &doc, 
                                     map<wstring,wstring> &open, map<wstring,wstring> &close) {

  for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {
    list<int> mentions = doc.get_coref_id_mentions(*g);
    wstring gid = util::int2wstring(*g);
    for (list<int>::iterator m=mentions.begin(); m!=mentions.end(); m++) {
      const mention & ment = doc.get_mention(*m);
      wstring sid = util::int2wstring(ment.get_n_sentence()+1);
      
      map<wstring,wstring>::iterator p;
      wstring w1 = sid + L"." + util::int2wstring(ment.get_pos_begin()+1);
      wstring w2 = sid + L"." + util::int2wstring(ment.get_pos_end()+1);

      if (w1==w2)
        add_mention_oc(open,w1,L"("+gid+L")");
      else {
        add_mention_oc(open,w1,L"("+gid);
        add_mention_oc(close,w2,gid+L")");
      }
    } 
  }  
}

void output_conll::add_mention_oc(map<wstring,wstring> &table, const wstring &key, const wstring &value) {
  map<wstring,wstring>::iterator p = table.find(key);
  if (p==table.end()) table.insert(make_pair(key,value));
  else p->second += L"|"+value;
}
