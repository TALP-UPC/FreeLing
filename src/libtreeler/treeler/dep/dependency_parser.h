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


#ifndef TREELER_DEPENDENCY_PARSER
#define TREELER_DEPENDENCY_PARSER

#include <string>
#include <map>
#include <set>
#include <vector>

#include "treeler/base/windll.h"

#include "treeler/dep/dep-symbols.h"
#include "treeler/base/label.h"
#include "treeler/control/models.h"

using namespace std;

namespace treeler {

class WINDLL dependency_parser {
  public:   
    typedef treeler::BasicSentence<string,string> sentence;
    typedef treeler::BasicToken<string,string> token;
    typedef treeler::DepVector<string> dep_vector;

    /// Constructor
    dependency_parser(treeler::Options &);
    /// Destructor
    ~dependency_parser();
    
    // main method for parsing
    void parse(const dependency_parser::sentence &, dep_vector&) const;
    
  private:  
    // redefine treeler typenames
    typedef treeler::control::Model<treeler::control::DEP1>::I TreelerParser;
    //    typedef treeler::control::Model<treeler::control::DEP1>::S TreelerScorer;

    typedef treeler::BasicSentence<int,int> DEP1_X;
    typedef treeler::PartDep1 DEP1_R;
    typedef treeler::FGenDepV0<DEP1_X, DEP1_R> DEP1_FGEN;
    typedef treeler::WFScorer<treeler::DepSymbols, DEP1_X, DEP1_R, DEP1_FGEN> TreelerScorer;
    typedef treeler::Label<DEP1_R> TreelerTree;
    
    // symbol dictionaries
    treeler::DepSymbols _symbols;
    
    // treeler configuration structures
    TreelerParser::Configuration _parser_config;
    TreelerScorer _scorer; 

    // other options
    bool _verbose;
  };
  
}

#endif

