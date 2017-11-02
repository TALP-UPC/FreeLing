//////////////////////////////////////////////////////////////////
//
//    FreeLing - Open Source Language Analyzers
//
//    Copyright (C) 2004   TALP Research Center
//                         Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public
//    License as published by the Free Software Foundation; either
//    version 3 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: Lluis Padro (padro@lsi.upc.es)
//             TALP Research Center
///             despatx C6.212 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
////////////////////////////////////////////////////////////////


#include <sstream>
#include <fstream>

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/rel_extract_SPR.h"

#include <list>
#include <map>

using namespace std;

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"SEMGRAPH_REL_SPR"
#define MOD_TRACECODE SEMGRAPH_TRACE

namespace freeling {

  ////////////////////////////////////////////////////
  /////                                          /////
  /////        Class rel_extract_SPR             /////
  /////                                          /////
  /////  This class derives a relation extractor /////
  /////  for semantic graphs, based on Syntactic /////
  /////  Pattern Rules (SPR) over a dependency   /////
  /////  tree.                                   /////
  /////  It requires the input document is       /////
  /////  dependency parsed and NEC.              /////
  /////                                          /////
  ////////////////////////////////////////////////////
  
  
  //// AUXILIARY CLASSES ///
  
  /////////////////////////////////////////////////////
  /// Constructor: build rule given regex string and key positions
  
  rx_rule::rx_rule(const std::wstring& rid, const wstring &r, 
                   const wstring &e1, const wstring &rel, const wstring &e2) {
    // store rule identifier
    id = rid;
    
    // store varaibles to extract
    entity1=e1; relation=rel; entity2=e2;
    
    // normalize rule
    wstring rs=r; 
    freeling::util::find_and_replace(rs,L" ",L"");   // Remove whitespaces.
    freeling::util::find_and_replace(rs,L"<",L"< "); // Insert whitespaces after each < or >
    freeling::util::find_and_replace(rs,L">",L"> "); // to mark splitting points. 
    TRACE(3,L"Loading rule. normalized pattern is: "+rs);
    
    // split at whitespaces, to get single nodes (plus ">" or "<")
    list<wstring> ln = freeling::util::wstring2list(rs,L" ");
    
    for (list<wstring>::iterator pt=ln.begin(); pt!=ln.end(); pt++) {
      // for each node pattern in the rule
      // store direction for next node, and remove it from the pattern
      node_pattern np;
      np.dir=0;
      size_t s=pt->find_last_of(L"<>");
      if (s!=wstring::npos) {
        if ((*pt)[s]==L'<') np.dir= -1; // down
      else if ((*pt)[s]==L'>') np.dir= 1; // up
        pt->replace(s,1,L"");
      }
      
      if ((*pt)==L"...")
        np.flex = true;
      else {
        np.flex = false;
        // split node components (lemma:PoS:function)
        vector<wstring> nv = freeling::util::wstring2vector(*pt,L":");
        np.lemma = nv[0]; np.pos= nv[1]; np.funct= nv[2];
      }
      
      // store node pattern
      rpath.push_back(np);
    }
  }
  
  /////////////////////////////////////////////////////
  /// Destructor for class  destructor
  
  rx_rule::~rx_rule() {}
  
  
  
  ////////////////////////////////////////////////////
  /////        Class rel_extract_SPR             /////
  ////////////////////////////////////////////////////
  
  /////////////////////////////////////////////////////
  /// Constructor 
  
