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
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


#ifndef SEMGRAPH_H
#define SEMGRAPH_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>

namespace freeling {


  namespace semgraph {

    ///////////////////////////////////////////////
    /// Auxiliary classes to contain a semantic graph
    ///////////////////////////////////////////////

    class semantic_graph;

    //////////////////////////////////////
    /// Auxiliary class to store mentions to entities found in text
  
    class SG_mention {
    private:
      std::wstring id;
      std::wstring sentenceId;
      std::list<std::wstring> words;
    public:
      SG_mention();
      SG_mention(const std::wstring &mid, const std::wstring &sid, const std::list<std::wstring> &wds);
      ~SG_mention();
      std::wstring get_id() const;
      std::wstring get_sentence_id() const;
      const std::list<std::wstring> & get_words() const;
    };
    
    //////////////////////////////////////
    /// Auxiliary class to store entities found in text (maybe mentioned several times)

    typedef enum {ENTITY,WORD} entityType;
    
    class SG_entity {  
      friend class semantic_graph;
    private:
      /// node id
      std::wstring id;
      /// node lemma
      std::wstring lemma;
      /// entity semantic class (person,location,..)
      std::wstring semclass;
      /// node type (ENTITY|WORD)
      entityType type;
      /// WN sense (for "word" type nodes)
      std::wstring sense;
      /// Synset synonyms
      std::list<std::wstring> synonyms;
      /// URI descriptors, list of pairs <knowledgeBase,URI>
      std::list<std::pair<std::wstring,std::wstring> > uris;
      /// mentions
      std::vector<SG_mention> mentions;

    protected:
      void add_mention(const SG_mention &m);
      void set_id(const std::wstring &eid);

    public:
      SG_entity();
      SG_entity(const std::wstring &elemma,
                const std::wstring &eclass, 
                entityType type, 
                const std::wstring &sense=L"");
      ~SG_entity();

      void set_lemma(const std::wstring &lem);
      std::wstring get_id() const;
      std::wstring get_lemma() const;
      std::wstring get_semclass() const;
      entityType get_type() const;
      std::wstring get_sense() const;
      const std::vector<SG_mention> &get_mentions() const;
      const std::list<std::wstring> &get_synonyms() const;
      const std::list<std::pair<std::wstring,std::wstring> > &get_URIs() const;

      void set_synonyms(const std::list<std::wstring> &syn);
      void add_URI(const std::wstring &kb, const std::wstring &uri);
    };

    //////////////////////////////////////
    /// Auxiliary class to store frame arguments
  
    class SG_argument {
    private:
      std::wstring role;
      std::wstring entity;
    public:
      SG_argument();
      SG_argument(const std::wstring &r, const std::wstring &e);
      ~SG_argument();

      std::wstring get_role() const;
      std::wstring get_entity() const;
    };

    //////////////////////////////////////
    /// Auxiliary class to store frames (relations between n entities)
  
    class SG_frame {
      friend class semantic_graph;
    private:
      /// relation id
      std::wstring id;
      /// predicate
      std::wstring lemma;
      /// Wn sense for predicate
      std::wstring sense;
      /// Synset synonyms
      std::list<std::wstring> synonyms;
      /// URI descriptors, list of pairs <knowledgeBase,URI>
      std::list<std::pair<std::wstring,std::wstring> > uris;
      /// Token id
      std::wstring tokenId;
      /// sentence id
      std::wstring sentenceId;
      /// relation arguments
      std::vector<SG_argument> arguments;

    protected:
      // add argument to frame
      void add_argument(const std::wstring &role, const std::wstring &eid);
      void set_id(const std::wstring &fid);

    public:
      // constructor
      SG_frame();
      SG_frame(const std::wstring &lem, const std::wstring &sns, const std::wstring &tk, const std::wstring &sid);
      ~SG_frame();

      std::wstring get_id() const;
      std::wstring get_lemma() const;
      std::wstring get_sense() const;
      std::wstring get_token_id() const;
      std::wstring get_sentence_id() const;
      const std::vector<SG_argument> &get_arguments() const;
      const std::list<std::wstring> &get_synonyms() const;
      const std::list<std::pair<std::wstring,std::wstring> > &get_URIs() const;

      void set_synonyms(const std::list<std::wstring> &syn);
      void add_URI(const std::wstring &kb, const std::wstring &uri);
    };

    //////////////////////////////////////
    /// Auxiliary class to store a semantic graph
  
    class semantic_graph {
    private:
      // list of entity nodes in the graph. indexed by id
      std::vector<SG_entity> entities;
      // list of event nodes in the graph, with relations to their arguments
      std::vector<SG_frame> frames;
    
      // given a mention id, get entity it belongs to
      std::map<std::wstring,std::wstring> mention_idx;
      // given a lemma, get entity with that lemma
      std::map<std::wstring,std::wstring> lemma_idx;
      // given an entity id, get its position in vector
      std::map<std::wstring,int> entity_idx;
      // given a frame id, get its position in vector
      std::map<std::wstring,int> frame_idx;

      // set to store nodes that are argument to some predicate
      std::set<std::wstring> is_arg;
      // set to store predicate nodes that have some argument
      std::set<std::wstring> has_args;

      // auxiliary counter to number graph nodes
      int last_id;

    public:
      semantic_graph();
      ~semantic_graph();
      
      std::wstring add_entity(SG_entity &ent);
      std::wstring add_frame(SG_frame &fr);

      const SG_frame & get_frame(const std::wstring &fid) const;
      SG_frame & get_frame(const std::wstring &fid);

      std::wstring get_entity_id_by_mention(const std::wstring &sid,const std::wstring &wid) const;
      std::wstring get_entity_id_by_lemma(const std::wstring &lemma,const std::wstring &sens=L"") const;
      SG_entity & get_entity(const std::wstring &eid);
      const SG_entity & get_entity(const std::wstring &eid) const;

      const std::vector<SG_entity> & get_entities() const;
      std::vector<SG_entity> & get_entities();

      const std::vector<SG_frame> & get_frames() const;
      std::vector<SG_frame> & get_frames();

      void add_mention_to_entity(const std::wstring &eid, const SG_mention &m);
      void add_argument_to_frame(const std::wstring &fid, const std::wstring &role, const std::wstring &eid);

      bool is_argument(const std::wstring &eid) const;     
      bool has_arguments(const std::wstring &fid) const;

      bool empty() const;
    };

  }
}

#endif
