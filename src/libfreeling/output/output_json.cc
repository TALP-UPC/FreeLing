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
// print obtained analysis in json
//---------------------------------------------

void output_json::PrintResults (wostream &sout, const list<sentence> &ls) const {
   PrintSentences(sout,ls);
}

//---------------------------------------------
// print sentences in list
//---------------------------------------------

void output_json::PrintSentences (wostream &sout, const list<sentence> &ls) const {

  if (ls.empty()) return;

  sout<<L"   { \"sentences\" : [" << endl;
  for (list<sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) {

    if (s->empty()) continue;
    if (s!=ls.begin()) sout << L", " << endl;

    sout << L"      { \"id\":\"" << s->get_sentence_id() << L"\"," << endl;
    sout << L"        \"tokens\" : [" << endl;    
    for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {

      if (w!=s->begin()) sout << L"," << endl;

      // basic token stuff
      sout << L"           { \"id\" : \"" << get_token_id(s->get_sentence_id(),w->get_position()+1) << L"\"";
      sout << L", \"begin\" : \"" << w->get_span_start() << L"\"";
      sout << L", \"end\" : \"" << w->get_span_finish() << L"\"";
      sout << L", \"form\" : \"" << escapeJSON(w->get_form()) << L"\"";

      // no analysis, nothing else to print
      if (w->empty()) {
        sout << L"}";
        continue;
      }

      // morpho & tagger stuff
      wstring lemma,tag;
      if (w->selected_begin()->is_retokenizable()) {
        const list <word> &rtk = w->selected_begin()->get_retokenizable();
        list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
        lemma = la.begin()->get_lemma();
        tag = la.begin()->get_tag();
      }
      else {
        lemma = w->get_lemma();
        tag = w->get_tag();
      }      
      sout << L", \"lemma\" : \"" << escapeJSON(lemma) << L"\"";
      sout << L", \"tag\" : \"" << tag << L"\"";

      if (Tags!=NULL) {
        if (lemma.find(L"+")!=wstring::npos) { // is a retokenizable analysis
          list<wstring> tgs=util::wstring2list(tag,L"+");
          wstring shtag,msd;
          for (list<wstring>::const_iterator t=tgs.begin(); t!=tgs.end(); t++) {
            shtag += L"+" + Tags->get_short_tag(*t);
            msd += L"+" + Tags->get_msd_string(*t);
          }
          sout << L", \"ctag\" : \"" << shtag.substr(1) << L"\"";
          sout << L", \"msd\" : \"" << msd.substr(1) << L"\"";
        }
        
        else { // not retokenizable
          sout << L", \"ctag\" : \"" << Tags->get_short_tag(tag) << L"\"";
          list<pair<wstring,wstring> > feats = Tags->get_msd_features(tag);
          for (list<pair<wstring,wstring> >::iterator f=feats.begin(); f!=feats.end(); f++) 
            sout << L", \"" << f->first << L"\" : \"" << f->second << "\"";
        }
      }

      // NEC output, if any
      wstring nec=L"";
      if (w->get_tag()==L"NP00SP0") nec=L"PER";
      else if (w->get_tag()==L"NP00G00") nec=L"LOC";
      else if (w->get_tag()==L"NP00O00") nec=L"ORG";
      else if (w->get_tag()==L"NP00V00") nec=L"MISC";
      if (not nec.empty()) sout << L", \"nec\" : \"" << nec << L"\"";
      
      // WSD output, if any
      if (not w->get_senses().empty()) sout << L", \"wn\" : \"" << w->get_senses().begin()->first << L"\"";

      if (AllAnalysis) {
        sout << L","<<endl;
        sout << "             \"analysis\" : [" <<endl;
        for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
          if (a!=w->begin()) sout << L"," << endl;
          sout << L"                    { \"lemma\" : \"" <<  escapeJSON(a->get_lemma()) << L"\"";
          sout << L", \"tag\" : \"" << a->get_tag() << "\"";
          if (Tags!=NULL) {
            sout << L", \"ctag\" : \"" << Tags->get_short_tag(a->get_tag()) << L"\"";
            list<pair<wstring,wstring> > feats = Tags->get_msd_features(a->get_tag());
            for (list<pair<wstring,wstring> >::iterator f=feats.begin(); f!=feats.end(); f++) 
              sout << L", \"" << f->first << L"\" : \"" << f->second << "\"";
          }
          if (a->is_selected()) sout << L", \"selected\" : \"1\"";
          sout << L"}";         
        }
        sout << L"]";
      }

      if (AllSenses and not w->get_senses().empty()) {
        sout << L"," << endl;
        sout << "             \"senses\" : [" <<endl;
        for (list<pair<wstring,double> >::const_iterator s=w->get_senses().begin(); s!=w->get_senses().end(); s++) {
          if (s!=w->get_senses().begin()) sout << L"," << endl;
          sout << L"                    { \"wn\" : \"" << s->first << L"\"";
          if (s->second!=0) sout << L", \"pgrank\" : \"" << s->second << "\"";
          sout << L"}";         
        }
        sout << L"]";
      }

      sout << L"}";
    }
    sout << L"]";

    // print parse tree, if any
    if (s->is_parsed()) {
      sout << L"," << endl;
      sout << L"        \"constituents\" : [" << endl;
      PrintTreeJSON (sout, s->get_sentence_id(), s->get_parse_tree().begin(), 5); 
      sout << L"]";
    }

    //  print dependency tree, if any
    if (s->is_dep_parsed()) {
      sout << L"," << endl;
      sout << L"        \"dependencies\" : [" << endl;
      PrintDepTreeJSON (sout, s->get_sentence_id(), s->get_dep_tree().begin(), 5);
      sout << L"]";
    
      // predicates, if any
      if (not s->get_predicates().empty()) {
        sout << ", " << endl;
        PrintPredArgsJSON(sout,*s);
      }
    }

    sout << L"}";  // end sentence
  }

  sout<<L"]}" << endl;  // end list of sentences
}