  rel_extract_SPR::rel_extract_SPR(const wstring &erFile) : rel_extract(erFile) {
    
    wstring path=erFile.substr(0,erFile.find_last_of(L"/\\")+1);
    
    enum sections {SG_TYPE,EXT_RULES};
    config_file cfg(true);
    cfg.add_section(L"ExtractorType",SG_TYPE);
    cfg.add_section(L"ExtractionRules",EXT_RULES);
    
    if (not cfg.open(erFile))
      ERROR_CRASH(L"Error opening file "+erFile);
    
    wstring line; 
    while (cfg.get_content_line(line)) {
      
      // process each content line according to the section where it is found
      switch (cfg.get_section()) {
        
      case SG_TYPE: {
        if (line!=L"SPR")
          ERROR_CRASH(L"Invalid configuration file '"+erFile+L"' for SPR semgraph extractor.");
        break;
      }
        
      case EXT_RULES: {
        // read rule (anything before "extract" keyword)
        size_t p = line.find(L"extract");
        wstring rex = line.substr(0,p); 
        // skip "extract" keyword
        line = line.substr(p+7); 
        // read extraction variables for SVO
        wistringstream sin;  
        sin.str(line);
        wstring e1,rel,e2;
        sin>>e1>>rel>>e2;
        // add rule to current package 'pname'
        xrules.push_back(rx_rule(freeling::util::int2wstring(cfg.get_line_num()),rex,e1,rel,e2));      
        break;
      }
      }
    }
    cfg.close(); 
    
    if (A0_label.empty())
      ERROR_CRASH(L"A0_label not specified in 'ArgumentRoles' section in semgraph configuration file'"+erFile+L"'");
    if (A1_label.empty())
      ERROR_CRASH(L"A1_label not specified in 'ArgumentRoles' section in semgraph configuration file'"+erFile+L"'");
    
    TRACE(2,L"Module successfully created");
  }
  
  
  /////////////////////////////////////////////////////
  /// Destructor 
  
  rel_extract_SPR::~rel_extract_SPR() {}
  
  
  
  /////////////////////////////////////////////////////
  /// extract entities and relations from given sentences, 
  /// using requested rule package.
  
  void rel_extract_SPR::extract_relations(freeling::document &doc) const {

    /// Syntactic pattern-matching extraction requested 
    
    semgraph::semantic_graph &sg = doc.get_semantic_graph();

    TRACE(2,L"Extracting relations using SPM rules");
    
    /// if requested package doesn't contain any rules, save the trouble.
    if (xrules.empty()) {
      TRACE(2,L"Pack is empty");
      return;
    }
        
    // for each sentence in document
    int ns=1;
    for (freeling::document::const_iterator ls=doc.begin(); ls!=doc.end(); ls++) {
      for(list<freeling::sentence>::const_iterator s = ls->begin(); s != ls->end(); ++s, ++ns) {

        int best = s->get_best_seq();
        // get dependency tree, and iterate over all nodes
        const freeling::dep_tree &t=s->get_dep_tree(best); 
        
        for (freeling::dep_tree::const_preorder_iterator nd=t.begin(); nd!=t.end(); nd++) {
          
          TRACE(3,L"  Checking depnode "+nd->get_label());
          // if it is the virtual root, skip it
          if (nd->get_label()==L"VIRTUAL_ROOT") continue;
          // check each possible rule for current node
          list<rx_rule>::const_iterator r;
          for (r=xrules.begin(); r!=xrules.end(); r++) {
            TRACE(4,L"Checking rule "+r->id+L" at node "+nd->get_word().get_form());
            map<wstring,freeling::dep_tree::const_iterator> values;
            if (match_rule(nd, r->rpath.begin(), values, best)) {
              
              const freeling::word & wd = values[r->relation]->get_word();

              semgraph::SG_frame fr(wd.get_lemma(best),
                                    (wd.get_senses(best).empty() ? L"" : wd.get_senses(best).begin()->first),
                                    s->get_sentence_id() + L"." + freeling::util::int2wstring(wd.get_position()+1),
                                    s->get_sentence_id());
              wstring fid = sg.add_frame(fr);
              TRACE(4,L"Match found. Extracting relation id="+fr.get_id()+L" "+fr.get_lemma());
              
              wstring wsarg,wsrole;
              // add subject argument
              const freeling::word & sbj = values[r->entity1]->get_word();
              wsarg = find_entity_node(sg, *s, sbj);
              wsrole = argument_to_role(A0_label, fr.get_sense());
              sg.add_argument_to_frame(fid, wsrole, wsarg);
              TRACE(4,L"Added subject "+wsrole+L":"+wsarg);
              
              // add object argument
              const freeling::word & obj = values[r->entity2]->get_word();
              wsarg = find_entity_node(sg, *s, obj);
              wsrole = argument_to_role(A1_label, fr.get_sense());
              sg.add_argument_to_frame(fid, wsrole, wsarg);
              TRACE(4,L"Added object "+wsrole+L":"+wsarg);
            }
          }
          TRACE(3,L"reached end of rule pack for this node");
        }
      }
    }

    TRACE(2,L"Finished extracting relations");
  }

  
  ////////////////////////////////////////////////////////////////////
  /// recursively match a path given by a rule, starting at given node.
  
