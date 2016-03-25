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

#include "srl_parser.h"

#include "treeler/base/sentence.h"
#include "treeler/control/factory-scores.h"
#include "treeler/srl/factory-srl.h"
#include "treeler/srl/io.h"

namespace treeler {

///////////////////////////////////////////////////////////////
/// Constructor
///////////////////////////////////////////////////////////////

  srl_parser::srl_parser(treeler::Options & srl_options) : _parser(NULL), _scorer(_symbols) {

    _verbose = 0;
    srl_options.get("verbose", _verbose);

    //load the srl parser options, use gold syntax scope
    bool use_gold_syntax;
    if  (not srl_options.get("use-gold-syntax", use_gold_syntax)) srl_options.set("use_gold_syntax", 0, false);
    string scope;
    if  (not srl_options.get("scope", scope)) srl_options.set("scope", "ancestor", true);

    // Symbols
    treeler::control::Factory<symbols_type>::configure(_symbols, srl_options, _verbose>0);

    // parser
    int L=_symbols.d_syntactic_labels.size();
    srl_options.set("L",L,true);    
    _parser = new TreelerSRLParser(_symbols);
    treeler::control::Factory<TreelerSRLParser>::configure(*_parser, srl_options, _verbose>0);

    // scorer

    // syn-offset is a configuration option of the weight vector of SRL
    // models trained with version 0.3 should set it to 0
    // newer models should set it to 1
    // we set it at 0 by default to maintain compatibility
    int tmp;
    if (not srl_options.get("syn-offset", tmp)) {      
      srl_options.set("syn-offset", 0);
    }
    treeler::control::Factory<TreelerSRLScorer>::configure(_symbols, _scorer, srl_options, _verbose>0);

  }

///////////////////////////////////////////////////////////////
/// Destructor
///////////////////////////////////////////////////////////////

  srl_parser::~srl_parser() {
    delete _parser;
  }



///////////////////////////////////////////////////////////////
/// Parse given sentence + dep tree and + set of predicates.
///////////////////////////////////////////////////////////////

  void srl_parser::parse(const basic_sentence &ts,
                         const dep_vector &dv,
                         const srl::PossiblePreds& preds, 
                         pred_arg_set& args_output) const {

    // Convert from basic_sentence to treeler::srl::sentence 
    srl_sentence_type srl_sentence;
    for (int i=0; i<ts.size(); i++) {
      const token &tk = ts.get_token(i);
      srl_sentence.add_token(tk);
    }
    
    // add the given syntax and predicates to the sentence
    srl_sentence.add_syn_and_preds(dv, preds);

    // call the parser
    TreelerSRLScorer::Scores scores;
    args_output.clear();
    _scorer.scores(srl_sentence, scores);
    _parser->argmax(srl_sentence, scores, args_output); 


    if (_verbose >= 2) {
      ostream& log = cerr; 
      string prefix = "SRL_PARSER "; 
      CoNLLStream cstr;
      cstr.prefix = prefix; 
      cstr.add_padding = true;
      cstr << srl_sentence << ">>>" << args_output; 
      log << cstr;
	

      // Xivato per veure els ARGS que ha posat
      for (treeler::srl::PredArgSet::iterator ps=args_output.begin(); ps!=args_output.end(); ps++) {
	log << prefix << " pred: "<<ps->first<<endl;
	for (treeler::srl::PredArgStructure::iterator pa=ps->second.begin(); pa!=ps->second.end(); pa++)
	  log << prefix << "     arg "<<pa->first<<" "<<pa->second<<endl;
	log << prefix <<endl;
      } 
    }

  }

}

