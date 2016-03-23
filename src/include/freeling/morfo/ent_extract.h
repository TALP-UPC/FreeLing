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

#ifndef ENTITY_EXTRACT_H
#define ENTITY_EXTRACT_H

#include <string>
#include <list>
#include <map>

#include "freeling/morfo/language.h"
#include "freeling/morfo/semgraph.h"

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
  
  class ent_extract {
  private:

    void extract_entities_nocoref(freeling::document &doc) const;
    
    void extract_entities_coref(freeling::document &doc) const;
    
    bool is_alias(const std::wstring &e1, const std::wstring &e2) const;
    bool is_NE(const freeling::word &w) const;
    std::wstring get_NE_class(const freeling::word &w) const;
    
    /// PoS tag for named entites
    std::wstring NEtag;
    std::map<std::wstring,std::wstring> NEclasses;
    
  public:
    ent_extract(const std::wstring &);
    ~ent_extract();
    
    /// Extract entities from given sentences
    void extract_entities(freeling::document &doc) const;
    

    std::wstring search_alias(const semgraph::semantic_graph &sg, 
                              const std::wstring &lemma) const;

  };

}

#endif
