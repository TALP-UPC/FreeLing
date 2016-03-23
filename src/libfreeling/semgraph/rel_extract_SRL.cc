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
#include "freeling/morfo/rel_extract_SRL.h"

#include <list>
#include <map>

using namespace std;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"SEMGRAPH_REL_SRL"
#define MOD_TRACECODE SEMGRAPH_TRACE


namespace freeling {

  //////////////////////////////////////////////////////
  /////                                            /////
  /////        Class rel_extract_SRL               /////
  /////                                            /////
  /////  This class derives a relation extractor   /////
  /////  for semantic graphs, based on dep parsing /////
  /////  and SRL annotations.                      /////
  /////  The input document must be dependency     /////
  /////  parsed and SRL-ed                         /////
  /////                                            /////
  //////////////////////////////////////////////////////

  /////////////////////////////////////////////////////
  /// Constructor 

  rel_extract_SRL::rel_extract_SRL(const wstring &erFile) : rel_extract(erFile), ContentArgument(RE_CONTARG) {

    wstring path=erFile.substr(0,erFile.find_last_of(L"/\\")+1);

    enum sections {SG_TYPE,CONTENTARG,NH_RELATIVES,AUXVERBS};
    config_file cfg(true);
    cfg.add_section(L"ExtractorType",SG_TYPE);
    cfg.add_section(L"ContentArgument",CONTENTARG);
    cfg.add_section(L"NonHeadRelatives",NH_RELATIVES);
    cfg.add_section(L"AuxiliaryVerbs",AUXVERBS);
  
    if (not cfg.open(erFile))
      ERROR_CRASH(L"Error opening file "+erFile);
  
    wstring line; 
    while (cfg.get_content_line(line)) {

      // process each content line according to the section where it is found
      switch (cfg.get_section()) {
      
      case SG_TYPE: {
        if (line!=L"SRL")
          ERROR_CRASH(L"Invalid configuration file '"+erFile+L"' for SRL semgraph extractor.");
        break;
      }
      
      case CONTENTARG: {
        ContentArgument = freeling::regexp(line);
        break;
      }
      
      case AUXVERBS: {
        wistringstream sin;  sin.str(line);
        wstring verb,child;
        sin>>verb>>child;
        auxiliary.insert(make_pair(verb,child));
        break;
      }
      
      case NH_RELATIVES: {
        wistringstream sin;  sin.str(line);
        wstring rel,tag;
        sin>>rel>>tag;
        nh_relatives.insert(make_pair(rel,tag));
        break;
      }
      }
    }
    cfg.close(); 

    if (not A0_label.empty())
      ERROR_CRASH(L"Unexpected A0_label specified in 'ArgumentRoles' section in semgraph configuration file'"+erFile+L"'");
    if (not A1_label.empty())
      ERROR_CRASH(L"Unexpected A1_label specified in 'ArgumentRoles' section in semgraph configuration file'"+erFile+L"'");
    
    TRACE(2,L"Module successfully created");
  }

  /////////////////////////////////////////////////////
  /// Destructor 

  rel_extract_SRL::~rel_extract_SRL() { delete ArgMap; }


  ///////////////////////////////////////////////////////////////
  /// Returns true if a verb is considered auxiliary
  ///////////////////////////////////////////////////////////////

  bool rel_extract_SRL::is_aux(const freeling::sentence &s, int pos, int &childpos) const {

    childpos = -1;

    // look up lemma in auxiliary verbs list
    map<wstring,wstring>::const_iterator aux = auxiliary.find(s[pos].get_lemma());
  
    // if not found, it is not an auxiliary verb
    if (aux==auxiliary.end()) return false;

    // if found with no child constraints, it is auxiliary
    if (aux->second==L"*") return true;

    // if found with child constraints, check it has a child with required PoS
    freeling::dep_tree::const_iterator dn = s.get_dep_tree().get_node_by_pos(pos);
    freeling::dep_tree::const_sibling_iterator ch;
    for (ch=dn.sibling_begin(); ch!=dn.sibling_end(); ch++) {
      if (ch->get_word().get_tag().find(aux->second)==0) {
        childpos = ch->get_word().get_position();
        return true;
      }
    }

    // no matching child, return false.
    return false;
  }
  

  //////////////////////////////////////////////////////////////////
  /// get content node in the subtree headed by word in given position
  /// (used to skip preposition in prep-headed arguments, and get the 
  /// content word)

