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
#include "freeling/output/output_xml.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"OUTPUT_XML"
#define MOD_TRACECODE OUTPUT_TRACE


//---------------------------------------------
// Constructor
//---------------------------------------------

output_xml::output_xml()  {
  // default values
  AllAnalysis = false;
  AllSenses = false;
}

//---------------------------------------------
// Constructor from cfg file
//---------------------------------------------

output_xml::output_xml(const wstring &cfgFile) {

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
      if (util::lowercase(line)!=L"xml")
        ERROR_CRASH(L"Invalid configuration file for 'xml' output handler, "+cfgFile);
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

output_xml::~output_xml() {}


//---------------------------------------------
// auxiliary to print one word analysis in XML
//---------------------------------------------
void output_xml::print_analysis(wostream &sout, const analysis &a, bool print_sel_status, bool print_probs) const {

  sout << L" lemma=\"" << escapeXML(a.get_lemma()) << L"\"";
  sout << L" tag=\"" << a.get_tag() << L"\"";

  if (Tags!=NULL) {
    if (a.get_lemma().find(L"+")!=wstring::npos) { // is a retokenizable analysis
      list<wstring> tgs=util::wstring2list(a.get_tag(),L"+");
      wstring shtag,msd;
      for (list<wstring>::const_iterator t=tgs.begin(); t!=tgs.end(); t++) {
        shtag += L"+" + Tags->get_short_tag(*t);
        msd += L"+" + Tags->get_msd_string(*t);
      }
      sout << L" ctag=\"" << shtag.substr(1) << L"\"";
      sout << L" msd=\"" << msd.substr(1) << L"\"";
    }

    else { // not retokenizable
      sout << L" ctag=\"" << Tags->get_short_tag(a.get_tag()) << L"\"";
      list<pair<wstring,wstring> > feats = Tags->get_msd_features(a.get_tag());
      for (list<pair<wstring,wstring> >::iterator f=feats.begin(); f!=feats.end(); f++) 
        sout << L" " << f->first << L"=\"" << f->second << "\"";
    }
  }

  if (print_probs and a.get_prob()>=0) sout << L" prob=\"" << a.get_prob() << "\"";
  if (print_sel_status and a.is_selected()) sout << L" selected=\"1\"";
}


//---------------------------------------------
// print obtained analysis in XML
//---------------------------------------------

void output_xml::PrintResults (wostream &sout, const list<sentence> &ls) const {

  for (list<sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) {

    wstring sid = s->get_sentence_id();
    sout << L"<sentence id=\"" << sid << L"\">" <<endl;

    for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {
      // basic token stuff
      sout << L"  <token id=\"" << get_token_id(sid,w->get_position()+1) << L"\"";
      sout << L" begin=\"" << w->get_span_start() << L"\"";
      sout << L" end=\"" << w->get_span_finish() << L"\"";
      sout << L" form=\"" << escapeXML(w->get_form()) << L"\"";
      if (not w->get_ph_form().empty())
        sout << L" phon=\"" << escapeXML(w->get_ph_form()) << L"\"";
      
      // no analysis, nothing else to print
      if (w->empty()) {
        sout << L" />" << endl;
        continue;
      }
      
      if (not s->is_tagged()) {
        // morpho output (all analysis)
        sout << L">" << endl;
        for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
          if (a->is_retokenizable()) {
            const list <word> &rtk = a->get_retokenizable();
            list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
            for (list<analysis>::iterator aa=la.begin(); aa!=la.end(); aa++) {
              double p=a->get_prob();
              if (p>=0) aa->set_prob(p/la.size());
              else aa->set_prob(-1.0);
              sout << L"    <analysis";
              print_analysis(sout,*aa,false,true);
              sout << " />" << endl;
            }
          }
          else {
            sout << L"    <analysis";
            print_analysis(sout,*a,false,true);
            sout << " />" << endl;
          }
        }
        sout << L"  </token>" <<endl;
      }
      else {
        // tagger output 
        if (w->selected_begin()->is_retokenizable()) {
          const list <word> &rtk = w->selected_begin()->get_retokenizable();
          list <analysis> la=compute_retokenization(rtk, rtk.begin(), L"", L"");
          print_analysis(sout,*(la.begin()),false,false);
        }
        else
          print_analysis(sout,*(w->selected_begin()),false,false);

        // print other info (NEC, WSD...)
        // NEC output, if any
        wstring nec=L"";
        if (w->get_tag()==L"NP00SP0") nec=L"PER";
        else if (w->get_tag()==L"NP00G00") nec=L"LOC";
        else if (w->get_tag()==L"NP00O00") nec=L"ORG";
        else if (w->get_tag()==L"NP00V00") nec=L"MISC";
        if (not nec.empty()) sout << L" nec=\"" << nec << L"\"";
        
        // WSD output, if any
        if (not w->get_senses().empty()) sout << L" wn=\"" << w->get_senses().begin()->first << L"\"";

        sout<< L" >" << endl;

        // if all analysis requested, print them all
        if (AllAnalysis) {
          sout << L"    <morpho>" << endl;
          for (word::const_iterator a=w->begin(); a!=w->end(); a++) {
            sout << L"       <analysis";
            print_analysis(sout,*a,true,false);
            sout << " />" << endl;
          }
          sout << L"    </morpho>" << endl;
        }

        // if all senses requested, print them all
        if (AllSenses) { 
          if (not w->get_senses().empty()) {
            sout << L"    <senses>" << endl;
            for (list<pair<wstring,double> >::const_iterator s=w->get_senses().begin(); s!=w->get_senses().end(); s++) {
              sout << L"       <sense wn=\"" << s->first << L"\"";
              if (s->second!=0) sout << " pgrank=\"" << s->second << "\"";
              sout << " />" << endl; 
            }
            sout << L"    </senses>" << endl;
          }
        }
        sout << L"  </token>" << endl;

      }
    }

    // print parse tree, if any
    if (s->is_parsed()) {
      sout << L"  <constituents>" << endl;
      PrintTreeXML (sout, s->get_sentence_id(), s->get_parse_tree().begin(), 2);      
      sout << L"  </constituents>" << endl;
    }

    //  print dependency tree, if any
    if (s->is_dep_parsed()) {
      sout << L"  <dependencies>" << endl;
      PrintDepTreeXML (sout, s->get_sentence_id(), s->get_dep_tree().begin(), 2);
      sout << L"  </dependencies>" << endl;
    
      // predicates, if any
      if (not s->get_predicates().empty()) 
        PrintPredArgsXML(sout,*s);
    }

    sout << L"</sentence>" << endl;  // end sentence
  }
}

