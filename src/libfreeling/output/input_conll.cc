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
#include "freeling/output/input_conll.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"INPUT_CONLL"
#define MOD_TRACECODE OUTPUT_TRACE


//---------------------------------------------
// Constructor
//---------------------------------------------

input_conll::input_conll() : input_handler(), conll_handler() { use_msd=false; }

//---------------------------------------------
// Constructor from config file
//---------------------------------------------

input_conll::input_conll(const wstring &cfgFile) : input_handler(), conll_handler(cfgFile) {

  enum sections {INPUT_TYPE,TAGSET};

  config_file cfg(true);
  cfg.add_section(L"Type",INPUT_TYPE);
  cfg.add_section(L"TagsetFile",TAGSET);
  
  if (not cfg.open(cfgFile))
    ERROR_CRASH(L"Error opening file "+cfgFile);
  
  wstring line; 
  while (cfg.get_content_line(line)) {
    
    // process each content line according to the section where it is found
    switch (cfg.get_section()) {
      
    case INPUT_TYPE: {
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

  if (FieldPos.find(L"SHORT_TAG")!=FieldPos.end()) {
    if (FieldPos.find(L"TAG")!=FieldPos.end()) {
      WARNING(L"SHORT_TAG column in CoNLL input will be ignored, since TAG column is also present.");
    }
    else {
      WARNING(L"SHORT_TAG column in CoNLL input will be used as TAG, since TAG column is not present.");
    }
  }

  use_msd=false;
  if (FieldPos.find(L"MSD")!=FieldPos.end()) {
    if (FieldPos.find(L"TAG")!=FieldPos.end()) {
      WARNING(L"MSD column in CoNLL input will be ignored, since TAG column is also present.");
    }
    else if (Tags!=NULL) {
      WARNING(L"MSD column in CoNLL input will be used to compute TAG, since TAG column is not present.");
      use_msd = true;
    }
    else {
      WARNING(L"MSD column in CoNLL input can not be used to compute TAG, since Tagset file was not specifed. No PoS tags will be loaded");
    }
  }
  
  if (FieldPos.find(L"DEPREL")!=FieldPos.end() and FieldPos.find(L"DEPHEAD")==FieldPos.end()) {
    ERROR_CRASH(L"DEPREL column in CoNLL input requires DEPHEAD column to be present.");
  }
  if (FieldPos.find(L"COREF")!=FieldPos.end() and FieldPos.find(L"SYNTAX")==FieldPos.end()) {
    ERROR_CRASH(L"COREF column in CoNLL input requires SYNTAX column to be present.");
  }
  if (FieldPos.find(L"SRL")!=FieldPos.end() and FieldPos.find(L"SRL")->second!=FieldPos.size()-1) {
    ERROR_CRASH(L"SRL column in CoNLL input must be in the rightmost position.");
  }


}

//---------------------------------------------
// Destructor
//---------------------------------------------

input_conll::~input_conll() {}


//---------------------------------------------
// load sentences in CoNLL format
//---------------------------------------------

void input_conll::input_sentences (const wstring &lines, list<sentence> &ls) const {

  ls.clear();
  conll_sentence cs;

  wistringstream ss(lines);
  wstring line;
  while (getline(ss,line)) {
    if (not line.empty()) {
      // token line, add fields to vector
      wistringstream sl(line);

      // read fields into 'token'
      wstring field;
      vector<wstring> token;
      while (sl>>field) 
        token.push_back(field);

      // add token to conll_sentence
      cs.add_token(token);
    }

    else {
      // end of sentence. Convert cs from conll_sentence to freeling::sentence
      sentence s;
      conll2freeling(cs,s);
      // add completed sentence to result
      ls.push_back(s);
      // clear and go for next sentence
      cs.clear();
    } 
  }
}

//---------------------------------------------
// load a document in CoNLL format
//---------------------------------------------

void input_conll::input_document(const wstring &lines, document &doc) const {

  // load sentences into document
  doc.clear();
  doc.push_back(list<sentence>());

  size_t nsent = 1;
  int nment = 0;
  conll_sentence cs;
  wistringstream ss(lines);
  wstring line;
  while (getline(ss,line)) {
    if (not line.empty()) {
      // token line, add fields to vector
      wistringstream sl(line);

      // read fields into 'token'
      wstring field;
      vector<wstring> token;
      while (sl>>field) 
        token.push_back(field);

      // add token to conll_sentence
      cs.add_token(token);
    }

    else {
      // end of sentence. Convert cs from conll_sentence to freeling::sentence
      sentence s;
      // add ident to sentence
      s.set_sentence_id(util::int2wstring(nsent));
      nsent++;
      // add sentence to document
      doc.back().push_back(s);

      // extract information from columns into sentence
      conll2freeling(cs,doc.back().back(),doc,nment); 

      // clear and go for next sentence
      cs.clear();
    } 
  }
}



//---------------------------------------------
// Fill freeling::sentence from conll_sentence 
//---------------------------------------------

void input_conll::conll2freeling(const conll_sentence &cs, sentence &s) const {

  if (FieldPos.find(L"COREF")!=FieldPos.end()) {
    ERROR_CRASH(L"COREF Column requires document mode. Use input_conll::input_document instead of input_conll::input_sentences");
  }

  // dummy document and nment. Will not be used since there is no COREF column.
  document doc;
  int nm=0;
  conll2freeling(cs,s,doc,nm);
}


//---------------------------------------------
// Fill freeling::sentence from conll_sentence 
//---------------------------------------------

void input_conll::conll2freeling(const conll_sentence &cs, sentence &s, document &doc, int &nment) const {
  
  vector<dep_tree*> dtrees(cs.size(),NULL);
  vector<wstring> drels(cs.size(),L"");
  list<pair<wstring,size_t> > opened; // for coreference mentions
  
  // add a virtual root node in case the tree is not complete
  parse_tree ptree(node(L"VIRTUAL_ROOT"));
  parse_tree::iterator ptr=ptree.begin();
  
  int sp=0;
  for (size_t i=0; i<cs.size(); i++) {

    TRACE(6,L"  conll2FL. processing word "<<i);

    int sp1,sp2; 
    sp1=sp2= -1;
    
    // add new word to sentence
    word nw;
    s.push_back(nw);
    
    // get reference to new word in sentece, to enrich it with input fields
    word &w = s.back();
    
    for (size_t f=0; f<FieldName.size(); ++f) {

      TRACE(6,L"  conll2FL. loading field "<<f<<L" = "<<FieldName[f]);
      
      if (FieldName[f].find(UserPrefix)==0) {
        // user fields (one column, but name is variable 'USERxx')
        size_t k = util::wstring2int(FieldName[f].substr(UserPrefix.size()));
        if (w.user.size()<k+1) w.user.resize(k+1);
        w.user[k] = cs.get_value(i,f);
      }
      
      // Span fields must be treated together
      else if (FieldName[f]==L"SPAN_BEGIN") {
        sp1 = util::wstring2int(cs.get_value(i,f));
      }
      else if (FieldName[f]==L"SPAN_END") {
        sp2 = util::wstring2int(cs.get_value(i,f));
      }
      
      else {
        // other fields
        add_field_content(FieldName[f], cs.get_value(i,f), i, w, ptr, dtrees, drels);
      }
    }

    // set spans if provided or infer from word length if not
    if (sp1==-1) sp1 = sp;
    if (sp2==-1) sp2 = sp1 + w.get_form().length();
    w.set_span(sp1,sp2);
    sp = sp2+1;
  }

  // All words created. Now complete other sentence structures (trees, mentions, SRL, coref)
 
  // complete parse tree and add it to sentence
  if (FieldPos.find(L"SYNTAX")!=FieldPos.end()) {
    TRACE(6,L"  conll2FL. completing SYNTAX");
    // remove head numbers from node labels.
    for (parse_tree::preorder_iterator n=ptree.begin(); n!=ptree.end(); n++) {
      wstring lab=n->get_label();
      n->set_label(lab.substr(0,lab.find(L":")));
    }
    
    // set parse tree for the sentence
    // if the virtual root has only one child, that will be the parse tree. Remove virtual root
    if (ptree.num_children()==1) 
      s.set_parse_tree((freeling::parse_tree&)ptree.nth_child_ref(0));
    else 
      s.set_parse_tree(ptree);

  }


  // complete dep tree and add it to sentnece
  if (FieldPos.find(L"DEPHEAD")!=FieldPos.end()) {
    TRACE(6,L"  conll2FL. completing DEPHEAD");

    size_t dhpos = FieldPos.find(L"DEPHEAD")->second;

    // add labels to dep tree nodes, if present
    if (FieldPos.find(L"DEPREL")!=FieldPos.end()) {
      TRACE(6,L"  conll2FL. completing DEPREL");
      for (size_t i=0; i<dtrees.size(); ++i)
        dtrees[i]->begin()->set_label(drels[i]);
    }

    // Complete the dep tree: hang each dep node under its head
    list<freeling::dep_tree*> roots;
    for (size_t i=0; i<s.size(); i++) {
      int dephead = util::wstring2int(cs.get_value(i,dhpos));
      if (dephead==0) 
        roots.push_back(dtrees[i]);
      else 
        dtrees[dephead-1]->hang_child(*dtrees[i]);
    }      
    
    // handle multiple roots if needed
    dep_tree *dtree;
    if (roots.size()==1)
      // just one root, that one is the tree root
      dtree = *(roots.begin());
    else {
      // more than one root, create virtual root
      dtree = new dep_tree(depnode(L"VIRTUAL_ROOT"));
      // hang all subtrees under fake root
      for (list<dep_tree*>::iterator r=roots.begin(); r!=roots.end(); r++)
        dtree->hang_child(**r);
    }
    
    // store tree in the sentence
    s.set_dep_tree(*dtree);
  }

  if (FieldPos.find(L"SRL")!=FieldPos.end()) {
    TRACE(6,L"  conll2FL. completing SRL");

    int fpos = FieldPos.find(L"SRL")->second;
    // predicate-arguments columns present
    size_t npred=0;
    for (size_t i=0; i<s.size(); i++) {
      wstring predsense = cs.get_value(i,fpos); 
      if (predsense != L"-") {
        // create new predicate
        predicate pred(i, predsense);
        
        // search arguments and add them to predicate
        for (size_t j=0; j<s.size(); j++) {          
          wstring role = cs.get_value(j,fpos+1+npred);
          if (role != L"-") 
            pred.add_argument(j,role);
        }
        
        // add predicate to the sentence
        s.add_predicate(pred);
        npred++;
      }
    }
  }

  if (FieldPos.find(L"COREF")!=FieldPos.end()) {
    TRACE(6,L"  conll2FL. completing COREF");
    list<sentence>::const_iterator si=doc.back().end();
    --si;
    load_corefs(cs,si,nment,doc);
  }
}




void input_conll::add_field_content(const wstring &field, const wstring &val, int i, word &w, parse_tree::iterator &ptr, vector<dep_tree*> &dtrees, vector<wstring> &drels) const {

  TRACE(6,L"     adding field "<<field<<L" content: "<<val<<L" to word "<<i<<L" "<<w.get_form());
  
  // no information in the field, do nothing
  if (val==L"-") return;

  switch (field_code(field)) {

  case ID: {
    w.set_position(util::wstring2int(val)-1);
    break;
  }
    
  case FORM: {
    w.set_form(val);
    break;
  }
    
  case LEMMA: {
    if (w.empty()) {
      analysis a;
      w.add_analysis(a);
      w.select_all_analysis();
    }
    w.begin()->set_lemma(val);
    break;
  }

  case TAG: {
    if (w.empty()) {
      analysis a;
      w.add_analysis(a);
      w.select_all_analysis();
    }
    w.begin()->set_tag(val);
    break;
  }

  case MSD: {
    if (use_msd) {
      if (w.empty()) {
        analysis a;
        w.add_analysis(a);
        w.select_all_analysis();
      }    
      w.begin()->set_tag(Tags->msd_to_tag(L"-",val));
    }
  }


  case NEC: {
    if (w.empty()) {
      analysis a;
      w.add_analysis(a);
      w.select_all_analysis();
    }    
    if (val==L"B-PER") w.begin()->set_tag(L"NP00SP0");
    else if (val==L"B-LOC") w.begin()->set_tag(L"NP00G00");
    else if (val==L"B-ORG") w.begin()->set_tag(L"NP00O00");
    else if (val==L"B-MISC") w.begin()->set_tag(L"NP00V00");
    break;
  }

  case SENSE: {
    // WSD parsing column present
    list<pair<wstring, double> > sns;
    sns.push_back(make_pair(val,0.0));
    w.begin()->set_senses(sns);
    break;
  }

  case ALL_SENSES: {
    // WSD parsing column present
    w.begin()->set_senses(util::wstring2pairlist<wstring,double>(val,L":",L"/"));
    break;
  }
    
  case SYNTAX: {
    // parse column to find out which constituents open/close here
    wstring stx = val;
    
    // find where do closing parenthesis start, and count them
    size_t c = stx.find(L")");
    size_t nclose = 0;
    if (c!=wstring::npos) nclose=stx.substr(c).size();
    // keep only openings, and split them
    stx = stx.substr(0,c);
    list<wstring> ochunk;
    if ( stx[0]==L'(' ) ochunk = util::wstring2list(stx.substr(1),L"(");
    // open and close a terminal node for the current word
    ochunk.push_back(w.get_tag()+L":"+ util::int2wstring(i+1));
    nclose++;
    
    // for each opened chunk, create a new node and place under current last opened node
    for (list<wstring>::iterator oc=ochunk.begin(); oc!=ochunk.end(); oc++) {
      // create new node
      node pn(*oc);
      parse_tree *nc = new freeling::parse_tree(pn);
      
      // set as head if appropriate
      wstring lab = ptr->get_label();
      size_t pl = lab.find(L":");
      if (pl!=wstring::npos and oc->substr(oc->find(L":"))==lab.substr(pl)) 
        nc->begin()->set_head(true);
      
      // place it under current last opened node
      ptr.hang_child(*nc);           
      // current last opened node is now the new node
      ptr = ptr.nth_child(ptr.num_children()-1);         
    }
    // last opened node is the terminal for current word
    ptr->set_word(w);
    
    // close nodes, going up in the tree as many levels as chunks are closed
    for (size_t k=0; k<nclose; k++) ptr = ptr.get_parent();

    break;
  }
    
  case DEPHEAD: {
    // dep parsing columns present.
    // create a dependency tree with just one node for this word, and store it in dtrees[i];
    depnode dn; 
    dn.set_word(w);
    dtrees[i] = new freeling::dep_tree(dn);
    break;
  }

  case DEPREL: {
    drels[i] = val;
    break;
  }

  default: break;

  }

}


//---------------------------------------------
// Load coreference column and convert it to mentions in the document
//---------------------------------------------

void input_conll::load_corefs(const conll_sentence &cs, list<sentence>::const_iterator s, int &nment, document &doc) const {
  list<pair<wstring,size_t> > opened;
   
  int nsent=util::wstring2int(s->get_sentence_id())-1;
  
  for (size_t i=0; i<cs.size(); i++) {
    // mention/coref columns present
         
    // split list of mentions opening and/or closing here
    list<wstring> ms = util::wstring2list(cs.get_value(i,FieldPos.find(L"COREF")->second),L"|");
    for (list<wstring>::iterator oc=ms.begin(); oc!=ms.end(); oc++) {
     
      // if mention is opening AND closing, create new single-word mention.
      if ((*oc)[0]==L'(' and (*oc)[oc->size()-1]==L')') { 
        // create a mention in span (i,i)
        sentence::const_iterator wi = s->get_word_iterator((*s)[i]);
        mention m(nment, nsent, s, wi, wi);
        m.set_group(util::wstring2int(oc->substr(1,oc->size()-2)));
        doc.add_mention(m);
        nment++;
      }
     
      // if mention is only opening, push into stack of opened mentions
      else if ((*oc)[0]==L'(') 
        opened.push_front(make_pair(oc->substr(1),i));
     
      // if mention is closing, find out where it was last opened,
      // create mention, and pop opening from stack
      else if ((*oc)[oc->size()-1]==L')') {
        wstring m = oc->substr(0,oc->size()-1); 
        list<pair<wstring,size_t> >::iterator p;
        for (p=opened.begin(); p!=opened.end() and p->first!=m; p++);
        if (p!=opened.end()) { // should not happed, just for safety.
          size_t j = p->second;
          // create_mention (j,i);
          sentence::const_iterator wj = s->get_word_iterator((*s)[j]);
          sentence::const_iterator wi = s->get_word_iterator((*s)[i]);
          mention m(nment, nsent, s, wj, wi);
          m.set_group(util::wstring2int(oc->substr(0,oc->size()-1)));
          doc.add_mention(m);
          nment++;
          // pop mention closing from stack
          opened.erase(p);
        }
      }
    }
  }
}