  bool rel_extract_SPR::match_rule(freeling::dep_tree::const_preorder_iterator nd, 
                                   list<node_pattern>::const_iterator p, 
                                   map<wstring,freeling::dep_tree::const_iterator> &val,
                                   int best) const {
    
    wstring synset=nd->get_word().get_senses_string(best);
    synset=synset.substr(0,synset.find(L":"));
    
    TRACE(5,L" Recursing. Item ("+p->lemma+L":"+p->pos+L":"+p->funct+L"), node "+nd->get_word().get_form()+L"  synset="+synset);
    
    if (p->flex) {   
      TRACE(5,L"   it is flex");
      // flexible pattern. Check wether current node matches next pattern
      list<node_pattern>::const_iterator q=p; q++;
      bool b=false;
      if (check_attr(q->lemma,nd->get_word().get_lemma(best),val) and
          check_attr(q->pos,nd->get_word().get_tag(best),val,true) and
          check_attr(q->funct,nd->get_label(),val)) {
        // current node matched. Store node
        val.insert(make_pair(p->lemma,nd));
        // continue with next node patterns      
        b = next_node(nd,q,val,best);
      }
      
      TRACE(5,L"   back from flex b="+(b?wstring(L"true"):wstring(L"false")));
      
      if (b) return true; // recursion matched skipping zero nodes
      else return next_node(nd,p,val,best); // no success on current node, or recursion failed. Try skipping this node
    }
    else if (check_attr(p->lemma,nd->get_word().get_lemma(best),val) and
             check_attr(p->pos,nd->get_word().get_tag(best),val,true) and
             check_attr(p->funct,nd->get_label(),val)) {
      // current node matched. Store node
      val.insert(make_pair(p->lemma,nd));
      // Recursively check rest of the rule.
      return next_node(nd,p,val,best);
    }
    
    TRACE(4,L"  rule match failed");
    return false;
  }
  
  
  ////////////////////////////////////////////////////////////////////
  /// Decide whether next recursive call is up or down in the tree
  
  bool rel_extract_SPR::next_node(freeling::dep_tree::const_preorder_iterator nd, 
                                  list<node_pattern>::const_iterator p, 
                                  map<wstring,freeling::dep_tree::const_iterator> &val,
                                  int best) const {
    
    TRACE(5,L"   next node. dir="+freeling::util::int2wstring(p->dir));
    
    // no direction -> rule ends here, just return.
    if (p->dir == 0) 
      return true;
    
    if (p->dir > 0) { // rule goes up. Pass rest of the rule to the parent
      if (not p->flex) p++;
      TRACE(5,L"   next node. calling parent.");
      return match_rule(nd.get_parent(), p, val, best);
    }
    else  { // p->dir<0, rule goes down. Check rest of the rule agains each children until one matches.
      list<node_pattern>::const_iterator q=p; q++;
      bool mch=false;
      for (freeling::dep_tree::const_sibling_iterator child=nd.sibling_begin(); not mch and child!=nd.sibling_end(); child++) {
        TRACE(5,L"   next node. calling child.");
        if (p->flex) mch = match_rule(child, p, val, best);
        else mch = match_rule(child, q, val, best);
      }
      return mch;
    }
    
  }
  
  
  /////////////////////////////////////////////////////
  /// match component against rule. Unify if rule contains variables.
  
  bool rel_extract_SPR::check_attr(const wstring &pat, const wstring &prop, map<wstring,freeling::dep_tree::const_iterator> &val, bool pref) const {
    
    TRACE(5,L"   checking attr '"+pat+L"' vs '"+prop+L"'");
    
    if (pat[0]==L'$') { // pattern is a variable, match
      return true;
    }
    else if (pat==L"*") // pattern is a wildcard, match
      return true;
    
    else 
      // pattern is a literal.
      // It must match the whole string, or only a prefix if pref=true
      return (pat==prop) or (pref and prop.find(pat)==0);
  }
  
}
