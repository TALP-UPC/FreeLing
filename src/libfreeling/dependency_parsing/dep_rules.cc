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

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dep_rules.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DEPENDENCIES"
#define MOD_TRACECODE DEP_TRACE

  //---------- Class matching_attrib ----------------------------------

  ////////////////////////////////////////////////////////////////
  /// constructor
  ////////////////////////////////////////////////////////////////

  matching_attrib::matching_attrib() : re(L"") {}

  ////////////////////////////////////////////////////////////////
  /// copy
  ////////////////////////////////////////////////////////////////

  matching_attrib::matching_attrib(const matching_attrib &ma) : re(ma.re) {
    type=ma.type; value=ma.value;
  }

  ////////////////////////////////////////////////////////////////
  /// constructor
  ////////////////////////////////////////////////////////////////

  matching_attrib::~matching_attrib() {}


  //---------- Class completer_rule ----------------------------------

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  completer_rule::completer_rule() {
    weight=0;
    line=L"-";
    pairClass=L"-";
    context_neg=false;
  }

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  completer_rule::completer_rule(const wstring &pnewNode1, const wstring &pnewNode2, const wstring &poperation) {
    operation=poperation;
    newNode1=pnewNode1;
    newNode2=pnewNode2;
    line=L"-";
    context_neg=false;
    weight=0;
    pairClass=L"-";
  }


  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  completer_rule::completer_rule(const completer_rule & cr) {
    leftChk=cr.leftChk;     rightChk=cr.rightChk;
    leftConds=cr.leftConds; rightConds=cr.rightConds;
    newNode1=cr.newNode1;   newNode2=cr.newNode2;
    matchingCond=cr.matchingCond;
    leftContext=cr.leftContext; rightContext=cr.rightContext;
    operation=cr.operation;
    weight=cr.weight;
    context_neg=cr.context_neg;
    line=cr.line;
    enabling_flags = cr.enabling_flags;
    flags_toggle_on = cr.flags_toggle_on;
    flags_toggle_off = cr.flags_toggle_off;
    pairClass = cr.pairClass;
    node1 = cr.node1;  node2 = cr.node2;
    attr1 = cr.attr1;  attr2 = cr.attr2;
  }

  ////////////////////////////////////////////////////////////////
  ///  Assignment
  ////////////////////////////////////////////////////////////////

  completer_rule & completer_rule::operator=( const completer_rule & cr) {
    leftChk=cr.leftChk;     rightChk=cr.rightChk;
    leftConds=cr.leftConds; rightConds=cr.rightConds;
    newNode1=cr.newNode1;   newNode2=cr.newNode2;
    operation=cr.operation;
    matchingCond=cr.matchingCond;
    leftContext=cr.leftContext; rightContext=cr.rightContext;
    weight=cr.weight;
    context_neg=cr.context_neg;
    line=cr.line;
    enabling_flags = cr.enabling_flags;
    flags_toggle_on = cr.flags_toggle_on;
    flags_toggle_off = cr.flags_toggle_off;  
    pairClass = cr.pairClass;
    node1 = cr.node1;  node2 = cr.node2;
    attr1 = cr.attr1;  attr2 = cr.attr2;
    return *this;
  }

  ////////////////////////////////////////////////////////////////
  ///  Comparison. The smaller weight, the higher priority 
  ////////////////////////////////////////////////////////////////

  int completer_rule::operator<(const completer_rule & a ) const  { 
    return ((weight<a.weight) && (weight>0));
  }


  //---------- Class rule_expression (and derived) -------------------

  ///////////////////////////////////////////////////////////////
  /// Constructor for basic rule_expression (AND, NOT) 
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression() : wordclasses(NULL), semdb(NULL) {}

  ///////////////////////////////////////////////////////////////
  /// Constructor for basic rule_expression (AND, NOT) 
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression(expression_type t) : type(t), wordclasses(NULL), semdb(NULL) {}

  ///////////////////////////////////////////////////////////////
  /// Constructor for basic rule_expression (AND,NOT) with a subexpression
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression(expression_type t, 
                                   const rule_expression &re) : type(t), 
                                                                wordclasses(NULL), 
                                                                pairclasses(NULL), 
                                                                semdb(NULL)  {
    assert (t==AND or t==NOT);
    check_list.push_back(re);
  }

  ///////////////////////////////////////////////////////////////
  /// Constructor of a basic expression (side, lemma, pos, category...)
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression(expression_type t, 
                                   const std::wstring &nd, 
                                   const std::wstring &val) : node1(nd), type(t), 
                                                              wordclasses(NULL), 
                                                              pairclasses(NULL), 
                                                              semdb(NULL)  {
    valueList=util::wstring2set(val,L"|");

    if (type==SIDE and (valueList.size()>1 or (val!=L"right" and val!=L"left"))) 
      WARNING(L"Error reading dependency rules. Invalid condition "+node1+L".side="+val+L". Must be one of 'left' or 'right'.");
  }

  ///////////////////////////////////////////////////////////////
  /// Constructor of wordclass expression
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression(expression_type t, 
                                   const std::wstring &nd, 
                                   const std::wstring &val, 
                                   const std::set<std::wstring> &wclas) : node1(nd), type(t), 
                                                                          wordclasses(&wclas), 
                                                                          pairclasses(NULL), 
                                                                          semdb(NULL) {
    assert (t==WORDCLASS);
    valueList=util::wstring2set(val,L"|");
  }

  ///////////////////////////////////////////////////////////////
  /// Constructor of pairclass expression
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression(expression_type t, 
                                   const std::wstring &nd1, 
                                   const std::wstring &nd2, 
                                   const std::wstring &val, 
                                   const std::set<std::wstring> &pclas) : node1(nd1), node2(nd2), type(t), 
                                                                          wordclasses(NULL),
                                                                          pairclasses(&pclas), 
                                                                          semdb(NULL) {
    assert (t==PAIRCLASS);
    valueList=util::wstring2set(val,L"|");
  }

  ///////////////////////////////////////////////////////////////
  /// Constructor of SEMDB expression (tonto, semfile, synon, asynon)
  ///////////////////////////////////////////////////////////////

  rule_expression::rule_expression(expression_type t, 
                                   const std::wstring &nd, 
                                   const std::wstring &val, 
                                   const semanticDB &sdb) : node1(nd), type(t), 
                                                            wordclasses(NULL),
                                                            pairclasses(NULL), 
                                                            semdb(&sdb) {

    assert (type==TONTO or type==SEMFILE or type==SYNON or type==ASYNON);
    valueList=util::wstring2set(val,L"|");    
  }

  ///////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  // no need to destroy wordclasses and semdb, since we get them from outside
  rule_expression::~rule_expression() {}

  ///////////////////////////////////////////////////////////////
  /// set expression type
  ///////////////////////////////////////////////////////////////

  void rule_expression::set_type(expression_type t) { type = t; }

  ///////////////////////////////////////////////////////////////
  /// add a subexpression to the list (for AND)
  ///////////////////////////////////////////////////////////////

  void rule_expression::add(const rule_expression &re) {
    assert (type==AND or (type==NOT and check_list.empty()));
    check_list.push_back(re);
  }


  ///////////////////////////////////////////////////////////////
  /// Search for a value in the list of an expression
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find(const wstring &v) const {
    return (valueList.find(v) != valueList.end());
  }

  ///////////////////////////////////////////////////////////////
  /// Match the value against a RegExp
  ///////////////////////////////////////////////////////////////

  bool rule_expression::match(const wstring &v) const {
    freeling::regexp re(util::set2wstring(valueList,L"|"));
    return (re.search(v));
  }

  ///////////////////////////////////////////////////////////////
  /// Search for any value of a list in the list of an expression
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find_any(const list<wstring> &ls) const {
    bool found=false;
    for (list<wstring>::const_iterator s=ls.begin(); !found && s!=ls.end(); s++)
      found=find(*s);
    return(found);
  }


  ///////////////////////////////////////////////////////////////
  /// Search for a value in the list of an expression, 
  /// taking into account wildcards
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find_match(const wstring &v) const {
    for (set<wstring>::const_iterator i=valueList.begin(); i!=valueList.end(); i++) {
      TRACE(4,L"      eval "+node1+L".label="+(*i)+L" (it is "+v+L")");
      // check for plain match
      if (v==(*i)) return true;  
      // not straight, check for a wildcard
      wstring::size_type p=i->find_first_of(L"*");
      if (p!=wstring::npos && i->substr(0,p)==v.substr(0,p)) 
        // there is a wildcard and matches
        return true;
    }
    return false;
  }


  ///////////////////////////////////////////////////////////////
  /// Search for any value of a list in the list of an expression,
  /// taking into account wildcards
  ///////////////////////////////////////////////////////////////

  bool rule_expression::find_any_match(const list<wstring> &ls) const {
    bool found=false;
    for (list<wstring>::const_iterator s=ls.begin(); !found && s!=ls.end(); s++)
      found=find_match(*s);
    return(found);
  }

  ///////////////////////////////////////////////////////////////
  /// Recursive disassembly of node reference string (e.g. p:sn:sajd)
  /// to get the right iterator. When (if) found, add it to given list.
  ///////////////////////////////////////////////////////////////

  void rule_expression::parse_node_ref(wstring nd, dep_tree::const_iterator k, list<dep_tree::const_iterator> &res) {

    wstring top;
    if (nd.size()==0)
      res.push_back(k);
    else {
      TRACE(4,L"       recursing at "+nd+L", have parent "+k->get_link()->get_label());
      wstring::size_type t=nd.find(L':');
      if (t==wstring::npos) {
        top=nd;
        nd=L"";
      }
      else {
        top=nd.substr(0,t);
        nd=nd.substr(t+1);
      }
    
      TRACE(4,L"        need child "+nd);
      dep_tree::const_sibling_iterator j;
      for (j=k.sibling_begin(); j!=k.sibling_end(); ++j) {
        TRACE(4,L"           looking for "+top+L", found child with: "+j->get_link()->get_label());
        if (j->get_link()->get_label() == top)
          parse_node_ref(nd,j,res);
      }  
    }  
  }

  ///////////////////////////////////////////////////////////////
  /// Recursive disassembly of node reference string (e.g. p:sn:sajd)
  /// to get the right iterator. When (if) found, add it to given list.
  ///////////////////////////////////////////////////////////////

  void rule_expression::parse_node_ref(wstring nd, parse_tree::const_iterator k, list<parse_tree::const_iterator> &res) {

    wstring top;
    if (nd.size()==0) {
      TRACE(4,L"       reached target node, have parent "+k->get_label());
      res.push_back(k);
    }
    else {
      TRACE(4,L"       recursing at "+nd+L", have parent "+k->get_label());
      wstring::size_type t=nd.find(L':');
      if (t==wstring::npos) {
        top=nd;
        nd=L"";
      }
      else {
        top=nd.substr(0,t);
        nd=nd.substr(t+1);
      }
    
      TRACE(4,L"        need child "+nd);
      parse_tree::const_sibling_iterator j;
      for (j=k.sibling_begin(); j!=k.sibling_end(); ++j) {
        TRACE(4,L"           looking for "+top+L", found child with: "+j->get_label());
        if (j->get_label() == top)
          parse_node_ref(nd,j,res);
      }  
    }  
  }




  ///////////////////////////////////////////////////////////////
  /// Givent parent and daughter iterators, resolve which of them 
  /// is to be checked in this condition.
  ///////////////////////////////////////////////////////////////

  bool rule_expression::nodes_to_check(const wstring &node,
                                       dep_tree::const_iterator p,
                                       dep_tree::const_iterator d, 
                                       list<dep_tree::const_iterator> &res) const{
    wstring top, nd;
    wstring::size_type t=node.find(L':');
    if (t==wstring::npos) {
      top=node;
      nd=L"";
    }
    else {
      top=node.substr(0,t);
      nd=node.substr(t+1);
    }

    if (top==L"p") 
      parse_node_ref(nd,p,res);
    else if (top==L"d") 
      parse_node_ref(nd,d,res);
    else if (top==L"As" or top==L"Es") {
      // add to the list all children (except d) of the same parent (p)
      for (dep_tree::const_sibling_iterator s=p.sibling_begin(); s!=p.sibling_end(); ++s)
        if (s!=d)
          parse_node_ref(nd,s,res);
    }

    return (top==L"As"); // return true for AND, false for OR.
  }



  ///////////////////////////////////////////////////////////////
  /// Check wheter a rule_expression can be applied to the
  /// given pair of nodes
  ///////////////////////////////////////////////////////////////

  bool rule_expression::check(dep_tree::const_iterator ancestor, dep_tree::const_iterator descendant) const {

    bool res=false;

    switch (type) {
      case AND: {
        TRACE(4,L"      eval AND");
        res=true;
        list<rule_expression>::const_iterator ci=check_list.begin();
        while(res && ci!=check_list.end()) { 
          res=ci->check(ancestor,descendant); 
          ++ci;
        }
        break;
      }
      case NOT: {
        TRACE(4,L"      eval NOT");
        res = not check_list.begin()->check(ancestor,descendant);
        break;
      }
      case SIDE: {
        wstring side=*valueList.begin();
        res=false;
        TRACE(4,L"      eval SIDE="+side+L" node="+node1);
        TRACE(4,L"          d="+util::int2wstring(descendant->get_word().get_position())
              +L" p="+util::int2wstring(ancestor->get_word().get_position()) );
        if ((side==L"left" && node1==L"d") || (side==L"right" && node1==L"p")) 
          res= (descendant->get_word().get_position())<(ancestor->get_word().get_position());
        else if ((side==L"left" && node1==L"p") || (side==L"right" && node1==L"d")) 
          res = (descendant->get_word().get_position())>(ancestor->get_word().get_position());
        
        TRACE(4,L"          result = "+wstring(res?L"true":L"false"));
        break;
      }
      case PAIRCLASS: {        
        TRACE(4,L"      eval PAIRCLASS");
        list<dep_tree::const_iterator> ln;

        int p = node1.rfind(L".");
        nodes_to_check(node1.substr(0,p), ancestor, descendant, ln);          
        p = node2.rfind(L".");
        nodes_to_check(node2.substr(0,p), ancestor, descendant, ln);
        
        list<dep_tree::const_iterator>::const_iterator n1 = ln.begin();
        list<dep_tree::const_iterator>::const_iterator n2 = n1;
        n2++;

        TRACE(4,L"      nodes "+(*n1)->get_word().get_lemma()+L","+(*n2)->get_word().get_lemma());
        res=eval(*n1,*n2);
        break;
      }

      default: { // any other: LEMMA, POS, LABEL, WORDCLASS, TONTO, SEMFILE, SYNON, ASYNON
        list<dep_tree::const_iterator> ln;
        
        // if "which_ao"=true then "eval" of all nodes in "ln" must be joined with AND
        // if it is false, and OR must be used 
        bool which_ao = nodes_to_check(node1, ancestor, descendant, ln);  
        if (ln.empty()) return false;
        
        TRACE(4,L"      found nodes to check.");
        
        // start with "true" for AND and "false" for OR
        res= which_ao; 
        // the loop goes on when res==true for AND and when res==false for OR
        for (list<dep_tree::const_iterator>::iterator n=ln.begin(); n!=ln.end() and (res==which_ao); n++) {
          TRACE(4,L"      checking node.");
          res=eval(*n);
        }
        break;
      }
    }

    return res;
  }


  ///////////////////////////////////////////////////////////////
  /// access requested information (lemma, pos, label) from given node
  ///////////////////////////////////////////////////////////////

  wstring rule_expression::extract_value(const wstring &fun, dep_tree::const_iterator n) const {

    if (fun==L"lemma") return n->get_word().get_lemma();
    else if (fun==L"pos") return n->get_word().get_tag();
    else if (fun==L"label") return n->get_link()->get_label();
    else {
      WARNING(L"Ignoring value "+fun+L" used in pairclass expression. Not supported in this version");
      return L"";
    }
  }

  ///////////////////////////////////////////////////////////////
  /// eval whether a single node matches a condition.
  ///////////////////////////////////////////////////////////////

  bool rule_expression::eval(dep_tree::const_iterator n1, dep_tree::const_iterator n2) const {

    bool found=false;

    switch (type) {
      case LEMMA: {
        TRACE(4,L"      evaluate "+node1+L".lemma "+n1->get_word().get_lemma());
        found = find(n1->get_word().get_lemma());
        break;
      }

      case POS: {
        TRACE(4,L"      evaluate "+node1+L".pos "+n1->get_word().get_tag());
        found = match(n1->get_word().get_tag());
        break;
      }

      case LABEL: {
        TRACE(4,L"      evaluate "+node1+L".label "+n1->get_link()->get_label());
        found = find_match(n1->get_link()->get_label());
        break;
      }

      case WORDCLASS: {
        TRACE(4,L"      evaluate "+node1+L".class="+util::set2wstring(valueList,L"|")+L" ? lemma="+ n1->get_word().get_lemma());
        found = false;
        for (set<wstring>::const_iterator wclass=valueList.begin(); !found && wclass!=valueList.end(); wclass++)
          found = (wordclasses->find((*wclass)+L"#"+(n1->get_word().get_lemma())) != wordclasses->end());
        break;
      }

      case PAIRCLASS: {        
        int p = node1.rfind(L".");  wstring val1 = extract_value(node1.substr(p+1),n1);
        p = node2.rfind(L".");      wstring val2 = extract_value(node2.substr(p+1),n2);

        TRACE(4,L"      evaluate ["+node1+L","+node2+L"].pairclass="+util::set2wstring(valueList,L"|")+L" ? values=["+ val1 + L"," + val2 +L"]");
        found = false;
        for (set<wstring>::const_iterator pclass=valueList.begin(); !found && pclass!=valueList.end(); pclass++)
          found = (pairclasses->find((*pclass) + L"#" + val1 + L"#" + val2) != pairclasses->end());
        break;
      }

      default: { // any other: TONTO, SEMFILE, SYNON, ASYNON
        assert (type==TONTO or type==SEMFILE or type==SYNON or type==ASYNON);

        wstring form=n1->get_word().get_lc_form();
        wstring lem=n1->get_word().get_lemma();
        wstring pos=n1->get_word().get_tag().substr(0,1);
        list<wstring> sens = semdb->get_word_senses(form,lem,pos);

        #ifdef VERBOSE
          map<expression_type,wstring> m = {{TONTO,L"tonto"}, {SEMFILE,L"semfile"}, 
                                          {SYNON,L"synon"}, {ASYNON,L"ASYNON"}};
          TRACE(4,L"      evaluate "+node1+L"."+m[type]+L" "+n1->get_link()->get_label());
        #endif

        found=false;
        for (list<wstring>::iterator s=sens.begin(); !found && s!=sens.end(); s++) {
          switch (type) {
            case TONTO:
              found = find_any(semdb->get_sense_info(*s).tonto);
              break;
              
            case SEMFILE:
              found = find(semdb->get_sense_info(*s).semfile);
              break;
              
            case SYNON:
              found = find_any(semdb->get_sense_info(*s).words);
              break;
              
            case ASYNON:
              found = find_any(semdb->get_sense_info(*s).words);
              // if not found, enlarge the list of senses to explore
              // with all parents of the current sense
              if (!found) {
                list<wstring> lpar=semdb->get_sense_info(*s).parents;
                if (lpar.size()>0) sens.splice(sens.end(),lpar);
              }
                            
            default: break; // we will never reach here
          }
        }
        break;
      }
    }

    return found;
  }


  //---------- Class labeler_rule -------------------

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  labeler_rule::labeler_rule() {}

  ////////////////////////////////////////////////////////////////
  ///  Destructor
  ////////////////////////////////////////////////////////////////

  labeler_rule::~labeler_rule() {}

  ////////////////////////////////////////////////////////////////
  ///  Constructor
  ////////////////////////////////////////////////////////////////

  labeler_rule::labeler_rule(const wstring &plabel, const rule_expression &pre) {
    re=pre;
    label=plabel; 
  }

  ////////////////////////////////////////////////////////////////
  ///  Evaluate rule conditions
  ////////////////////////////////////////////////////////////////

  bool labeler_rule::check(dep_tree::const_iterator ancestor, dep_tree::const_iterator descendant) const {
    TRACE(4,L"      eval labeler_rule");
    return re.check(ancestor, descendant);
  }

} // namespace
