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

#include "freeling/version.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/output/output_naf.h"

using namespace std;
using namespace freeling;
using namespace freeling::io;

#define MOD_TRACENAME L"OUTPUT_NAF"
#define MOD_TRACECODE 0

//---------------------------------------------
// Constructor
//---------------------------------------------

output_naf::output_naf() {
  // by default, all layers are active
  ActivateLayer(L"text",true);   ActivateLayer(L"terms",true);        ActivateLayer(L"entities",true);
  ActivateLayer(L"chunks",true); ActivateLayer(L"constituency",true); ActivateLayer(L"deps",true);
  ActivateLayer(L"srl",true);    ActivateLayer(L"coreferences",true);
}

//---------------------------------------------
// Constructor from cfg file
//---------------------------------------------

output_naf::output_naf(const wstring &cfgFile) {

  enum sections {OUTPUT_TYPE,LANGUAGE,TAGSET,LAYERS};
  config_file cfg(true);
  cfg.add_section(L"Type",OUTPUT_TYPE);
  cfg.add_section(L"Language",LANGUAGE);
  cfg.add_section(L"TagsetFile",TAGSET);
  cfg.add_section(L"ActiveLayers",LAYERS);

  if (not cfg.open(cfgFile))
    ERROR_CRASH(L"Error opening file "+cfgFile);

  // by default, all layers are active
  ActivateLayer(L"text",true);   ActivateLayer(L"terms",true);        ActivateLayer(L"entities",true);
  ActivateLayer(L"chunks",true); ActivateLayer(L"constituency",true); ActivateLayer(L"deps",true);
  ActivateLayer(L"srl",true);    ActivateLayer(L"coreferences",true);

  wstring line; 
  while (cfg.get_content_line(line)) {
    
    // process each content line according to the section where it is found
    switch (cfg.get_section()) {
      
    case OUTPUT_TYPE: {
      if (util::lowercase(line)!=L"naf")
        ERROR_CRASH(L"Invalid configuration file for 'naf' output handler, "+cfgFile);
      break;
    }

    case LANGUAGE: { 
      Lang = util::lowercase(line);
      break;
    }
      
    case TAGSET: { 
      wstring path = cfgFile.substr(0,cfgFile.find_last_of(L"/\\")+1);
      Tags = new tagset(util::absolute(line,path));
      break;
    }
      
    case LAYERS: {  // reading Function words
      // layers are being explicitly set. Forget about default
      if (cfg.at_section_start()) layers.clear();  
      line = util::lowercase(line);      
      if (line==L"text" or line==L"terms" or line==L"entities" or line==L"chunks" or 
          line==L"constituency" or line==L"deps" or line==L"srl" or line==L"coreferences")
        ActivateLayer(line,true);

      else 
        WARNING(L"Invalid layer '"+line+L" in output_naf config file '"+cfgFile+L"'");

      break;
    }
      
    default: break;
    }
  }
  
  cfg.close(); 

  if (Lang.empty()) Lang=L"???";
}



//---------------------------------------------
// Destructor
//---------------------------------------------

output_naf::~output_naf() {}


//--------------------------------------------
// Auxiliary to print tokens inside a multiword
//--------------------------------------------

void output_naf::print_tokens(wostream &sout, const word &w, bool wf, const wstring &nsent, int &ntok) {

  if (w.is_multiword()) {
    for (list<word>::const_iterator s=w.get_words_mw().begin(); s!=w.get_words_mw().end(); s++) 
      print_tokens(sout, *s, wf, nsent, ntok);
  }
  else {
    if (wf) {
      sout << L"<wf id=\"w" << ntok
           << L"\" length=\"" << w.get_span_finish() - w.get_span_start() 
           << L"\" offset=\"" << w.get_span_start()
           << L"\" sent=\"" << nsent 
           << "\">" 
           << escapeXML(w.get_form())
           << L"</wf>"<<endl;
    }
    else 
      sout << L"<target id=\"w" << ntok << "\" />" << endl;
    
    ntok++;
  }
}


//--------------------------------------------
// Auxiliary to print terms in a span
//--------------------------------------------

void output_naf::print_span(wostream &sout, const sentence &s, int from, int to) {

  wstring sid = s.get_sentence_id();
  sout << L"<span>" << endl;
  for (int w=from; w<=to; w++) 
    sout << L"<target id=\"" << get_term_id(sid,s[w]) << "\"/>" << endl;
  sout << L"</span>" << endl;
}


//---------------------------------------------
// auxiliary to print externalReferences of a word
//---------------------------------------------

