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

#ifndef _LANGUAGE
#define _LANGUAGE

#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>

#include "freeling/regexp.h"
#include "freeling/windll.h"
#include "freeling/tree.h"
#include "freeling/morfo/semgraph.h"

namespace freeling {

  class word; // predeclaration

  ////////////////////////////////////////////////////////////////
  ///   Class analysis stores a possible reading (lemma, PoS, probability, distance)
  ///   for a word.
  ////////////////////////////////////////////////////////////////

  class WINDLL analysis {

  private:
    /// lemma
    std::wstring lemma;
    /// PoS tag
    std::wstring tag;
    /// probability of that lemma-tag given the word
    double prob;
    /// distance from a added analysis from corrector to the original word
    double distance;
    ///  list of possible senses for that analysis, along with their rank as attributed by the ukb PPR algorithm
    std::list<std::pair<std::wstring,double> > senses;
    /// information to retokenize the word after tagging if this analysis is selected
    std::list<word> retok;

    // store which sequences --among the kbest proposed by 
    // the tagger-- contain this analysis
    std::set<int> selected_kbest;

  public:
    /// user-managed data, we just store it.
    std::vector<std::wstring> user;

    /// constructor
    analysis();
    /// constructor
    analysis(const std::wstring &, const std::wstring &);
    /// assignment
    analysis& operator=(const analysis&);

    void init(const std::wstring &l, const std::wstring &t);
    void set_lemma(const std::wstring &);
    void set_tag(const std::wstring &);
    void set_prob(double);
    void set_distance(double);
    void set_retokenizable(const std::list<word> &);

    bool has_prob() const;
    bool has_distance() const;
    const std::wstring& get_lemma() const;
    const std::wstring& get_tag() const;
    double get_prob() const;
    double get_distance() const;
    bool is_retokenizable() const;
    std::list<word>& get_retokenizable();
    const std::list<word>& get_retokenizable() const;

    const std::list<std::pair<std::wstring,double> > & get_senses() const;
    std::list<std::pair<std::wstring,double> > & get_senses();
    void set_senses(const std::list<std::pair<std::wstring,double> > &);
    // useful for java API
    std::wstring get_senses_string() const;

    // get the largest kbest sequence index the analysis is selected in.
    int max_kbest() const;
    // find out whether the analysis is selected in the tagger k-th best sequence
    bool is_selected(int k=0) const;
    // mark this analysis as selected in k-th best sequence
    void mark_selected(int k=0);
    // unmark this analysis as selected in k-th best sequence
    void unmark_selected(int k=0);

    /// Comparison to sort analysis by *decreasing* probability
    bool operator>(const analysis &) const;
    /// Comparison to sort analysis by *increasing* probability
    bool operator<(const analysis &) const;
    /// Comparison (to please MSVC)
    bool operator==(const analysis &) const;
  };

  
  
  ////////////////////////////////////////////////////////////////
  ///   Class alternative stores all info related to a word
  ///  alternative: form, edit distance
  ////////////////////////////////////////////////////////////////

  class WINDLL alternative {
  private:
    // lexical form
    std::wstring form;
    // edition distance
    int distance;
    // probability based on edition distance
    float probability;
    // store which sequences --among the kbest proposed by 
    // the corrector module-- contain this alternative
    std::set<int> kbest;
  
  public:
    /// constructor
    alternative();
    /// constructor
    alternative(const std::wstring &);
    /// constructor
    alternative(const std::wstring &, const int);
    /// Copy constructor
    alternative(const alternative &);
    /// assignment
    alternative& operator=(const alternative&);
    /// comparison
    bool operator==(const alternative&) const;

    /// Get form of the alternative
    std::wstring get_form() const;
    /// Get edit distance
    int get_distance() const;
    /// Get edit distance probability
    float get_probability() const;
    /// Whether the alternative is selected in the kbest path or not
    bool is_selected(int k = 1) const;
    /// Clear the kbest selections
    void clear_selections();
    /// Add a kbest selection
    void add_selection(int);
    
