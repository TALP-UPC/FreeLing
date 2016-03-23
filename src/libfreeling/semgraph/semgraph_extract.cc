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
#include "freeling/morfo/semgraph_extract.h"
#include "freeling/morfo/rel_extract_SPR.h"
#include "freeling/morfo/rel_extract_SRL.h"

#include <list>
#include <map>

using namespace std;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"SEMGRAPH_EXTRACT"
#define MOD_TRACECODE SEMGRAPH_TRACE

namespace freeling {

  ////////////////////////////////////////////////////
  /////                                          /////
  /////        Class semgraph_extract            /////
  /////                                          /////
  /////  Semantic graph extractor. Uses          ///// 
  /////  deptree, SRL, coreference, to build     /////
  /////  a semantic graph with the information   /////
  /////  in the document                         /////
  /////                                          /////
  ////////////////////////////////////////////////////
  
  
  /////////////////////////////////////////////////////
  /// Constructor 
  
  semgraph_extract::semgraph_extract(const wstring &erFile) {

    relations_extr = NULL;
    entities_extr = NULL;
    semdb = NULL;

    wstring path=erFile.substr(0,erFile.find_last_of(L"/\\")+1);

    enum sections {SG_TYPE,SEMDB,KBS};
    config_file cfg(true);
    cfg.add_section(L"ExtractorType",SG_TYPE);
    cfg.add_section(L"SemanticDB",SEMDB);
    cfg.add_section(L"KnowledgeBases",KBS);
    
    if (not cfg.open(erFile))
      ERROR_CRASH(L"Error opening file "+erFile);

    wstring line; 
    while (cfg.get_content_line(line)) {
      // process each content line according to the section where it is found
      switch (cfg.get_section()) {

      case SG_TYPE: {
        entities_extr = new ent_extract(erFile);
        if (line==L"SPR") relations_extr = new rel_extract_SPR(erFile);
        else if (line==L"SRL") relations_extr = new rel_extract_SRL(erFile);
        else if (line==L"Entities") relations_extr = NULL;
        else 
          ERROR_CRASH(L"Unkown semantic graph extractor type '"+line+L"' in file '"+erFile+L"'");
        break;
      }

      case SEMDB: {
        wstring path = erFile.substr(0,erFile.find_last_of(L"/\\")+1);
        semdb = new semanticDB(util::absolute(line,path));
        break;
      }

      case KBS: {
        wistringstream sin; sin.str(line);
        wstring name, url;
        sin >> name >> url;
        KBs.push_back(make_pair(name,url));
        break;
      }

      default: break;
      }
    }
    cfg.close(); 

    TRACE(2,L"Module successfully created");
  }
  
  /////////////////////////////////////////////////////
  /// Destructor 
  
  semgraph_extract::~semgraph_extract() { 
    delete relations_extr; 
    delete entities_extr;
    delete semdb;
  }
  

  /////////////////////////////////////////////////////
  /// After relation extraction, clean argless predicates and non-argument entities,
  /// and add non-entity arguments to the graph.
  
  void semgraph_extract::extract(freeling::document &doc) const {

    // extract entities into SG list of entities
    entities_extr->extract_entities(doc);
    // extract relations into SG list of relations, adding new entities to sg if needed
    if (relations_extr != NULL) 
      relations_extr->extract_relations(doc);

    if (semdb!=NULL) {

      semgraph::semantic_graph &sg = doc.get_semantic_graph();

      std::vector<semgraph::SG_entity> & ents = sg.get_entities();
      for (std::vector<semgraph::SG_entity>::iterator e=ents.begin(); e!=ents.end(); ++e) {
        add_semantic_info(*e);
      }

      std::vector<semgraph::SG_frame> & frames = sg.get_frames();
      for (std::vector<semgraph::SG_frame>::iterator f=frames.begin(); f!=frames.end(); ++f) {
        add_semantic_info(*f);
      }
    }    
  }
  
}

  