//---------------------------------------------
// print parse tree
//--------------------------------------------

void output_json::PrintTreeJSON (wostream &sout, const std::wstring &sid,
                                 parse_tree::const_iterator n, int depth) const {

  wstring indent=wstring (depth * 2, ' ');
  if (n.num_children () == 0) 
    sout << indent << L"{\"leaf\" : \"1\"" << (n->is_head () ? L", \"head\" : \"1\"" : L"") << L", \"token\" : \"" << get_token_id(sid,n->get_word().get_position()+1) << L"\", \"word\" : \"" << escapeJSON(n->get_word().get_form()) << "\"}";

  else {
    sout << indent << L"{\"label\" : \"" << n->get_label() << L"\"" << (n->is_head () ? L", \"head\" : \"1\"" : L"") << ", \"children\" : [" << endl;

    parse_tree::const_sibling_iterator d = n.sibling_begin (); 
    while (d != n.sibling_end ()) {
      PrintTreeJSON (sout, sid, d, depth + 1);
      d++;
      if (d!= n.sibling_end()) sout << L", ";
      sout << endl;
    }
    sout << indent << L"]}";
  }

}


//---------------------------------------------
// print dependency tree
//---------------------------------------------

void output_json::PrintDepTreeJSON (wostream &sout, const std::wstring &sid,
                                    dep_tree::const_iterator n, int depth) const {

  wstring indent=wstring (depth * 2, ' ');
  if (n.num_children () == 0) 
    sout << indent << L"{\"token\" : \"" << get_token_id(sid,n->get_word().get_position()+1) << "\", \"function\" : \"" << n->get_label() << L"\", \"word\" : \"" << escapeJSON(n->get_word().get_form()) << "\"}";

  else {
    sout << indent;
    if (n->get_label()==L"VIRTUAL_ROOT") 
      sout << L"{\"token\" : \"VIRTUAL_ROOT\""; 
    else {
      sout << L"{\"token\" : \"" <<  get_token_id(sid,n->get_word().get_position()+1) 
           << "\", \"function\" : \"" << n->get_label() 
           << L"\", \"word\" : \"" << escapeJSON(n->get_word().get_form());
    }
    sout << "\", \"children\" : [" << endl;

    // Sort children. Chunks first, then by their word position.
    // (this is just for aesthetic reasons)
    list<dep_tree::const_sibling_iterator> children;
    for (dep_tree::const_sibling_iterator d = n.sibling_begin (); d != n.sibling_end (); ++d)
      children.push_back(d);
    children.sort(ascending_position);

    // print children in the right order
    list<dep_tree::const_sibling_iterator>::const_iterator ch=children.begin();
    while (ch != children.end()) {      
      PrintDepTreeJSON (sout, sid, (*ch), depth + 1);
      ch++;
      if (ch!=children.end()) sout<<L", ";
      sout << endl;
    }

    sout << indent << L"]}";    
  }
}

