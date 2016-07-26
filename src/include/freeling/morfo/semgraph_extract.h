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


#ifndef SG_EXTRACT_H
#define SG_EXTRACT_H

#include "freeling/morfo/language.h"
#include "freeling/morfo/semdb.h"
#include "freeling/morfo/semgraph.h"
#include "freeling/morfo/rel_extract.h"
#include "freeling/morfo/ent_extract.h"


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
  

  class WINDLL semgraph_extract {

  private:
    /// entity extractor to use
    ent_extract *entities_extr;
    /// relation extractor to use 
    rel_extract *relations_extr;
    /// access to WN
    semanticDB *semdb;
    /// Known knowledge bases
    std::list<std::pair<std::wstring,std::wstring>> KBs;

    template<class T> void add_semantic_info (T &e) const;
    
  public:
    /// Constructor   
    semgraph_extract(const std::wstring &erFile);
    
    /// Destructor   
    ~semgraph_extract();
    
    /// extract graph from given document
    void extract(freeling::document &doc) const;

  };


  ///////////////////////////////////////////////////////
  /// Add semantic information (synonyms, sumo, cyc, etc)
  /// to a SG element (entity or frame)
  ///////////////////////////////////////////////////////
  
  template<class T> 
  void semgraph_extract::add_semantic_info (T &e) const {
    // add synonym list to element
    // get sense info
    sense_info si = semdb->get_sense_info(e.get_sense());
    e.set_synonyms(si.words);

    for (std::list<std::pair<std::wstring,std::wstring>>::const_iterator kb=KBs.begin(); kb!=KBs.end(); ++kb) {
      std::wstring code;
      if (kb->first==L"WordNet") code = e.get_sense();
      else if (kb->first==L"OpenCYC") code = si.cyc;
      else if (kb->first==L"SUMO") code = si.sumo.substr(0,si.sumo.length()-1);
      if (not code.empty())
        e.add_URI(kb->first, kb->second + code);
    }
  }
  
}



#endif
