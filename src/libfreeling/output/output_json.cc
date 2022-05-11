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
#include "freeling/output/output_json.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;


#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"OUTPUT_JSON"
#define MOD_TRACECODE OUTPUT_TRACE

using namespace jsn;

//---------------------------------------------
// Constructor
//---------------------------------------------

output_json::output_json() {
  // default values
  AllAnalysis = false;
  AllSenses = false;
}

//---------------------------------------------
// Constructor from cfg file
//---------------------------------------------

output_json::output_json(const wstring &cfgFile) {

  enum sections {OUTPUT_TYPE,TAGSET,OPTIONS};
  config_file cfg(true);
  cfg.add_section(L"Type",OUTPUT_TYPE);
  cfg.add_section(L"TagsetFile",TAGSET);
  cfg.add_section(L"Options",OPTIONS);

  if (not cfg.open(cfgFile))
    ERROR_CRASH(L"Error opening file "+cfgFile);

  // default values
  AllAnalysis = false;
  AllSenses = false;
  
  wstring line; 
  while (cfg.get_content_line(line)) {
    
    // process each content line according to the section where it is found
    switch (cfg.get_section()) {
      
    case OUTPUT_TYPE: {
      if (util::lowercase(line)!=L"json")
        ERROR_CRASH(L"Invalid configuration file for 'json' output handler, "+cfgFile);
      break;
    }

    case TAGSET: { 
      wstring path = cfgFile.substr(0,cfgFile.find_last_of(L"/\\")+1);
      Tags = new tagset(util::absolute(line,path));
      break;
    }
      
    case OPTIONS: {  // reading Function words
      wistringstream sin; sin.str(line);
      wstring key,val;
      sin >> key >> val;
      val = util::lowercase(val);
      bool b = (val==L"true" or val==L"yes" or val==L"y" or val==L"on");
      if (key == L"AllAnalysis") AllAnalysis=b;
      else if (key == L"AllSenses") AllSenses=b;
      else WARNING(L"Invalid option '"+key+L"' in output_xml config file '"+cfgFile+L"'");
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

output_json::~output_json() {}


//---------------------------------------------
// auxiliary to print one word analysis in json
//---------------------------------------------

jsn::ordered_json output_json::json_analysis(const analysis &a, bool print_sel_status, bool print_probs) const {

  jsn::ordered_json an;
  wstring lemma = a.get_lemma();
  wstring tag = a.get_tag();

  an["lemma"] = util::wstring2string(lemma);
  an["tag"] = util::wstring2string(tag); 
  
  if (Tags!=NULL) {
    if (lemma.find(L"+")!=wstring::npos) { // is a retokenizable analysis
      list<wstring> tgs=util::wstring2list(tag,L"+");
      wstring shtag,msd;
      for (list<wstring>::const_iterator t=tgs.begin(); t!=tgs.end(); t++) {
	shtag += L"+" + Tags->get_short_tag(*t);
	msd += L"+" + Tags->get_msd_string(*t);
      }
      an["ctag"] = jsn::json(util::wstring2string(shtag.substr(1)));
      an["msd"] = jsn::json(util::wstring2string(msd.substr(1)));
    }
    
    else { // not retokenizable
      an["ctag"] = jsn::json(util::wstring2string(Tags->get_short_tag(tag)));
      list<pair<wstring,wstring> > feats = Tags->get_msd_features(tag);
      for (list<pair<wstring,wstring> >::iterator f=feats.begin(); f!=feats.end(); f++) 
	an[freeling::util::wstring2string(f->first)] = jsn::json(util::wstring2string(f->second));
    }
  }

  if (print_probs and a.get_prob()>=0) an["prob"] = jsn::json(a.get_prob());
  if (print_sel_status and a.is_selected()) an["selected"] = jsn::json(true);

  return an;
}

  
//---------------------------------------------
// print sentences in list
//---------------------------------------------

jsn::ordered_json output_json::json_Sentences (const list<sentence> &ls) const {

  
  jsn::ordered_json sentlist = json::array();
  if (ls.empty()) return sentlist;
    
  for (list<sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) {
    
    if (s->empty()) continue;
    
    int best = s->get_best_seq();
    
    jsn::ordered_json sent;
    sent["id"] = jsn::json(util::wstring2string(s->get_sentence_id()));
    jsn::ordered_json tokens = json::array();
    
    for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {
      // basic token stuff
      jsn::ordered_json token;
      token["id"] = jsn::json(util::wstring2string(get_token_id(s->get_sentence_id(),w->get_position()+1)));
      token["begin"] = jsn::json(util::wstring2string(util::wstring_from(w->get_span_start())));
      token["end"] = jsn::json(util::wstring2string(util::wstring_from(w->get_span_finish())));
      token["form"] = jsn::json(util::wstring2string(w->get_form()));
      if (not w->get_ph_form().empty())
	token["phon"] = jsn::json(util::wstring2string(w->get_ph_form()));
      
      // no analysis, nothing else to add
      if (w->empty()) continue;

      if (not s->is_tagged()) {
        // morpho output (all analysis)
	jsn::ordered_json anlist = json::array();	
        for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
          if (a->is_retokenizable()) {
            const list <word> &rtk = a->get_retokenizable();
            list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
            for (list<analysis>::iterator aa=la.begin(); aa!=la.end(); aa++) {
              double p=a->get_prob();
              if (p>=0) aa->set_prob(p/la.size());
              else aa->set_prob(-1.0);
              anlist.push_back(json_analysis(*aa,false,true));
            }
          }
          else {
	    anlist.push_back(json_analysis(*a,false,true));
          }
        }
	token["analysis"] = anlist;
      }
      
      else {
	//  tagger output
	jsn::ordered_json an;
	if (w->selected_begin(best)->is_retokenizable()) {
	  const list <word> &rtk = w->selected_begin(best)->get_retokenizable();
	  list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
	  an = json_analysis(*(la.begin()),false,false);
        }
        else {
          an = json_analysis(*(w->selected_begin(best)),false,false);
	}

	for (auto &x : an.items()) {
	  token[x.key()] = x.value();
	}
	
	// NEC output, if any
	wstring nec=L"";
	if (w->get_tag(best)==L"NP00SP0") nec=L"PER";
	else if (w->get_tag(best)==L"NP00G00") nec=L"LOC";
	else if (w->get_tag(best)==L"NP00O00") nec=L"ORG";
	else if (w->get_tag(best)==L"NP00V00") nec=L"MISC";
	if (not nec.empty())
	  token["nec"] = jsn::json(util::wstring2string(nec));
	
	// WSD output, if any
	if (not w->get_senses(best).empty())
	  token["wn"] = jsn::json(util::wstring2string(w->get_senses(best).begin()->first));
	
	if (AllAnalysis) {
	  jsn::ordered_json anlist = json::array();

	  for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
	    jsn::ordered_json mrf;
	    mrf["lemma"] = jsn::json(util::wstring2string(a->get_lemma()));
	    mrf["tag"] = jsn::json(util::wstring2string(a->get_tag()));
	    if (Tags!=NULL) {
	      mrf["ctag"] = jsn::json(util::wstring2string(Tags->get_short_tag(a->get_tag())));
	      list<pair<wstring,wstring> > feats = Tags->get_msd_features(a->get_tag());
	      for (list<pair<wstring,wstring> >::iterator f=feats.begin(); f!=feats.end(); f++) 
		mrf[freeling::util::wstring2string(f->first)] = jsn::json(util::wstring2string(f->second));
	    }
	    if (a->is_selected()) mrf["selected"] = jsn::json(true);

	    anlist.push_back(mrf);
	  }
	  token["analysis"] = anlist;
	}
	
	if (AllSenses and not w->get_senses(best).empty()) {
	  jsn::ordered_json senses = json::array();
	  for (list<pair<wstring,double> >::const_iterator s=w->get_senses(best).begin(); s!=w->get_senses(best).end(); s++) {
	    jsn::ordered_json sns;
	    sns["wn"] = util::wstring2string(s->first);
	    sns["pgrank"] = s->second;
	    senses.push_back(sns);
	  }
	  token["senses"] = senses;
	}
      }

      tokens.push_back(token);
    }

    sent["tokens"] = tokens;
    
    // print parse tree, if any
    if (s->is_parsed()) {
      sent["constituents"] = json_Tree (s->get_sentence_id(), s->get_parse_tree(s->get_best_seq()).begin()); 
    }

    //  print dependency tree, if any
    if (s->is_dep_parsed()) {
      sent["dependencies"].push_back(json_DepTree (s->get_sentence_id(), s->get_dep_tree(s->get_best_seq()).begin()));

      // predicates, if any
      if (not s->get_predicates().empty()) {        
        sent ["predicates"] = json_PredArgs(*s);
      }
    }

    sentlist.push_back(sent);
  }

  return sentlist;
}


//---------------------------------------------
// print parse tree
//--------------------------------------------

jsn::ordered_json output_json::json_Tree (const std::wstring &sid, parse_tree::const_iterator n) const {

  jsn::ordered_json tree;
  if (n.num_children () == 0) {
    tree["leaf"] = jsn::json(true);
    tree["head"] = n->is_head();
    tree["token"] = jsn::json(util::wstring2string(get_token_id(sid,n->get_word().get_position()+1)));
    tree["word"] = jsn::json(util::wstring2string(n->get_word().get_form()));
  }
  else {
    tree["leaf"] = jsn::json(false);
    tree["label"] = jsn::json(util::wstring2string(n->get_label()));
    tree["head"] = n->is_head();
    
    jsn::ordered_json children = json::array();
    parse_tree::const_sibling_iterator d = n.sibling_begin (); 
    while (d != n.sibling_end ()) {
      children.push_back(json_Tree(sid, d));
      d++;
    }

    tree["children"] = children;
  }

  return tree;
}


//---------------------------------------------
// print dependency tree
//---------------------------------------------

jsn::ordered_json output_json::json_DepTree (const std::wstring &sid, dep_tree::const_iterator n) const {

  jsn::ordered_json tree;
  
  if (n.num_children () == 0) {
    tree["token"] = jsn::json(util::wstring2string(get_token_id(sid,n->get_word().get_position()+1)));
    tree["function"] = jsn::json(util::wstring2string(n->get_label()));
    tree["word"] = jsn::json(util::wstring2string(n->get_word().get_form()));
  }
  else {
    if (n->get_label()==L"VIRTUAL_ROOT") 
      tree["token"] = jsn::json("VIRTUAL_ROOT");
    else {
      tree["token"] = jsn::json(util::wstring2string(get_token_id(sid,n->get_word().get_position()+1)));
      tree["function"] = jsn::json(util::wstring2string(n->get_label()));
      tree["word"] = jsn::json(util::wstring2string(n->get_word().get_form()));
    }

    jsn::ordered_json chld = json::array();

    // Sort children. Chunks first, then by their word position.
    // (this is just for aesthetic reasons)
    list<dep_tree::const_sibling_iterator> children;
    for (dep_tree::const_sibling_iterator d = n.sibling_begin (); d != n.sibling_end (); ++d)
      children.push_back(d);
    children.sort(ascending_position);

    // print children in the right order
    list<dep_tree::const_sibling_iterator>::const_iterator ch=children.begin();
    while (ch != children.end()) {      
      chld.push_back(json_DepTree(sid, (*ch)));
      ch++;
    }

    tree["children"] = chld;
  }

  return tree;
}

//---------------------------------------------
// print predicate-argument structure of the sentence
//---------------------------------------------

jsn::ordered_json output_json::json_PredArgs(const sentence &s) const {

  jsn::ordered_json preds = json::array();
  
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

  // print all predicates, referring to entities as their arguments
  int npred=1;
  for (sentence::predicates::const_iterator pred=s.get_predicates().begin(); 
       pred!=s.get_predicates().end(); 
       pred++) {

    jsn::ordered_json prd;
    prd["id"] = jsn::json(util::wstring2string(get_token_id(sid,npred,L"P")));
    prd["head_token"] = jsn::json(util::wstring2string(get_token_id(sid,pred->get_position()+1)));
    prd["sense"] = jsn::json(util::wstring2string(pred->get_sense()));
    prd["words"] = jsn::json(util::wstring2string(s[pred->get_position()].get_form()));
    
    if (not pred->empty()) {
      jsn::ordered_json args = json::array();
      for (predicate::const_iterator a=pred->begin(); a!=pred->end(); a++) {
	jsn::ordered_json arg;
	arg["role"] =  jsn::json(util::wstring2string(a->get_role()));
	arg["words"] =  jsn::json(util::wstring2string(arg_words[a->get_position()]));
	arg["head_token"] =  jsn::json(util::wstring2string(get_token_id(sid,a->get_position()+1)));
	arg["from"] =  jsn::json(util::wstring2string(get_token_id(sid,arg_span[a->get_position()].first+1)));
	arg["to"] =  jsn::json(util::wstring2string(get_token_id(sid,arg_span[a->get_position()].second+1)));	
	args.push_back(arg);
      }

      prd["arguments"] = args;
    }

    preds.push_back(prd);
    npred++;
  }

  return preds;
}

//---------------------------------------------
// print coreference information of the document
//---------------------------------------------

jsn::ordered_json output_json::json_Corefs(const document &doc) const {

  jsn::ordered_json corefs = json::array();
  
  for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {
    
    jsn::ordered_json group;
    group["id"] = jsn::json("co"+std::to_string(*g+1));
    
    jsn::ordered_json ments = json::array();
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

      jsn::ordered_json mt;
      mt["id"] = jsn::json("m"+std::to_string(*g+1)+"."+std::to_string(nm));
      mt["from"] = jsn::json(util::wstring2string(get_token_id(sid,ment.get_pos_begin()+1)));
      mt["to"] = jsn::json(util::wstring2string(get_token_id(sid,ment.get_pos_end()+1)));
      mt["words"] = jsn::json(util::wstring2string(words));      

      ments.push_back(mt);
      nm++;
    }

    group["mentions"] = ments;

    corefs.push_back(group);     
  }

  return corefs;
}



//----------------------------------------------------------
// print semantic graph of the document 
//----------------------------------------------------------

jsn::ordered_json output_json::json_Semgraph(const document &doc) const {

  jsn::ordered_json semgr;
  
  jsn::ordered_json entities = json::array();
  
  int ne=0;
  for (vector<semgraph::SG_entity>::const_iterator e=doc.get_semantic_graph().get_entities().begin();
       e!=doc.get_semantic_graph().get_entities().end();
       e++) {
    // skip non NE entities that are not argument to any predicate
    if (not doc.get_semantic_graph().is_argument(e->get_id())) continue;
    
    jsn::ordered_json entity;
    entity["id"] = jsn::json(util::wstring2string(e->get_id()));
    entity["lemma"] = jsn::json(util::wstring2string(e->get_lemma()));
    if (not e->get_semclass().empty()) entity["class"] = jsn::json(util::wstring2string(e->get_semclass()));
    if (not e->get_sense().empty()) entity["sense"] = jsn::json(util::wstring2string(e->get_sense()));
    
    jsn::ordered_json mentions = json::array();
    for (vector<semgraph::SG_mention>::const_iterator m=e->get_mentions().begin(); m!=e->get_mentions().end(); m++) {
      jsn::ordered_json ment;
      ment["id"] =  jsn::json(util::wstring2string(get_token_id(m->get_sentence_id(),util::wstring2int(m->get_id())) ));
      ment["words"] =  jsn::json(util::wstring2string(util::list2wstring(m->get_words(),L" ")));
      mentions.push_back(ment);
    }
    entity["mentions"] = mentions;
    
    const list<wstring> &syns = e->get_synonyms();
    if (not syns.empty()) {
      jsn::ordered_json synonyms = json::array();
      for (list<wstring>::const_iterator s=syns.begin(); s!=syns.end(); ++s) {
	synonyms.push_back(util::wstring2string(*s));
      }
      entity["synonyms"] = synonyms;
    }
    
    const list<pair<wstring,wstring> > &uris = e->get_URIs();
    if (not uris.empty()) {
      jsn::ordered_json urilist = json::array();
      for (list<pair<wstring,wstring> >::const_iterator s=uris.begin(); s!=uris.end(); ++s) {
	jsn::ordered_json uri;
	uri["knowledgeBase"] = jsn::json(util::wstring2string(s->first));
	uri["URI"] = jsn::json(util::wstring2string(s->second));
	urilist.push_back(uri);
      }
      entity["URIs"] = urilist;
    }
    
    ne++;
    entities.push_back(entity); 
  }
  
  semgr["entities"] = entities;

  jsn::ordered_json frames = json::array();
    
  int nf=0;
  for (vector<semgraph::SG_frame>::const_iterator f=doc.get_semantic_graph().get_frames().begin();
       f!=doc.get_semantic_graph().get_frames().end();
       f++) {

    // skip argless nominal predicates
    if (not doc.get_semantic_graph().has_arguments(f->get_id())) continue;

    jsn::ordered_json frame;
    frame["id"] = jsn::json(util::wstring2string( f->get_id()));
    frame["token"] = jsn::json(util::wstring2string(get_token_id(f->get_sentence_id(),util::wstring2int(f->get_token_id()))));
    frame["lemma"] = jsn::json(util::wstring2string(f->get_lemma()));
    frame["sense"] = jsn::json(util::wstring2string(f->get_sense() ));

    jsn::ordered_json args = json::array();
    for (vector<semgraph::SG_argument>::const_iterator a=f->get_arguments().begin(); 
	 a!=f->get_arguments().end();
	 a++) {
      jsn::ordered_json arg;
      arg["role"] = jsn::json(util::wstring2string(a->get_role()));
      arg["entity"] = jsn::json(util::wstring2string(a->get_entity()));
      args.push_back(arg);
    }
    frame["arguments"] = args;
    
    const list<wstring> &syns = f->get_synonyms();
    if (not syns.empty()) {
      jsn::ordered_json synonyms = json::array();
      for (list<wstring>::const_iterator s=syns.begin(); s!=syns.end(); ++s) {
	synonyms.push_back(util::wstring2string(*s));
      }
      frame["synonyms"] = synonyms;
    }
    
    const list<pair<wstring,wstring> > &uris = f->get_URIs();
    if (not uris.empty()) {
      jsn::ordered_json urilist = json::array();
      for (list<pair<wstring,wstring> >::const_iterator s=uris.begin(); s!=uris.end(); ++s) {	
	jsn::ordered_json uri;
	uri["knowledgeBase"] = jsn::json(util::wstring2string(s->first));
	uri["URI"] = jsn::json(util::wstring2string(s->second));
	urilist.push_back(uri);
      }
      frame["URIs"] = urilist;
    }
    
    frames.push_back(frame);
    nf++;
  }

  semgr["frames"] = frames;
  
  return semgr;
}


//---------------------------------------------
// get json object for whole document
//---------------------------------------------

jsn::ordered_json output_json::json_Document(const document &doc) const {

  jsn::ordered_json tot;  
  if (doc.empty()) return tot;

  jsn::ordered_json pars = json::array();
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    jsn::ordered_json par;
    par["sentences"] = json_Sentences(*p);
    pars.push_back(par);
  }
  tot["paragraphs"] = pars;
  
  if (doc.get_num_groups()>0) {
    // there are coreferences, print them
    tot["coreferences"] = json_Corefs(doc);
  }
  
  // print semantic graph if there is one
  if (not doc.get_semantic_graph().empty()) {
    tot["semantic_graph"] = json_Semgraph(doc);
  }

  return tot;
}



//---------------------------------------------
// print obtained analysis in json
//---------------------------------------------

void output_json::PrintResults (wostream &sout, const list<sentence> &ls) const {

  if (ls.empty()) return;
  
  jsn::ordered_json sents = json_Sentences(ls);
  sout << util::string2wstring(sents.dump(3)) << endl;

}


//----------------------------------------------------------
// print global information of the document 
//----------------------------------------------------------

void output_json::PrintResults(wostream &sout, const document &doc) const {

  if (doc.empty()) return;

  jsn::ordered_json res = json_Document(doc);  
  sout << util::string2wstring(res.dump(3)) << endl;
}



