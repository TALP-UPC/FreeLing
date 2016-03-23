
#include "treeler/dep/dependency_parser.h"

//#include "treeler/base/sentence.h"
#include "treeler/control/factory-scores.h"
#include "treeler/control/factory-dep.h"


namespace treeler {

  /// Constructor
  dependency_parser::dependency_parser(treeler::Options &options) : _scorer(_symbols) {

    _verbose = false; 
    options.get("verbose", _verbose);
 
    // Symbols
    treeler::control::Factory<treeler::DepSymbols>::configure(_symbols, options, _verbose);    

    int L=_symbols.d_syntactic_labels.size();
    options.set("L",L,true);

    // inference
    treeler::control::Factory<TreelerParser>::configure(_parser_config, options);    
    treeler::control::Factory<TreelerScorer>::configure(_symbols, _scorer, options, _verbose);
  }



  /// Destructor
  dependency_parser::~dependency_parser() { }
  


  // main method for parsing
  void dependency_parser::parse(const dependency_parser::sentence &ts, dep_vector &dv) const {

    // Convert to treeler::sentence and perform mappings
    DEP1_X tl_sentence;

    for (int i=0; i<ts.size(); i++) {
      const token &tok = ts.get_token(i);
  
      // port token information using symbol dictionaries
      int i_word = _symbols.map_field<treeler::WORD,int>(tok.word());
      int i_fine_pos = _symbols.map_field<treeler::FINE_POS,int>(tok.fine_pos());
      int i_lemma = _symbols.map_field<treeler::LEMMA,int>(tok.lemma());
      int i_coarse_pos = _symbols.map_field<treeler::COARSE_POS,int>(tok.coarse_pos());

      // build the treeler token, add to treeler sentence
      DEP1_X::Token tk(i_word, i_lemma, i_coarse_pos, i_fine_pos);

      // translate MSD features 
      for (list<string>::const_iterator f=tok.morpho_begin(); f!=tok.morpho_end(); f++) 
        tk.morpho_push(_symbols.map_field<treeler::MORPHO_TAG,int>(*f));

      // and new ported token to sentence
      tl_sentence.add_token(tk);
    }
        
    // Do the parsing: call parser argmax to parse the example
    TreelerScorer::Scores scores;
    treeler::DepVector<int> idv;
    _scorer.scores(tl_sentence, scores);
    TreelerParser::argmax(_parser_config, tl_sentence, scores, idv); 

    // convert output and return results
    dv.clear();
    for (treeler::DepVector<int>::iterator k=idv.begin(); k!=idv.end(); k++) {
      dv.push_back(treeler::HeadLabelPair<string>(k->h,_symbols.map_field<treeler::SYNTACTIC_LABEL,string>(k->l)));
    }
  }
}