//---------------------------------------------
// print predicate-argument structure of the sentence
//---------------------------------------------

void output_json::PrintPredArgsJSON(wostream &sout, const sentence &s) const {

  const dep_tree & dt = s.get_dep_tree();
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
  sout << L"        \"predicates\" : ["<<endl;
  int npred=1;
  for (sentence::predicates::const_iterator pred=s.get_predicates().begin(); 
       pred!=s.get_predicates().end(); 
       pred++) {

    if (pred!=s.get_predicates().begin()) sout << L"," << endl;

    sout<<L"            { \"id\" : \"" << get_token_id(sid,npred,L"P") 
        << L"\", \"head_token\" : \"" << get_token_id(sid,pred->get_position()+1) 
        << "\", \"sense\" : \"" << pred->get_sense()
        <<L"\", \"words\" : \"" << escapeJSON(s[pred->get_position()].get_form()) << "\"";
    
    if (not pred->empty()) {
      sout << L"," << endl;
      sout << "              \"arguments\" : [" << endl;
      
      for (predicate::const_iterator arg=pred->begin(); arg!=pred->end(); arg++) {
        if (arg!=pred->begin()) sout << L"," << endl;

        sout << L"                  { "
             << L" \"role\" : \"" << arg->get_role() << L"\", "
             << L" \"words\" : \"" << escapeJSON(arg_words[arg->get_position()]) << L"\", "
             << L" \"head_token\" : \"" << get_token_id(sid,arg->get_position()+1) << L"\", "
             << L" \"from\" : \"" << get_token_id(sid,arg_span[arg->get_position()].first+1) << L"\", "
             << L" \"to\" : \"" << get_token_id(sid,arg_span[arg->get_position()].second+1) << L"\" "
             << L"}";
      }

      sout << "]";
    }
    sout << "}";


    npred++;
  }

  sout << L"]";

}

//---------------------------------------------
// print coreference information of the document
//---------------------------------------------

void output_json::PrintCorefs(wostream &sout, const document &doc) const {

  sout << L"\"coreferences\" : [" << endl;
  for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {
    if (g!=doc.get_groups().begin()) sout << L"," << endl;
    
    sout << L"     { \"id\" : \"co" << *g+1 << L"\"," << endl;
    sout << L"       \"mentions\" : [" << endl;      
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
      
      if (m!=mentions.begin()) sout << L"," << endl;
      
      sout << L"           { \"id\" : \"m" << *g+1 << L"." << nm << "\", "
           << L" \"from\" : \"" << get_token_id(sid,ment.get_pos_begin()+1) << "\", "
           << L" \"to\" : \"" << get_token_id(sid,ment.get_pos_end()+1) << "\", "
           << L" \"words\" : \"" << escapeJSON(words) << "\" }";
      nm++;
    }      
    sout << L"]}";
  }
  sout << L"]";
}



//----------------------------------------------------------
// print semantic graph of the document 
//----------------------------------------------------------

