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


#ifndef RELEXTRACT_SRL_H
#define RELEXTRACT_SRL_H

#include "freeling/morfo/rel_extract.h"


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

namespace freeling {

  const std::wstring RE_CONTARG = L"_none_";  // default tags for content arg

  class rel_extract_SRL : public rel_extract {
  private:
    /// PoS regexp for content arguments (create a new node in the graph)
    freeling::regexp ContentArgument;
    /// list of auxiliary verbs, and child requirements to be considered as such
    std::map<std::wstring,std::wstring> auxiliary;
    /// list of relative pronouns and required tag. If the parser sets the verb
    /// of the relative clause as head, then the argument is the PR, and has to be
    /// raised to the head above the verb of the relative clause.
    std::map<std::wstring,std::wstring> nh_relatives;
    
    int get_argument_head(const freeling::sentence &s, int apos, int ppos) const;
    bool is_aux(const freeling::sentence &s, int pos, int &childpos) const;
    
  public:
    rel_extract_SRL(const std::wstring &);
    ~rel_extract_SRL();
    
    /// Extract relations from given sentences
    void extract_relations(freeling::document &doc) const;
  };
  
} // namespace

#endif