  int rel_extract_SRL::get_argument_head(const freeling::sentence &s, int apos, int ppos) const {

    int p,child;

    // unless changed below, the given argument is the head
    p = apos;

    if (not ContentArgument.search(s[apos].get_tag())) {
      TRACE(4,L"    Non content argument. Check children");
      // s[apos] is not the head (it is not ContentWord, 
      // (e.g.  "go A1=[to Paris]"  "say A1=[that he comes]" 
      //  The content is under it.

      // There should be a unique child that should be the content word
      freeling::dep_tree::const_iterator dn = s.get_dep_tree().get_node_by_pos(apos);

      // but the tagger may mess it up, so we look for the first content child.
      freeling::dep_tree::const_sibling_iterator ch = dn.sibling_begin();
      while (ch != dn.sibling_end() and not ContentArgument.search(ch->get_word().get_tag()) )
        ch++;

      if (ch == dn.sibling_end()) {
        WARNING(L"    Non content argument '"+s[apos].get_form()+L"' with no content children ignored.");
        return -1;
      }

      const freeling::word &w = ch->get_word();  
      if (is_aux(s,w.get_position(),child)) {
        // If it is an auxiliary verb.   "say A1=[that he *is* coming]"
        // the content is the verb under it
        p = child;
      }
      else // Normal child, it is the head
        p = w.get_position();
    }

    else if (is_aux(s,apos,child)) {
      TRACE(4,L"    Auxiliary verb. Get child.");
      // It is an auxiliary verb.   "likes A1=[*having* arrived]" 
      // the content is the verb under it
      p = child;
    }
  
    TRACE(5,L"    Check for non-head relative.");
    map<wstring,wstring>::const_iterator r=nh_relatives.find(s[p].get_lemma());
    if (r!=nh_relatives.end() and s[p].get_tag().find(r->second)==0) {
      TRACE(4,L"    Non-head relative. Get predicate parent.");
      // it is a relative and the parser puts them under the verb of the 
      // relative clause. The PR antecedent is the parent of the verb (if it
      // is a contentArgument).  "The man [that talked fast] went away"
    
      // parent of the predicate (if any)
      freeling::dep_tree::const_iterator par = s.get_dep_tree().get_node_by_pos(ppos);
      if (not par.is_root()) {
        par=par.get_parent();
        // if it is ContentArgument, that is the head
        if (ContentArgument.search(par->get_word().get_tag())) 
          p = par->get_word().get_position();
      }

      // in any other case, leave it as it is
    }

    // the argument is the predicate itself (probably via an auxiliary... SRL error)
    if (p == ppos) {
      WARNING(L"    Self-argument '"+s[ppos].get_form()+L"' ignored.");
      return -1;
    }

    // no special case, the original pos is the head
    return p;
  }


  /////////////////////////////////////////////////////
  /// extract entities and relations from given sentences, 
  /// using requested rule package.

  void rel_extract_SRL::extract_relations(freeling::document &doc) const {
    
    semgraph::semantic_graph &sg = doc.get_semantic_graph();
    TRACE(2,L"Extracting relations from SRL predicate-argument structure");
  
    // for each sentence in document
    int ns=1;
    for (freeling::document::const_iterator ls=doc.begin(); ls!=doc.end(); ls++) {
      for(list<freeling::sentence>::const_iterator s = ls->begin(); s != ls->end(); s++, ns++) {
        // remember which words (positions) are which predicates
        map<int,wstring> pospred;
        // for each predicate, create a frame and fill arguments
        for (freeling::sentence::predicates::const_iterator p=s->get_predicates().begin(); 
             p!=s->get_predicates().end(); p++) {
          // skip nominal predicates without arguments (or with just one self-argument)
          if (not ((*s)[p->get_position()].get_tag().substr(0,1)==L"N" and 
                   (p->empty() or (p->size()==1 and p->has_argument(p->get_position()))))) {                        
            // create frame for current predicate
            int ppos = p->get_position();
            TRACE(3,L"Creating frame for predicate "+freeling::util::int2wstring(ppos)+L" ("+(*s)[ppos].get_form()+L")");
            semgraph::SG_frame fr(p->get_sense(),
                                  ((*s)[ppos].get_senses().empty() ? L"" : (*s)[ppos].get_senses().begin()->first),
                                  util::int2wstring(ppos+1),
                                  s->get_sentence_id());
            wstring fid = sg.add_frame(fr);
            pospred.insert(make_pair(ppos,fid));
          }
          else 
            TRACE(4,L"Argumentless nominal predicate '"+(*s)[p->get_position()].get_form()+L"' removed. Will reappear as node if needed.");
          
        }

        // for each predicate, fill arguments
        for (freeling::sentence::predicates::const_iterator p=s->get_predicates().begin(); 
             p!=s->get_predicates().end(); p++) {

          // skip argless nominal predicates
          map<int,wstring>::const_iterator pp = pospred.find(p->get_position());
          if (pp==pospred.end()) continue;

          wstring fid = pp->second;          

          // add arguments
          TRACE(3,L"Adding arguments to predicate "+fid+L" ("+(*s)[p->get_position()].get_form()+L")");        
          for (freeling::predicate::const_iterator a=p->begin(); a!=p->end(); a++) {
            int apos = a->get_position();
            TRACE(3,L"  Check argument "+freeling::util::int2wstring(apos)+L" ("+(*s)[apos].get_form()+L")");
            if (a->get_role()==L"_") {
              TRACE(3,L"  ... No role found, skipping");
              continue;               
            }
          
            // Locate content word in the argument (i.e. skip heading prepositions or conjunctions and
            // get noun or verb bearing the argument content)
            int argh = get_argument_head(*s, apos, p->get_position());
          
            if (argh < 0) {
              // no content word found for this argument (should not happen in a correct tree and SRL)
              TRACE(3,L"  No valid argument head found for "+(*s)[apos].get_form());
              continue; 
            }
          
            // Self arguments (due to SRL errors) are not valid.
            if (argh == p->get_position()) continue;
          
            wstring wsarg;
            map<int,wstring>::iterator k = pospred.find(argh); 
            if (k!=pospred.end()) {  // argument is another predicate
              TRACE(3,L"  Found valid predicate argument at position "+freeling::util::int2wstring(argh));
              wsarg = k->second;
            }            
            else {
              // argument is a regular node. Check whether it is an existing entity node, 
              // we need to create a new one, or it is just a modifier.            
              TRACE(3,L"  Found valid regular argument at position "+freeling::util::int2wstring(argh));
              wsarg = find_entity_node(sg, *s, (*s)[argh]); 
            }
          
            TRACE(3,L"  Argument is "+wsarg);
            wstring wsrole = argument_to_role(a->get_role(),sg.get_frame(fid).get_sense());
            sg.add_argument_to_frame(fid,wsrole,wsarg);         
          }     
        }
      }
    }    
  
    TRACE(2,L"Finished extracting relations");
    return; 
  }

}