    /// Set alternative form
    void set_form(const std::wstring &);
    /// Set alternative distance
    void set_distance(int);
    /// Set alternative probability
    void set_probability(float);
  };
  
  ////////////////////////////////////////////////////////////////
  ///   Class word stores all info related to a word:
  ///  form, list of analysis, list of tokens (if multiword).
  ////////////////////////////////////////////////////////////////

  class WINDLL word : public std::list<analysis> {
  private:
    /// lexical form
    std::wstring form;
    /// lexical form, lowercased
    std::wstring lc_form;
    /// phonetic form
    std::wstring ph_form;
    /// empty list if not a multiword
    std::list<word> multiword;
    /// whether the multiword presents segmentantion ambiguity (i.e. could not be a mw)
    bool ambiguous_mw;
    /// alternative forms provided by orthographic or phonetic SED
    std::list<alternative> alternatives;
    /// token span
    unsigned long start, finish;
    /// word morphological shouldn't be further modified
    bool locked_analysis;
    /// word shouldn't be considered to be part of a multiword
    bool locked_multiwords;
    /// clone word (used by assignment/copy constructors)
    void clone(const word &);
    /// position of word in the sentence (count from 0)
    size_t position;

    // mask storing which maco modules analyzed this word.
    unsigned analyzed_by;
 
    /// Values for word::iterator types
    static const int SELECTED=0;
    static const int UNSELECTED=1;
    static const int ALL=2;
    static const std::wstring NOT_FOUND;

  public:
    // morphological modules that may add analysis to the word
    typedef enum  { USERMAP=0x0001,    NUMBERS=0x0002,    PUNCTUATION=0x0004,   DATES=0x0008, 
                    DICTIONARY=0x0010, AFFIXES=0x0020,    COMPOUNDS=0x0040,     MULTIWORDS=0x0080, 
                    NER=0x0100,        QUANTITIES=0x0200, PROBABILITIES=0x0400, GUESSER=0x0800 } Modules;

    // predeclarations
    class iterator; 
    class const_iterator; 

    /// user-managed data, we just store it.
    std::vector<std::wstring> user;

    /// constructor
    word();
    /// constructor
    word(const std::wstring &);
    /// constructor
    word(const std::wstring &, const std::list<word> &);
    /// constructor
    word(const std::wstring &, const std::list<analysis> &, const std::list<word> &);
    /// Copy constructor
    word(const word &);
    /// assignment
    word& operator=(const word&);

    /// copy analysis from another word
    void copy_analysis(const word &);
    /// Get the number of selected analysis
    int get_n_selected(int k=0) const;
    /// get the number of unselected analysis
    int get_n_unselected(int k=0) const;
    /// true iff the word is a multiword compound
    bool is_multiword() const;
    /// true iff the word is a multiword marked as ambiguous
    bool is_ambiguous_mw() const;
    /// set mw ambiguity status
    void set_ambiguous_mw(bool);
    /// get number of words in compound
    int get_n_words_mw() const;
    /// get word objects that compound the multiword
    const std::list<word>& get_words_mw() const;
    /// get word form
    const std::wstring& get_form() const;
    /// Get word form, lowercased.
    const std::wstring& get_lc_form() const;
    /// Get word phonetic form
    const std::wstring& get_ph_form() const;
    /// Get an iterator to the first selected analysis
    word::iterator selected_begin(int k=0);
    /// Get an iterator to the first selected analysis
    word::const_iterator selected_begin(int k=0) const;
    /// Get an iterator to the end of selected analysis list
    word::iterator selected_end(int k=0);
    /// Get an iterator to the end of selected analysis list
    word::const_iterator selected_end(int k=0) const;
    /// Get an iterator to the first unselected analysis
    word::iterator unselected_begin(int k=0);
    /// Get an iterator to the first unselected analysis
    word::const_iterator unselected_begin(int k=0) const;
    /// Get an iterator to the end of unselected analysis list
    word::iterator unselected_end(int k=0);
    /// Get an iterator to the end of unselected analysis list
    word::const_iterator unselected_end(int k=0) const;
    /// Get how many kbest tags the word has
    unsigned int num_kbest() const;
    /// get lemma for the selected analysis in list
    const std::wstring& get_lemma(int k=0) const;
    /// get tag for the selected analysis
    const std::wstring& get_tag(int k=0) const;

