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


#include "freeling/morfo/util.h"
#include "freeling/morfo/semgraph.h"

using namespace std;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"SEMGRAPH"
#define MOD_TRACECODE SEMGRAPH_TRACE

namespace freeling {

  namespace semgraph {

    ///////////////////////////////////////////////
    /// Auxiliary classes to contain a semantic graph
    ///////////////////////////////////////////////

    /// SG_mention 
    SG_mention::SG_mention() {}
    SG_mention::SG_mention(const wstring &mid, 
                           const wstring &sid,
                           const list<wstring> &wds) : id(mid), sentenceId(sid), words(wds) {}
    SG_mention::~SG_mention() {}
    wstring SG_mention::get_id() const {return id;}
    wstring SG_mention::get_sentence_id() const {return sentenceId;}
    const list<wstring> & SG_mention::get_words() const {return words;}
   
    /// SG_entity
    SG_entity::SG_entity() {}
    SG_entity::SG_entity(const wstring &elemma, 
                         const wstring &eclass, 
                         entityType etype, 
                         const wstring &esense) : lemma(elemma),
                                                  semclass(eclass), type(etype),
                                                  sense(esense) {}
    SG_entity::~SG_entity() {}
    void SG_entity::add_mention(const SG_mention &m) { mentions.push_back(m); }
    void SG_entity::set_id(const wstring &eid) {id=eid;}
    void SG_entity::set_lemma(const wstring &lem) {lemma=lem;}

    void SG_entity::set_synonyms(const list<wstring> &syn) { synonyms = syn; }
    void SG_entity::add_URI(const wstring &kb, const wstring &uri) { uris.push_back(make_pair(kb,uri)); }

    wstring SG_entity::get_id() const {return id;}
    wstring SG_entity::get_lemma() const {return lemma;}
    wstring SG_entity::get_semclass() const {return semclass;}
    entityType SG_entity::get_type() const {return type;}
    wstring SG_entity::get_sense() const {return sense;}
    const vector<SG_mention> &SG_entity::get_mentions() const {return mentions;}
    const list<wstring> &SG_entity::get_synonyms() const { return synonyms; }
    const list<pair<wstring,wstring> > &SG_entity::get_URIs() const { return uris; }
    
    /// SG_argument
    SG_argument::SG_argument() {}
    SG_argument::SG_argument(const wstring &r, const wstring &e) : role(r), entity(e) {}
    SG_argument::~SG_argument() {}
    wstring SG_argument::get_role() const {return role;}
    wstring SG_argument::get_entity() const {return entity;}
    
    // SG_frame
    // FAlta ampliar constructora amb la resta d'informacio
    SG_frame::SG_frame() {}
    SG_frame::SG_frame(const std::wstring &lem,
                       const std::wstring &sns, 
                       const std::wstring &tk, 
                       const std::wstring &sid) : lemma(lem), sense(sns), tokenId(tk), sentenceId(sid) {}
    SG_frame::~SG_frame() {}
    void SG_frame::add_argument(const wstring &role, const wstring &ent) { arguments.push_back(SG_argument(role,ent)); }
    void SG_frame::set_id(const std::wstring &fid) {id=fid;}
    void SG_frame::set_synonyms(const list<wstring> &syn) { synonyms = syn; }
    void SG_frame::add_URI(const wstring &kb, const wstring &uri) { uris.push_back(make_pair(kb,uri)); }

    wstring SG_frame::get_id() const {return id;}
    wstring SG_frame::get_lemma() const {return lemma;}
    wstring SG_frame::get_sense() const {return sense;}
    wstring SG_frame::get_token_id() const {return tokenId;}
    wstring SG_frame::get_sentence_id() const {return sentenceId;}
    const vector<SG_argument>& SG_frame::get_arguments() const {return arguments;}
    const list<wstring> &SG_frame::get_synonyms() const { return synonyms; }
    const list<pair<wstring,wstring> > &SG_frame::get_URIs() const { return uris; }

    /// semantic_graph
    semantic_graph::semantic_graph() : last_id(0) {}
    semantic_graph::~semantic_graph() {}
    
