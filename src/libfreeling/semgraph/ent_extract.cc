//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
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
///             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


#include <sstream>
#include <fstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/ent_extract.h"

#include <list>
#include <map>

using namespace std;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"SEMGRAPH_ENTS"
#define MOD_TRACECODE SEMGRAPH_TRACE

namespace freeling {


  ////////////////////////////////////////////////////
  /////                                          /////
  /////        Class ent_extract                 /////
  /////                                          /////
  /////  Entity extractor for semantic graph     /////
  /////  construction                            /////
  /////  It provides entity extraction           /////
  /////  functionalities, using coreference      ///// 
  /////  if available.                           /////
  /////  Input document must have NERC info.     /////
  /////                                          /////
  ////////////////////////////////////////////////////
    
  /////////////////////////////////////////////////////
  /// Constructor 
  
  ent_extract::ent_extract(const wstring &erFile) {
    
    wstring path=erFile.substr(0,erFile.find_last_of(L"/\\")+1);
    
    enum sections {NE_TAG,NE_CLASSES};
    config_file cfg(true);
    cfg.add_section(L"NE_Tag",NE_TAG);
    cfg.add_section(L"NE_Classes",NE_CLASSES);

    if (not cfg.open(erFile))
      ERROR_CRASH(L"Error opening file "+erFile);
    
    wstring line; 
    while (cfg.get_content_line(line)) {
      // process each content line according to the section where it is found
      switch (cfg.get_section()) {
                
      case NE_TAG: {
        // read tag that NEs will have
        NEtag = line;
        break;
      }
        
      case NE_CLASSES: {
        // read conversion list NEtag -> NE class (e.g. NP00G00  -> location)
        wistringstream sin;  
        sin.str(line);
        wstring tag,clas;
        sin>>tag>>clas;
        NEclasses.insert(make_pair(tag,clas));
        break;
      }
       
      }
    }
    cfg.close(); 
    
    TRACE(2,L"Module successfully created");
  }
  
  /////////////////////////////////////////////////////
  /// Destructor 
  
  ent_extract::~ent_extract() {}
  

  /////////////////////////////////////////////////////
  /// Check if a word is a NE
  
  bool ent_extract::is_NE(const word &w) const { return (w.get_tag().find(NEtag)==0); }

  /////////////////////////////////////////////////////
  /// get class of a NE
  
  wstring ent_extract::get_NE_class(const word &w) const { 
    map<wstring,wstring>::const_iterator p = NEclasses.find(w.get_tag());
    return (p==NEclasses.end() ? L"unknown" : p->second); 
  }
  
  /////////////////////////////////////////////////////
  /// Check if two NEs are alias 
  
  bool ent_extract::is_alias(const wstring &e1, const wstring &e2) const {
    
    if (util::lowercase(e1)==util::lowercase(e2)) return true;
    
    // if one entity is subsumed in the other, its an alias (e.g. Obama/Barak_Obama,
    // United_States/United_States_of_America, etc)
    if (e2.find(e1)!=wstring::npos or e1.find(e2)!=wstring::npos) return true;
    
    // check if one is acronym of the other (e.g. USA/United_States_of_America, EU/European_Union, etc)
    list<wstring> ls1 = util::wstring2list(e1,L"_");
    list<wstring> ls2 = util::wstring2list(e2,L"_");
    
    if (ls1.size()>ls2.size()) ls1.swap(ls2);  // make sure ls1 is the shortest NE
    
    if (ls1.size()>1) return false; // if shortest is multiword, is not an acronym, forget about it
    
    bool acr=true;
    size_t i=0;
    list<wstring>::iterator w=ls2.begin();
    while (w!=ls2.end() and i<ls1.begin()->size() and acr) {
      wstring c1=ls1.begin()->substr(i,1);
      wstring c2=w->substr(0,1);
      
      if (not util::RE_all_caps.search(c2))  // not uppercase -> skip, do not count int the acronym (e.g. "of")
        w++; 
      else if (c1==c2) { // uppercase and match, keep on
        i++; 
        w++;
      }
      else  // no match, forget it
        acr = false;    
    }
    
    if (i<ls1.begin()->size()) // There are more initials in ls1, than matching words in ls2. Invalid acronym.
      acr=false;  
    
    return acr;
  }

  /////////////////////////////////////////////////////
  /// Check if some existing entity is an alias of lemma

  wstring ent_extract::search_alias(const semgraph::semantic_graph &sg, const wstring &lemma) const {
    for (vector<semgraph::SG_entity>::const_iterator p=sg.get_entities().begin();
         p!=sg.get_entities().end(); 
         p++) {
      if (is_alias(p->get_lemma(),lemma)) return p->get_id();
    }
    return L"";   
  }
  
  
  /////////////////////////////////////////////////////
  /// extract entities from given sentences, using coref information
  