    /// get sense list for the selected analysis
    const std::list<std::pair<std::wstring,double> >& get_senses(int k=0) const;
    std::list<std::pair<std::wstring,double> >& get_senses(int k=0);
    // useful for java API
    std::wstring get_senses_string(int k=0) const;
    /// set sense list for the selected analysis
    void set_senses(const std::list<std::pair<std::wstring,double> > &, int k=0);

    /// get token span.
    unsigned long get_span_start() const;
    unsigned long get_span_finish() const;

    /// check if there is any retokenizable analysis
    bool has_retokenizable() const;
    /// mark word as having definitive analysis
    void lock_analysis();
    /// unmark word as having definitive analysis
    void unlock_analysis();
    /// check if word is marked as having definitive analysis
    bool is_locked_analysis() const;
    /// mark word as non-multiwordable
    void lock_multiwords();
    /// unmark word as non-multiwordable
    void unlock_multiwords();
    /// check if word is marked as not being multiwordable
    bool is_locked_multiwords() const;

    /// control which maco modules added analysis to this word
    void set_analyzed_by(unsigned);
    bool is_analyzed_by(unsigned) const;
    unsigned get_analyzed_by() const;

    /// add an alternative to the alternatives list
    void add_alternative(const alternative &);
    /// add an alternative to the alternatives list (wstring, int)
    void add_alternative(const std::wstring &, int);
    /// replace alternatives list with list given
    void set_alternatives(const std::list<std::pair<std::wstring,int> > &);
    /// clear alternatives list
    void clear_alternatives();
    /// find out if the speller checked alternatives
    bool has_alternatives() const;
    /// get alternatives list const &
    const std::list<alternative>& get_alternatives() const;
    /// get alternatives list &
    std::list<alternative>& get_alternatives();
    /// get alternatives begin iterator
    std::list<alternative>::iterator alternatives_begin();
    /// get alternatives end iterator
    std::list<alternative>::iterator alternatives_end();
    /// get alternatives begin iterator
    std::list<alternative>::const_iterator alternatives_begin() const;
    /// get alternatives end iterator
    std::list<alternative>::const_iterator alternatives_end() const;

    /// add one analysis to current analysis list  (no duplicate check!)
    void add_analysis(const analysis &);
    /// set analysis list to one single analysis, overwriting current values
    void set_analysis(const analysis &);
    /// set analysis list, overwriting current values
    void set_analysis(const std::list<analysis> &);
    /// set word form
    void set_form(const std::wstring &);
    /// Set word phonetic form
    void set_ph_form(const std::wstring &);
    /// set token span
    void set_span(unsigned long, unsigned long);

    // get/set word position
    void set_position(size_t);
    size_t get_position() const;

    /// look for an analysis with a tag matching given regexp
    bool find_tag_match(const freeling::regexp &) const;

    /// get number of analysis in current list
    int get_n_analysis() const;
    /// empty the list of selected analysis
    void unselect_all_analysis(int k=0);
    /// mark all analysisi as selected
    void select_all_analysis(int k=0);
    /// add the given analysis to selected list.
    void select_analysis(word::iterator, int k=0);
    /// remove the given analysis from selected list.
    void unselect_analysis(word::iterator, int k=0);
    /// get list of analysis (useful for perl API)
    std::list<analysis> get_analysis() const;
    /// get begin iterator to analysis list (useful for perl/java API)
    word::iterator analysis_begin();
    word::const_iterator analysis_begin() const;
    /// get end iterator to analysis list (useful for perl/java API)
    word::iterator analysis_end();
    word::const_iterator analysis_end() const;