//---------------------------------------------
// print XML file header
//--------------------------------------------

void output_xml::PrintHeader(wostream &sout) const { 
  sout<<L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
  sout<<L"<document>"<<endl;
}

//---------------------------------------------
// print XML footer
//--------------------------------------------

void output_xml::PrintFooter(wostream &sout) const { 
  sout<<L"</document>"<<endl;
}



//---------------------------------------------
// print parse tree
//--------------------------------------------

void output_xml::PrintTreeXML (wostream &sout, const wstring &sid, parse_tree::const_iterator n, int depth) const {

  wstring indent=wstring (depth * 2, ' ');
  if (n.num_children () == 0) 
    sout << indent << L"<node leaf=\"1\"" << (n->is_head () ? L" head=\"1\"" : L"") << L" token=\"" << get_token_id(sid,n->get_word().get_position()+1) << L"\" word=\"" << escapeXML(n->get_word().get_form()) << "\" />" << endl;

  else {
    sout << indent << L"<node" << (n->is_head () ? L" head=\"1\"" : L"") << L" label=\"" << n->get_label() << L"\" >" << endl;
    for (parse_tree::const_sibling_iterator d = n.sibling_begin (); d != n.sibling_end (); ++d)  PrintTreeXML (sout, sid, d, depth + 1);
    sout << indent << L"</node>" << endl;
  }

}


//---------------------------------------------
// print dependency tree
//---------------------------------------------

