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

#include "freeling/morfo/language.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///   Class analysis stores a possible reading (lemma, PoS, probability, distance)
  ///   for a word.
  ////////////////////////////////////////////////////////////////

  /// Create empty analysis
  analysis::analysis() {}
  /// Create a new analysis with provided data.
  analysis::analysis(const wstring &l, const wstring &p) {lemma=l; tag=p; prob= -1.0; distance= -1.0;}
  /// Assignment
  analysis& analysis::operator=(const analysis &a) {
    if(this!=&a) {
      lemma=a.lemma; tag=a.tag; prob=a.prob; distance=a.distance;
      senses=a.senses; retok=a.retok; user=a.user;
      selected_kbest=a.selected_kbest;
    }
    return *this;
  }

  // Set lemma, tag and default properties
  void analysis::init(const std::wstring &l, const std::wstring &t)
  {
    this->lemma=l; this->tag=t; this->prob=-1.0; this->distance= -1.0;
    user.clear();
    selected_kbest.clear();
    retok.clear();
    senses.clear();
  }
  /// Set lemma for analysis.
  void analysis::set_lemma(const wstring &l) {lemma=l;}
  /// Set PoS tag for analysis
  void analysis::set_tag(const wstring &p) {tag=p;}
  /// Set probability for analysis
  void analysis::set_prob(double p) { prob=p;}
  /// Set distance for analysis
  void analysis::set_distance(double d) { distance=d;}
  /// Set retokenization info for analysis
  void analysis::set_retokenizable(const list<word> &lw) {retok=lw;}
  /// Check whether probability has been set.
  bool analysis::has_prob() const {return(prob>=0.0);}
  /// Check whether distance has been set.
  bool analysis::has_distance() const {return(distance>=0.0);}
  /// Get lemma value for analysis.
  const wstring& analysis::get_lemma() const {return(lemma);}
  /// Get probability value for analysis (-1 if not set).
  double analysis::get_prob() const {return(prob);}
  /// Get distance value for analysis (-1 if not set).
  double analysis::get_distance() const {return(distance);}
  /// Find out if the analysis may imply retokenization
  bool analysis::is_retokenizable() const {return(!retok.empty());}
  /// Get retokenization info for analysis.
  list<word>& analysis::get_retokenizable() {return(retok);}
  /// Get retokenization info for analysis.
  const list<word>& analysis::get_retokenizable() const {return(retok);}
  /// Get PoS tag value for analysis.
  const wstring& analysis::get_tag() const {return(tag);}
  /// get analysis sense list
  const list<pair<wstring, double> >& analysis::get_senses() const {return(senses);}
  /// get ref to analysis sense list
  list<pair<wstring, double> >& analysis::get_senses() {return(senses);}
  wstring analysis::get_senses_string() const {return(util::pairlist2wstring(senses,L":",L"/"));}
  /// set analiysis sense list
  void analysis::set_senses(const list<pair<wstring, double> > &ls) {senses=ls;}
  // get the largest kbest sequence index the analysis is selected in.
  int analysis::max_kbest() const { 
    if (selected_kbest.empty()) return 0; 
    else return *(--selected_kbest.end());
  }
  /// find out whether the analysis is selected in the tagger k-th best sequence
  bool analysis::is_selected(int k) const {return selected_kbest.find(k)!=selected_kbest.end();}
  /// mark this analysis as selected in k-th best sequence
  void analysis::mark_selected(int k) {selected_kbest.insert(k);}
  /// unmark this analysis as selected in k-th best sequence
  void analysis::unmark_selected(int k) {selected_kbest.erase(k);}

  /// Comparison to sort analysis by *ascending* probability and ascending alphabetical tag
  bool analysis::operator>(const analysis &a) const {return(prob>a.prob or (prob==a.prob and tag<a.tag));}
  /// Comparison to sort analysis by *descending* probability and ascending alphabetical tag
  bool analysis::operator<(const analysis &a) const {return(prob<a.prob or (prob==a.prob and tag<a.tag));}
  /// comparison (just to please MSVC)
  bool analysis::operator==(const analysis &a) const {return(lemma==a.lemma && tag==a.tag);}


  ////////////////////////////////////////////////////////////////
  ///   Class alternative stores all info related to a word
  ///  alternative: form, edit distance
  ////////////////////////////////////////////////////////////////
  
  /// Create an empty alternative
  alternative::alternative() {
    form = L"";
    distance = 0;
    probability = 0.0f;
  }
  /// Create an alternative with form
  alternative::alternative(const std::wstring &f) {
    form = f;
    distance = 0;
    probability = 0.0f;
  }
  /// Create an alternative with form and edit distance
  alternative::alternative(const std::wstring &f, const int d) {
    form = f;
    distance = d;
    probability = 0.0f;
  }
  /// Copy constructor
  alternative::alternative(const alternative &alt) {
    form = alt.form;
    distance = alt.distance;
    probability = alt.probability;
    kbest = std::set<int>(alt.kbest);
  }
  /// assignment
  alternative& alternative::operator=(const alternative& alt) {
    if (this != &alt) {
      form = alt.form;
      distance = alt.distance;
      probability = alt.probability;
      kbest = std::set<int>(alt.kbest);
    }
    return *this;
  }

  /// Get form of the alternative
  std::wstring alternative::get_form() const {return form;}
  /// Get edit distance
  int alternative::get_distance() const {return distance;}
  /// Get edit distance probability
  float alternative::get_probability() const {return probability;}
  /// Whether the alternative is selected in the kbest path or not
  bool alternative::is_selected(int k) const {return (kbest.count(k) == 1);}
  /// Clear the kbest selections
  void alternative::clear_selections() {kbest.clear();}
  /// Add a kbest selection
  void alternative::add_selection(int k) {kbest.insert(k);}
  /// Set alternative form
  void alternative::set_form(const std::wstring &f) {form = f;}
  /// Set alternative distance
  void alternative::set_distance(int d) {distance = d;}
  /// Set alternative probability
  void alternative::set_probability(float p) {probability = p;}
  /// Comparison operator
  bool alternative::operator==(const alternative &alt) const {return (alt.form == form);}
  
  ////////////////////////////////////////////////////////////////
  ///   Class word stores all info related to a word:
  ///  form, list of analysis, list of tokens (if multiword).
  ////////////////////////////////////////////////////////////////

  /// Create an empty new word
  word::word() { 
    form=L"";
    ph_form=L"";
    lc_form=L"";
    analyzed_by=0x0;
    locked_analysis=false; 
    locked_multiwords=false; 
    ambiguous_mw=false;
    position=-1;
    start=-1;
    finish=-1;
  }

  /// Create a new word with given form.
  word::word(const wstring &f) {
    form=f;
    ph_form=L"";
    lc_form=util::lowercase(f);
    analyzed_by=0x0;
    locked_analysis=false; 
    locked_multiwords=false; 
    ambiguous_mw=false;
    position=-1;
    start=-1;
    finish=-1;
  }
  /// Create a new multiword, including given words.
  word::word(const wstring &f, const list<word> &a) {
    form=f; 
    ph_form=L"";
    lc_form=util::lowercase(f);
    multiword=a;
    start=a.front().get_span_start();
    finish=a.back().get_span_finish();
    analyzed_by=0x0;
    locked_analysis=false; 
    locked_multiwords=false; 
    ambiguous_mw=false;
    position=-1;
  }
  /// Create a new multiword, including given words, and with given analysis list.
  word::word(const wstring &f, const list<analysis> &la, const list<word> &a) {
    word::const_iterator i;
    this->clear(); 
    for (i=la.begin(); i!=la.end(); i++) {
      this->push_back(*i);
      this->back().mark_selected();
    }
    form=f; 
    lc_form=util::lowercase(f);
    ph_form=L"";
    multiword=a;
    start=a.front().get_span_start();
    finish=a.back().get_span_finish();
    analyzed_by=0x0;
    locked_analysis=false; 
    locked_multiwords=false; 
    ambiguous_mw=false;
    position=-1;
  }

  /// Clone word
  void word::clone(const word &w) {
    form=w.form; lc_form=w.lc_form;
    ph_form=w.ph_form;
    multiword=w.multiword;
    start=w.start; finish=w.finish;
    analyzed_by=w.analyzed_by;
    locked_analysis=w.locked_analysis;
    locked_multiwords=w.locked_multiwords;
    user=w.user;
    alternatives=w.alternatives;
    copy_analysis(w);
    ambiguous_mw=w.ambiguous_mw;
    position=w.position;
  }

  /// Copy constructor.
  word::word(const word & w) { clone(w);}

  /// Assignment.
  word& word::operator=(const word& w){
    if (this!=&w) clone(w);
    return *this;
  }

  /// Copy analysis list of given word.
  void word::copy_analysis(const word &w) {
    word::const_iterator i;
    // copy all analysis
    this->clear();
    for (i=w.begin(); i!=w.end(); i++) 
      this->push_back(*i);
  }
  /// Set (override) word analysis list with one single analysis
  void word::set_analysis(const analysis &a) {
    this->clear(); 
    this->push_back(a); 
    this->back().mark_selected();
  }

  /// Set (override) word analysis list.
  void word::set_analysis(const list<analysis> &a) {
    list<analysis>::const_iterator i;
    this->clear();
    for (i=a.begin(); i!=a.end(); ++i) {
      this->push_back(*i);
      this->back().mark_selected();
    }
  } 

  /// Add one analysis to word analysis list.
  void word::add_analysis(const analysis &a) {
    this->push_back(a);
    this->back().mark_selected();
  }
  /// Set word form.
  void word::set_form(const wstring &f) {form=f;  lc_form=util::lowercase(f);}
  void word::set_ph_form(const wstring &f) {ph_form=f;}
  /// Set token span.
  void word::set_span(unsigned long s, unsigned long e) {start=s; finish=e;}

  /// check if there is any retokenizable analysis
  bool word::has_retokenizable() const {
    word::const_iterator i;
    bool has=false;
    for (i=this->begin(); i!=this->end() && !has; i++) has=i->is_retokenizable();
    return(has);
  }

  /// control which maco modules added analysis to this word
  void word::set_analyzed_by(unsigned module) { analyzed_by |= module; }
  bool word::is_analyzed_by(unsigned module) const { return (analyzed_by & module) != 0; }
  unsigned word::get_analyzed_by() const { return analyzed_by; }

  /// add an alternative to the alternatives list
  void word::add_alternative(const alternative &alt) { alternatives.push_back(alt); }
  /// add an alternative to the alternatives list (wstring, int)
  void word::add_alternative(const wstring &w, int d) { alternatives.push_back(alternative(w, d)); }
  /// replace alternatives list with list given
  void word::set_alternatives(const list<pair<wstring,int> > &lw) {
    clear_alternatives();
    for (auto it = lw.begin(); it != lw.end(); it++) {
      alternatives.push_back(alternative(it->first, it->second));
    }
  }
  /// clear alternatives list
  void word::clear_alternatives() { alternatives.clear(); }
  /// find out if the speller checked alternatives
  bool word::has_alternatives() const {return (alternatives.size()>0);}
  /// get alternatives list const &
  const list<alternative>& word::get_alternatives() const {return(alternatives);}
  /// get alternatives list &
  list<alternative>& word::get_alternatives() {return(alternatives);}
  /// get alternatives begin iterator
  list<alternative>::iterator word::alternatives_begin() {return alternatives.begin();}
  /// get alternatives end iterator
  list<alternative>::iterator word::alternatives_end() {return alternatives.end();}
  /// get alternatives begin const iterator
  list<alternative>::const_iterator word::alternatives_begin() const {return alternatives.begin();}
  /// get alternatives end const iterator
  list<alternative>::const_iterator word::alternatives_end() const {return alternatives.end();}

  /// mark word as having definitive analysis
  void word::lock_analysis() { locked_analysis=true; }
  /// unmark word as having definitive analysis
  void word::unlock_analysis() { locked_analysis=false; }
  /// check if word is marked as having definitive analysis
  bool word::is_locked_analysis() const { return locked_analysis; }

  /// mark word as non-multiwordable
  void word::lock_multiwords() { locked_multiwords=true; }
  /// unmark word as non-multiwordable
  void word::unlock_multiwords() { locked_multiwords=false; }
  /// check if word is marked as non-multiwordable
  bool word::is_locked_multiwords() const { return locked_multiwords; }

  /// Get length of analysis list.
  int word::get_n_analysis() const {return(this->size());}
  /// get list of analysis (only useful for perl API)
  list<analysis> word::get_analysis() const {return(*this);}
  /// get begin iterator to analysis list.
  word::iterator word::analysis_begin() {return this->begin();}
  word::const_iterator word::analysis_begin() const {return this->begin();}
  /// get end iterator to analysis list.
  word::iterator word::analysis_end() {return this->end();}
  word::const_iterator word::analysis_end() const {return this->end();}
  /// mark all analysis as selected for k-th best sequence
  void word::select_all_analysis(int k) {
    for (word::iterator i=this->begin(); i!=this->end(); i++) i->mark_selected(k);
  }
  /// un mark all analysis as selected for k-th best sequence
  void word::unselect_all_analysis(int k) {
    for (word::iterator i=this->begin(); i!=this->end(); i++) i->unmark_selected(k);
  }
  /// Mark given analysis as selected.
  void word::select_analysis(word::iterator tag, int k) { tag->mark_selected(k); }
  /// Unmark given analysis as selected.
  void word::unselect_analysis(word::iterator tag, int k) { tag->unmark_selected(k); }
  /// Get the number of selected analysis
  int word::get_n_selected(int k) const {
    int n=0;
    for (list<analysis>::const_iterator i=this->begin(); i!=this->end(); i++)
      if (i->is_selected(k)) n++;
    return(n);
  }
  /// Get the number of unselected analysis
  int word::get_n_unselected(int k) const {return(this->size() - this->get_n_selected(k));}
  /// Check whether the word is a compound.
  bool word::is_multiword() const {return(!multiword.empty());}
  /// Check whether the word is an ambiguous multiword
  bool word::is_ambiguous_mw() const {return(ambiguous_mw);}
  /// Set mw ambiguity status
  void word::set_ambiguous_mw(bool a) {ambiguous_mw=a;}
  /// Get number of words in compound.
  int word::get_n_words_mw() const {return(multiword.size());}
  /// Get list of words in compound.
  const list<word>& word::get_words_mw() const {return(multiword);}
  /// Get word form.
  const wstring& word::get_form() const {return(form);}
  /// Get word form, lowercased.
  const wstring& word::get_lc_form() const {return(lc_form);}
  ///Get word phonetic form.
  const wstring& word::get_ph_form() const {return(ph_form);}
  /// Get the first selected analysis iterator
  word::iterator word::selected_begin(int k) {
    list<analysis>::iterator p;
    p=this->begin();
    while (p!=this->end() and not p->is_selected(k)) p++;
    return word::iterator(this->begin(),this->end(),p,SELECTED,k);
  }
  /// Get the first selected analysis iterator
  word::const_iterator word::selected_begin(int k) const {
    list<analysis>::const_iterator p;
    p=this->begin();
    while (p!=this->end() and not p->is_selected(k)) p++;
    return word::const_iterator(this->begin(),this->end(),p,SELECTED,k);
  }
  /// Get the end of selected analysis list
  word::iterator word::selected_end(int k) {return(this->end());}
  /// Get the end of selected analysis list
  word::const_iterator word::selected_end(int k) const {return(this->end());}
  /// Get the first unselected analysis iterator
  word::iterator word::unselected_begin(int k) {
    list<analysis>::iterator p;
    p=this->begin();
    while (p!=this->end() and p->is_selected(k)) p++;
    return word::iterator(this->begin(),this->end(),p,UNSELECTED,k);
  }
  /// Get the first unselected analysis iterator
  word::const_iterator word::unselected_begin(int k) const {
    list<analysis>::const_iterator p;
    p=this->begin();
    while (p!=this->end() and p->is_selected(k)) p++;
    return word::const_iterator(this->begin(),this->end(),p,UNSELECTED,k);
  }
  /// Get the end of unselected analysis list
  word::iterator word::unselected_end(int k) {return(this->end());}
  /// Get the end of unselected analysis list
  word::const_iterator word::unselected_end(int k) const {return(this->end());}
  /// Get how many kbest tags the word stores
  unsigned int word::num_kbest() const {
    unsigned int mx= 0;
    for (list<analysis>::const_iterator a=this->begin(); a!=this->end(); a++) {
      unsigned int y = (unsigned int)(a->max_kbest()+1);
      mx = (y>mx? y : mx);
    }
    return mx;
  }

  const wstring word::NOT_FOUND=L"";
  /// Get lemma for the selected analysis in list.
  const wstring& word::get_lemma(int k) const {
    return (this->num_kbest()>k ? selected_begin(k)->get_lemma() : NOT_FOUND);
  }
  /// Get PoS tag for the selected analysis in list.
  const wstring& word::get_tag(int k) const {
    return (this->num_kbest()>k ? selected_begin(k)->get_tag() : NOT_FOUND);
  }
  /// get reference to sense list for the selected analysis
  const list<pair<wstring,double> > & word::get_senses(int k) const {
    return(selected_begin(k)->get_senses());
  }
  /// get reference to sense list for the selected analysis
  list<pair<wstring,double> > & word::get_senses(int k) {
    return(selected_begin(k)->get_senses());
  }
  /// get sense list (as string) for the selected analysis
  wstring word::get_senses_string(int k) const {
    return(util::pairlist2wstring(selected_begin(k)->get_senses(),L":", L"/"));
  }
  /// set sense list for the selected analysis
  void word::set_senses(const list<pair<wstring,double> > &ls, int k) {
    selected_begin(k)->set_senses(ls);
  }

  // get/set position of word in sentence (start from 0)
  void word::set_position(size_t p) { position=p; }
  size_t word::get_position() const { return position; }

  /// Get token span.
  unsigned long word::get_span_start() const {return(start);}
  unsigned long word::get_span_finish() const {return(finish);}
  /// look for a tag in the analysis list of a word
  bool word::find_tag_match(const freeling::regexp &re) const {
    bool found=false;
    for (word::const_iterator an=this->begin(); an!=this->end() && !found; an++) 
      found = re.search(an->get_tag());
    return found;
  }


  //////////////////////////////////////////////////////////////////
  ///   word::iterator may act as basic list iterator over all   ///
  ///   analysis of a word, or may act as a filtered iterator,   ///
  ///   providing access only to selected/unselected analysis    ///
  //////////////////////////////////////////////////////////////////

  /// Empty constructor
  word::iterator::iterator() : type(ALL),kbest(0) {}
  /// Copy
  word::iterator::iterator(const word::iterator &x) : list<analysis>::iterator(x) {
    type=x.type; kbest=x.kbest; 
    ibeg=x.ibeg; iend=x.iend;
  }
  /// Constructor from std::list iterator
  word::iterator::iterator(const list<analysis>::iterator &x) 
    : list<analysis>::iterator(x),type(ALL),kbest(0) {}
  /// Constructor for filtered iterators (selected/unselected)
  word::iterator::iterator(const list<analysis>::iterator &b, 
                           const list<analysis>::iterator &e, 
                           const list<analysis>::iterator &x,
                           int t, int k) : list<analysis>::iterator(x),ibeg(b),iend(e),type(t),kbest(k) {}
  /// Generic preincrement, for all cases
  word::iterator& word::iterator::operator++() {
    do {
      this->std::list<analysis>::iterator::operator++();
    } while (type!=ALL and (*this)!=iend and (*this)->is_selected(kbest)!=(type==SELECTED) );
    return (*this);
  }
  /// Generic postincrement, for all cases
  word::iterator word::iterator::operator++(int) {
    word::iterator b=(*this);
    ++(*this);
    return b;
  }

  //////////////////////////////////////////////////////////////////////////
  ///  word::const_iterator is the same than word::iterator,  but const  ///
  //////////////////////////////////////////////////////////////////////////

  /// Empty constructor
  word::const_iterator::const_iterator() : type(ALL),kbest(0) {}
  /// Copy
  word::const_iterator::const_iterator(const word::const_iterator &x) : list<analysis>::const_iterator(x) {
    type=x.type; kbest=x.kbest; 
    ibeg=x.ibeg; iend=x.iend;
  }
  /// Copy from nonconst iterator
  word::const_iterator::const_iterator(const word::iterator &x) : list<analysis>::const_iterator(x) {
    type=x.type; kbest=x.kbest; 
    ibeg=x.ibeg; iend=x.iend;  
  }
  /// Constructor from std::list iterator
  word::const_iterator::const_iterator(const list<analysis>::const_iterator &x) 
    : list<analysis>::const_iterator(x),type(ALL),kbest(0) {}
  /// Constructor from nonconst std::list iterator
  word::const_iterator::const_iterator(const list<analysis>::iterator &x) 
    : list<analysis>::const_iterator(x),type(ALL),kbest(0) {}
  /// Constructor for filtered iterators (selected/unselected)
  word::const_iterator::const_iterator(const list<analysis>::const_iterator &b, 
                                       const list<analysis>::const_iterator &e, 
                                       const list<analysis>::const_iterator &x,
                                       int t,int k) : list<analysis>::const_iterator(x),ibeg(b),iend(e),type(t),kbest(k) {}
  /// Generic preincrement, for all cases
  word::const_iterator& word::const_iterator::operator++() {
    do {
      this->std::list<analysis>::const_iterator::operator++();
    } while (type!=ALL and (*this)!=iend and (*this)->is_selected(kbest)!=(type==SELECTED) );  
    return (*this);
  }
  /// Generic increment, for all cases
  word::const_iterator word::const_iterator::operator++(int) {
    word::const_iterator b=(*this);
    ++(*this);
    return b;
  }

  ////////////////////////////////////////////////////////////////
  ///   Class node stores nodes of a parse_tree
  ///  Each node in the tree is either a label (intermediate node)
  ///  or a word (leaf node)
  ////////////////////////////////////////////////////////////////

  /// Methods for parse_tree nodes
  node::node(const wstring & s) : label(s), w(NULL) {head=false;chunk=false;nodeid=L"-";}
  node::node() : label(L""), w(NULL) {head=false;chunk=false;nodeid=L"-";}
  const wstring& node::get_node_id() const {return (nodeid);}
  void node::set_node_id(const wstring &id) {nodeid=id;}
  const wstring& node::get_label() const {return(label);}
  const word& node::get_word() const {return (*w);}
  word& node::get_word() {return (*w);}
  bool node::has_word() const {return w!=NULL;}
  void node::set_label(const wstring &s) {label=s;}
  void node::set_word(word &wd)  {w = &wd;}
  bool node::is_head() const {return head;}
  void node::set_head(const bool h) {head=h;}
  bool node::is_chunk() const {return (chunk!=0);}
  void node::set_chunk(const int c) {chunk=c;}
  int  node::get_chunk_ord() const {return (chunk);}


  ////////////////////////////////////////////////////////////////
  ///   Class parse tree is used to store the results of parsing
  ////////////////////////////////////////////////////////////////

  /// Methods for parse_tree
  parse_tree::parse_tree() : tree<node>() {}
  parse_tree::parse_tree(parse_tree::const_iterator p) : tree<node>(p) {}
  parse_tree::parse_tree(const node & n) : tree<node>(n) {}
  /// assign id's to nodes and build index
  void parse_tree::build_node_index(const wstring &sid) {
    parse_tree::iterator k;
    int i=0;
    node_index.clear();
    for (k=this->begin(); k!=this->end(); ++k, i++) {
      wstring id=sid+L"."+util::int2wstring(i);
      k->set_node_id(id);
      node_index.insert(make_pair(id,k));
    }
  }
  // rebuild index maintaining id's
  void parse_tree::rebuild_node_index() {
    node_index.clear();
    word_index.clear();
    for (parse_tree::iterator k=this->begin(); k!=this->end(); ++k) {
      wstring id=k->get_node_id();
      if (id != L"-") node_index.insert(make_pair(id,k));
      if (k.num_children()==0) word_index.push_back(k);
    }
  }

  /// get node with given index, normal iterator
  parse_tree::iterator parse_tree::get_node_by_id(const wstring & id) { 
    map<wstring,parse_tree::iterator>::iterator p = node_index.find(id); 
    if (p!=node_index.end()) return p->second; 
    else return parse_tree::iterator(this->end());
  }

  /// get node with given index, const iterator
  parse_tree::const_iterator parse_tree::get_node_by_id(const wstring & id) const { 
    map<wstring,parse_tree::iterator>::const_iterator p = node_index.find(id); 
    if (p!=node_index.end()) return p->second; 
    else return parse_tree::const_iterator(this->end());
  }

  /// get leaf node corresponding to word at given position in sentence, normal iterator
  parse_tree::iterator parse_tree::get_node_by_pos(size_t pos) { return word_index[pos]; }
  /// get leaf node corresponding to word at given position in sentence, const iterator
  parse_tree::const_iterator parse_tree::get_node_by_pos(size_t pos) const { return word_index[pos]; }

  /// obtain a reference to head word of a parse_tree
  const word& parse_tree::get_head_word(parse_tree::const_iterator pt) {
    if (pt.num_children()==0)
      return pt->get_word(); 
    else {
      parse_tree::const_sibling_iterator k;
      // locate head among children of current node (there must be exactly one in well-formed trees)
      for (k=pt.sibling_begin(); k!=pt.sibling_end() and not k->is_head(); k++);
      // should never happen
      if (k==pt.sibling_end()) { wcerr<<L"No head found in parse_tree::get_head_word"<<endl; exit(1); }
      // recurse into found head 
      return parse_tree::get_head_word(k);
    }
  }

  /// obtain the head label of a parse_tree
  const wstring& parse_tree::get_head_label(parse_tree::const_iterator pt) {
    if (pt.num_children()==0)
      return pt->get_label(); 
    else {
      parse_tree::const_sibling_iterator k;
      // locate head among children of current node (there must be exactly one in well-formed trees)
      for (k=pt.sibling_begin(); k!=pt.sibling_end() and not k->is_head(); k++);
      // should never happen
      if (k==pt.sibling_end()) { wcerr<<L"No head found in parse_tree::get_head_word"<<endl; exit(1); }
      // recurse into found head 
      return parse_tree::get_head_label(k);
    }
  }

  /// obtain the position in the sentence of the head word of a parse_tree
  int parse_tree::get_head_position(parse_tree::const_iterator pt) {
    if (pt.num_children()==0)
      return pt->get_word().get_position(); 
    else {
      parse_tree::const_sibling_iterator k;
      // locate head among children of current node (there must be exactly one in well-formed trees)
      for (k=pt.sibling_begin(); k!=pt.sibling_end() and not k->is_head(); k++);
      // No head (only possible for fake root in chunking trees)
      if (k==pt.sibling_end()) return -1;
      // recurse into found head 
      return parse_tree::get_head_position(k);
    }
  }

  /// C-commands
  ///    1 - pt1 does not dominate pt2
  ///    2 - pt2 does not dominate pt1
  ///    3 - The first branching node that dominates pt1 also dominates pt2
  bool parse_tree::C_commands(parse_tree::const_iterator pt1, parse_tree::const_iterator pt2) {
    // they are root => 1, 2 and 3 are not satisfied
    if (pt1.is_root() or pt2.is_root()) return false;

    // find the first branching node ancestor of pt1
    parse_tree::const_iterator parent=pt1.get_parent();
    while (not parent.is_root() and parent.num_children()==1 and parent!=pt2)
      parent=parent.get_parent();

    // pt1 dominates pt2 => 1 is not satisfied
    // (the ancestor found for pt1 is the root and it is not a branching node)
    if (parent.is_root() and parent.num_children()==1) return false;
    // pt2 dominates pt1 => 2 is not satisfied
    // (the first branching node that dominates pt1 is pt2)
    if (parent==pt2) return false;
    
    // parent is the first branching node of pt1
    // check whether pt2 is descendant of parent
    while (not pt2.is_root() and pt2!=parent) 
      pt2 = pt2.get_parent(); 
    return (pt2==parent);
  }

  /// get lowest parse tree node that subsumes the *whole* word span i..j
  template <class T>
  auto subsuming_node(T &t, size_t i, size_t j) -> decltype(t.begin()) {
    
    auto ptree = t.get_node_by_pos(i);
    
    // look for a parent that subsumes the largest portion of (i,j) but not more
    while (!ptree.is_root() and 
           parse_tree::get_rightmost_leaf(ptree)->get_word().get_position() < j and
           parse_tree::get_leftmost_leaf(ptree)->get_word().get_position() == i)  {
      
      ptree = ptree.get_parent();
    }
        
    return ptree;
  }

  /// get highest parse tree node that subsumes longest word span i..k, such that k<=j
  template <class T>
  auto left_subsuming_node(T &t, size_t i, size_t j) -> decltype(t.begin()) {
    
    auto prev = t.begin();
    auto ptree = t.get_node_by_pos(i);
    
    // look for a parent that subsumes the largest portion of (i,j) but not more
    while (!ptree.is_root() and 
           parse_tree::get_rightmost_leaf(ptree)->get_word().get_position() < j and
           parse_tree::get_leftmost_leaf(ptree)->get_word().get_position() == i)  {
      
      prev = ptree;
      ptree = ptree.get_parent();
    }
    
    // if we went out of bounds, use last valid subtree visited
    if (parse_tree::get_rightmost_leaf(ptree)->get_word().get_position() > j or
        parse_tree::get_leftmost_leaf(ptree)->get_word().get_position() < i) { 
      ptree = prev;
    }
    
    return ptree;
  }

  /// get highest parse tree node that subsumes longest word span k..j, such that i<=k
  template <class T>
  auto right_subsuming_node(T &t, size_t i, size_t j) -> decltype(t.begin()) {
    
    auto prev = t.begin();
    auto ptree = t.get_node_by_pos(j);
    
    // look for a parent that subsumes the largest portion of (i,j) but not more
    while (!ptree.is_root() and 
           parse_tree::get_rightmost_leaf(ptree)->get_word().get_position() == j and
           parse_tree::get_leftmost_leaf(ptree)->get_word().get_position() > i)  {
      
      prev = ptree;
      ptree = ptree.get_parent();
    }
    
    // if we went out of bounds, use last valid subtree visited
    if (parse_tree::get_rightmost_leaf(ptree)->get_word().get_position() > j or
        parse_tree::get_leftmost_leaf(ptree)->get_word().get_position() < i) { 
      ptree = prev;
    }
    
    return ptree;
  }

  
  parse_tree::const_iterator parse_tree::get_subsuming_node(size_t i, size_t j) const {
    return subsuming_node<const parse_tree>(*this,i,j);
  }
  parse_tree::iterator parse_tree::get_subsuming_node(size_t i, size_t j) {
    return subsuming_node<parse_tree>(*this,i,j);
  }
  parse_tree::const_iterator parse_tree::get_left_subsuming_node(size_t i, size_t j) const {
    return left_subsuming_node<const parse_tree>(*this,i,j);
  }
  parse_tree::iterator parse_tree::get_left_subsuming_node(size_t i, size_t j) {
    return left_subsuming_node<parse_tree>(*this,i,j);
  }
  parse_tree::const_iterator parse_tree::get_right_subsuming_node(size_t i, size_t j) const {
    return right_subsuming_node<const parse_tree>(*this,i,j);
  }
  parse_tree::iterator parse_tree::get_right_subsuming_node(size_t i, size_t j) {
    return right_subsuming_node<parse_tree>(*this,i,j);
  }
  
  /// Print a parse tree -- tracing purposes mainly
  void parse_tree::PrintTree(parse_tree::const_iterator n, int k, int depth) {
    parse_tree::const_sibling_iterator d;  
    wcerr<<wstring(depth*2,L' '); 
    if (n.num_children()==0) { 
      if (n->is_head()) { wcerr<<L"+";}
      const word & w=n->get_word();
      wcerr<<L"("<<w.get_form();
      wcerr<<L" "<<w.get_lemma(k);
      wcerr<<L" "<<w.get_tag(k);
      wcerr<<L")"<<endl;
    }
    else { 
      if (n->is_head()) { wcerr<<L"+";}
      wcerr<<n->get_label()<<L"_[ "<<endl;
      for (d=n.sibling_begin(); d!=n.sibling_end(); ++d)
        PrintTree(d, k, depth+1);
      wcerr<<wstring(depth*2,L' ')<<L"]"<<endl;
    }
  }


  ////////////////////////////////////////////////////////////////
  /// class denode stores nodes of a dependency tree and
  ///  parse tree <-> deptree relations
  ////////////////////////////////////////////////////////////////

  /// Methods for dependency tree nodes
  depnode::depnode() {}
  depnode::depnode(const wstring & s) : node(s),link(NULL) {}
  depnode::depnode(const node & n) : node(n),link(NULL) {}
  void depnode::set_link(const parse_tree::iterator p) {link=p;}
  parse_tree::iterator depnode::get_link() { return link;}
  parse_tree::const_iterator depnode::get_link() const { return link;}
  /// tree<node>& depnode::get_link_ref() { return (link);}  ///  (useful for Java API)

  ////////////////////////////////////////////////////////////////
  /// class dep_tree stores a dependency tree
  ////////////////////////////////////////////////////////////////

  /// Constructors for dep_tree
  dep_tree::dep_tree() : tree<depnode>() {}
  dep_tree::dep_tree(const depnode & n) : tree<depnode>(n) {}

  /// get depnode corresponding to word in given position, const iterator
  dep_tree::const_iterator dep_tree::get_node_by_pos(size_t pos) const { return word_index[pos]; }
  /// get depnode corresponding to word in given position, normal iterator
  dep_tree::iterator dep_tree::get_node_by_pos(size_t pos) { return word_index[pos]; }

  /// obtain position of first word in the span subsumed by a dep_tree
  size_t dep_tree::get_first_word(dep_tree::const_iterator t) {
    size_t m1 = t->get_word().get_position();
    for (dep_tree::const_sibling_iterator c=t.sibling_begin(); c!=t.sibling_end(); c++) 
      m1 = min(m1, get_first_word(c));
    return m1;
  }

  /// obtain position of last word in the span subsumed by a dep_tree
  size_t dep_tree::get_last_word(dep_tree::const_iterator t) {
    size_t m1 = t->get_word().get_position();
    for (dep_tree::const_sibling_iterator c=t.sibling_begin(); c!=t.sibling_end(); c++) 
      m1 = max(m1, get_last_word(c));
    return m1;
  }

  /// rebuild index maintaining word positions
  void dep_tree::rebuild_node_index() {
    word_index.clear();
    for (dep_tree::iterator d=this->begin(); d!=this->end(); ++d) {
      if (d->has_word()) { 
        size_t pos=d->get_word().get_position();
        if (pos>=word_index.size()) word_index.resize(pos+1, dep_tree::iterator());
        word_index[pos]=d;
      }
    }  
  }

  /// Print a parse tree -- tracing purposes mainly
  void dep_tree::PrintDepTree (dep_tree::const_iterator n, int depth) {
    dep_tree::const_sibling_iterator d, dm;
    int last, min;
    bool trob;

    wcerr << wstring (depth*2, ' ');

    parse_tree::const_iterator pn = n->get_link();
    wcerr<<pn->get_label(); 
    wcerr<<L"/" << n->get_label() << L"/";

    const word & w = n->get_word();
    wcerr << L"(" << w.get_form() << L" " << w.get_lemma() << L" " << w.get_tag () << L")";
  
    if (n.num_children () > 0) {
      wcerr << L" [" << endl;
    
      // Print Nodes
      for (d = n.sibling_begin (); d != n.sibling_end (); ++d)
        if (!d->is_chunk ())
          PrintDepTree (d, depth + 1);
    
      // print CHUNKS (in order)
      last = 0;
      trob = true;
      while (trob) {
        // while an unprinted chunk is found look, for the one with lower chunk_ord value
        trob = false;
        min = 9999;
        for (d = n.sibling_begin (); d != n.sibling_end (); ++d) {
          if (d->is_chunk ()) {
            if (d->get_chunk_ord () > last
                and d->get_chunk_ord () < min) {
              min = d->get_chunk_ord ();
              dm = d;
              trob = true;
            }
          }
        }
        if (trob)
          PrintDepTree (dm, depth + 1);
        last = min;
      }
    
      wcerr << wstring (depth * 2, ' ') << L"]";
    }
    wcerr << endl;
  }

  ////////////////////////////////////////////////////////////////
  /// Virtual class to store the processing state of a sentence.
  /// Each processor will define a derived class with needed contents,
  /// and store it in the sentence being processed.
  ////////////////////////////////////////////////////////////////

  processor_status::processor_status() {}

  ////////////////////////////////////////////////////////////////
  ///   Class argument stores information about a predicate argument
  ////////////////////////////////////////////////////////////////

  const wstring argument::EMPTY_ROLE=L"";

  argument::argument() {}
  argument::~argument() {}
  argument::argument(int p, const wstring &r) : position(p), role(r) {}
      
  /// get the position of the argument head
  int argument::get_position() const { return position; }
  /// get the role of the argument
  const wstring& argument::get_role() const { return role; }

  ////////////////////////////////////////////////////////////////
  ///   Class predicate stores information about a predicate
  /// and its arguments in a sentence
  ////////////////////////////////////////////////////////////////

  predicate::predicate(int p, const wstring &s) : position(p), sense(s) {}
  predicate::predicate() {}
  predicate::~predicate() {}

  /// Copy constructor
  predicate::predicate(const predicate &p) {
    position = p.position;
    sense = p.sense;
    arg_index = p.arg_index;
    for (vector<argument>::const_iterator a=p.begin(); a!=p.end(); a++)
      this->push_back(*a);
  }

  /// assignment
  predicate& predicate::operator=(const predicate &p) {
    if (this!=&p) {
      position = p.position;
      sense = p.sense;
      arg_index = p.arg_index;
      this->clear();
      for (vector<argument>::const_iterator a=p.begin(); a!=p.end(); a++)
        this->push_back(*a);
    }
    return *this;
  }
    
  /// get the propbank sense of the predicate
  const wstring& predicate::get_sense() const {
    return sense;
  }

  /// get the position of the predicate head
  int predicate::get_position() const {
    return position;
  }

  /// check whether word in position p is an argument to the predicate
  bool predicate::has_argument(int p) const { return (arg_index.find(p)!=arg_index.end()); }

  /// add argument to predicate
  void predicate::add_argument(int p, const wstring &r) {
    argument a(p,r);
    this->push_back(a); 
    arg_index.insert(make_pair(p,this->size()-1));
  }
  
  /// get access to an argument by word position
  const argument & predicate::get_argument_by_pos(int p) const {
    map<int,int>::const_iterator x=arg_index.find(p);
    if (x!=arg_index.end()) return (*this)[x->second];
    else {
      wcerr << L"Error. Word at position "<< p << " is not an argument of predicate " << position << endl;
      exit(1);
    }
  }


  /// add a predicate to the sentence
  void sentence::add_predicate(const predicate &pr) {
    preds.push_back(pr); 
    pred_index.insert(make_pair(pr.get_position(),preds.size()-1));
  }

  /// see if word in given position is a predicate
  bool sentence::is_predicate(int p) const {
    return pred_index.find(p)!=pred_index.end();
  }

  /// get predicate number n for word in position p
  int sentence::get_predicate_number(int p) const { 
    map<int,int>::const_iterator x=pred_index.find(p);
    if (x!=pred_index.end()) return x->second;
    else return -1;
  }

  /// get word position for predicate number n
  int sentence::get_predicate_position(int n) const {
    if (n<int(preds.size())) return preds[n].get_position();
    else return -1;
  }

  /// get predicate for word at position p
  const predicate& sentence::get_predicate_by_pos(int p) const {
    int x = get_predicate_number(p);
    if (x>=0) 
      return preds[x];
    else {
      wcerr << L"Error. No predicate at position "<< p << " in the sentence." << endl;
      exit(1);
    }
  }

  /// get predicate number n
  const predicate& sentence::get_predicate_by_number(int n) const {
    if (n>=0 and n<int(preds.size())) 
      return preds[n];
    else {
      wcerr << L"Error. Predicate number "<< n << " does not exist." << endl;
      exit(1);
    }
  }

  /// get predicates of the sentence 
  const sentence::predicates & sentence::get_predicates() const { return preds; }


  ////////////////////////////////////////////////////////////////
  ///   Class sentence is just a list of words that someone
  /// (the splitter) has validated it as a complete sentence.
  /// It may include a parse tree.
  ////////////////////////////////////////////////////////////////

  /// Create a new sentence.
  sentence::sentence() { 
    pts.clear(); dts.clear(); wpos.clear(); 
    preds.clear();  pred_index.clear(); 
    status.clear(); 
    sent_id=L"0"; tagged=false;
    best_seq=0;
  }
  /// Create a new sentence from list<word>
  sentence::sentence(const list<word> &lw) : list<word>(lw) { 
    pts.clear(); dts.clear(); status.clear(); 
    preds.clear(); pred_index.clear();
    sent_id=L"0"; tagged=false;
    best_seq=0;
    rebuild_word_index();
  }
  /// add a word to the sentence
  void sentence::push_back(const word &w) {
    this->list<word>::push_back(w);
    this->back().set_position(this->size()-1);
    wpos.push_back(&(this->back()));
  }

  /// rebuild word index by position
  void sentence::rebuild_word_index() {
    wpos = vector<word*>(this->size(),(word*)NULL);
    size_t i=0;
    for (sentence::iterator w=this->begin(); w!=this->end(); w++) {
      wpos[i]= &(*w);
      w->set_position(i);
      i++;
    }

    // if there is a constituency tree, index its leaf nodes by position
    if (this->is_parsed()) {
      for (map<int,parse_tree>::iterator k=pts.begin(); k!=pts.end(); k++) 
        k->second.rebuild_node_index();
    }

    // if there is a dependency tree, index its nodes by position
    if (this->is_dep_parsed()) {
      for (map<int,dep_tree>::iterator k=dts.begin(); k!=dts.end(); k++) 
        k->second.rebuild_node_index();
    } 
  }

  /// Clone sentence
  void sentence::clone(const sentence &s) {
    // copy processing status
    status = s.status;
    // copy identifier
    sent_id = s.sent_id;
    // copy tagged status
    tagged = s.tagged;
    // copy best tag seq #
    best_seq = s.best_seq;
    // copy word list
    wpos = vector<word*>(s.size(),(word*)NULL);
    map<const word*,word*> wps;
    this->list<word>::clear(); 
    int i=0;
    for (sentence::const_iterator w=s.begin(); w!=s.end(); w++) {
      this->push_back(*w);
      // remember pointers to new and old words, to update trees (see below).
      wps.insert(make_pair(&(*w),&(this->back())));
      // store word pointer for positional acces
      wpos[i++] = &(this->back());
    }

    // copy parse trees, and fix pointers to words in leaves
    pts=s.pts; 
    for (map<int,parse_tree>::iterator k=pts.begin(); k!=pts.end(); k++) {
      sentence::iterator j=this->begin();
      for (parse_tree::iterator p=k->second.begin(); p!=k->second.end(); ++p) {
        if (p.num_children()==0) {
          p->set_word(*j);
          j++;
        }
      }
      k->second.rebuild_node_index();
    }

    // copy dependency trees, and fix links to parse_tree and words.
    dts=s.dts;
    for (map<int,dep_tree>::iterator k=dts.begin(); k!=dts.end(); k++) {
      for (dep_tree::iterator d=k->second.begin(); d!=k->second.end(); ++d) {
        // update link to parse_tree, if any
        if (d->get_link()!=parse_tree::iterator(NULL)) {
          wstring id=d->get_link()->get_node_id();
          parse_tree::iterator p=pts[k->first].get_node_by_id(id);
          d->set_link(p);
        }
        // update link to word
        word &w=d->get_word();   
        d->set_word(*wps[&w]);
      }
      k->second.rebuild_node_index();
    }

    // copy SRL argument structures, if any
    preds = s.preds;
    pred_index = s.pred_index;
  }

  /// Copy constructor.
  sentence::sentence(const sentence & s) { clone(s); }
  /// Destructor
  sentence::~sentence() { this->clear(); }

  /// Assignment.
  sentence& sentence::operator=(const sentence& s) {
    if (this!=&s) clone(s);
    return *this;
  }
  /// positional access to a word
  const word& sentence::operator[](size_t i) const { return (*wpos[i]); }
  word& sentence::operator[](size_t i) { return (*wpos[i]); }
  /// find out how many kbest sequences the tagger computed
  unsigned int sentence::num_kbest() const {
    if (this->empty()) return 0;
    else return this->begin()->num_kbest();
  }

  /// best tag sequence (Zero by default, unless the user changes it)
  void sentence::set_best_seq(int k) { best_seq = k; }
  int sentence::get_best_seq() const { return best_seq; }

  /// Clear sentence and possible trees
  void sentence::clear() { 
    this->list<word>::clear(); 
    pts.clear(); dts.clear();
    wpos.clear(); 
    preds.clear();
    pred_index.clear();
    while (not status.empty()) clear_processing_status();
  }

  /// Set sentence identifier
  void sentence::set_sentence_id(const wstring &sid) {sent_id=sid;}
  /// Get sentence identifier
  const wstring& sentence::get_sentence_id() const {return sent_id;}

  // set whether sentence is tagged
  void sentence::set_is_tagged(bool b) {tagged = b;}
  // retrieve whether sentence is tagged
  bool sentence::is_tagged() const {return tagged;}

  /// Set the parse tree.
  void sentence::set_parse_tree(const parse_tree &tr, int k) {
    pts[k]=tr;
    pts[k].rebuild_node_index();
  }
  /// Obtain the parse tree.
  parse_tree & sentence::get_parse_tree(int k) {
    map<int,parse_tree>::iterator t=pts.find(k);
    return t->second;
  }
  const parse_tree & sentence::get_parse_tree(int k) const {
    map<int,parse_tree>::const_iterator t=pts.find(k);
    return t->second;
  }
  /// Find out whether the sentence is parsed.
  bool sentence::is_parsed() const {return not pts.empty();}
  /// Set the dependency tree.
  void sentence::set_dep_tree(const dep_tree &tr, int k) {
    dts[k]=tr;
    dts[k].rebuild_node_index();
  }
  /// Obtain the parse dependency tree
  dep_tree & sentence::get_dep_tree(int k) {
    map<int,dep_tree>::iterator t=dts.find(k);
    return t->second;
  }
  const dep_tree & sentence::get_dep_tree(int k) const {
    map<int,dep_tree>::const_iterator t=dts.find(k);
    return t->second;
  }
  /// Find out whether the sentence is dependency parsed.
  bool sentence::is_dep_parsed() const {return not dts.empty();}
  /// obtain list of words (useful for perl APIs)
  vector<word> sentence::get_words() const {
    vector<word> v;
    for (sentence::const_iterator i=this->begin(); i!=this->end(); i++)
      v.push_back(*i);
    return (v);
  }

  /// get/set processing status
  processor_status* sentence::get_processing_status() {return status.back();}
  const processor_status* sentence::get_processing_status() const {return status.back();}
  void sentence::set_processing_status(processor_status *s) {status.push_back(s);}
  void sentence::clear_processing_status() {
    if (status.empty()) return; 
    processor_status *s=status.back(); 
    status.pop_back(); 
    delete s;
  }

  /// obtain iterators (useful for perl/java APIs)
  sentence::iterator sentence::words_begin() {return this->begin();}
  sentence::const_iterator sentence::words_begin() const {return this->begin();}
  sentence::iterator sentence::words_end() {return this->end();}
  sentence::const_iterator sentence::words_end() const {return this->end();}

  // obtain iterator to a word given its position
  sentence::iterator sentence::get_word_iterator(const word &w) {
    sentence::iterator it=this->begin();
    for (; &(*it)!=&w and it!=this->end(); it++);
    return it;
  }
  sentence::const_iterator sentence::get_word_iterator(const word &w) const {
    sentence::const_iterator it=this->begin();
    for (; &(*it)!=&w and it!=this->end(); it++);
    return it;
  }

  ////////////////////////////////////////////////////////////////
  ///   Class paragraph is just a list of sentences that someone
  ///  has validated it as a paragraph.
  ////////////////////////////////////////////////////////////////

  paragraph::paragraph() {}
  paragraph::paragraph(const std::list<sentence> &x) : std::list<sentence>(x) {}
  void paragraph::set_paragraph_id(const std::wstring &id) { par_id = id; }
  const wstring & paragraph::get_paragraph_id() const { return par_id; } 

  //////////////////////////////////////////////////////////////////
  /// Class mention is a node in the parse tree, as well as the sequence of tokens      
  /// subsumed by the node. 
  /// (It is assumed that a depnode corresponds to a parse tree node) 
  ////////////////////////////////////////////////////////////////// 

  /// Constructor from a parse_tree pointer
  mention::mention(int i, int ns, paragraph::const_iterator ps, parse_tree::const_iterator pt, int word_b, sentence::const_iterator itword) {
    id = i;
    sid = std::to_wstring(id);
    sent = ns;
    s = ps;
    maximal = false;
    initial = false;
    posBegin = word_b; 
    itBegin = itword;
    ptree = pt;
    set_tokens(pt, word_b, itword);
    posEnd = word_b-1;
    itEnd = itword;
    itHead = s->get_word_iterator(parse_tree::get_head_word(pt));
    chain = -1;
  }

  /// Constructor from a dep_tree node
  mention::mention(int i, int ns, paragraph::const_iterator ps, dep_tree::const_iterator dt, int begin, int end) {
    id = i;
    sid = std::to_wstring(id);
    sent = ns;
    s = ps;
    maximal = false;
    initial = false;

    dtree = dt;

    // use given begin/end positions, if given.  Otherwise, whole mention
    if (begin>=0) posBegin = begin;
    else posBegin = dep_tree::get_first_word(dt);

    if (end >= 0) posEnd = end;
    else posEnd = dep_tree::get_last_word(dt);

    itBegin = ps->get_word_iterator((*ps)[posBegin]);
    itEnd = ps->get_word_iterator((*ps)[posEnd]);
    ++itEnd;

    itHead = ps->get_word_iterator((*ps)[dt->get_word().get_position()]);
    chain = -1;
  }


  /// fuzzy constructor from start/end positions
  /// the mention is created from start1 to end1 (start1 >= start and end1 <= end) 
  /// [start1, end1] is the sequence closest to start with the maximal subsuming node ptree 
  mention::mention(int i, int ns, paragraph::const_iterator ps, sentence::const_iterator start_it, sentence::const_iterator end_it) {
    id = i;
    sid = std::to_wstring(id);
    sent = ns;
    s = ps;
    maximal = false;
    initial = false;
    // initially the maximal subsumed sequence is [start_it,start_it] and the node
    // subsuming it is the node corresponding to start_it word
    parse_tree::const_iterator prev;
    ptree = ps->get_parse_tree().get_node_by_pos(start_it->get_position());

    // look for a parent that subsumes the largest portion of (start_it,end_it) without going out of bounds
    while (!ptree.is_root() and parse_tree::get_rightmost_leaf(ptree)->get_word().get_position() < end_it->get_position()
           and parse_tree::get_leftmost_leaf(ptree)->get_word().get_position() == start_it->get_position()) {
      wstring ws1 = parse_tree::get_rightmost_leaf(ptree)->get_word().get_form();
      wstring ws2 = parse_tree::get_leftmost_leaf(ptree)->get_word().get_form();
    
      prev = ptree;
      ptree = ptree.get_parent();
    }

    // if we went out of bound, use last valid subtree visited
    if (parse_tree::get_rightmost_leaf(ptree)->get_word().get_position() > end_it->get_position()
        or parse_tree::get_leftmost_leaf(ptree)->get_word().get_position() < start_it->get_position()) 
      ptree = prev;
      
    posBegin = start_it->get_position(); 
    itBegin = start_it;

    posEnd = end_it->get_position();
    itEnd = end_it; 
    ++itEnd;

    itHead = s->get_word_iterator(parse_tree::get_head_word(ptree));
    chain = -1;
  }

  /// Clone mention
  void mention::clone(const mention &m) {
    id = m.id;
    sid = m.sid;
    mType = m.mType;
    sent = m.sent;
    s = m.s;
    maximal = m.maximal;
    initial = m.initial;
    ptree = m.ptree;
    dtree = m.dtree;
    posBegin = m.posBegin;
    posEnd = m.posEnd;
    itBegin = m.itBegin;
    itEnd = m.itEnd;
    itHead = m.itHead;
    chain = m.chain;
  }

  /// Copy constructor.
  mention::mention(const mention &m) : s(m.s) { clone(m);}

  /// Assignment.
  mention& mention::operator=(const mention &m) {
    if (this!=&m) { clone(m); }
    return *this;
  }

  /// True when the mention starts before m in the document
  bool mention::operator<(const mention &m) const {
    return (posBegin < m.posBegin) or 
      ((posBegin == m.posBegin) and (posEnd >= m.posEnd));
  }

  /// setters
  void mention::set_id(int id) {
    this->id = id;
    this->sid = std::to_wstring(id);
  }
  void mention::set_type(mention::mentionType t) { 
    mType=t; 
  }
  void mention::set_initial(bool val) { 
    initial=val; 
  }
  void mention::set_group(int g) {
    chain=g;
  }
  void mention::set_maximal(bool b) {
    maximal=b;
  }

  /// getters
  int mention::get_id() const {
    return id;
  }
  wstring mention::get_str_id() const {
    return sid;
  }
  int mention::get_n_sentence() const {
    return sent;
  }
  paragraph::const_iterator mention::get_sentence() const {
    return s;
  }
  parse_tree::const_iterator mention::get_ptree() const {
    return ptree;
  }
  dep_tree::const_iterator mention::get_dtree() const {
    return dtree;
  }
  const word& mention::get_head() const {
    return *itHead;
  }
  int mention::get_pos_begin() const {
    return posBegin;
  }
  int mention::get_pos_end() const {
    return posEnd;
  }
  sentence::const_iterator mention::get_it_begin() const {
    return itBegin;
  }
  sentence::const_iterator mention::get_it_end() const {
    return itEnd;
  }
  sentence::const_iterator mention::get_it_head() const {
    return itHead;
  }
  mention::mentionType mention::get_type() const {
    return mType;
  }
  int mention::get_group() const {
    return chain;
  }
  bool mention::is_type(mention::mentionType t) const {
    return mType == t;
  }
  bool mention::is_initial() const {
    return initial;
  }
  bool mention::is_maximal() const {
    return maximal;
  }
  
  /// get string of mention words, lowercasing the first "lc"
  /// default: lc=0;  Use lc=-1 to lowercase all words
  wstring mention::value(int lc) const {
    if (lc == -1) lc = posEnd - posBegin + 1;
    int p=1;
    wstring v=L"";
    for (sentence::const_iterator it=itBegin; it!=itEnd; it++) {
      if (it!=itBegin) v = v + L" ";
      v = v + (p<=lc ? it->get_lc_form() : it->get_form());
      p++;
    }
    return v;
  }

  /// fills tokens and tags from parse_tree pt and wordn
  /// last parameters return posEnd+1 and itEnd+1; 
  void mention::set_tokens(parse_tree::const_iterator pt, int &wordn, sentence::const_iterator &itword) {

    if (pt.num_children()==0) {
      wordn++;
      itword++;
    }
    else 
      for (parse_tree::const_sibling_iterator d=pt.sibling_begin(); d!=pt.sibling_end(); ++d) {
        set_tokens(d, wordn, itword);
      }
  }

  /// obtain the node that maximize the subsumtion of the sequence defined by the first 
  /// and second iterators last parameters return the iterator of the last word within 
  /// the subsumtion and the tree node subsuming the sequence
  void mention::set_iterators(sentence::const_iterator ps1, sentence::const_iterator ps2, const parse_tree &parse, sentence::const_iterator &ps_res, parse_tree::const_iterator &ptree) {
    // if ps1 is ps2, ptree and ps_res are the input ones. 
    // if ptree is root, ptree and ps_res are also the input ones.
    // Otherwise...

    if ((ps1 != ps2) and not ptree.is_root()){
      parse_tree::const_iterator new_ptree = ptree.get_parent();

      // if the first descendant of the parent is not ptree, 
      // ptree and ps_res are the input ones. Otherwise...
      if (ptree == new_ptree.sibling_begin()) {
	size_t w_end_pos = parse_tree::get_rightmost_leaf(new_ptree)->get_word().get_position();
	sentence::const_iterator new_ps_res = ps1;
	for (unsigned int i=1; i<w_end_pos-ps1->get_position(); i++) new_ps_res++;

	// if the last word is the one in ps2, the new pointers are selected
	if (w_end_pos == ps2->get_position()) {
	  ps_res = new_ps_res;
	  ptree = new_ptree;
	}
	// if the last word is before the one in p2, we try the new pointers
	else if (w_end_pos <  ps2->get_position()) {
	  ps_res = new_ps_res;
	  ptree = new_ptree;
	  set_iterators(ps1, ps2, parse, ps_res, ptree);
        }   
	// if the last word is after ps2, ptree and ps_res are the input ones.
      }
    }
  }


  ////////////////////////////////////////////////////////////////
  ///   Class document is a list of paragraphs. It may have additional
  ///  information (such as title)
  ////////////////////////////////////////////////////////////////

  /// Construct  or
  document::document() {}

  document::~document() {}

  /// Add node1 to the group group1 of coreferents
  /*void document::add_positive(const wstring &node1, int group1) {
    group2node.insert( make_pair(group1,node1) );
    node2group[node1] = group1;
    }*/  

  bool document::is_parsed() const {
    return this->front().front().is_parsed();
  }

  bool document::is_dep_parsed() const {
    return this->front().front().is_dep_parsed();
  }

  /// Adds one mention to the doc within a coreference group
  void document::add_mention(const mention &m) {
    mentions.push_back(m);
    if (group2mentions.find(m.get_group()) == group2mentions.end()) 
      groups.push_back(m.get_group());
    group2mentions.insert( make_pair(m.get_group(), m.get_id()) );

  }

  // count number of words in the document
  int document::get_num_words() const {
    int nw = 0;
    for (document::const_iterator p=this->begin(); p!=this->end(); p++)
      for (list<sentence>::const_iterator s=p->begin(); s!=p->end(); s++)
        nw += s->size();
    return nw;
  }

  /// Gets the number of groups found
  int document::get_num_groups() const {
    return groups.size();
  }

  const std::list<int> & document::get_groups() const {
    return groups;
  }

  /// Gets an iterator pointing to the beginning of the mentions 
  vector<mention>::const_iterator document::begin_mentions() const {
    return mentions.begin();
  }
  vector<mention>::iterator document::begin_mentions() {
    return mentions.begin();
  }

  /// Gets an iterator pointing to the end of the mentions 
  vector<mention>::const_iterator document::end_mentions() const {
    return mentions.end();
  }
  vector<mention>::iterator document::end_mentions() {
    return mentions.end();
  }

  /// get reference to i-th mention
  /*const mention& document::get_mention(int i) const {
    return mentions[i];
    }*/

  /// get reference to mention id
  const mention& document::get_mention(int id) const {
    vector<mention>::const_iterator it = mentions.begin();
    while (it != mentions.end() and it->get_id() != id) it++;
    return *it;
  }

  /// Gets all the nodes in a coreference group id
  /*list<wstring> document::get_coref_nodes(int id) const {
    list<wstring> ret;
    multimap<int,wstring>::const_iterator it;
    pair<multimap<int,wstring>::const_iterator,multimap<int,wstring>::const_iterator> par;
    par = group2node.equal_range(id);
    for (it=par.first; it!=par.second; ++it) ret.push_back(it->second);
    return(ret);
    } */

  /// Gets all the mentions' ids in the ith coreference group
  list<int> document::get_coref_id_mentions(int ith) const {
    list<int> ret;
    multimap<int,int>::const_iterator it;
    pair<multimap<int,int>::const_iterator,multimap<int,int>::const_iterator> par;
    par = group2mentions.equal_range(ith);
    for (it=par.first; it!=par.second; ++it) ret.push_back(it->second);
    ret.sort();
    return(ret);
  }

  /// Returns whether two nodes are in the same coreference group
  /*bool document::is_coref(const wstring &node1, const wstring &node2) const {
    int g1, g2;
    g1 = get_coref_group(node1);
    g2 = get_coref_group(node2);
    return ( g1!= -1 && g1==g2 );
    }*/

  
  const semgraph::semantic_graph & document::get_semantic_graph() const {return sem_graph;}
  semgraph::semantic_graph & document::get_semantic_graph() {return sem_graph;}

} // namespace