    /// iterator over word analysis (either all, only selected, only unselected)
    class WINDLL iterator : public std::list<analysis>::iterator {
      friend class word::const_iterator;
    private:
      /// Iterator range begin
      std::list<analysis>::iterator ibeg;
      /// Iterator range end
      std::list<analysis>::iterator iend;
      /// Iterator type (ALL,SELECTED,UNSELECTED)
      int type;
      /// Which of k-best sequences proposed by the tagger is the iterator associated with.
      int kbest;

    public:
      /// empty constructor
      iterator();
      /// copy
      iterator(const word::iterator &);
      /// Constructor from std::list iterator
      iterator(const std::list<analysis>::iterator &);
      /// Constructor for filtered iterators (selected/unselected)
      iterator(const std::list<analysis>::iterator &, 
               const std::list<analysis>::iterator &, 
               const std::list<analysis>::iterator &, int,int k=0);  
      /// Generic increment, for all cases
      iterator& operator++();
      iterator operator++(int);
    };
  
    /// const_iterator over word analysis (either all, only selected, only unselected)
    class WINDLL const_iterator : public std::list<analysis>::const_iterator {
    private:
      /// Iterator range begin
      std::list<analysis>::const_iterator ibeg;
      /// Iterator range end
      std::list<analysis>::const_iterator iend;
      /// Iterator type (ALL,SELECTED,UNSELECTED)
      int type;
      /// Which of k-best sequences proposed by the tagger is the iterator associated with.
      int kbest;

    public:
      /// empty constructor
      const_iterator();
      /// copy
      const_iterator(const word::const_iterator &);
      /// copy from nonconst iterator
      const_iterator(const word::iterator &);
      /// Constructor from std::list iterator
      const_iterator(const std::list<analysis>::const_iterator &);
      /// Constructor from std::list nonconst iterator
      const_iterator(const std::list<analysis>::iterator &);
      /// Constructor for filtered iterators (selected/unselected)
      const_iterator(const std::list<analysis>::const_iterator &,
                     const std::list<analysis>::const_iterator &, 
                     const std::list<analysis>::const_iterator &, int, int k=0);
      /// Generic increment, for all cases
      const_iterator& operator++();  
      const_iterator operator++(int);  
    };

  };



  ////////////////////////////////////////////////////////////////
  ///   Class node stores nodes of a parse_tree
  ///  Each node in the tree is either a label (intermediate node)
  ///  or a word (leaf node)
  ////////////////////////////////////////////////////////////////

  class WINDLL node {
  protected:
    /// node identifier
    std::wstring nodeid;
    /// is the node the head of the rule?
    bool head;
    /// is the node the root of a chunk? which?
    int chunk;
    /// node label
    std::wstring label;
    /// sentence word related to the node (if leaf)
    word * w;

  public:
    /// user-managed data, we just store it.
    std::vector<std::wstring> user;
    /// constructors
    node();
    node(const std::wstring &);
    /// get node identifier
    const std::wstring& get_node_id() const;
    /// set node identifier
    void set_node_id(const std::wstring &);
    /// get node label
    const std::wstring& get_label() const;
    /// find out whether the node has an associated word
    bool has_word() const;
    /// get node word
    const word & get_word() const;
    /// get reference to node word
    word & get_word();
    /// set node label
    void set_label(const std::wstring &);
    /// set node word
    void set_word(word &);
    /// find out whether node is a head
    bool is_head() const;
    /// set whether node is a head
    void set_head(const bool);
    /// find out whether node is a chunk
    bool is_chunk() const;
    /// set position of the chunk in the sentence
    void set_chunk(const int);
    /// get position of the chunk in the sentence
    int get_chunk_ord() const;

  };