    wstring semantic_graph::add_entity(SG_entity &ent) {
      ent.set_id((ent.get_type()==ENTITY ? L"E" : L"W") + util::int2wstring(++last_id));
      entities.push_back(ent);
      entity_idx.insert(make_pair(ent.get_id(),entities.size()-1));
      lemma_idx.insert(make_pair(ent.get_lemma()+L"#"+ent.get_sense(),ent.get_id()));      
      return ent.get_id();
    }

    wstring semantic_graph::add_frame(SG_frame &fr) {
      fr.set_id(L"F" + util::int2wstring(++last_id));
      frames.push_back(fr);
      frame_idx.insert(make_pair(fr.get_id(),frames.size()-1));
      return fr.get_id();
    }

    SG_frame & semantic_graph::get_frame(const wstring &fid) {
      map<wstring,int>::iterator f=frame_idx.find(fid);
      if (f==frame_idx.end())
        ERROR_CRASH(L"Frame "+fid+L" does not exist in semantic graph.");
      return frames[f->second];
    }

    const SG_frame & semantic_graph::get_frame(const wstring &fid) const {
      map<wstring,int>::const_iterator f=frame_idx.find(fid);
      if (f==frame_idx.end())
        ERROR_CRASH(L"Frame "+fid+L" does not exist in semantic graph.");
      return frames[f->second];
    }

    wstring semantic_graph::get_entity_id_by_lemma(const wstring &lemma,const wstring &sens) const {
      map<wstring,wstring>::const_iterator p=lemma_idx.find(lemma+L"#"+sens);
      if (p==lemma_idx.end()) return L"";
      return p->second;
    }
    
    wstring semantic_graph::get_entity_id_by_mention(const wstring &sid, const wstring &wid) const {
      map<wstring,wstring>::const_iterator p=mention_idx.find(sid+L"."+wid);
      if (p==mention_idx.end()) return L"";
      return p->second;
    }
    
    SG_entity & semantic_graph::get_entity(const wstring &eid) {
      map<wstring,int>::iterator e=entity_idx.find(eid);
      if (e==entity_idx.end())
        ERROR_CRASH(L"Entity "+eid+L" does not exist in semantic graph.");
      return entities[e->second];
    }

    const SG_entity & semantic_graph::get_entity(const wstring &eid) const {
      map<wstring,int>::const_iterator e=entity_idx.find(eid);
      if (e==entity_idx.end())
        ERROR_CRASH(L"Entity "+eid+L" does not exist in semantic graph.");
      return entities[e->second];
    }

    void semantic_graph::add_mention_to_entity(const wstring &eid, const SG_mention &ment) {
      map<wstring,int>::const_iterator e=entity_idx.find(eid);
      if (e==entity_idx.end())
        ERROR_CRASH(L"Entity "+eid+L" does not exist in semantic graph.");
      
      entities[e->second].add_mention(ment);
      mention_idx.insert(make_pair(ment.get_sentence_id()+L"."+ment.get_id(),eid));
      lemma_idx.insert(make_pair(util::list2wstring(ment.get_words(),L"_")+L"#",eid));
    }

    void semantic_graph::add_argument_to_frame(const wstring &fid, const wstring &role, const wstring &eid) {
      map<wstring,int>::const_iterator e=frame_idx.find(fid);
      if (e==frame_idx.end())
        ERROR_CRASH(L"Frame "+fid+L" does not exist in semantic graph.");

      frames[frame_idx[fid]].add_argument(role,eid);
      has_args.insert(fid);
      is_arg.insert(eid);
    }

    const vector<SG_entity> & semantic_graph::get_entities() const { return entities; }
    vector<SG_entity> & semantic_graph::get_entities() { return entities; }

    bool semantic_graph::is_argument(const wstring &eid) const { return is_arg.find(eid)!=is_arg.end(); }
    bool semantic_graph::has_arguments(const wstring &fid) const { return has_args.find(fid)!=has_args.end(); }

    bool semantic_graph::empty() const { return entities.empty() and frames.empty(); }

    const vector<SG_frame> & semantic_graph::get_frames() const {return frames;}
    vector<SG_frame> & semantic_graph::get_frames() {return frames;}


  }
}

