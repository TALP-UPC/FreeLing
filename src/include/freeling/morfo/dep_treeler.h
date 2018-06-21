//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2014   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Affero General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
//             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


#ifndef _DEP_TREELER
#define _DEP_TREELER

#include <string>
#include <map>
#include <set>
#include <vector>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/tagset.h"
#include "freeling/morfo/dependency_parser.h"

//#include "treeler/dep/dep-symbols.h"
//#include "treeler/control/script.h"
//#include "treeler/io/io-conll.h"
//#include "treeler/base/label.h"
//#include "treeler/control/models.h"

#include "treeler/dep/dependency_parser.h"

namespace freeling {

/// Class dep_treeler is a wrapper for a Treeler dependency parser

class WINDLL dep_treeler : public freeling::dependency_parser {

 public:   
  /// Constructor
  dep_treeler(const std::wstring &);
  /// Destructor
  ~dep_treeler();

  /// Analyze given sentences
  void analyze(freeling::sentence &) const;

  /// inherit other methods
  using processor::analyze;
    
private:  
  //typedef treeler::control::Model<treeler::control::DEP1>::R DEP1_R;
  //typedef treeler::Label<DEP1_R> TreelerTree;

  /// treeler parser
  treeler::dependency_parser *dp;
  // tagset handler
  freeling::tagset *tags;
 
  /// Convert FL sentence to Treeler
  void FL2Treeler(const freeling::sentence& fl_sentence, treeler::dependency_parser::sentence &tl_sentence) const;
  /// Add treeler output tree to FL sentence
  void Treeler2FL(freeling::sentence &fl_sentence,
                  const treeler::dependency_parser::dep_vector &tl_tree) const;

  /// Build a FL dep_tree from treeler output
  freeling::dep_tree* build_dep_tree(int node_id, 
                                     const std::vector<std::list<int> > &sons, 
                                     const std::vector<std::string> &labels,
                                     std::map<int,freeling::depnode*> &depnods,
                                     freeling::sentence &fl_sentence) const;

  /// Load dep_parser configuration file into a treeler::Options object
  void load_options(const std::string &cfg, treeler::Options &opts);
};

}
#endif

