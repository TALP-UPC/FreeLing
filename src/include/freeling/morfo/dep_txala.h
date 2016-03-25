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

#ifndef _DEP_TXALA
#define _DEP_TXALA

#include <string>
#include <map>
#include <set>
#include <vector>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/semdb.h"
#include "freeling/morfo/dep_rules.h"
#include "freeling/morfo/dependency_parser.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Store parsing status information
  ////////////////////////////////////////////////////////////////

  class dep_txala_status : public processor_status {
  public:
    /// precomputed last node matching the "last_left/right" condition
    /// for a certain rule number
    std::map<std::wstring,parse_tree::iterator> last;

    /// set of active flags, which control applicability of rules
    std::set<std::wstring> active_flags;
  };


  ///////////////////////////////////////////////////////////////////////
  ///
  /// dep_txala is a class for obtaining a dependency tree from chunks.
  ///  this implementation uses two subclasses:
  /// txala_completer: to complete the chunk analysis in a full parse tree
  /// txala_labeler: to set the labels once the class has build a dependency tree
  ///
  ///////////////////////////////////////////////////////////////////////

  class WINDLL dep_txala : public dependency_parser {

  private:
    // Root symbol used by the chunk parser when the tree is not complete.
    std::wstring start;
    /// set of completer rules, indexed by labels of nodes
    std::map<std::pair<std::wstring,std::wstring>,std::list<completer_rule> > chgram;
    // set of labeller rules
    std::map<std::wstring, std::list<labeler_rule> > rules;
    // "unique" labels for labeller
    std::set<std::wstring> unique;
    // semantic database to check for semantic conditions in rules
    semanticDB * semdb;
    // semantic classes for words, declared in CLASS section
    std::set<std::wstring> wordclasses;  // items are class#lemma
    // semantic classes for words, declared in CLASS section
    std::set<std::wstring> pairclasses;  // items are class#lemma1#lemma2

    /// grammar file parsing methods -------------------

    /// auxiliary to load CLASS section of config file
    void load_classes(const std::wstring &, const std::wstring &, 
                      const std::wstring &, std::set<std::wstring> &);
    /// Separate extra lemma/form/class conditions from the chunk label
    void extract_conds(std::wstring &, matching_condition &) const;
    void create_subexpression(const std::wstring &node, const std::wstring &func, 
                              const std::wstring &value, bool negated, rule_expression &re) const;
    void add_subexpression(const std::wstring &condition, rule_expression &re) const;
    /// parse a line containing a completer rule, and fill completer_rule instance
    void load_rule(const std::wstring &line, completer_rule &r);
    // parse a line containing a labeller rule, and fill labeler_rule instance
    void load_rule(const std::wstring &line, labeler_rule &r);


    /// tree-completing methods  ---------------------

    /// retrieve rule from grammar
    completer_rule find_grammar_rule(const std::vector<parse_tree *> &, const size_t, dep_txala_status*) const;
    /// apply a completion rule
    parse_tree * applyRule(const completer_rule &, int, parse_tree*, parse_tree*, dep_txala_status*) const;
    /// Extract values for requested atribute frm given node
    void extract_attrib(const std::wstring &attr, const std::list<parse_tree::const_iterator> &nds, std::list<std::wstring> &val) const;
    /// Locate actual node for given path
    void locate_node(const std::vector<parse_tree *> &trees, const size_t chk, 
                     const std::wstring &node, std::list<parse_tree::const_iterator> &res) const;
    /// check if the extra lemma/form/class conditions are satisfied
    bool match_condition(parse_tree::const_iterator, const matching_condition &) const;
    /// check if the current context matches the given rule
    bool matching_context(const std::vector<parse_tree *> &, const size_t, const completer_rule &) const;
    /// check if the operation is executable (for last_left/last_right cases)
    bool matching_operation(const std::vector<parse_tree *> &, const size_t, const completer_rule &, dep_txala_status*) const;
  /// Check if the chunk pair matches pair condition specified in the given rule.
    bool matching_pair(const std::vector<parse_tree *> &trees, const size_t chk, const completer_rule &r) const;
    /// check left or right context
    bool match_side(const int, const std::vector<parse_tree *> &, const size_t, const std::vector<matching_condition> &) const;
    /// Find out if currently active flags enable the given rule
    bool enabled_rule(const completer_rule &, dep_txala_status*) const;
    /// find best completions for given parse tree
    parse_tree complete(parse_tree &, const std::wstring &, dep_txala_status*) const;

    /// function labelling methods ---------------

    /// Label nodes in a dependency tree. (recursive)
    void label(dep_tree*, dep_tree::iterator) const;
    /// Label nodes in a dependency tree. (Initial call)
    void label(dep_tree*) const;

    /// recursively convert parse_tree to dependency tree
    static dep_tree* dependencies(parse_tree::iterator, parse_tree::iterator);

  public:   
    /// constructor
    dep_txala(const std::wstring &, const std::wstring &);
    /// destructor
    ~dep_txala();

    /// apply completion rules to get a full parse tree
    void complete_parse_tree(sentence &) const;
    /// apply completion rules to get a full parse tree
    void complete_parse_tree(std::list<sentence> &) const;
    /// apply completion rules to get a full parse tree
    void complete_parse_tree(document &) const;
    /// convert parse_tree to dependency tree
    static dep_tree* parse2dep(parse_tree &);

    /// analyze given sentences, obtain dependency tree 
    void analyze(sentence &) const;

    /// inherit other methods
    using processor::analyze;
   
  };

} // namespace

#endif