void output_xml::PrintDepTreeXML (wostream &sout, const wstring &sid, dep_tree::const_iterator n, int depth) const {

  wstring indent=wstring (depth * 2, ' ');
  if (n.num_children () == 0) 
    sout << indent 
         << L"<depnode token=\"" << get_token_id(sid,n->get_word().get_position()+1) 
         << "\" function=\"" << n->get_label() 
         << L"\" word=\"" << escapeXML(n->get_word().get_form()) << "\" />" << endl;
  
  else {
    if (n->get_label()==L"VIRTUAL_ROOT") 
      sout << indent << L"<depnode function=\"" << n->get_label() << "\" >" << endl;
    else 
      sout << indent 
           << L"<depnode token=\"" << get_token_id(sid,n->get_word().get_position()+1) 
           << "\" function=\"" << n->get_label() 
           << L"\" word=\"" << escapeXML(n->get_word().get_form()) << "\" >" << endl;
    
    // Sort children. Chunks first, then by their word position.
    // (this is just for aesthetic reasons)
    list<dep_tree::const_sibling_iterator> children;
    for (dep_tree::const_sibling_iterator d = n.sibling_begin (); d != n.sibling_end (); ++d)
      children.push_back(d);
    children.sort(ascending_position);

    // print children in the right order
    for (list<dep_tree::const_sibling_iterator>::const_iterator ch=children.begin();
         ch != children.end(); 
         ch++) 
      PrintDepTreeXML (sout, sid, (*ch), depth + 1);

    sout << indent << L"</depnode>" << endl;    
  }
}

//---------------------------------------------
// print predicate-argument structure of the sentence
//---------------------------------------------

void output_xml::PrintPredArgsXML(wostream &sout, const sentence &s) const {

  sout << L"  <predicates>" << endl;

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

  // print all predicates, with their arguments
  int npred=1;
  for (sentence::predicates::const_iterator pred=s.get_predicates().begin(); pred!=s.get_predicates().end(); pred++) {
    sout << L"    <predicate id=\"" << get_token_id(sid, npred, L"P")
         << L"\" head_token=\"" << get_token_id(sid,pred->get_position()+1) 
         << L"\" sense=\"" << pred->get_sense()
         << L"\" words=\"" << escapeXML(s[pred->get_position()].get_form()) << L"\"";
    
    if (pred->empty()) 
      sout << L" />" << endl;

    else {
      sout << L" >" << endl;

      for (predicate::const_iterator arg=pred->begin(); arg!=pred->end(); arg++) 
        sout << L"      <argument role=\"" << arg->get_role() << L"\""
             << L" words=\"" << escapeXML(arg_words[arg->get_position()]) << L"\""
             << L" head_token=\"" << get_token_id(sid,arg->get_position()+1) << L"\""
             << L" from=\"" << get_token_id(sid,arg_span[arg->get_position()].first+1) << L"\""
             << L" to=\"" << get_token_id(sid,arg_span[arg->get_position()].second+1) << L"\""
             << L" />" << endl;

      sout<<L"    </predicate>" << endl;
    }

    npred++;
  }
  
  sout << L"  </predicates>" << endl;
}

//---------------------------------------------
// print coreference information of the document
//---------------------------------------------

void output_xml::PrintCorefs(std::wostream &sout, const freeling::document &doc) const {
  sout << L"<coreferences>" << endl;
  for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {
    sout << L"  <coref id=\"co" << *g+1 << L"\">" << endl;      
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
      
      sout << L"    <mention id=\"m" << *g+1 << L"." << nm << "\""
           << L" from=\"" << get_token_id(sid,ment.get_pos_begin()+1) << "\""
           << L" to=\"" << get_token_id(sid,ment.get_pos_end()+1) << "\""
           << L" words=\"" << escapeXML(words) << "\" />" << endl;
      nm++;
    }      
    sout << L"  </coref>" << endl;
  }  
  sout << L"</coreferences>" << endl;  
}


//----------------------------------------------------------
// print semantic graph of the document 
//----------------------------------------------------------