void output_naf::print_external_refs(wostream &sout, const word &w) const {

  if (not w.get_senses().empty()) {
    sout << L"<externalReferences>" << endl;

    for (list<pair<wstring,double> >::const_iterator s=w.get_senses().begin(); s!=w.get_senses().end(); s++)
      sout << L"<externalRef resource=\"WN-3.1\""
           << L" reference=\""  << Lang << "-3.1-" << s->first << L"\"" 
           << L" confidence=\"" << s->second << L"\" />" << endl;

    sout << L"</externalReferences>" << endl;
  }    
}

//---------------------------------------------
// auxiliary to compute term id for a word
//---------------------------------------------

wstring output_naf::get_term_id(const wstring &sid, const word &w, const wstring &pref) {
  wstring pr;
  if (pref.empty()) pr = (w.is_multiword() && w.get_n_words_mw()>1 ? L"t.mw" : L"t");
  else pr = pref;

  return pr + sid + L"." + util::int2wstring(w.get_position());
}


//---------------------------------------------
// print text in NAF
//---------------------------------------------

void output_naf::PrintTextLayer (wostream &sout, const document &doc) const {

  if (doc.empty()) return;

  sout << L"<text>" << endl;
  int nsent=1;
  int ntok=1;
  int nword=1;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) {    
      for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {
        print_tokens(sout, *w, true, s->get_sentence_id(), ntok);
        nword++;
      }
      nsent++;
    }
  }
  sout << L"</text>" << endl;
}

//---------------------------------------------
// auxiliary to convert a PoS tag to NAF
//---------------------------------------------

wstring output_naf::naf_pos_tag(const wstring &tag) const {

  if (Tags==NULL) return tag;

  map<wstring,wstring> m = Tags->get_msd_features_map(tag);
  
  if (m[L"pos"]==L"noun" and m[L"type"]==L"common") return L"N";
  else if (m[L"pos"]==L"noun" and m[L"type"]==L"proper") return L"R";
  else if (m[L"pos"]==L"adjective") return L"G";
  else if (m[L"pos"]==L"verb") return L"V";
  else if ( m[L"pos"]==L"preposition" or (m[L"pos"]==L"adposition" and m[L"type"]==L"preposition")) return L"P";
  else if (m[L"pos"]==L"adverb") return L"A";
  else if (m[L"pos"]==L"conjunction") return L"C";
  else if (m[L"pos"]==L"determiner") return L"D";
  else return L"O";
}


//---------------------------------------------
// print terms in NAF
//---------------------------------------------

void output_naf::PrintTermsLayer (wostream &sout, const document &doc) const {

  if (doc.empty()) return;
  if (not doc.begin()->begin()->is_tagged()) return;

  sout << L"<terms>" << endl;  
  int nsent=1;
  int ntok=1;
  int nword=1;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) {    
      wstring sid = s->get_sentence_id();
      for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {
        sout << L"<term id=\"" << get_term_id(sid,*w) << "\""
             << L" lemma=\"" << escapeXML(w->get_lemma()) << "\""
             << L" pos=\"" << naf_pos_tag(w->get_tag()) << "\"";
        if (Tags!=NULL) sout<< L" morphofeat=\"" << Tags->get_msd_string(w->get_tag()) << "\"";        
        sout << ">" << endl;
        
        sout << L"<span>" << endl;
        print_tokens(sout, *w, false, sid, ntok);
        sout << L"</span>" << endl; 
        
        // print <externalReferences>, if any
        print_external_refs(sout,*w);
        
        sout<<L"</term>"<<endl;         
        nword++;
      }
      nsent++;
    }
  }
  sout << L"</terms>" << endl;
}

//---------------------------------------------
// print named entities in NAF
//---------------------------------------------

void output_naf::PrintEntitiesLayer(wostream &sout, const document &doc) const {

  if (doc.empty()) return;
  if (not doc.begin()->begin()->is_tagged()) return;

  int nent=1;
  sout << L"<entities>" << endl;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) {    
      wstring sid = s->get_sentence_id();
      for (sentence::const_iterator w=s->begin(); w!=s->end(); w++) {

        wstring nec=L"";
        if (w->get_tag()==L"NP00SP0") nec=L"PER";
        else if (w->get_tag()==L"NP00G00") nec=L"LOC";
        else if (w->get_tag()==L"NP00O00") nec=L"ORG";
        else if (w->get_tag()==L"NP00V00") nec=L"MISC";
        else if (w->get_tag()==L"NP00000") nec=L"UNK";
        
        else if (w->get_tag()==L"W") nec=L"DATE-TIME";
        else if (w->get_tag()==L"Zm") nec=L"MONEY";
        else if (w->get_tag()==L"Zp") nec=L"PERCENT";
        
        if (not nec.empty()) {
          sout << L"<entity id=\"e" << nent << L"\" type=\"" << nec << L"\">" << endl;
          sout << L"<references>" << endl;
          print_span(sout, *s, w->get_position(), w->get_position());
          sout << L"</references>" << endl;
          sout << L"</entity>" << endl;
          nent++;
        }
      }
    }
  }
  sout << L"</entities>" << endl;
}