  ////////////////////////////////////////////////////////////////
  ///   Class parse tree is used to store the results of parsing
  ////////////////////////////////////////////////////////////////

  class WINDLL parse_tree : public tree<node> {
  private:
    // access nodes by id
    std::map<std::wstring,parse_tree::iterator> node_index;
    // acces leaf nodes by word position
    std::vector<parse_tree::iterator> word_index;

  public:
    parse_tree();
    parse_tree(parse_tree::const_iterator p);
    parse_tree(const node &);

    /// assign an id to each node and build index
    void build_node_index(const std::wstring &);
    /// rebuild index maintaining node id's
    void rebuild_node_index();
    /// access the node with given id
    parse_tree::const_iterator get_node_by_id(const std::wstring &) const;
    /// access the node by word position
    parse_tree::const_iterator get_node_by_pos(size_t) const;
    /// access the node with given id
    parse_tree::iterator get_node_by_id(const std::wstring &);
    /// access the node by word position
    parse_tree::iterator get_node_by_pos(size_t);

    /// obtain a reference to head word of a parse_tree
    static const word& get_head_word(parse_tree::const_iterator);
    /// obtain the head label of a parse_tree
    static const std::wstring& get_head_label(parse_tree::const_iterator);
    /// obtain position of head word of a parse_tree (or -1 if no head)
    static int get_head_position(parse_tree::const_iterator);

    /// C-commands
    ///    1 - pt1 does not dominate pt2
    ///    2 - pt2 does not dominate pt1
    ///    3 - The first branching node that dominates pt1 also dominates pt2
    static bool C_commands(parse_tree::const_iterator, parse_tree::const_iterator);
    
    /// print a parse tree (debugging purposes mainly)
    static void PrintTree(parse_tree::const_iterator n, int k, int depth);

    /// get lowest tree node subsuming whole word span i..j
    parse_tree::const_iterator get_subsuming_node(size_t i, size_t j) const;
    parse_tree::iterator get_subsuming_node(size_t i, size_t j);
    /// get tree node subsuming longest span i..k such that k<=j
    parse_tree::const_iterator get_left_subsuming_node(size_t i, size_t j) const;
    parse_tree::iterator get_left_subsuming_node(size_t i, size_t j);
    /// get tree node subsuming longest span k..j such that i<=k
    parse_tree::const_iterator get_right_subsuming_node(size_t i, size_t j) const;
    parse_tree::iterator get_right_subsuming_node(size_t i, size_t j);
  };


  ////////////////////////////////////////////////////////////////
  /// class denode stores nodes of a dependency tree and
  ///  parse tree <-> deptree relations
  ////////////////////////////////////////////////////////////////

  class WINDLL depnode : public node {

  private:
    /// corresponding node of the parse tree in the same sentence.
    parse_tree::iterator link;

  public:
    depnode();
    depnode(const std::wstring &);
    depnode(const node &);

    /// set link to corresponding node in the parse tree
    void set_link(const parse_tree::iterator);
    /// get link to corresponding node in the parse tree
    parse_tree::iterator get_link();
    parse_tree::const_iterator get_link() const;
  };

  ////////////////////////////////////////////////////////////////
  /// class dep_tree stores a dependency tree
  ////////////////////////////////////////////////////////////////

  class WINDLL dep_tree :  public tree<depnode> {

  private:
    // acces nodes by word position
    std::vector<dep_tree::iterator> word_index;

  public:
    dep_tree();
    dep_tree(const depnode &);

    /// get depnode corresponding to word in given position
    dep_tree::const_iterator get_node_by_pos(size_t) const;
    /// get depnode corresponding to word in given position
    dep_tree::iterator get_node_by_pos(size_t);
    /// rebuild index maintaining words positions
    void rebuild_node_index();

    /// obtain position of first/last word in the span subsumed by a dep_tree
    static size_t get_first_word(dep_tree::const_iterator);
    static size_t get_last_word(dep_tree::const_iterator);