void output_json::PrintSemgraph(wostream &sout, const document &doc) const {
    sout << L"\"semantic_graph\" : {" << endl;
    sout << L"     \"entities\" : [" << endl;
    int ne=0;
    for (vector<semgraph::SG_entity>::const_iterator e=doc.get_semantic_graph().get_entities().begin();
         e!=doc.get_semantic_graph().get_entities().end();
         e++) {
      // skip non NE entities that are not argument to any predicate
      if (not doc.get_semantic_graph().is_argument(e->get_id())) continue;

      if (ne>0) sout << L"," << endl;
      sout << L"           { \"id\" : \"" << e->get_id() << L"\"" 
           << L", \"lemma\" : \"" <<  escapeJSON(e->get_lemma()) << L"\"";
      if (not e->get_semclass().empty()) sout << L", \"class\" : \"" << e->get_semclass() << L"\"";
      if (not e->get_sense().empty()) sout << L", \"sense\" : \"" << e->get_sense() << L"\"";
      sout << "," << endl;      
      
      sout << L"             \"mentions\" : [" << endl;
      for (vector<semgraph::SG_mention>::const_iterator m=e->get_mentions().begin(); m!=e->get_mentions().end(); m++) {
        if (m!=e->get_mentions().begin()) sout << L"," << endl;
        sout << L"                   { \"id\" : \"" << get_token_id(m->get_sentence_id(),util::wstring2int(m->get_id())) << L"\""
             << L", \"words\" : \"" << escapeJSON(util::list2wstring(m->get_words(),L" ")) << L"\" "
             << L"}";
      }      
      sout << L"]";

      const list<wstring> &syns = e->get_synonyms();
      if (not syns.empty()) {
        sout << L"," << endl;
        sout << L"             \"synonyms\" : [ ";
        for (list<wstring>::const_iterator s=syns.begin(); s!=syns.end(); ++s) {
          if (s!=syns.begin()) sout << L", ";
          sout << L"\""<< *s << L"\"";
        }
        sout << L" ]";
      }
      
      const list<pair<wstring,wstring> > &uris = e->get_URIs();
      if (not uris.empty()) {
        sout << L"," << endl;
        sout << L"             \"URIs\" : [ " << endl;
        for (list<pair<wstring,wstring> >::const_iterator s=uris.begin(); s!=uris.end(); ++s) {
          if (s!=uris.begin()) sout << L", " << endl;
          sout << L"                   { \"knowledgeBase\" : \"" << s->first << L"\", \"URI\" : \"" << s->second << L"\" }";
        }          
        sout << "]";
      }

      sout << "}";
      ne++;
    }
    sout << L"]," << endl;
    
    sout << L"     \"frames\" : [" << endl;    
    int nf=0;
    for (vector<semgraph::SG_frame>::const_iterator f=doc.get_semantic_graph().get_frames().begin();
         f!=doc.get_semantic_graph().get_frames().end();
         f++) {
      // skip argless nominal predicates
      if (not doc.get_semantic_graph().has_arguments(f->get_id())) continue;

      if (nf>0) sout << L"," << endl;
      sout << L"           { \"id\" : \"" << f->get_id() << L"\"" 
           << L", \"token\" : \"" << get_token_id(f->get_sentence_id(),util::wstring2int(f->get_token_id())) << L"\""
           << L", \"lemma\" : \"" << escapeJSON(f->get_lemma()) << L"\"" 
           << L", \"sense\" : \"" << f->get_sense() << L"\""
           << L"," << endl; 
      
      sout << L"             \"arguments\" : [" << endl;
      for (vector<semgraph::SG_argument>::const_iterator a=f->get_arguments().begin(); 
           a!=f->get_arguments().end();
           a++) {
        if (a!=f->get_arguments().begin()) sout << L"," << endl;
        sout << L"                         { \"role\" : \"" << a->get_role() << L"\"" 
             << L", \"entity\" : \"" << a->get_entity() << "\" "
             << L"}"; 
      }
      sout << L"]";

      const list<wstring> &syns = f->get_synonyms();
      if (not syns.empty()) {
        sout << L"," << endl;
        sout << L"             \"synonyms\" : [ ";
        for (list<wstring>::const_iterator s=syns.begin(); s!=syns.end(); ++s) {
          if (s!=syns.begin()) sout << L", ";
          sout << L"\""<< *s << L"\"";
        }
        sout << L" ]";
      }
      
      const list<pair<wstring,wstring> > &uris = f->get_URIs();
      if (not uris.empty()) {
        sout << L"," << endl;
        sout << L"             \"URIs\" : [ " << endl;
        for (list<pair<wstring,wstring> >::const_iterator s=uris.begin(); s!=uris.end(); ++s) {
          if (s!=uris.begin()) sout << L", " << endl;
          sout << L"                   { \"knowledgeBase\" : \"" << s->first << L"\", \"URI\" : \"" << s->second << L"\" }";
        }          
        sout << "]";
      }
        
      sout << "}";
      nf++;
    }
    
    sout << L"]}" << endl;
}


//----------------------------------------------------------
// print global information of the document 
//----------------------------------------------------------

void output_json::PrintResults(wostream &sout, const document &doc) const {

  sout << L"{ \"paragraphs\" : [" << endl;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    if (p!=doc.begin()) sout << L"," << endl;
    PrintSentences(sout,*p);
  }
  sout << L"]" ;

  if (doc.get_num_groups()>0) {
    // there are coreferences, print them
    sout << L"," << endl;
    PrintCorefs(sout,doc);
  }

  // print semantic graph if there is one
  if (not doc.get_semantic_graph().empty()) {
    sout << "," << endl;
    PrintSemgraph(sout,doc);
  }

  sout << L"}" << endl;
}