void output_xml::PrintSemgraph(std::wostream &sout, const freeling::document &doc) const {

  sout << L"<semantic_graph>" << endl;

  /// ouput entities
  for (vector<semgraph::SG_entity>::const_iterator e=doc.get_semantic_graph().get_entities().begin();
       e!=doc.get_semantic_graph().get_entities().end();
       e++) {
    // skip non NE entities that are not argument to any predicate
    if (not doc.get_semantic_graph().is_argument(e->get_id())) continue;
    
    sout << L"   <entity id=\"" << e->get_id() << L"\" " 
         << L"lemma=\"" << escapeXML(e->get_lemma()) << L"\" ";
    if (not e->get_semclass().empty()) sout << L"class=\"" << e->get_semclass() << L"\" ";
    if (not e->get_sense().empty()) sout << L"sense=\"" << e->get_sense() << L"\" ";
    sout << L">" << endl;      

    for (vector<semgraph::SG_mention>::const_iterator m=e->get_mentions().begin(); m!=e->get_mentions().end(); m++) {
      sout << L"      <mention id=\"" << get_token_id(m->get_sentence_id(),util::wstring2int(m->get_id())) << L"\" "
           << L"words=\"" << escapeXML(util::list2wstring(m->get_words(),L" ")) << L"\" "
           << L"/>" << endl;
    }      

    const list<wstring> &syns = e->get_synonyms();
    if (not syns.empty()) {
      for (list<wstring>::const_iterator s=syns.begin(); s!=syns.end(); ++s)
        sout << L"      <synonym lemma=\""<< *s << L"\"/>" << endl;
    }
    const list<pair<wstring,wstring> > &uris = e->get_URIs();
    if (not uris.empty()) {
      for (list<pair<wstring,wstring>>::const_iterator s=uris.begin(); s!=uris.end(); ++s)
        sout << L"      <URI knowledgeBase=\"" << s->first << L"\" URI=\"" << s->second << L"\"/>" << endl;
    }
    
    sout << L"   </entity>" << endl;
  }  
  
  /// ouput frames
  for (vector<semgraph::SG_frame>::const_iterator f=doc.get_semantic_graph().get_frames().begin();
       f!=doc.get_semantic_graph().get_frames().end();
       f++) {

    // skip argless nominal predicates that are not argument to other predicates.
    if (not doc.get_semantic_graph().has_arguments(f->get_id()) and
	not doc.get_semantic_graph().is_argument(f->get_id()) ) 
      continue;
    
    sout << L"   <frame id=\"" << f->get_id() << L"\" " 
         << L"token=\"" << get_token_id(f->get_sentence_id(),util::wstring2int(f->get_token_id())) << L"\" "
         << L"lemma=\"" << escapeXML(f->get_lemma()) << L"\" " 
         << L"sense=\"" << f->get_sense() << L"\" "
         << ">" << endl; 
    
    for (vector<semgraph::SG_argument>::const_iterator a=f->get_arguments().begin(); 
         a!=f->get_arguments().end();
         a++) {
      sout << L"      <argument role=\"" << a->get_role() << L"\" " 
           << L"entity=\"" << a->get_entity() << "\" "
           << L"/>" << endl; 
    }

    const list<wstring> &syns = f->get_synonyms();
    if (not syns.empty()) {
      for (list<wstring>::const_iterator s=syns.begin(); s!=syns.end(); ++s)
        sout << L"      <synonym lemma=\""<< *s << L"\"/>" << endl;
    }
    const list<pair<wstring,wstring> > &uris = f->get_URIs();
    if (not uris.empty()) {
      for (list<pair<wstring,wstring>>::const_iterator s=uris.begin(); s!=uris.end(); ++s)
        sout << L"      <URI knowledgeBase=\"" << s->first << L"\" URI=\"" << s->second << L"\"/>" << endl;
    }

    sout << L"   </frame>" << endl;
  }
  
  sout << L"</semantic_graph>" << endl;
}


//----------------------------------------------------------
// print global information of the document 
//----------------------------------------------------------

void output_xml::PrintResults(wostream &sout, const document &doc) const {

  PrintHeader(sout);
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    sout<<L"<paragraph>"<<endl;
    PrintResults(sout,*p);
    sout<<L"</paragraph>"<<endl;
  }

  // print coreferences if there are any
  if (doc.get_num_groups()>0) 
    PrintCorefs(sout,doc);

  // print semantic graph if there is one
  if (not doc.get_semantic_graph().empty()) 
    PrintSemgraph(sout,doc);

  PrintFooter(sout);
}



