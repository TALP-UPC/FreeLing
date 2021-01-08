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

#ifndef RELEXTRACT_H
#define RELEXTRACT_H

#include <string>
#include <list>
#include <map>

#include "freeling/morfo/database.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/semgraph.h"

namespace freeling {

  ////////////////////////////////////////////////////
  /////                                          /////
  /////        Class SG_Extractor                /////
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
  
  class rel_extract {
  protected:
    std::wstring argument_to_role(const std::wstring &wsr, const std::wstring &sense) const;
        
    /// PoS tag for named entites
    std::wstring NEtag;
    
    /// map to convert arguments (A0, A1, etc) to roles (Agent, Patient, etc)
    freeling::database *ArgMap;    
    /// type of argument map.
    ///    "DIRECT":  "arg0-agt" maps to "A0:Agent" 
    ///    "SYNSARG" : each synset code maps to a list "A0:Agent A1:Patient..."
    enum {DIRECT,SYNSARG} mapType;
    /// labels for A0,A1 produced by parser (SPR extractor only)
    std::wstring A0_label, A1_label; 

    /// whether to merge singletons with lemma+sense matching coref gropus
    bool MergeSingletons;
    
    /// check if mention has already appeared
    std::wstring find_entity_node(freeling::semgraph::semantic_graph &sg,
                                  const freeling::sentence &sent, 
                                  const freeling::word &wd) const;

  public:
    rel_extract(const std::wstring &);
    virtual ~rel_extract();
    
    /// Extract relations from given sentences
    virtual void extract_relations(freeling::document &doc) const = 0;

  };

}

#endif