    /// print a dependency tree (debugging purposes mainly)
    static void PrintDepTree(dep_tree::const_iterator n, int depth);
  };



  ////////////////////////////////////////////////////////////////
  /// Virtual class to store the processing state of a sentence.
  /// Each processor will define a derived class with needed contents,
  /// and store it in the sentence being processed.
  ////////////////////////////////////////////////////////////////

  class processor_status {
  public:
    processor_status();
    virtual ~processor_status() {};
  };



  ////////////////////////////////////////////////////////////////
  ///   Class argument stores information about a predicate argument
  ////////////////////////////////////////////////////////////////

  class WINDLL argument  {
    private:
      int position;
      std::wstring role;

    public:
      static const std::wstring EMPTY_ROLE;

      argument();
      ~argument();
      argument(int p, const std::wstring &r);
      
      /// getters
      int get_position() const;
      const std::wstring& get_role() const;
  };

  ////////////////////////////////////////////////////////////////
  ///   Class predicate stores a predicate and its arguments
  ////////////////////////////////////////////////////////////////

  class WINDLL predicate : public std::vector<argument> {

    private:
      // index to find arguments by word position
      std::map<int,int> arg_index;
      // position of the predicate head in the sentence
      int position;
      // propbank sense of the predicate
      std::wstring sense;

    public:

      predicate();
      ~predicate();
      predicate(int p, const std::wstring &s);
      /// Copy constructor
      predicate(const predicate &);
      /// assignment
      predicate& operator=(const predicate&);

      /// get the propbank sense of the predicate
      const std::wstring& get_sense() const;
      /// get the position of the predicate head
      int get_position() const;
      /// check whether word in position p is an argument to the predicate
      bool has_argument(int p) const;
      /// add new argument to the predicate
      void add_argument(int p, const std::wstring &r);
      /// get access to an argument by word position
      const argument & get_argument_by_pos(int p) const;

  };


  ////////////////////////////////////////////////////////////////
  ///   Class sentence is just a list of words that someone
  /// (the splitter) has validated it as a complete sentence.
  /// It may include a parse tree.
  ////////////////////////////////////////////////////////////////

  class WINDLL sentence : public std::list<word> {

  public:
    typedef std::vector<predicate> predicates;

  private:
    // sentence identifier, in case user application wants to set it.
    std::wstring sent_id;
    // vector with pointers to sentence words, for fast access by position
    std::vector<word*> wpos; 
    // remember if it is PoS tagged
    bool tagged;
    // parse tree (if sentence parsed)
    std::map<int,parse_tree> pts;
    // dependencey tree (if sentence dep. parsed)
    std::map<int,dep_tree> dts;
    // clone sentence (used by assignment/copy constructors)
    void clone(const sentence &);
    // stack processing status for processor currently analyzing the sentence
    // (there might be a hierarchy of embeeded processors, thus the stack)
    std::list<processor_status*> status;
    // store map of predicates by position
    predicates preds;
    // index to access predicates by word position
    std::map<int,int> pred_index;

  public:

    sentence();
    ~sentence();
    sentence(const std::list<word>&);
    /// Copy constructor
    sentence(const sentence &);
    /// assignment
    sentence& operator=(const sentence&);
    /// positional access to a word
    const word& operator[](size_t) const;
    word& operator[](size_t);
    /// find out how many kbest sequences the tagger computed
    unsigned int num_kbest() const;
    /// add a word to the sentence
    void push_back(const word &);
    /// rebuild word positional index
    void rebuild_word_index();
     
    void clear();

    void set_sentence_id(const std::wstring &);
    const std::wstring& get_sentence_id() const;

    void set_is_tagged(bool);
    bool is_tagged() const;

    void set_parse_tree(const parse_tree &, int k=0);
    parse_tree & get_parse_tree(int k=0);
    const parse_tree & get_parse_tree(int k=0) const;
    bool is_parsed() const;