  void ent_extract::extract_entities_coref(document &doc) const {

    TRACE(2,L"Extracting entities, coref info");
    semgraph::semantic_graph &sg = doc.get_semantic_graph();

    for (list<int>::const_iterator g=doc.get_groups().begin(); g!=doc.get_groups().end(); g++) {

      // add each mention in the group to the entity, remembering longest lemma
      list<semgraph::SG_mention> lmen;
      list<int> ments = doc.get_coref_id_mentions(*g);
      const word *mx = NULL;
      for (list<int>::const_iterator im=ments.begin(); im!=ments.end(); im++) {
        const mention &m = doc.get_mention(*im);
        const word &hw = m.get_head();

        // remember longest NE mention, (or just longest mention if none is NE)
        if (mx==NULL) mx = &hw;
        if (hw.get_form().length()>mx->get_form().length() and (is_NE(hw) or not is_NE(*mx)))
          mx = &hw;
        
        list<wstring> lw;
        for (int k=m.get_pos_begin(); k<=m.get_pos_end(); k++) 
          lw.push_back((*m.get_sentence())[k].get_form());
        
        semgraph::SG_mention sgm (util::int2wstring(hw.get_position()+1),
                                  util::int2wstring(m.get_n_sentence()+1),
                                  lw);
        lmen.push_back(sgm);
      }
        
      // create a SG_entity for this coref group
      semgraph::entityType ntype = (is_NE(*mx) ? semgraph::ENTITY : semgraph::WORD);
      semgraph::SG_entity ent(mx->get_lemma(),
                              get_NE_class(*mx),
                              ntype, 
                              (mx->get_senses().empty() ? L"" : mx->get_senses().begin()->first));
      wstring eid = sg.add_entity(ent);

      for (list<semgraph::SG_mention>::const_iterator m=lmen.begin(); m!=lmen.end(); m++ )
        sg.add_mention_to_entity(eid,*m);
    }

    TRACE(2,L"Finished extracting coref entities");
  }
  
  
  /////////////////////////////////////////////////////
  /// extract entities from given sentences 
  
  void ent_extract::extract_entities_nocoref(document &doc) const {
        
    TRACE(2,L"Extracting entities, no coref info");
    semgraph::semantic_graph &sg = doc.get_semantic_graph();
    
    for (document::const_iterator ls=doc.begin(); ls!=doc.end(); ls++) {
      for(list<sentence>::const_iterator s = ls->begin(); s != ls->end(); ++s) {        
        TRACE(3,L"   Extracting entities from sentence ");
        for (sentence::const_iterator w = s->begin(); w != s->end(); ++w) {
          // if word is a NE, add to list of entities
          if (is_NE(*w)) {
            wstring ne_lemma = w->get_lemma();
            TRACE(3,L"Detected mention: form="+w->get_form()+L"  lemma="+ne_lemma);

            // see whether the entity already appeared
            wstring eid = sg.get_entity_id_by_lemma(ne_lemma);

            // if no exact match, check for alias
            if (eid.empty()) eid = search_alias(sg, ne_lemma);            

            // neither match nor alias found. Create new entity, assign new id, and insert into index
            if (eid.empty()) {
              semgraph::SG_entity newent(ne_lemma,
                                         get_NE_class(w->get_tag()),
                                         semgraph::ENTITY);
              eid = sg.add_entity(newent);
            }
            
            // create new mention for this entity (either existing or just created)
            semgraph::SG_mention newment(util::int2wstring(w->get_position()+1),
                                         s->get_sentence_id(),
                                         util::wstring2list(w->get_form(),L"_") );
            sg.add_mention_to_entity(eid,newment);            
          }
        }
      }
    }
    
    /// Review built entities and pick longest mention as lemma
    for (vector<semgraph::SG_entity>::iterator e=sg.get_entities().begin(); e!=sg.get_entities().end(); e++) {  
      wstring mxl=L"";
      for (vector<semgraph::SG_mention>::const_iterator m=e->get_mentions().begin(); m!=e->get_mentions().end(); m++) {
        wstring mform = util::list2wstring(m->get_words(),L"_");
        if (mxl.length() < mform.length()) mxl = mform;
      }
      e->set_lemma(util::lowercase(mxl));
    }
    
    TRACE(2,L"Finished extracting no-coref entities");
  }


  
  /////////////////////////////////////////////////////
  /// extract entities and relations from given sentences, 
  /// using requested rule package.
  
  void ent_extract::extract_entities(document &doc) const {
        
    if (doc.get_num_groups()==0) 
      // no coref available, use naive approach
      extract_entities_nocoref(doc);

    else 
      // coref available, use detected entites
      extract_entities_coref(doc);

    TRACE(2, L"Extracted " + util::int2wstring(doc.get_semantic_graph().get_entities().size()));
  }
  
  
} // namespace

