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


#ifndef _SRL_PARSER_H
#define _SRL_PARSER_H

#include <string>
#include <map>
#include <list>
#include <set>
#include <vector>

#include "treeler/srl/srl.h"
#include "treeler/srl/sentence.h"
//#include "treeler/control/script.h"
//#include "treeler/io/io-conll.h"
#include "treeler/base/label.h"
#include "treeler/control/models.h"
#include "treeler/srl/simple-parser.h"
#include "treeler/srl/fgen-srl-v1.h"
#include "treeler/srl/scorers.h"
#include "treeler/base/fidx.h"

#include "treeler/base/windll.h"

namespace treeler {


/// Class SRL_parser, wraps a treeler SRL parser

class WINDLL srl_parser {
  public:   
    typedef treeler::BasicSentence<string,string> basic_sentence;
    typedef treeler::srl::PredArgSet pred_arg_set;
    typedef treeler::DepVector<string> dep_vector;

    /// Constructor
    srl_parser(treeler::Options &);
    /// Destructor
    ~srl_parser();
    
    // Parse gicen sentence+tree+predicates
    void parse(const basic_sentence &, const dep_vector &, const srl::PossiblePreds &, pred_arg_set &) const;

  private:  
    typedef treeler::BasicToken<string,string> token;
    typedef treeler::srl::Sentence srl_sentence_type;
    typedef treeler::srl::SRLSymbols symbols_type;
    typedef treeler::srl::PartSRL srl_part_type;

    typedef treeler::srl::SimpleParser  TreelerSRLParser;
    typedef treeler::FIdxBits FIdx;
    typedef treeler::srl::FGenSRLV1<FIdx> srl_fgen_type;
    typedef treeler::WFScorer<symbols_type, srl_sentence_type, srl_part_type, srl_fgen_type> TreelerSRLScorer;
    typedef treeler::Label<srl_part_type> TreelerSRLTree;
    
    // the parser object
    TreelerSRLParser* _parser;
    // symbol dictionaries
    symbols_type _symbols;
    
    // treeler configuration structures
    TreelerSRLParser::Configuration _parser_config;
    TreelerSRLScorer _scorer; 

    // other options
    int _verbose; 
  };
  
}

#endif

