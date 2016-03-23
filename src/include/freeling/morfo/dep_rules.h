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

#ifndef _DEPRULES
#define _DEPRULES

#include <sstream>
#include <iostream>
#include <set>
#include <list>

#include "freeling/regexp.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/semdb.h"

namespace freeling {
  
  class dep_txala;

  ///////////////////////////////////////////////////////////////
  ///  The class matching attrib stores attributes for maching condition
  ///////////////////////////////////////////////////////////////

  class matching_attrib {
  public:
    /// condition type ("<": lemma, "(": form, "[": class, "{": PoS)
    std::wstring type;
    /// string to match (for lemma, form and class)
    std::wstring value;
    /// regexp to match (for PoS)
    freeling::regexp re;

    /// constructor
    matching_attrib();
    /// copy
    matching_attrib(const matching_attrib &);
    /// destructor
    ~matching_attrib();     
  };

  ////////////////////////////////////////////////////////////////
  ///  The class matching condition stores a condition used
  /// in a MATCHING operation of a completer rule. Also used
  /// in context and chunk expressions in completer rules.
  ////////////////////////////////////////////////////////////////

  class matching_condition {
  public:
    bool neg;
    std::wstring label;
    std::list<matching_attrib> attrs;
  };


  ////////////////////////////////////////////////////////////////
  ///
  ///  The class completer_rule stores rules used by the
  /// completer of parse trees
  ///
  ////////////////////////////////////////////////////////////////

  class completer_rule {
 
  public:
    /// line in the file where rule was, useful to trace and issue errors.
    /// Used also as rule id when storing last_left/right matches in status
    std::wstring line;

    /// chunk labels to which the rule is applied
    std::wstring leftChk;
    std::wstring rightChk;
    /// extra conditions on the chunks (pos, lemma, form, class)
    matching_condition leftConds;
    matching_condition rightConds;

    /// pair class to check for additional semantic compatibility
    std::wstring pairClass;
    /// properties of each chunk to check in pair file
    std::wstring node1;
    std::wstring attr1;
    std::wstring node2;
    std::wstring attr2;

    /// new label/s (if any) for the nodes after the operation. 
    std::wstring newNode1;
    std::wstring newNode2;
    /// condition for MATCHING operation.
    matching_condition matchingCond;
    /// operation to perform
    std::wstring operation;
    /// context (if any) required to apply the rule
    std::vector<matching_condition> leftContext;
    std::vector<matching_condition> rightContext;
    /// whether the context is negated
    bool context_neg;
    /// priority of the rule
    int weight;

    /// flags that enable the rule to be applied
    std::set<std::wstring> enabling_flags;
    /// flags to toggle on after applying the rule
    std::set<std::wstring> flags_toggle_on;
    /// flags to toggle off after applying the rule
    std::set<std::wstring> flags_toggle_off;
   
    /// constructors
    completer_rule();
    completer_rule(const std::wstring &, const std::wstring &, const std::wstring&);
    completer_rule( const completer_rule &);
    /// assignment
    completer_rule & operator=( const completer_rule &);
   
    /// Comparison. The more weight the higher priority 
    int operator<(const completer_rule & a ) const;
  };




  ////////////////////////////////////////////////////////////////
  /// The class rule_expression stores the conditions for a labeler_rule.
  ////////////////////////////////////////////////////////////////

  class rule_expression {
  friend class dep_txala;

  protected:
    static void parse_node_ref(std::wstring, dep_tree::const_iterator, std::list<dep_tree::const_iterator> &);
    static void parse_node_ref(std::wstring, parse_tree::const_iterator, std::list<parse_tree::const_iterator> &);

    // node the expression has to be checked against (p/d)
    std::wstring node1,node2;
    // set of values (if any) to check against.
    std::set<std::wstring> valueList;
    // obtain the iterator to the nodes to be checked 
    // and the operation AND/OR to perform
    bool nodes_to_check(const std::wstring &node, dep_tree::const_iterator, dep_tree::const_iterator, std::list<dep_tree::const_iterator> &) const;

  public:
    // types of operations that the conditions may perform
    typedef enum {AND,NOT,SIDE,LEMMA,POS,LABEL,WORDCLASS,TONTO,SEMFILE,SYNON,ASYNON,PAIRCLASS} expression_type;

    // constructors and destructors
    rule_expression();
    rule_expression(expression_type t);
    rule_expression(expression_type t, const rule_expression &re);
    rule_expression(expression_type t, const std::wstring &nd, const std::wstring &val);
    rule_expression(expression_type t, const std::wstring &nd, const std::wstring &val, const std::set<std::wstring> &wclas);
    rule_expression(expression_type t, const std::wstring &nd1, const std::wstring &nd2, const std::wstring &val, const std::set<std::wstring> &pairclas);
    rule_expression(expression_type t, const std::wstring &nd, const std::wstring &val, const semanticDB &sdb);
    ~rule_expression();

    // add a subexpression to the list
    void add(const rule_expression &re);
    // set expression type;
    void set_type(expression_type t);

    // search a value in expression list
    bool find(const std::wstring &) const;
    bool find_match(const std::wstring &) const;
    bool match(const std::wstring &) const;
    bool find_any(const std::list<std::wstring> &) const;
    bool find_any_match(const std::list<std::wstring> &) const;

    // evaluate expressions involving parent and daughter (AND, NOT, A.x, E.x, side)
    bool check(dep_tree::const_iterator, dep_tree::const_iterator) const;
    // evaluate expressions involving one node and one constant value (lemma, class, pos, label, tonto, semfile, synon, asynon)
    bool eval(dep_tree::const_iterator n1, dep_tree::const_iterator n2=(const dep_tree*)NULL) const;

  private:
    // type of the expression
    expression_type type;
    // list of subexpressions (many for type=AND, one for type=NOT, empty for others)
    std::list<rule_expression> check_list;
    // word class set from labeler we belong to (only needed for type=WORDCLASS)
    const std::set<std::wstring> *wordclasses;
    // pair class set from labeler we belong to (only needed for type=PAIRCLASS)
    const std::set<std::wstring> *pairclasses;
    // semantic DB  from labeler we belong to (only needed for type=TONTO,SEMFILE,SYNON,ASYNON)
    const semanticDB *semdb;
 
    /// access requested information (lemma, pos, label) from given node
    std::wstring extract_value(const std::wstring &fun, dep_tree::const_iterator n) const;
       
  };

  ////////////////////////////////////////////////////////////////
  /// labeler_rule is an auxiliary class for the depLabeler
  ////////////////////////////////////////////////////////////////

  class labeler_rule {

  public:
    std::wstring label;
    rule_expression re;
    std::wstring ancestorLabel;
    /// line in the file where rule was, useful to trace and issue errors
    std::wstring line;

    labeler_rule();
    labeler_rule(const std::wstring &, const rule_expression &);
    ~labeler_rule();
    bool check(dep_tree::const_iterator, dep_tree::const_iterator) const;
  };

} // namespace

#endif