    void set_dep_tree(const dep_tree &, int k=0);
    dep_tree & get_dep_tree(int k=0);
    const dep_tree & get_dep_tree(int k=0) const;
    bool is_dep_parsed() const;

    /// get status at the top of stack
    processor_status* get_processing_status();
    const processor_status* get_processing_status() const;
    /// push status into the stack
    void set_processing_status(processor_status *);
    /// pop top status, and free it
    void clear_processing_status();

    /// get word list (useful for perl API)
    std::vector<word> get_words() const;
    /// get iterators to word list (useful for perl/java API)
    sentence::iterator words_begin();
    sentence::const_iterator words_begin() const;
    sentence::iterator words_end();
    sentence::const_iterator words_end() const;

    // obtain iterator to a word given its position
    sentence::iterator get_word_iterator(const word &w);
    sentence::const_iterator get_word_iterator(const word &w) const;

    void add_predicate(const predicate &pr);
    /// see if word in given position is a predicate
    bool is_predicate(int p) const;
    /// get predicate number n for word in position p
    int get_predicate_number(int p) const;
    /// get word position for predicate number n
    int get_predicate_position(int n) const;
    /// get predicate for word at position p
    const predicate& get_predicate_by_pos(int n) const;
    /// get predicate number n
    const predicate& get_predicate_by_number(int n) const;
    /// get predicates of the sentence (e.g. to iterate over them)
    const predicates & get_predicates() const;
  };

  ////////////////////////////////////////////////////////////////
  ///   Class paragraph is just a list of sentences that someone
  ///  has validated it as a paragraph.
  ////////////////////////////////////////////////////////////////

  class WINDLL paragraph : public std::list<sentence> {
  private:
    std::wstring par_id;
  public:
    paragraph();
    paragraph(const std::list<sentence> &x);
    void set_paragraph_id(const std::wstring &);
    const std::wstring & get_paragraph_id() const;
  };

  //////////////////////////////////////////////////////////////////            
  /// Class mention is a node in the parse tree, as well as the sequence of tokens
  /// subsumed by the node.
  /// (It is assumed that a depnode corresponds to a parse tree node)
  //////////////////////////////////////////////////////////////////           
  
  class WINDLL mention {

  public:
    typedef enum {PROPER_NOUN, PRONOUN, NOUN_PHRASE, COMPOSITE, VERB_PHRASE} mentionType;
    // NE type NOTPER is the supertype of ORG, GEO and OTHER
    typedef enum {PER, MALE, FEMALE, NOTPER, ORG, GEO, TIME, DATE, MONEY, OTHER} SEMmentionType;

  private:

    /// the id of the mention
    int id;
    /// the id of the mention as string
    std::wstring sid;
    /// the type of the mention
    mentionType mType;
    /// the number of sentence in which the mention occurs
    int sent;
    /// the sentence of the mention
    paragraph::const_iterator s;
    /// is the first mention in the sentence?
    bool initial;
    /// not subsumed by another mention ?
    bool maximal;
    /// the node_tree of the mention
    parse_tree::const_iterator ptree;
    dep_tree::const_iterator dtree;
    /// the starting position within the sentence
    int posBegin;
    /// the ending position
    int posEnd;
    /// the starting iterator within the sentence
    sentence::const_iterator itBegin;
    /// the ending iterator
    sentence::const_iterator itEnd;
    /// the head iterator
    sentence::const_iterator itHead;
    /// the coreference chain to which the mention belongs
    int chain;

    /// private functions

    /// clone a mention
    void clone(const mention &);
    /// recursively fills tokens and tags;
    /// last parameters return posEnd+1 and itEnd+1; 
    static void set_tokens(parse_tree::const_iterator, int&, sentence::const_iterator&);
    /// obtain the node that maximize the subsumtion of the sequence defined by the first and second iterators
    /// third parameter is the root 
    /// last two parameters return the iterator of the last word within the subsumtion and the tree node subsuming the sequence
    static void set_iterators(sentence::const_iterator, sentence::const_iterator, const parse_tree&, sentence::const_iterator&, parse_tree::const_iterator&);