//---------------------------------------------
// print chunks in NAF
//---------------------------------------------

void output_naf::PrintChunksLayer(wostream &sout, const document &doc) const {

  if (doc.empty()) return;
  if (not doc.begin()->begin()->is_parsed()) return;

  int nchunk=1;
  sout << L"<chunks>" << endl;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) {    
      wstring sid = s->get_sentence_id();
      const parse_tree &pt = s->get_parse_tree();
      for (parse_tree::const_sibling_iterator t=pt.sibling_begin(); t!=pt.sibling_end(); t++) {
      const word &head=parse_tree::get_head_word(t);
      sout << L"<chunk id=\"c" << nchunk 
           << "\" head=\"" << get_term_id(sid,head) 
           << "\" phrase=\"" << t->get_label() << "\">" << endl;
      
      print_span(sout, *s, 
                 parse_tree::get_leftmost_leaf(t)->get_word().get_position(),
                 parse_tree::get_rightmost_leaf(t)->get_word().get_position());
      
      sout << L"</chunk>" << endl;
      
      nchunk++;
      }
    }
  }
  sout << L"</chunks>" << endl;

}

//---------------------------------------------
// print constituents in NAF
//---------------------------------------------

void output_naf::PrintConstituencyLayer(wostream &sout, const document &doc) const {

  if (doc.empty()) return;
  if (not doc.begin()->begin()->is_parsed()) return;

  int nedge=1;
  sout << L"<constituency>" << endl;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) {    
      
      wstring sid = s->get_sentence_id();
      
      wstringstream nonterminals;
      wstringstream terminals;
      wstringstream edges;
      
      nonterminals << L"<nt id=\"nter" << s->get_sentence_id() << "\" label=\"ROOT\" />" << endl;
      
      const parse_tree & pt = s->get_parse_tree();
      for (parse_tree::const_iterator t=pt.begin(); t!=pt.end(); t++) {
        
        wstring from;
        if (t.num_children()>0) {
          nonterminals << L"<nt id=\"nter" << t->get_node_id() << "\" label=\"" << t->get_label() << "\"/>" << endl;
          from = L"nter" + t->get_node_id();
        }
        else {
          terminals << L"<t id=\"" << get_term_id(sid, t->get_word(), L"ter") << "\">" << endl;
          print_span(terminals, *s, t->get_word().get_position(), t->get_word().get_position());
          terminals << "</t>" <<endl;
          from = get_term_id(sid, t->get_word(), L"ter");
        }
        
        edges << L"<edge id=\"tre" << nedge 
              << L"\" from =\"" << from 
              << L"\" to=\"nter" << (t.is_root() ? s->get_sentence_id() : t.get_parent()->get_node_id()) 
              << L"\"/>" << endl;
        nedge++;
      }
      
      sout << L"<tree>" << endl;
      sout << nonterminals.str() << terminals.str() << edges.str();
      sout << L"</tree>" << endl;
    }
  }
  sout << L"</constituency>" << endl;
}

//---------------------------------------------
// print dependencies in NAF
//---------------------------------------------

void output_naf::PrintDepsLayer(wostream &sout, const document &doc) const {

  if (doc.empty()) return;
  if (not doc.begin()->begin()->is_dep_parsed()) return;

  sout << L"<deps>" << endl;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) 
    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) 
       PrintDepTreeNAF (sout, s->get_dep_tree().begin(), s->get_sentence_id());
 
  sout << L"</deps>" << endl;
}

//---------------------------------------------
// print SRL in NAF
//---------------------------------------------

void output_naf::PrintSRLLayer(wostream &sout, const document &doc) const {

  if (doc.empty()) return;

  sout << L"<srl>" << endl;
  for (document::const_iterator p=doc.begin(); p!=doc.end(); p++) {
    for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++) {
      wstring sid = s->get_sentence_id();
      
      // print all predicates, referring to entities (or other predicates) as their arguments
      for (sentence::predicates::const_iterator pred=s->get_predicates().begin(); pred!=s->get_predicates().end(); pred++) {
        
        wstring pid = sid+L"."+util::int2wstring(pred->get_position()+1); 
        sout << L"<predicate id=\"pr" << pid << "\" uri=\"PropBank:" << pred->get_sense() << "\">" << endl;
        
        // print <externalReferences>, if any
        print_external_refs(sout,(*s)[pred->get_position()]);
        
        print_span(sout, *s, pred->get_position(), pred->get_position());
        
        // print arguments for this predicate     
        int narg = 1;
        for (predicate::const_iterator arg=pred->begin(); arg!=pred->end(); arg++) {
          sout << L"<role id=\"r" << pid << L"-" << narg << L"\" semrole=\"" << arg->get_role() << L"\">" << endl;;
          
          // print <externalReferences>, if any
          print_external_refs(sout,(*s)[arg->get_position()]);
          
          dep_tree::const_iterator arghead = s->get_dep_tree().get_node_by_pos(arg->get_position());
          print_span(sout, *s, dep_tree::get_first_word(arghead), dep_tree::get_last_word(arghead));
          
          sout << L"</role>" << endl;
          narg++;
        }
        sout<<L"</predicate>" << endl;
      }
    }
  }
  sout << L"</srl>" << endl;
}


