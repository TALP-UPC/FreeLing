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


#ifndef RELEXTRACT_SPR_H
#define RELEXTRACT_SPR_H

#include "freeling/morfo/rel_extract.h"

namespace freeling {

  //////////////////////////////////////
  /// Auxiliary class to store relation extraction rules

  class node_pattern {
  public:
    std::wstring lemma;
    std::wstring pos;
    std::wstring funct;
    int dir;
    bool flex;
  };
  
  //////////////////////////////////////
  /// Auxiliary class to store relation extraction rules
  
  class rx_rule {
  public:   
    /// rule id, used for debugging purposes only 
    std::wstring id; 
    /// relation rule is a list of node patterns with a up/down direction
    std::list<node_pattern> rpath;
    /// extraction rule also stores information about which sub-regex must be 
    /// used to extract the components of the triple Entity1-Relation-Entity2
    std::wstring entity1, relation, entity2;      
    
    /// constructor: build rule given regex string and key positions
    rx_rule(const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&);
    /// destructor
    ~rx_rule();
  };
  
  ////////////////////////////////////////////////////
  /////                                          /////
  /////        Class rel_extract_SPR             /////
  /////                                          /////
  /////  This class derives a relation extractor /////
  /////  for semantic graphs, based on Syntactic /////
  /////  Pattern Rules (SPR) over a dependency   /////
  /////  tree.                                   /////
  /////  It requires the input document is       /////
  /////  dependency parsed and NEC.              /////
  /////                                          /////
  ////////////////////////////////////////////////////
  
  class rel_extract_SPR : public rel_extract {
  private:
    /// relation extraction rules.
    std::list<rx_rule> xrules;
    
    bool check_attr(const std::wstring &pat, const std::wstring &prop, 
                    std::map<std::wstring,freeling::dep_tree::const_iterator> &val, bool pref=false) const;
    bool next_node(freeling::dep_tree::const_preorder_iterator nd, 
                   std::list<node_pattern>::const_iterator p, 
                   std::map<std::wstring,freeling::dep_tree::const_iterator> &val,
                   int best=0) const;
    bool match_rule(freeling::dep_tree::const_preorder_iterator nd, 
                    std::list<node_pattern>::const_iterator p, 
                    std::map<std::wstring,freeling::dep_tree::const_iterator> &val,
                    int best=0) const;
        
  public:
    rel_extract_SPR(const std::wstring &);
    ~rel_extract_SPR();
    
    /// Extract relations from given sentences
    void extract_relations(freeling::document &doc) const;
  };
  
} // namespace

#endif