  public:

    /// constructor from a parse_tree
    mention(int, int, paragraph::const_iterator, parse_tree::const_iterator, int, sentence::const_iterator);
    /// constructor from a dep_tree
    mention(int i, int ns, paragraph::const_iterator ps, dep_tree::const_iterator dt, int begin=-1, int end=-1);
    /// constructor from start/end word iterators
    mention(int, int, paragraph::const_iterator, sentence::const_iterator, sentence::const_iterator);
    /// Copy constructor
    mention(const mention &);
    /// assignment
    mention& operator=(const mention&);

    /// True when the mention starts before m in the document
    bool operator<(const mention &m) const;

    /// setters
    void set_id(int);
    void set_type(mentionType);
    void set_initial(bool);
    void set_group(int);
    void set_maximal(bool b);

    /// getters
    int get_id() const;
    std::wstring get_str_id() const;
    int get_n_sentence() const;
    paragraph::const_iterator get_sentence() const;
    int get_pos_begin() const;
    int get_pos_end() const;
    sentence::const_iterator get_it_begin() const;
    sentence::const_iterator get_it_end() const;
    sentence::const_iterator get_it_head() const;
    mentionType get_type() const;
    int get_group() const;
    bool is_type(mentionType) const;
    bool is_initial() const;
    bool is_maximal() const;
    parse_tree::const_iterator get_ptree() const;
    dep_tree::const_iterator get_dtree() const;
    const word& get_head() const;
    std::wstring value(int lc=0) const;    
  };



  ////////////////////////////////////////////////////////////////
  ///   Class document is a list of paragraphs. It may have additional
  ///  information (such as title)
  ////////////////////////////////////////////////////////////////

  class WINDLL document : public std::list<paragraph> {

  private:
    static const size_t DIM = 500;

    paragraph title;

    // potentially coreferring mentions
    std::vector<mention> mentions;
    // set of mention_ids belonging to the same group
    std::multimap<int,int> group2mentions;
    // ids  (non correlative) for existing groups
    std::list<int> groups;

    // semantic graph
    semgraph::semantic_graph sem_graph;

  public:
    document();
    ~document();

    bool is_parsed() const;
    bool is_dep_parsed() const;

    /// Adds one mention to the doc (the mention is already included in a group of coreferents)
    void add_mention(const mention &m);

    /// Adds node2 to the group of node1
    ///void add_positive(const std::wstring &node1, const std::wstring &node2);
    /// Gets the id of the coreference group of the node
    ///int get_coref_group(const std::wstring&) const;
    /// Gets all the nodes in a coreference group id
    ///std::list<std::wstring> get_coref_nodes(int) const;

    // count number of words in the document
    int get_num_words() const;

    /// Gets the number of groups found
    int get_num_groups() const;
    /// get list of ids of existing groups
    const std::list<int> & get_groups() const;

    /// Gets an iterator pointing to the beginning of the mentions 
    std::vector<mention>::const_iterator begin_mentions() const;
    std::vector<mention>::iterator begin_mentions();
    /// Gets an iterator pointing to the end of the mentions 
    std::vector<mention>::const_iterator end_mentions() const;
    std::vector<mention>::iterator end_mentions();

    /// get reference to i-th mention
    const mention& get_mention(int) const;

    /// Gets all the nodes in a coreference group
    ///std::list<int> get_coref_nodes(int) const;

    /// Gets all the mentions' ids in the ith coreference group found
    std::list<int> get_coref_id_mentions(int) const;

    /// Returns if two nodes are in the same coreference group
    /// bool is_coref(const std::wstring &, const std::wstring &) const;

    const semgraph::semantic_graph & get_semantic_graph() const;
    semgraph::semantic_graph & get_semantic_graph();
  };

} // namespace

#endif