//---------------------------------------------
// print coreferences in NAF
//---------------------------------------------

void output_naf::PrintCoreferencesLayer(wostream &sout, const document &doc) const {

  if (doc.get_num_groups()==0) return;

  sout << L"<coreferences>" << endl;
  for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {

    sout << L"<coref id=\"co" << *g << L"\">" << endl;

    list<int> mentions = doc.get_coref_id_mentions(*g);
    for (list<int>::iterator m=mentions.begin(); m!=mentions.end(); m++) {
      const mention & ment = doc.get_mention(*m);
      print_span(sout, *(ment.get_sentence()), ment.get_pos_begin(), ment.get_pos_end());
    }

    sout << L"</coref>" << endl;
  }  
  sout << L"</coreferences>" << endl;
}


//---------------------------------------------
// print obtained analysis in NAF
//---------------------------------------------

void output_naf::PrintResults (wostream &sout, const list<sentence> &ls) const {

  document doc;
  doc.push_back(paragraph(ls));
  PrintResults(sout,doc);
}

//---------------------------------------------
// activate/deactivate printing of a NAF layer
//--------------------------------------------

void output_naf::ActivateLayer(const std::wstring &ly, bool b) {
  if (b) layers.insert(ly);
  else layers.erase(ly);
}


//---------------------------------------------
// print XML file header and NAF headers
//--------------------------------------------

void output_naf::PrintHeader(wostream &sout) const { 
  sout<<L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;

  sout<<L"<NAF xml:lang=\""<<Lang<<L"\" version=\"v3\">"<<endl;
  sout<<L"<nafHeader>"<<endl;
  sout<<L"<fileDesc />"<<endl;
  sout<<L"<public />"<<endl;

  for (set<wstring>::const_iterator ly=layers.begin(); ly!=layers.end(); ly++) {
    sout<<L"<linguisticProcessors layer=\""+*ly+L"\">"<<endl;
    sout<<L"<lp name=\""+util::string2wstring(FREELING_VERSION)+L"\" />"<<endl;
    sout<<L"</linguisticProcessors>"<<endl;
  }

  sout<<L"</nafHeader>"<<endl;
}

//---------------------------------------------
// print NAF footer
//--------------------------------------------

void output_naf::PrintFooter(wostream &sout) const { 
  sout<<L"</NAF>"<<endl;
}


//---------------------------------------------
// print dependency tree
//---------------------------------------------

void output_naf::PrintDepTreeNAF (wostream &sout, dep_tree::const_iterator n, const wstring &sid) const {

  // if node is root, it has neither head nor function, skip it
  if (not n.is_root()) {

    wstring head = (n.get_parent()->get_label()==L"VIRTUAL_ROOT" ? L"VIRTUAL_ROOT" 
                                                                 : get_term_id(sid,n.get_parent()->get_word()));

    // print dependency for current node    
    sout << L"<dep from=\"" << get_term_id(sid,n->get_word())
         << "\" to=\"" << head
         << "\" rfunc=\"" << n->get_label() << "\" />" << endl;
  }

  // if node has children, dive in
  if (n.num_children () > 0) {
    for (dep_tree::const_sibling_iterator d = n.sibling_begin (); d != n.sibling_end (); ++d)
      PrintDepTreeNAF (sout, d, sid);
  }
}

//----------------------------------------------------------
// print global information of the document 
//----------------------------------------------------------

void output_naf::PrintResults(wostream &sout, const document &doc) const {

  PrintHeader(sout);

  if (layers.find(L"text")!=layers.end())  PrintTextLayer(sout,doc);    
  if (layers.find(L"terms")!=layers.end()) PrintTermsLayer(sout,doc);
  if (layers.find(L"entities")!=layers.end()) PrintEntitiesLayer(sout,doc);
  if (layers.find(L"chunks")!=layers.end()) PrintChunksLayer(sout,doc);
  if (layers.find(L"constituency")!=layers.end()) PrintConstituencyLayer(sout,doc);
  if (layers.find(L"deps")!=layers.end()) PrintDepsLayer(sout,doc);
  if (layers.find(L"srl")!=layers.end()) PrintSRLLayer(sout,doc);
  if (layers.find(L"coreferences")!=layers.end()) PrintCoreferencesLayer(sout,doc);

  PrintFooter(sout);
  
}
