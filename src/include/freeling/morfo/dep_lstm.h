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

#ifndef _DEP_LSTM
#define _DEP_LSTM

#include <set>
#include <vector>
#include <unordered_map>

#include "dynet/lstm.h"
#include "dynet/model.h"

#include "freeling/morfo/language.h"
#include "freeling/morfo/dependency_parser.h"


namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Auxiliary class: bidirectional map (TODO: Move to util? or use boost?)
  ////////////////////////////////////////////////////////////////

  class bimap : public std::map<std::wstring, unsigned> {
  private:
    std::vector<std::wstring> id2str;
    unsigned iunk;
    bool has_unk;

  public:
    bimap();
    ~bimap();

    void add(const std::wstring &s, unsigned id);
    unsigned insert(const std::wstring &s);
    void set_unk_id(unsigned id);
  
    unsigned get_unk_id() const;    
    std::wstring id2string(unsigned id) const;
    unsigned string2id(const std::wstring &s) const;
    
  };

  
  ///////////////////////////////////////////////////////////////////////
  ///
  /// dep_lstm is a ANN-based dependency parser
  ///
  ///////////////////////////////////////////////////////////////////////
  
  class dep_lstm : public freeling::dependency_parser {
  
  private:
    const std::wstring ROOT_SYMBOL = L"ROOT";
    const std::wstring wUNK = L"<UNK>";
    unsigned kUNK;
    
    typedef enum {NO_ACTION,SHIFT,SWAP,LEFT,RIGHT} actionType;
    
    unsigned LAYERS;
    unsigned INPUT_DIM;
    unsigned HIDDEN_DIM;
    unsigned ACTION_DIM;
    unsigned PRETRAINED_DIM;
    unsigned LSTM_INPUT_DIM;
    unsigned POS_DIM;
    unsigned REL_DIM;
    
    unsigned ACTION_SIZE;
    unsigned VOCAB_SIZE;
    unsigned POS_SIZE;
    
    bimap _words;   
    bimap _tags;
    bimap _actions;
    
    std::set<unsigned> training_vocab;
    std::unordered_map<unsigned, std::vector<float>> pretrained;

    dynet::ParameterCollection* model;
    
    dynet::LSTMBuilder* stack_lstm; // (layers, input, hidden, trainer)
    dynet::LSTMBuilder* buffer_lstm;
    dynet::LSTMBuilder* action_lstm;
    dynet::LookupParameter p_w; // word embeddings
    dynet::LookupParameter p_t; // pretrained word embeddings (not updated)
    dynet::LookupParameter p_a; // input action embeddings
    dynet::LookupParameter p_r; // relation embeddings
    dynet::LookupParameter p_p; // pos tag embeddings
    dynet::Parameter p_pbias; // parser state bias
    dynet::Parameter p_A; // action lstm to parser state
    dynet::Parameter p_B; // buffer lstm to parser state
    dynet::Parameter p_S; // stack lstm to parser state
    dynet::Parameter p_H; // head matrix for composition function
    dynet::Parameter p_D; // dependency matrix for composition function
    dynet::Parameter p_R; // relation matrix for composition function
    dynet::Parameter p_w2l; // word to LSTM input
    dynet::Parameter p_p2l; // POS to LSTM input
    dynet::Parameter p_t2l; // pretrained word embeddings to LSTM input
    dynet::Parameter p_ib; // LSTM input bias
    dynet::Parameter p_cbias; // composition function bias
    dynet::Parameter p_p2a;   // parser state to action
    dynet::Parameter p_action_start;  // action bias
    dynet::Parameter p_abias;  // action bias
    dynet::Parameter p_buffer_guard;  // end of buffer
    dynet::Parameter p_stack_guard;  // end of stack
    
    static bool forbidden_action(const std::wstring &a, unsigned bsize, unsigned ssize, const std::vector<int>& stacki);
    void log_prob_parser(const std::vector<unsigned>& raw_sent,  // raw sentence
                         const std::vector<unsigned>& sent,  // sent with oovs replaced
                         const std::vector<unsigned>& sentPos,
                         std::map<int,int> &deps,
                         std::map<int,std::wstring> &rels) const;
    
    bool is_training_vocab(unsigned wid) const;
    
    static actionType get_action_type(const std::wstring &action);
    std::wstring get_action_rel(const std::wstring &action) const;
    
    freeling::dep_tree* build_dep_tree(int node_id,
                                       const std::vector<std::list<int> > &sons, 
                                       const std::vector<std::wstring> &labels,
                                       std::map<int,freeling::depnode*> &depnods,
                                       freeling::sentence &fl_sentence) const ;

    void lstm2FL(freeling::sentence &fl_sentence,
                 const std::map<int,int> &deps,  
                 const std::map<int,std::wstring> &rels) const;
    
  
  public:
    /// constructor
    dep_lstm(const std::wstring &fname);
    /// destructor
    ~dep_lstm();
    
    /// Analyze given sentence
    void analyze(freeling::sentence &sent) const;
    
    /// inherit other methods
    using processor::analyze;
  };

} // namespace

#endif
