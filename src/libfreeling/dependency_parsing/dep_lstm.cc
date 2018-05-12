
#include <sstream>

#include "freeling/morfo/dep_lstm.h"

#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/embeddings.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DEP_LSTM"
#define MOD_TRACECODE DEP_TRACE

  ///////////////////////////////////////////////////////////////////////
  /// bimap is a simple bidirectional map, auxiliary to dep_lstm
  ///////////////////////////////////////////////////////////////////////

  bimap::bimap() : has_unk(false) {}
  bimap::~bimap() {}

  /// add string with given id

  void bimap::add(const std::wstring &s, unsigned id) {
    id2str.push_back(s);
    (*this)[s] = id;
  }

  /// add string. assign next id if new, return id if existing

  unsigned bimap::insert(const std::wstring &s) {
    map<wstring,unsigned>::const_iterator p = this->find(s);
    if (p!=this->end())
      return p->second;
    else {
      unsigned n = this->size();
      (*this)[s] = n;
      id2str.push_back(s);
      return n;
    }
  }

  /// set id for unknown strings, if any.
  
  void bimap::set_unk_id(unsigned id) {
    has_unk = true;
    iunk = id;
  }

  unsigned bimap::get_unk_id() const {
    if (not has_unk) {
      ERROR_CRASH(L"Requested unk_id on bimap, but it was not set");
    }
    return iunk;
  }


  /// return  string given id
  
  std::wstring bimap::id2string(unsigned id) const {
    if (id < id2str.size()) return id2str[id];
    else if (has_unk) return id2str[iunk];
    else {
      ERROR_CRASH(L"Invalid argument for id2string: "<<id);
    }
  }

  /// return id given a string
  
  unsigned bimap::string2id(const std::wstring &s) const {
    map<wstring,unsigned>::const_iterator p = this->find(s);
    if (p!=this->end()) return p->second;
    else if (has_unk) return iunk;
    else {
      ERROR_CRASH(L"Invalid argument for string2id: "<<s);
    }
  }
    
  ///////////////////////////////////////////////////////////////////////
  ///
  /// dep_lstm is an ANN-based transition dependency parser
  ///
  ///////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////
  /// Constructor:  Load Model and create LSTM
  ////////////////////////////////////////////////////////////////

  dep_lstm::dep_lstm(const wstring &fname) {

    int c = 1;
    char **k;
    dynet::initialize(c,k);
    
    wstring path=fname.substr(0,fname.find_last_of(L"/\\")+1);
    
    enum sections {NETWORK,SIZES,WORDS,TAGS,ACTIONS};
    
    freeling::config_file cfg;
    cfg.add_section(L"NETWORK",NETWORK,true);
    cfg.add_section(L"WORDS",WORDS,true);
    cfg.add_section(L"TAGS",TAGS,true);
    cfg.add_section(L"ACTIONS",ACTIONS,true);
    
    if (not cfg.open(fname)) {
      ERROR_CRASH(L"Error opening file "<<fname);
    }
    
    LAYERS = INPUT_DIM = HIDDEN_DIM = ACTION_DIM = 0;
    PRETRAINED_DIM = LSTM_INPUT_DIM = POS_DIM = REL_DIM = 0;
    ACTION_SIZE = VOCAB_SIZE = POS_SIZE = 0;
    //nwords = ntags = nactions = 0;
    wstring embeddingsFile, modelFile;
    
    wstring line;
    while (cfg.get_content_line(line)) {
      wistringstream iss(line);
      
      switch (cfg.get_section()) {
      case NETWORK: {
        wstring key;
        iss >> key;
        if (key==L"Layers") iss>>LAYERS;
        else if (key==L"InputDIM") iss>>INPUT_DIM;
        else if (key==L"PretrainedDIM") iss>>PRETRAINED_DIM;
        else if (key==L"HiddenDIM") iss>>HIDDEN_DIM;
        else if (key==L"ActionDIM") iss>>ACTION_DIM;
        else if (key==L"LSTMInputDIM") iss>>LSTM_INPUT_DIM;
        else if (key==L"PosDIM") iss>>POS_DIM;
        else if (key==L"RelDIM") iss>>REL_DIM;
        else if (key==L"ModelFile") iss>>modelFile;
        else if (key==L"EmbeddingsFile") iss>>embeddingsFile;
        else {
          WARNING(L"Warning: Ignoring unexpected key "<<key<<" in NETWORK section of file "<<fname<<L".");
        }      
        break;
      }         

      case WORDS: {
        unsigned id; wstring word;
        iss >> id >> word;
        _words.add(word, id);
        if (word==wUNK)
          _words.set_unk_id(id);
        else if (id>=2)
          training_vocab.insert(id);
        break;
      }

      case TAGS: {
        unsigned id; wstring tag;
        iss >> id >> tag;
        _tags.add(tag,id);
        break;
      }

      case ACTIONS: {
        unsigned id; wstring action;
        iss >> id >> action;
        _actions.add(action,id);
        break;
      }
      default: break;
      }
    }

    cfg.close();

    if (LAYERS==0 or INPUT_DIM==0 or HIDDEN_DIM==0 or ACTION_DIM==0 or
        PRETRAINED_DIM==0 or LSTM_INPUT_DIM==0 or POS_DIM==0 or REL_DIM==0) {
      // error missing dimensions
      ERROR_CRASH(L"Not all required dimensions were specified.");
    }

    if (modelFile.empty()) {
      // error no model given
      ERROR_CRASH(L"No model file specified.");
    }

    if (PRETRAINED_DIM>0) { // use pretrained embeddings
    
      if (embeddingsFile.empty()) {
        // error no embeddings when there should be
        ERROR_CRASH(L"Model requires embeddings, but no embedding file was provided.");
      }

      TRACE(1,"Loading embeddings.");

      // load pretrained embeddings
      freeling::embeddings embeds(freeling::util::absolute(embeddingsFile,path));
      list<wstring> vocab = embeds.get_vocab();    
      for (auto w : vocab) {
        unsigned id = _words.insert(w);
        pretrained[id] = embeds.get_base_vector(w);
      }
      // embedding for unknown words
      pretrained[_words.get_unk_id()] = vector<float>(PRETRAINED_DIM, 0);
    }

    // add special tag for root fake word
    _tags.insert(L"ROOT");
  
    VOCAB_SIZE = _words.size() + 1;
    ACTION_SIZE = _actions.size() + 1;
    POS_SIZE = _tags.size() + 9;  // bad way of dealing with the fact that we may see new POS tags in the test set
  
    model = new dynet::ParameterCollection();

    stack_lstm = new dynet::LSTMBuilder(LAYERS, LSTM_INPUT_DIM, HIDDEN_DIM, *model);
    buffer_lstm = new dynet::LSTMBuilder(LAYERS, LSTM_INPUT_DIM, HIDDEN_DIM, *model);
    action_lstm = new dynet::LSTMBuilder(LAYERS, ACTION_DIM, HIDDEN_DIM, *model);
    p_w = model->add_lookup_parameters(VOCAB_SIZE, {INPUT_DIM});
    p_a = model->add_lookup_parameters(ACTION_SIZE, {ACTION_DIM});
    p_r = model->add_lookup_parameters(ACTION_SIZE, {REL_DIM});
    p_pbias = model->add_parameters({HIDDEN_DIM});
    p_A = model->add_parameters({HIDDEN_DIM, HIDDEN_DIM});
    p_B = model->add_parameters({HIDDEN_DIM, HIDDEN_DIM});
    p_S = model->add_parameters({HIDDEN_DIM, HIDDEN_DIM});
    p_H = model->add_parameters({LSTM_INPUT_DIM, LSTM_INPUT_DIM});
    p_D = model->add_parameters({LSTM_INPUT_DIM, LSTM_INPUT_DIM});
    p_R = model->add_parameters({LSTM_INPUT_DIM, REL_DIM});
    p_w2l = model->add_parameters({LSTM_INPUT_DIM, INPUT_DIM});
    p_ib = model->add_parameters({LSTM_INPUT_DIM});
    p_cbias = model->add_parameters({LSTM_INPUT_DIM});
    p_p2a = model->add_parameters({ACTION_SIZE, HIDDEN_DIM});
    p_action_start = model->add_parameters({ACTION_DIM});
    p_abias = model->add_parameters({ACTION_SIZE});
    p_buffer_guard = model->add_parameters({LSTM_INPUT_DIM});
    p_stack_guard = model->add_parameters({LSTM_INPUT_DIM});
  
    p_p = model->add_lookup_parameters(POS_SIZE, {POS_DIM});
    p_p2l = model->add_parameters({LSTM_INPUT_DIM, POS_DIM});

    if (pretrained.size() > 0) {
      p_t = model->add_lookup_parameters(VOCAB_SIZE, {PRETRAINED_DIM});
      for (auto it : pretrained)
        p_t.initialize(it.first, it.second);
      p_t2l = model->add_parameters({LSTM_INPUT_DIM, PRETRAINED_DIM});
    }
    else {
      p_t.zero();
      p_t2l.zero();
    }

    TRACE(1,L"Loading model.");
    load_dynet_model(freeling::util::wstring2string(modelFile), model);
  }

  ////////////////////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////////////////////

  dep_lstm::~dep_lstm() {}

  ////////////////////////////////////////////////////////////////
  /// word id was seen during training
  ////////////////////////////////////////////////////////////////

  bool dep_lstm::is_training_vocab(unsigned wid) const {
    return training_vocab.find(wid) != training_vocab.end();
  }

  
  ////////////////////////////////////////////////////////////////
  /// get action type
  ////////////////////////////////////////////////////////////////

  dep_lstm::actionType dep_lstm::get_action_type(const wstring &action) {
    if (action.substr(0,5)==L"SHIFT") return SHIFT;
    else if (action.substr(0,4)==L"SWAP") return SWAP;
    else if (action.substr(0,4)==L"LEFT") return LEFT;
    else if (action.substr(0,5)==L"RIGHT") return RIGHT;
    else return NO_ACTION;
  }

  ////////////////////////////////////////////////////////////////
  /// get action relation
  ////////////////////////////////////////////////////////////////

  wstring dep_lstm::get_action_rel(const wstring &action) const {
    size_t first = action.find('(') + 1;
    size_t last = action.rfind(')') - 1;
    return action.substr(first, last-first+1);  
  }

  ////////////////////////////////////////////////////////////////
  /// check if action is forbidden in current configuration
  ////////////////////////////////////////////////////////////////

  bool dep_lstm::forbidden_action(const wstring & act, unsigned bsize, unsigned ssize, const vector<int>& stacki) {

    actionType a = get_action_type(act);

    if (a==SWAP and ssize<3) return true;
    if (a==SWAP) {
      int top=stacki[stacki.size()-1];
      int sec=stacki[stacki.size()-2];
      if (sec>top) return true;
    }

    bool is_shift = (a==SHIFT);
    bool is_reduce = not is_shift;
    if (is_shift and bsize == 1) return true;
    if (is_reduce and ssize < 3) return true;
    if (bsize == 2 and // ROOT is the only thing remaining on buffer
        ssize > 2 and // there is more than a single element on the stack
        is_shift) return true;
    // only attach left to ROOT
    if (bsize==1 and ssize==3 and a==RIGHT) return true;
    return false;
  }


  ////////////////////////////////////////////////////////////////
  /// The parser itself
  /// *** if correct_actions is empty, this runs greedy decoding ***
  /// returns parse actions for input sentence (in training just returns the reference)
  /// OOV handling: raw_sent will have the actual words
  ///               sent will have words replaced by appropriate UNK tokens
  /// this lets us use pretrained embeddings, when available, for words that were 
  /// OOV in the parser training data
  ////////////////////////////////////////////////////////////////

  void dep_lstm::log_prob_parser(const vector<unsigned>& raw_sent,  // raw sentence
                                    const vector<unsigned>& sent,  // sent with oovs replaced
                                    const vector<unsigned>& sentPos,
                                    map<int,int> &deps,
                                    map<int,wstring> &rels) const {

    for(unsigned i=0; i<sent.size(); i++) {
      deps[i]=-1;
      rels[i]=L"ERROR";
    }

    dynet::ComputationGraph hg;

    vector<unsigned> results;
  
    stack_lstm->new_graph(hg);
    buffer_lstm->new_graph(hg);
    action_lstm->new_graph(hg);
  
    stack_lstm->start_new_sequence();
    buffer_lstm->start_new_sequence();
    action_lstm->start_new_sequence();

    // variables in the computation graph representing the parameters
    dynet::Expression pbias = parameter(hg, p_pbias);
    dynet::Expression H = parameter(hg, p_H);
    dynet::Expression D = parameter(hg, p_D);
    dynet::Expression R = parameter(hg, p_R);
    dynet::Expression cbias = parameter(hg, p_cbias);
    dynet::Expression S = parameter(hg, p_S);
    dynet::Expression B = parameter(hg, p_B);
    dynet::Expression A = parameter(hg, p_A);
    dynet::Expression ib = parameter(hg, p_ib);
    dynet::Expression w2l = parameter(hg, p_w2l);
    dynet::Expression p2l = parameter(hg, p_p2l);
    dynet::Expression t2l;
    if (pretrained.size() > 0) t2l = parameter(hg, p_t2l);
    dynet::Expression p2a = parameter(hg, p_p2a);
    dynet::Expression abias = parameter(hg, p_abias);
    dynet::Expression action_start = parameter(hg, p_action_start);
  
    action_lstm->add_input(action_start);
  
    vector<dynet::Expression> buffer(sent.size() + 1);  // variables representing word embeddings (possibly including POS info)
    vector<int> bufferi(sent.size() + 1);  // position of the words in the sentence

    // precompute buffer representation from left to right
    for (unsigned i = 0; i < sent.size(); ++i) {
      assert(sent[i] < VOCAB_SIZE);
      dynet::Expression w =lookup(hg, p_w, sent[i]);
    
      vector<dynet::Expression> args = {ib, w2l, w}; // learn embeddings
      dynet::Expression p = lookup(hg, p_p, sentPos[i]);
      args.push_back(p2l);
      args.push_back(p);
    
      if (pretrained.size()>0 and pretrained.count(raw_sent[i])) {  // include fixed pretrained vectors?
        dynet::Expression t = const_lookup(hg, p_t, raw_sent[i]);
        args.push_back(t2l);
        args.push_back(t);
      }
      buffer[sent.size() - i] = rectify(affine_transform(args));
      bufferi[sent.size() - i] = i;
    }
    // dummy symbol to represent the empty buffer
    buffer[0] = parameter(hg, p_buffer_guard);
    bufferi[0] = -999;
    for (auto& b : buffer)
      buffer_lstm->add_input(b);
  
    vector<dynet::Expression> stack;  // variables representing subtree embeddings
    vector<int> stacki; // position of words in the sentence of head of subtree
    stack.push_back(parameter(hg, p_stack_guard));
    stacki.push_back(-999); // not used for anything
    // drive dummy symbol on stack through LSTM
    stack_lstm->add_input(stack.back());
    vector<dynet::Expression> log_probs;
    wstring rootword;
    unsigned action_count = 0;  // incremented at each prediction
    while (stack.size() > 2 or buffer.size() > 1) {
      // get list of possible actions for the current parser state
      vector<unsigned> current_valid_actions;
      for (unsigned a=0; a<_actions.size(); ++a) {
        if (not forbidden_action(_actions.id2string(a), buffer.size(), stack.size(), stacki))
          current_valid_actions.push_back(a);
      }
    
      // p_t = pbias + S * slstm + B * blstm + A * almst
      dynet::Expression p_t = dynet::affine_transform({pbias, S, stack_lstm->back(), B, buffer_lstm->back(), A, action_lstm->back()});
      dynet::Expression nlp_t = rectify(p_t);
      // r_t = abias + p2a * nlp
      dynet::Expression r_t = dynet::affine_transform({abias, p2a, nlp_t});
    
      // adist = log_softmax(r_t, current_valid_actions)
      dynet::Expression adiste = log_softmax(r_t, current_valid_actions);
      vector<float> adist = dynet::as_vector(hg.incremental_forward(adiste));
      double best_score = adist[current_valid_actions[0]];
      unsigned best_a = current_valid_actions[0];
      for (unsigned i = 1; i < current_valid_actions.size(); ++i) {
        if (adist[current_valid_actions[i]] > best_score) {
          best_score = adist[current_valid_actions[i]];
          best_a = current_valid_actions[i];
        }
      }
      unsigned action = best_a;
      ++action_count;
      log_probs.push_back(pick(adiste, action));
      results.push_back(action);
    
      // add current action to action LSTM
      dynet::Expression actione = lookup(hg, p_a, action);
      action_lstm->add_input(actione);
    
      // get relation embedding from action (TODO: convert to relation from action?)
      dynet::Expression relation = lookup(hg, p_r, action);
    
      // do action
      //wstring actname = get_action(action);
      wstring actname = _actions.id2string(action);
      actionType which = get_action_type(actname);

      switch (which) {
      case SHIFT : { 
        assert(buffer.size() > 1); // dummy symbol means > 1 (not >= 1)
        stack.push_back(buffer.back());
        stack_lstm->add_input(buffer.back());
        buffer.pop_back();
        buffer_lstm->rewind_one_step();
        stacki.push_back(bufferi.back());
        bufferi.pop_back();
        break;
      }

      case SWAP : {
        assert(stack.size() > 2); // dummy symbol means > 2 (not >= 2)
      
        dynet::Expression toki, tokj;
        unsigned ii = 0, jj = 0;
        tokj=stack.back();
        jj=stacki.back();
        stack.pop_back();
        stacki.pop_back();
      
        toki=stack.back();
        ii=stacki.back();
        stack.pop_back();
        stacki.pop_back();
      
        buffer.push_back(toki);
        bufferi.push_back(ii);
      
        stack_lstm->rewind_one_step();
        stack_lstm->rewind_one_step();
      
        buffer_lstm->add_input(buffer.back());
      
        stack.push_back(tokj);
        stacki.push_back(jj);
      
        stack_lstm->add_input(stack.back());
        break;
      }
      
        // LEFT or RIGHT
      default : { 
        assert(stack.size() > 2); // dummy symbol means > 2 (not >= 2)
        assert(which == LEFT or which == RIGHT);
        dynet::Expression dep, head;
        unsigned depi = 0, headi = 0;
        (which==RIGHT ? dep : head) = stack.back();
        (which==RIGHT ? depi : headi) = stacki.back();
        stack.pop_back();
        stacki.pop_back();
        (which==RIGHT ? head : dep) = stack.back();
        (which==RIGHT ? headi : depi) = stacki.back();
        stack.pop_back();
        stacki.pop_back();
        if (headi == sent.size() - 1) rootword = _words.id2string(sent[depi]);
        // composed = cbias + H * head + D * dep + R * relation
        dynet::Expression composed = dynet::affine_transform({cbias, H, head, D, dep, R, relation});
        dynet::Expression nlcomposed = tanh(composed);
        stack_lstm->rewind_one_step();
        stack_lstm->rewind_one_step();
        stack_lstm->add_input(nlcomposed);
        stack.push_back(nlcomposed);
        stacki.push_back(headi);

        deps[depi] = headi;
        rels[depi] = get_action_rel(actname);
      }
      }
    }
    assert(stack.size() == 2); // guard symbol, root
    assert(stacki.size() == 2);
    assert(buffer.size() == 1); // guard symbol
    assert(bufferi.size() == 1);
    dynet::Expression tot_neglogprob = -sum(log_probs);
    assert(tot_neglogprob.pg != nullptr);
    //return results;
  }

  ////////////////////////////////////////////////////////////////
  /// analyze freeling sentence
  ////////////////////////////////////////////////////////////////

  void dep_lstm::analyze(freeling::sentence &sent) const {
  
    vector<unsigned> wordids, tagids;
    vector<wstring> unkwords;

    // use a local copy to keep thread-safety in case we need to add some unseen tag.
    bimap sent_tags = _tags;
    
    for (auto w : sent) {
      unsigned wid = _words.string2id(w.get_form());
      wordids.push_back(wid);

      if (wid == _words.get_unk_id()) unkwords.push_back(w.get_form());
      else unkwords.push_back(L"");

      unsigned tid = sent_tags.insert(w.get_tag());
      tagids.push_back(tid);
    }

    // add "root" at the end of the sentence
    unkwords.push_back(L"");
    wordids.push_back(_words.string2id(L"ROOT"));
    tagids.push_back(_tags.string2id(L"ROOT"));
  
    vector<unsigned> tsentence = wordids;
    for (auto& w : tsentence)
      if (not is_training_vocab(w)) w = _words.get_unk_id();
  
    map<int,int> deps;
    map<int,wstring> rels;
    log_prob_parser(wordids, tsentence, tagids, deps, rels);

    // convert output vector of deps and rels to freeling::dep_tree
    lstm2FL(sent, deps, rels);
  }


  ////////////////////////////////////////////////////////////////
  /// Extract dep tree from LSTM results
  ////////////////////////////////////////////////////////////////

  void dep_lstm::lstm2FL(freeling::sentence &fl_sentence,
                            const map<int,int> &deps,  
                            const map<int,wstring> &rels) const {

    // init variables and auxiliary structures
    int num_tokens = fl_sentence.size();
    vector<wstring> labels(num_tokens+1, L"unknown");
    vector<list<int> > sons(num_tokens+1, list<int>());
    list<int> roots;

    // loop over tree arcs and store their information in a suitable way
    for (unsigned i=0; i<fl_sentence.size(); ++i) {
      auto h = deps.find(i);
      int head = (h->second==fl_sentence.size() ? 0 : h->second+1);
      int mod = i+1;
      auto l = rels.find(i);
      wstring syn_label = l->second;
    
      // remember node is a tree root (node whose head is "0")
      // in some treebanks, there are more than one root per sentence, thus, the tagger may produce that.
      if (head==0) roots.push_back(mod);

      // store information to build tree
      sons[head].push_back(mod);
      labels[mod]=syn_label;
    }
  
    // build FreeLing dependency tree, starting from true root 
    map<int,freeling::depnode*> depnods;
    freeling::dep_tree *dt;
    if (roots.size()==1) {
      // only one root, normal tree
      dt = build_dep_tree(*(roots.begin()), sons, labels, depnods, fl_sentence);
    }
    else {
      // multiple roots, create fake root node
      freeling::depnode dn(L"VIRTUAL_ROOT");
      dt = new freeling::dep_tree(dn);
      // hang all subtrees under fake root
      for (list<int>::iterator r=roots.begin(); r!=roots.end(); r++) {
        freeling::dep_tree *st=build_dep_tree(*r, sons, labels, depnods, fl_sentence);
        dt->hang_child(*st);
      }
    }

    //add the dep_tree to FreeLing sentence
    fl_sentence.set_dep_tree(*dt, fl_sentence.get_best_seq());
  }


  ////////////////////////////////////////////////////////////////
  /// build dep_tree
  ////////////////////////////////////////////////////////////////

  freeling::dep_tree* dep_lstm::build_dep_tree(int node_id, const vector<list<int> > &sons, 
                                                  const vector<wstring> &labels,
                                                  map<int,freeling::depnode*> &depnods,
                                                  freeling::sentence &fl_sentence) const {
  
    //  Get function label
    wstring str_label = labels[node_id];

    // create node root of current subtree
    freeling::depnode dn(str_label);
    dn.set_word(fl_sentence[node_id-1]);

    // if a constituent tree is available, set link to constituent headed by this word
    if (fl_sentence.is_parsed()) {
      freeling::parse_tree::iterator pt=fl_sentence.get_parse_tree().get_node_by_pos(node_id-1);
      while (not pt.is_root() and pt->is_head()) pt = pt.get_parent();
      dn.set_link(pt);
    }
    // create current subtree
    freeling::dep_tree *dt = new freeling::dep_tree(dn);
    depnods.insert(make_pair(node_id,&(*(dt->begin()))));

    // recurse into children to build subtrees, and hang them under current node
    for (list<int>::const_iterator son=sons[node_id].begin(); son!=sons[node_id].end(); ++son) {
      freeling::dep_tree *dt_son=build_dep_tree(*son, sons, labels, depnods, fl_sentence);
      dt->hang_child(*dt_son);
    }

    return dt;
  }

} // namespace
