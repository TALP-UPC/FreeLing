#ifndef TREELER_SRL_IO
#define TREELER_SRL_IO

#include "treeler/io/conllstream.h"

namespace treeler {


  // write the sentence
  template <typename Mapping, typename Format, typename CharT> 
  CoNLLBasicStream<Mapping,Format,CharT>& operator<<(CoNLLBasicStream<Mapping,Format,CharT>& stream, const srl::Sentence& x) {
    const srl::Sentence::BaseType& basex = static_cast<const srl::Sentence::BaseType&>(x); 
    stream << basex;
    stream << x.dependency_vector(); 
    return stream; 
  }

  
  /** read a srl sentence */
  template <typename Mapping, typename Format, typename CharT> 
  CoNLLBasicStream<Mapping,Format,CharT>& operator>>(CoNLLBasicStream<Mapping,Format,CharT>& stream, srl::Sentence& x) {
    size_t m = stream.read_ptr;
   
    // token information
    if (m + 7 >= stream.num_columns()) {
      stream.good = false;
      return stream;
    }

    stream >> (srl::Sentence::BaseType&) x;
    
    if (!stream.good) {
      return stream;
    }
    
    // now read the dependency tree
    DepVector<string> pred_deps;
    stream >> pred_deps;
       
    //Read a pred arg set
    srl::PredArgSet pred_arg_set;
    srl::PossiblePreds possiblepreds;
    vector<string> is_pred_vec, pred_id_vec;
    // read the list of predicates and senses    
    {
      stream >> is_pred_vec;
      stream >> pred_id_vec;
      // rewind the stream
      stream.read_ptr = stream.read_ptr - 2; 

      // index the predicates
      for (size_t i=0; i<pred_id_vec.size(); ++i) {
	const string& pred_id = pred_id_vec[i];
	if (pred_id!="_") {
	  srl::PredArgStructure p(pred_id);
	  pred_arg_set[i] = p;
	  
	  srl::PossiblePredArgs pred;
	  size_t k = pred_id.find_last_of('.');
	  if (k == std::string::npos) {
	    pred.predsense.push_back(make_pair(pred_id,""));
	  }
	  else {
	    pred.predsense.push_back(make_pair(pred_id.substr(0,k),pred_id.substr(k+1)));
	  }

	  possiblepreds.insert(make_pair(i,pred));
	}
      }
    }
    x.add_syn_and_preds(pred_deps, possiblepreds);
    return stream;
  }
  

  template <typename Mapping, typename Format, typename CharT> 
  CoNLLBasicStream<Mapping,Format,CharT>& operator<<(CoNLLBasicStream<Mapping,Format,CharT>& stream, const Label<srl::PartSRL>& y) {
    return stream;
  }

  template <typename Mapping, typename Format, typename CharT> 
  CoNLLBasicStream<Mapping,Format,CharT>& operator<<(CoNLLBasicStream<Mapping,Format,CharT>& stream, const srl::PredArgSet& x) {
    vector<string> preds_y_n (x.sentence_token_count, "_");
    vector<string> senses (x.sentence_token_count, "_");
    for (srl::PredArgSet::const_iterator pred = x.begin(); pred != x.end(); ++pred) {
      preds_y_n[pred->first] = "Y";
      senses[pred->first] = pred->second.sense;
    }
    if (stream.format.predicate_indicator) {
      stream << preds_y_n;
    }
    stream << senses;
    for (srl::PredArgSet::const_iterator pred = x.begin(); pred != x.end(); ++pred) {
      vector<string> args (x.sentence_token_count, "_");
      for (map<int, string>::const_iterator arg = pred->second.begin(); arg != pred->second.end(); ++arg) {
        args[arg->first] = arg->second;
      }
      stream << args;
    }    
    return stream; 
  }


  template <typename Mapping, typename Format, typename CharT> 
  CoNLLBasicStream<Mapping,Format,CharT>& operator>>(CoNLLBasicStream<Mapping,Format,CharT>& stream, srl::PredArgSet& x) {
    vector<string> is_pred, senses;
    stream >> is_pred;  
    stream >> senses;

    x.sentence_token_count = senses.size();
    
    int predidx = 0;
    map<int,int> predidx_tokenidx;
    map<int,int> tokenidx_predidx;
    for (vector<string>::size_type tokenidx = 0; tokenidx < senses.size(); tokenidx++) {
      if (senses[tokenidx] != "_") {
        srl::PredArgStructure p(senses[tokenidx]);
        x[tokenidx] = p;
        predidx_tokenidx[predidx] = tokenidx;
        tokenidx_predidx[tokenidx] = predidx;
        predidx++;
      }
    }
    
    for (int i = 0; i < predidx; i++) {
      int tokenidx = predidx_tokenidx[i];
      vector<string> args;
      stream >> args;
      for (unsigned argidx = 0; argidx < args.size(); argidx++ ) {
        if (args[argidx] != "_") {
          x[tokenidx][argidx] = args[argidx];
        }
      }
    }
    return stream;
  }


}



#endif
