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
#include "freeling/morfo/rel_extract.h"

#include <list>
#include <map>

using namespace std;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"SEMGRAPH_REL"
#define MOD_TRACECODE SEMGRAPH_TRACE

namespace freeling {

  ////////////////////////////////////////////////////
  /////                                          /////
  /////        Class rel_extract                 /////
  /////                                          /////
  /////  Abstract class to derive relation       /////
  /////  extractors for semantic graphs.         /////
  /////  Input document must have NERC info      /////
  /////  and entities extracted.                 /////
  /////  Derived classes must provide frame      /////
  /////  extraction and create relations between /////
  /////  entities                                /////
  /////                                          /////
  ////////////////////////////////////////////////////
  
  
  /////////////////////////////////////////////////////
  /// Constructor 
  
  rel_extract::rel_extract(const wstring &erFile) {
    
    ArgMap = NULL;
    wstring path=erFile.substr(0,erFile.find_last_of(L"/\\")+1);
    
    enum sections {ARG_ROLES,OPTIONS};
    config_file cfg(true);
    cfg.add_section(L"ArgumentRoles",ARG_ROLES);
    cfg.add_section(L"GeneralOptions",OPTIONS);

    if (not cfg.open(erFile))
      ERROR_CRASH(L"Error opening file "+erFile);

    MergeSingletons = false; // default;
    
    wstring line; 
    while (cfg.get_content_line(line)) {
      // process each content line according to the section where it is found
      switch (cfg.get_section()) {

      case OPTIONS: {
        wistringstream sin;  sin.str(line);
        wstring key,val;
        sin>>key>>val;
        if (key == L"MergeSingletons") {
          val = util::lowercase(val);
          MergeSingletons = (val==L"true" or val==L"1" or val==L"on" or val==L"yes" or val==L"y");
        }
        break;
      }
        
      case ARG_ROLES: {
        wistringstream sin;  sin.str(line);
        wstring key,val;
        sin>>key>>val;
        
        // remember map type
        if (key==L"MapType") {
          if (val==L"direct") mapType=DIRECT;
          else if (val==L"synset-arg") mapType=SYNSARG;
          else ERROR_CRASH(L"Unknown MapType '"+val+L"' in extractor config file "+erFile);
        }
        
        // load map file
        else if (key==L"MapFile") 
          ArgMap = new database(util::absolute(util::expand_filename(val),path));
        
        // read argument labels for SPR extractors
        else if (key==L"A0_label") A0_label = val;
        else if (key==L"A1_label") A1_label = val;
        
        else 
          ERROR_CRASH(L"Unknown key '"+key+L"' in ArgumentRoles section of extractor config file "+erFile);        
        break;
      }
        
      default: break;      
      }
    }
    cfg.close(); 

    if (ArgMap==NULL) 
      ERROR_CRASH(L"No 'MapFile' defined in 'ArgumentRoles' section in semgraph configuration file '"+erFile+L"'");
    
    TRACE(2,L"Module successfully created");
  }
  
  /////////////////////////////////////////////////////
  /// Destructor 
  
  rel_extract::~rel_extract() { delete ArgMap; }
  
    
  ////////////////////////////////////////////////////////////////////
  /// Convert argument (A0, A1, etc) to appropriate role (Agent, 
  /// Patient, Theme, etc) for given verb sense.
  
  wstring rel_extract::argument_to_role(const wstring &wsr, const wstring &sense) const {
    
    wstring wsrole = wsr;
    
    // convert argument to semantic role, using appropriate map
    if (mapType==SYNSARG) {
      // retrieve list of roles for the predicate synset
      list<pair<wstring,wstring> > roles;
      roles = util::wstring2pairlist<wstring,wstring>(ArgMap->access_database(sense),L":",L" ");
      // locate semantic role for the argument
      bool found=false;
      list<pair<wstring,wstring> >::iterator r;
      for (r=roles.begin(); r!=roles.end() and not found; r++)
        found = (r->first==wsrole);
      // add the role to the argument, if any
      if (found) {
        r--;
        wsrole += L":"+r->second;
      }
    }
    else if (mapType==DIRECT) {
      // role must be of the format argX-YYY. 
      // see if there is a direct translation (e.g. argM-tmp --> AM-TMP)
      wstring rol=ArgMap->access_database(wsrole);
      
      if (not rol.empty()) 
        wsrole = rol;
      else {
        // no direct translation. Handle components (argX, YYY) separately
        size_t p=wsrole.find(L"-");
        wstring arg=ArgMap->access_database(wsrole.substr(0,p));
        rol=ArgMap->access_database(wsrole.substr(p+1));
        
        if (arg.empty() or rol.empty())
          WARNING(L"No mapping found for semantic role "+wsrole+L". Please check ArgMap file.");
        wsrole = arg+L":"+rol;
      }
    }  
    
    return wsrole;
  }


  ////////////////////////////////////////////////////////////////////
  /// find whether a word is an already existing entity node, 
  /// or there is a matching newly create node.  
  /// Create new node otherwise.
  
  wstring rel_extract::find_entity_node(semgraph::semantic_graph &sg, const sentence &sent, const word &wd) const {

    wstring wid = sent.get_sentence_id() + L"." + util::int2wstring(wd.get_position()+1);

    int best = sent.get_best_seq();
    
    TRACE(4,L"    Searching node for mention "+wid);
    wstring eid = sg.get_entity_id_by_mention(sent.get_sentence_id(),util::int2wstring(wd.get_position()+1));
    if (not eid.empty())
      { // is a known mention (a NE or a node already found), just retrieve node id
        TRACE(4,L"    Found node "+eid+L" for mention "+wid);
        return eid;   
      }
    
    // is not a known mention (i.e. it is a common noun singleton)  
    TRACE(4,L"    Non-entity argument "+wd.get_form());

    // create mention for this word and see to which node it should be added
    dep_tree dt = sent.get_dep_tree(best);
    dep_tree::const_iterator dn = dt.get_node_by_pos(wd.get_position());
    size_t w1 = dep_tree::get_first_word(dn);
    size_t w2 = dep_tree::get_last_word(dn);
    list<wstring> lw;
    for (size_t k=w1; k<=w2; k++) lw.push_back(sent[k].get_form());    
    semgraph::SG_mention m(util::int2wstring(wd.get_position()+1), sent.get_sentence_id(), lw);

    wstring sens = (wd.get_senses(best).empty() ? L"" : wd.get_senses(best).begin()->first);
    wstring lemma = wd.get_lemma(best);
    bool merged = false;
    if (MergeSingletons) {
      // Check if an entity with the same lemma-sense exists    
      wstring wsarg;
      TRACE(4,L"    ... check for entity with lemma-sense "+lemma+L"#"+sens);
      eid = sg.get_entity_id_by_lemma(lemma, sens);
      if (not eid.empty()) {
        TRACE(4,L"    ... merging to matching node with id ="+eid);
        merged = true;
      }
      else {
        TRACE(4,L"    ... no match found, not merged.");
      }
    }

    if (not merged) {	             
      // no node for this lemma, create a new one
      semgraph::SG_entity ent (lemma, L"", semgraph::WORD, sens);
      eid = sg.add_entity(ent);
      TRACE(4,L"    Created new node "+eid);
    }
    
    // add mention to node
    sg.add_mention_to_entity(eid,m);
    return eid;
  } 
  
} // namespace

