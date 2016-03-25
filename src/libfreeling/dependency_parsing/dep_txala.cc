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


#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/dep_rules.h"
#include "freeling/morfo/dep_txala.h"
#include "freeling/morfo/configfile.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"DEP_TXALA"
#define MOD_TRACECODE DEP_TRACE


  //---------- Class dep_txala ----------------------------------

  ///////////////////////////////////////////////////////////////
  /// constructor. Load a dependecy rule file.
  ///////////////////////////////////////////////////////////////

  dep_txala::dep_txala(const wstring & filename, const wstring & startSymbol) {

    TRACE(1,L"Loading dep_txala file "+filename);
    start = startSymbol;
    semdb = NULL;

    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
    wstring fname=filename.substr(filename.find_last_of(L"/\\")+1);

    enum sections {CLASS,PAIRS,GRPAR,SEMDB,GRLAB};
    config_file cfg(true,L"%");  
    cfg.add_section(L"CLASS",CLASS);
    cfg.add_section(L"PAIRS",PAIRS);
    cfg.add_section(L"GRPAR",GRPAR);
    cfg.add_section(L"SEMDB",SEMDB);
    cfg.add_section(L"GRLAB",GRLAB);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening dep_txala file "+filename);

    wstring line;
    wordclasses.clear();

    while (cfg.get_content_line(line)) {

      int lnum = cfg.get_line_num();

      switch (cfg.get_section()) {

      case CLASS: {
        // load CLASS section
        wstring vclass, vlemma;
        wistringstream sin;  sin.str(line);
        sin>>vclass>>vlemma;
        load_classes(vclass, vlemma, path, wordclasses);
        break;
      }

      case PAIRS: {
        // load PAIRS section
        wstring vclass, vlemma, vlem2;
        wistringstream sin;  sin.str(line);
        sin>>vclass>>vlemma>>vlem2;
        if (not vlem2.empty()) vlemma = vlemma+L"#"+vlem2;
        load_classes(vclass, vlemma, path, pairclasses);
        break;
      }

      case GRPAR: {
        ////// load GRPAR section defining tree-completion rules

        // if the line is an #include directive, load rules in given file
        if (line.substr(0,8)==L"#include") {
          wistringstream sline; sline.str(line);
          wstring key,includefile;
          sline>>key>>includefile;

          includefile = path+L"/"+includefile;
          wstring ifname=includefile.substr(includefile.find_last_of(L"/\\")+1);
          wifstream fs;
          util::open_utf8_file(fs,includefile);
          if (fs.fail())
            ERROR_CRASH(L"Error opening file "+includefile+L" included from dep_txala file "+filename);

          int ilnum=0;
          while (getline(fs,line)) {
            ilnum++;
            // skip comment and empty lines
            if (line.empty() or line.find(L"%")==0 ) continue;
            // load rule
            completer_rule r;
            r.line = fname+L"::"+ifname+L":"+util::int2wstring(ilnum);
            load_rule(line,r);
          }
        }        
        else {
          // if the line is a regular rule, just load it
          completer_rule r;
          r.line = fname+L":"+util::int2wstring(lnum);
          load_rule(line,r);
        }
        break;
      }

      case GRLAB: {

        ////// load GRLAB section defining labeling rules
        
        // Read first word in line
        wstring s;
        wistringstream sin;  sin.str(line);
        sin>>s;
        if (s==L"UNIQUE") {
          // "UNIQUE" key found, add labels in the line to the set
          wstring lab; 
          while (sin>>lab) unique.insert(lab);
        }

        else if (s==L"#include") {
          // if the line is an #include directive, load rules in given file            
          wstring includefile;
          sin>>includefile;
          includefile = path+L"/"+includefile;
          wstring ifname=includefile.substr(includefile.find_last_of(L"/\\")+1);
          wifstream fs;
          util::open_utf8_file(fs,includefile);
          if (fs.fail())
            ERROR_CRASH(L"Error opening file "+includefile+L" included from dep_txala file "+filename);

          int ilnum=0;
          while (getline(fs,line)) {
            ilnum++;
            // skip comment and empty lines
            if (line.empty() or line.find(L"%")==0 ) continue;
            // load rule
            labeler_rule r;
            r.line = fname+L"::"+ifname+L":"+util::int2wstring(ilnum);
            load_rule(line,r);
          }
        }

        else {
          // not "UNIQUE" key, it is a normal rule.
          labeler_rule r;
          r.line = fname+L":"+util::int2wstring(lnum);
          load_rule(line,r);
        }

        break;
      }
        
      case SEMDB: {
        /// load SEMDB section
        wstring sdb;
        wistringstream sin;  sin.str(line);
        sin>>sdb;
        sdb = util::absolute(sdb,path); 
        if ( not sdb.empty() ) {
          delete semdb; // free memory, just in case
          semdb = new semanticDB(sdb);
          TRACE(3,L"dep_txala loaded SemDB");
        }
        break;
      }

      default: break;
      }
    }

    cfg.close();

    TRACE(1,L"dep_txala successfully created");
  }


  ///////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  dep_txala::~dep_txala() {
    delete semdb;
  }


  ///////////////////////////////////////////////////////////////
  /// parse a line containing a completer rule, and fill completer_rule instance
  ///////////////////////////////////////////////////////////////

  void dep_txala::load_rule(const wstring &line, completer_rule &r) {
    
    wistringstream sline; sline.str(line);
    wstring keyw;
    
    wstring flags,chunks,pairfile,lit,newlabels,context;
    sline>>r.weight>>flags>>context>>chunks>>pairfile>>r.operation>>lit>>newlabels;
    r.enabling_flags=util::wstring2set(flags,L"|");

    // parse operation and its parameters
    if (lit==L"RELABEL") {
      if (newlabels==L"-") {
        r.newNode1 = L"-"; 
        r.newNode2 = L"-";
      }
      else {
        wstring::size_type p=newlabels.find(L":");
        if (p!=wstring::npos) {
          r.newNode1 = newlabels.substr(0,p);
          r.newNode2 = newlabels.substr(p+1);       
        }
        else {
          WARNING(L"Invalid RELABEL value in completer rule at line "+r.line+L". Rule will be ignored");
          return;
        }
      }
    }
    else if (lit==L"MATCHING") {
      extract_conds(newlabels,r.matchingCond);
    }
    else {
      WARNING(L"Invalid operation '"+lit+L"' in completer rule at line "+r.line+L". Rule will be ignored");
      return;
    }
    
    // parse flags to activate/deactivate (until line ends or a comment is found)
    bool comm=false;
    while (!comm && sline>>flags) {
      if (flags[0]==L'%') comm=true;
      else if (flags[0]==L'+') r.flags_toggle_on.insert(flags.substr(1));
      else if (flags[0]==L'-') r.flags_toggle_off.insert(flags.substr(1));
      else WARNING(L"Syntax error reading completer rule at line "+r.line+L". Flag must be toggled either on (+) or off (-)");
    }

    // consistency checks
    if ((r.operation==L"top_left" || r.operation==L"top_right") && lit!=L"RELABEL")
      WARNING(L"Syntax error reading completer rule at line "+r.line+L". "+r.operation+L" requires RELABEL.");
    if ((r.operation==L"last_left" || r.operation==L"cover_last_left") && lit!=L"MATCHING")     
      WARNING(L"Syntax error reading completer rule at line "+r.line+L". "+r.operation+L" requires MATCHING.");

    // parse context
    if (context==L"-") context=L"$$";
    r.context_neg=false;
    if (context[0]==L'!') {
      r.context_neg=true;
      context=context.substr(1);
    }
    
    vector<wstring> conds = util::wstring2vector(context,L"_");
    bool left=true;
    for (size_t c=0; c<conds.size(); c++) {
      if (conds[c]==L"$$") {left=false; continue;}
      
      matching_condition mc;
      extract_conds(conds[c],mc);
      if (left) r.leftContext.push_back(mc);
      else r.rightContext.push_back(mc);
    }
    
    // parse chunks
    wstring::size_type p=chunks.find(L",");
    if (chunks[0]!=L'(' || chunks[chunks.size()-1]!=L')' || p==wstring::npos)  
      WARNING(L"Syntax error reading completer rule at line "+r.line+L". Expected (leftChunk,rightChunk) pair at: "+chunks);
    
    r.leftChk=chunks.substr(1,p-1);
    r.rightChk=chunks.substr(p+1,chunks.size()-p-2);
    
    // check if the chunk labels carried extra lemma/form/class/tag
    // conditions and separate them if that's the case.
    extract_conds(r.leftChk,r.leftConds);
    extract_conds(r.rightChk,r.rightConds);

    // parse pairfile 
    if (pairfile!=L"-") {
      TRACE(4,L"Parsing pairfile "+pairfile);
      wstring::size_type p=pairfile.find(L"::(");
      r.pairClass=pairfile.substr(0,p);

      pairfile = pairfile.substr(p+3);
      pairfile = pairfile.substr(0,pairfile.length()-1);      
      p = pairfile.find(L",");
      r.node1 = pairfile.substr(0,p);
      r.node2 = pairfile.substr(p+1);

      p = r.node1.find(L".");
      r.attr1 = r.node1.substr(p+1); 
      r.node1 = r.node1.substr(0,p); 

      p = r.node2.find(L".");
      r.attr2 = r.node2.substr(p+1); 
      r.node2 = r.node2.substr(0,p); 
    }
    
    // Store rule
    TRACE(3,L"Loaded rule: [line "+r.line+L"] ");
    chgram[make_pair(r.leftChk,r.rightChk)].push_back(r);          
  }

  ///////////////////////////////////////////////////////////////
  /// parse a line containing a labeller rule, and fill labeler_rule instance
  ///////////////////////////////////////////////////////////////

  void dep_txala::load_rule(const wstring &line, labeler_rule &r) {

    wistringstream sin;  sin.str(line);
    sin>>r.ancestorLabel>>r.label;

    TRACE(4,L"RULE FOR:"+r.ancestorLabel+L" -> "+r.label+L" [line "+r.line+L"]");
    r.re.set_type(rule_expression::AND);
    wstring condition;
    while (sin>>condition) 
      add_subexpression(condition,r.re);

    rules[r.ancestorLabel].push_back(r);      
  }

  ///////////////////////////////////////////////////////////////
  /// Constructor private method: parse conditions and build rule expression
  ///////////////////////////////////////////////////////////////

  void dep_txala::add_subexpression(const wstring &condition, rule_expression &re) const {

    // parse condition and obtain components
    wstring::size_type pos=condition.find(L'=');
    wstring::size_type pdot=condition.rfind(L'.',pos);
    if (pos!=wstring::npos && pdot!=wstring::npos) {
      // Disassemble the condition. E.g. for condition "p.class=mov" 
      // we get: node="p";  func="class"; value="mov"; negated=false
      bool negated = (condition[pos-1]==L'!');
      wstring node=condition.substr(0,pdot);
      wstring func=condition.substr(pdot+1, pos-pdot-1-(negated?1:0) );
      wstring value=condition.substr(pos+1);
      TRACE(5,L"  adding condition:("+node+L","+func+L","+value+L")");    

      // check we do not request impossible things
      if (semdb==NULL && (func==L"tonto" || func==L"semfile" || func==L"synon" || func==L"asynon")) {
        ERROR_CRASH(L"Semantic function '"+func+L"' was used in labeling rules, but no previous <SEMDB> section was found. Make sure <SEMDB> section is defined before <GRLAB> section.");
      }

      // if the check is negated and we have As or Es, we must invert them
      if (negated) {
        if (node[0]==L'A') node[0]=L'E';
        else if (node[0]==L'E') node[0]=L'A';
      }

      create_subexpression(node,func,value,negated,re);
    } 
    else 
      WARNING(L"Error reading dependency rules. Ignored incorrect condition "+condition);

  }


  ///////////////////////////////////////////////////////////////
  /// Create appropriate type of rule_expression depending on
  /// the function
  ///////////////////////////////////////////////////////////////

  void dep_txala::create_subexpression(const wstring &node, const wstring &func, const wstring &value, bool negated, rule_expression &re) const {
    rule_expression sube;
    // create rule checker for function requested in condition
    if (func==L"label")        sube = rule_expression(rule_expression::LABEL,node,value);
    else if (func==L"side")    sube = rule_expression(rule_expression::SIDE,node,value);
    else if (func==L"lemma")   sube = rule_expression(rule_expression::LEMMA,node,value);
    else if (func==L"pos")     sube = rule_expression(rule_expression::POS,node,value);
    else if (func==L"class")   sube = rule_expression(rule_expression::WORDCLASS,node,value,wordclasses);
    else if (func==L"tonto")   sube = rule_expression(rule_expression::TONTO,node,value, *semdb);
    else if (func==L"semfile") sube = rule_expression(rule_expression::SEMFILE,node,value, *semdb);
    else if (func==L"synon")   sube = rule_expression(rule_expression::SYNON,node,value, *semdb);
    else if (func==L"asynon")  sube = rule_expression(rule_expression::ASYNON,node,value, *semdb);
    else if (func==L"pairclass") {
      // split [node1,node2] into two node references
      int cpos = node.find(L",");
      wstring node1 = node.substr(1,cpos-1);
      wstring node2 = node.substr(cpos+1,node.length()-cpos-2);
      TRACE(5,L"  node pair splitted as <"+node1+L","+node2+L">");    
      sube = rule_expression(rule_expression::PAIRCLASS, node1, node2, value, pairclasses);
    }
    else {
      WARNING(L"Error reading dependency rules. Ignored unknown function "+func);
      return;
    }
    
    if (negated) re.add(rule_expression(rule_expression::NOT,sube));
    else re.add(sube);
  }

  
  ///////////////////////////////////////////////////////////////
  /// Load set of classes from config file
  ///////////////////////////////////////////////////////////////
  
  void dep_txala::load_classes(const wstring &vclass, const wstring &vlemma,
                               const wstring &path, set<wstring> &cls) {

    // Class defined in a file, load it.
    if (vlemma[0]==L'"' && vlemma[vlemma.size()-1]==L'"') {

      wstring fname= util::absolute(vlemma,path);  // if relative, use main file path as base
    
      wifstream fclas;
      util::open_utf8_file(fclas,fname);
      if (fclas.fail()) ERROR_CRASH(L"Cannot open word class file "+fname);
    
      wstring line;
      while (getline(fclas,line)) {
        if (line.empty() or line[0]==L'%') continue;

        wistringstream sline;
        sline.str(line);
        wstring lems,lm;
        lems=L"";
        while(sline>>lm) lems=lems+L"#"+lm;
        cls.insert(vclass+lems);
      }
      fclas.close();
    }

    // Enumerated class, just store the pair
    else 
      cls.insert(vclass+L"#"+vlemma);
  
  }

  ///////////////////////////////////////////////////////////////
  /// Complete a partial parse tree.
  ///////////////////////////////////////////////////////////////

  parse_tree dep_txala::complete(parse_tree &tr, const wstring & startSymbol, dep_txala_status *st) const {

    TRACE(3,L"---- COMPLETING chunking to get a full parse tree ----");

    // we only complete trees with the fake start symbol set by the chart_parser
    if (tr.begin()->get_label() != startSymbol) 
      return tr;  

    int maxchunk = tr.num_children();
    int nchunk=1;
  
    vector<parse_tree *> trees;
  
    for(parse_tree::sibling_iterator ichunk=tr.sibling_begin(); ichunk!=tr.sibling_end(); ++ichunk,++nchunk) {

      parse_tree * mtree = new parse_tree(ichunk);

      if (ichunk.num_children()==0) {
        // the chunk is a leaf. Create a non-terminal node to ease 
        // the job in case no rules are found 
        node nod(ichunk->get_label());
        nod.set_chunk(true);
        nod.set_head(false);
        nod.set_node_id(ichunk->get_node_id()+L"x");
        parse_tree *taux=new parse_tree(nod);
        // hang the original node under the new one.
        mtree->begin()->set_head(true);
        taux->hang_child(*mtree);
        // use the new tree
        mtree=taux;
      }

      mtree->begin()->set_chunk(nchunk);
      trees.push_back(mtree);
    }
  
    // Apply as many rules as number of chunks minus one (since each rule fusions two chunks in one)
    for (nchunk=0; nchunk<maxchunk-1; ++nchunk) {
    
      if (traces::TraceLevel>=2) {
        TRACE(2,L"");
        TRACE(2,L"REMAINING CHUNKS:");
        vector<parse_tree *>::iterator vch; 
        int p=0;
        for (vch=trees.begin(); vch!=trees.end(); vch++) {
          TRACE(2,L"    chunk.num="+util::int2wstring((*vch)->begin()->get_chunk_ord())+L"  (trees["+util::int2wstring(p)+L"]) \t"+(*vch)->begin()->get_label());
          p++;
        }
      }

      TRACE(3,L"LOOKING FOR BEST APPLICABLE RULE");
      size_t chk=0;
      completer_rule bestR = find_grammar_rule(trees, chk, st);
      int best_prio= bestR.weight;
      size_t best_pchunk = chk;

      chk=1;
      while (chk<trees.size()-1) {
        completer_rule r = find_grammar_rule(trees, chk, st);

        if ( (r.weight==best_prio && chk<best_pchunk) || (r.weight>0 && r.weight<best_prio) || (best_prio<=0 && r.weight>best_prio) ) {
          best_prio = r.weight;
          best_pchunk = chk;
          bestR = r;
        }
      
        chk++;
      }
    
      TRACE(2,L"BEST RULE SELECTED. Apply rule [line "+util::int2wstring(bestR.line)+L"] to chunk trees["+util::int2wstring(best_pchunk)+L"]");
    
      parse_tree * resultingTree=applyRule(bestR, best_pchunk, trees[best_pchunk], trees[best_pchunk+1], st);

      TRACE(2,L"Rule applied - Erasing chunk in trees["+util::int2wstring(best_pchunk+1)+L"]");
      trees[best_pchunk]=resultingTree;
      trees[best_pchunk+1]=NULL;     
      vector<parse_tree*>::iterator end = remove (trees.begin(), trees.end(), (parse_tree*)NULL);    
      trees.erase (end, trees.end());    

      // clear rule appliactions to start fresh at next iteration
      st->last.clear();
    }
  
    // rebuild node index with new iterators, maintaining id's
    parse_tree ret_val = (*trees[0]); 
    delete trees[0];
    ret_val.rebuild_node_index();
    return (ret_val);
  }


  ///////////////////////////////////////////////////////////////
  /// Separate extra lemma/form/class conditions from the chunk label
  ///////////////////////////////////////////////////////////////

#define closing(x) (x==L'('?L")":(x==L'<'?L">":(x==L'{'?L"}":(x==L'['?L"]":L""))))

  void dep_txala::extract_conds(wstring &chunk, matching_condition &cond) const {

    wstring seen=L"";
    wstring con=L"";
    cond.attrs.clear();

    cond.neg = false;
    if (chunk[0]==L'~') {
      cond.neg=true;
      chunk=chunk.substr(1);  
    }

    size_t p=chunk.find_first_of(L"<([{");
    if (p==wstring::npos) {
      // If the label has no pairs of "<>" "()" or "[]", we're done
      cond.label = chunk;
    }
    else  {
      // locate start of first pair, and separate chunk label
      con = chunk.substr(p);
      chunk = chunk.substr(0,p);
      cond.label = chunk;

      // process pairs
      p=0;
      while (p!=wstring::npos) {

        wstring close=closing(con[p]);
        size_t q=con.find_first_of(close);
        if (q==wstring::npos) {
          WARNING(L"Missing closing "+close+L" in dependency rule. All conditions ignored.");
          cond.attrs.clear();
          return;
        }
      
        // check for duplicates
        if (seen.find(con[p])!=wstring::npos) {
          WARNING(L"Duplicate bracket pair "+con.substr(p,1)+close+L" in dependency rule. All conditions ignored.");
          cond.attrs.clear();
          return;
        }

        // add the condition to the list
        matching_attrib ma;
        ma.type=con.substr(p,1);

        // store original condition string
        ma.value = con.substr(p,q-p+1);
        // if PoS tag condition, compile as regex (without {} brackets)
        if (ma.type==L"{") ma.re = freeling::regexp(con.substr(p+1,q-p-1));     
        // add to list of attribs
        cond.attrs.push_back(ma);

        // remember this type already appeared
        seen = seen + con[p];

        // find start of next pair, if any
        p=con.find_first_of(L"<([{",q);
      }
    }
  }


  ///////////////////////////////////////////////////////////////
  /// Check if the current context matches the one specified 
  /// in the given rule.
  ///////////////////////////////////////////////////////////////

#define LEFT 1
#define RIGHT 2
#define first_cond(d) (d==LEFT? conds.size()-1 : 0)
#define first_chk(d)  (d==LEFT? chk-1  : chk+2)
#define last(i,v,d)   (d==LEFT? i<0    : i>=(int)v.size())
#define next(i,d)     (d==LEFT? i-1    : i+1);
#define prev(i,d)     (d==LEFT? i+1    : i-1);

  bool dep_txala::match_side(const int dir, const vector<parse_tree *> &trees, const size_t chk, const vector<matching_condition> &conds) const {

    TRACE(4,L"        matching "+wstring(dir==LEFT?L"LEFT":L"RIGHT")+L" context.  chunk position="+util::int2wstring(chk));
  
    // check whether left/right context matches
    bool match=true;
    int j=first_cond(dir); // condition index
    int k=first_chk(dir);  // chunk index
    while ( !last(j,conds,dir) && match ) {
      TRACE(4,L"        condition.idx j="+util::int2wstring(j)+L"   chunk.idx k="+util::int2wstring(k));

      if (last(k,trees,dir) && conds[j].label!=L"OUT") {
        // fail if context is shorter than rule (and the condition is not OUT-of-Bounds)
        match=false; 
      }
      else if (!last(k,trees,dir)) {
        bool b = (conds[j].label!=L"*") && (conds[j].label==L"?" || match_condition(trees[k],conds[j]));
        TRACE(4,L"        conds[j]="+conds[j].label+L" trees[k]="+trees[k]->begin()->get_label());

        if ( !b && conds[j].label==L"*" ) {
          // let the "*" match any number of items, looking for the next condition
          TRACE(4,L"        matching * wildcard.");

          j=next(j,dir);
          if (conds[j].label==L"OUT") // OUT after a wildcard will always match
            b=true;
          else {
            while ( !last(k,trees,dir) && !b ) {
              b = (conds[j].label==L"?") || match_condition(trees[k],conds[j]);
              TRACE(4,L"           j="+util::int2wstring(j)+L"  k="+util::int2wstring(k));
              TRACE(4,L"           conds[j]="+conds[j].label+L" trees[k]="+trees[k]->begin()->get_label()+L"   "+(b?wstring(L"matched"):wstring(L"no match")));
              k=next(k,dir);
            }
          
            k=prev(k,dir);        
          }
        }

        match = b;
      }

      k=next(k,dir);
      j=next(j,dir);
    }
  
    return match;  
  }


  ///////////////////////////////////////////////////////////////
  /// Locate actual node for given path
  ///////////////////////////////////////////////////////////////

  void dep_txala::locate_node(const vector<parse_tree *> &trees, const size_t chk, const wstring &node, list<parse_tree::const_iterator> &res) const {  
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
    
    if (top==L"L") rule_expression::parse_node_ref(nd,trees[chk]->begin(),res);
    else if (top==L"R") rule_expression::parse_node_ref(nd,trees[chk+1]->begin(),res);
  }

  ///////////////////////////////////////////////////////////////
  /// Extract values for requested atribute frm given node
  ///////////////////////////////////////////////////////////////

  void dep_txala::extract_attrib(const wstring &attr, const list<parse_tree::const_iterator> &nds, list<wstring> &val) const {

    val.clear();
    for (list<parse_tree::const_iterator>::const_iterator n=nds.begin(); n!=nds.end(); n++) {

      const word & w = parse_tree::get_head_word(*n);
      
      if (attr==L"lemma") 
        // retrieve lemma
        val.push_back(w.get_lemma());

      else if (attr==L"pos")
        // retrieve PoS
        val.push_back(w.get_tag());
      
      else {
        // it is a semdb-based query
        list<wstring> sens = semdb->get_word_senses(w.get_lc_form(), w.get_lemma(), w.get_tag().substr(0,1));

        if (attr==L"semfile") {
          for (list<wstring>::const_iterator s=sens.begin(); s!=sens.end(); s++)
            val.push_back(semdb->get_sense_info(*s).semfile);
        }
        else if (attr==L"tonto") {
          for (list<wstring>::const_iterator s=sens.begin(); s!=sens.end(); s++) {
            list<wstring> tonto = semdb->get_sense_info(*s).tonto; 
            val.splice(val.end(),tonto);
          }
        }
        else if (attr==L"synon")  {
          for (list<wstring>::const_iterator s=sens.begin(); s!=sens.end(); s++) {
            list<wstring> words = semdb->get_sense_info(*s).words; 
            val.splice(val.end(),words);
          }
        }
        else if (attr==L"asynon") {
          for (list<wstring>::const_iterator s=sens.begin(); s!=sens.end(); s++) {
            list<wstring> words = semdb->get_sense_info(*s).words; 
            val.splice(val.end(),words);
            list<wstring> lpar = semdb->get_sense_info(*s).parents;
            if (lpar.size()>0) sens.splice(sens.end(),lpar);
          }
        }
      }
    }
  }

  ///////////////////////////////////////////////////////////////
  /// Check if the chunk pair matches pair condition specified in the given rule.
  ///////////////////////////////////////////////////////////////

  bool dep_txala::matching_pair(const vector<parse_tree *> &trees, const size_t chk, const completer_rule &r) const {  

    bool match = true;
    if (r.pairClass==L"-") {
      TRACE(5,L"            empty pair condition match");
    }
    else {
      // locate nodes
      list<parse_tree::const_iterator> nds1;
      locate_node(trees,chk,r.node1, nds1);
      list<parse_tree::const_iterator> nds2;
      locate_node(trees,chk,r.node2, nds2);

      // extract requested properties
      list<wstring> val1;
      extract_attrib(r.attr1, nds1, val1);
      list<wstring> val2;
      extract_attrib(r.attr2, nds2, val2);
      
      TRACE(4,L"              checking pair condition. class="+r.pairClass+L" n1="+util::list2wstring(val1,L"/")+L" n2="+util::list2wstring(val2,L"/"));

      // check in pairclasses
      match=false;
      for (list<wstring>::iterator v1=val1.begin(); v1!=val1.end() and not match; v1++)
        for (list<wstring>::iterator v2=val2.begin(); v2!=val2.end() and not match; v2++)
          match = (pairclasses.find(r.pairClass+L"#"+(*v1)+L"#"+(*v2)) != pairclasses.end());

      TRACE(4,L"            pair condition "+wstring(match?L"match":L"does NOT match"));
    }
    
    return match;
  }
    
    ///////////////////////////////////////////////////////////////
  /// Check if the current context matches the one specified 
  /// in the given rule.
  ///////////////////////////////////////////////////////////////

  bool dep_txala::matching_context(const vector<parse_tree *> &trees, const size_t chk, const completer_rule &r) const {  
    // check whether the context matches      
    bool match = match_side(LEFT, trees, chk, r.leftContext) && match_side(RIGHT, trees, chk, r.rightContext);

    // apply negation if necessary
    if (r.context_neg) match = !match;

    TRACE(4,L"        Context "+wstring(match?L"matches":L"does NOT match"));
    return match;
  }


  ///////////////////////////////////////////////////////////////
  /// check if the extra lemma/form/class conditions are satisfied.
  ///////////////////////////////////////////////////////////////

  bool dep_txala::match_condition(parse_tree::const_iterator chunk, const matching_condition &cond) const {
    
    bool ok;
    // if labels don't match, forget it.
    if (chunk->get_label() != cond.label) ok =false;
    
    // if no extra attributes, we're done.
    else if (cond.attrs.empty())  ok = true;
    
    else { 
      // dive into the tree to locate the head word.
      parse_tree::const_iterator head = chunk;
      // while we don't reach a leaf
      while (head.num_children()>0) {
        // locate the head children..
        parse_tree::const_sibling_iterator s;
        for (s=head.sibling_begin(); s!=head.sibling_end() && !s->is_head(); ++s);    
        if (s==head.sibling_end())
          WARNING(L"NO HEAD Found!!! Check your chunking grammar and your dependency-building rules.");
      
        // ...and get down to it
        head=s;
      }
    
      // get word if head leaf node.
      const word & w = head->get_word();

      ok = true;    
      // check it satisfies all the condition attributes
      for (list<matching_attrib>::const_iterator c=cond.attrs.begin(); ok && c!=cond.attrs.end(); c++) {
        TRACE(4,L"        matching condition attr "+c->value+L" with word ("+w.get_form()+L","+w.get_lemma()+L","+w.get_tag()+L")");
        switch (c->type[0]) {
        case L'<': ok = (L"<"+w.get_lemma()+L">" == c->value); break;   
        case L'(': ok = (L"("+w.get_form()+L")" == c->value); break;
        case L'{': ok = c->re.search(w.get_tag()); break;
        case L'[': {
          wstring vclass=c->value.substr(1,c->value.size()-2);
          ok = (wordclasses.find(vclass+L"#"+w.get_lemma()) != wordclasses.end());        
          TRACE(4,L"        CLASS: "+c->value+wstring(ok?L" matches lemma":L" does NOT match lemma"));
          break;
        }    
        default: break; // should never get this far
        }
      }
    }

    // apply negation if the condition has one.
    if (cond.neg) ok = not ok;
  
    TRACE(4,L"            condition "+wstring(ok?L"match":L"does NOT match"));
    return ok;
  
  }    
  
  ///////////////////////////////////////////////////////////////
  /// check if the operation is executable (for last_left cases)
  ///////////////////////////////////////////////////////////////

  bool dep_txala::matching_operation(const vector<parse_tree *> &trees, const size_t chk, const completer_rule &r, dep_txala_status *st) const {

    // "top" operations are always feasible
    if (r.operation != L"last_left" && r.operation!=L"cover_last_left") 
      return true;

    // build id to store that this is the application of rule num "r.line" to chunk number "chk"
    wstring aid = r.line+L":"+util::int2wstring(chk);

    // locate last_left matching node 
    st->last[aid] = trees[chk]->end();
    for (parse_tree::iterator i=trees[chk]->begin(); i!=trees[chk]->end(); ++i) {
      TRACE(5,L"           matching operation: "+r.operation+L". Rule expects "+r.matchingCond.label+L", node is "+i->get_label());
      if (match_condition(i,r.matchingCond)) 
        st->last[aid] = i;  // remember node location in case the rule is finally selected.
    }
   
    // No matching node found
    if (st->last[aid]==trees[chk]->end()) {
      TRACE(4,L"        Operation does NOT match");
      return false;
    }
    
    // Matching node exists. Check whether rule application would break tree projectivity.
    int head_end = parse_tree::get_rightmost_leaf(st->last[aid])->get_word().get_position();
    int chunk_end = parse_tree::get_rightmost_leaf(trees[chk])->get_word().get_position();
    int child_begin = parse_tree::get_leftmost_leaf(trees[chk+1])->get_word().get_position();
    if (chunk_end>head_end and chunk_end<child_begin) {
      TRACE(4,L"        Operation matches but would break projectivity -> skipped");
      return false;
    }
    
    TRACE(4,L"        Operation matches");
    return true;    
  }


  ///////////////////////////////////////////////////////////////
  /// Find out if currently active flags enable the given rule
  ///////////////////////////////////////////////////////////////

  bool dep_txala::enabled_rule(const completer_rule &r, dep_txala_status *st) const {
   
    // if the rule is always-enabled, ignore everthing else
    if (r.enabling_flags.find(L"-") != r.enabling_flags.end()) 
      return true;

    // look through the rule enabling flags, to see if any is active.
    bool found=false;
    set<wstring>::const_iterator x;
    for (x=r.enabling_flags.begin(); !found && x!=r.enabling_flags.end(); x++)
      found = (st->active_flags.find(*x)!=st->active_flags.end());
  
    return found;
  }

  ///////////////////////////////////////////////////////////////
  /// Look for a completer grammar rule matching the given
  /// chunk in "chk" position of "trees" and his right-hand-side mate.
  ///////////////////////////////////////////////////////////////

  completer_rule dep_txala::find_grammar_rule(const vector<parse_tree *> &trees, const size_t chk, dep_txala_status *st) const {

    wstring leftChunk = trees[chk]->begin()->get_label();
    wstring rightChunk = trees[chk+1]->begin()->get_label();
    TRACE(3,L"  Look up rule for: ("+leftChunk+L","+rightChunk+L")");

    // find rules matching the chunks
    map<pair<wstring,wstring>,list<completer_rule> >::const_iterator r;
    r = chgram.find(make_pair(leftChunk,rightChunk));
 
    list<completer_rule>::const_iterator i;  
    list<completer_rule>::const_iterator best;
    int bprio= -1;
    bool found=false;
    if (r != chgram.end()) { 

      // search list of candidate rules for any rule matching conditions, context, and flags
      for (i=r->second.begin(); i!=r->second.end(); i++) {

        TRACE(4,L"    Checking candidate: [line "+util::int2wstring(i->line)+L"] ");

        // check extra conditions on chunks and context
        if (enabled_rule(*i,st)  
            && match_condition(trees[chk]->begin(),i->leftConds) 
            && match_condition(trees[chk+1]->begin(),i->rightConds) 
            && matching_pair(trees,chk,*i)
            && matching_context(trees,chk,*i)
            && matching_operation(trees,chk,*i,st)) {
     
          if (bprio == -1 || bprio>i->weight) {
            found = true;
            best=i;
            bprio=i->weight;
          }

          TRACE(3,L"    Candidate: [line "+util::int2wstring(i->line)+L"] -- MATCH");
        }
        else  {
          TRACE(3,L"    Candidate: [line "+util::int2wstring(i->line)+L"] -- no match");
        }
      }
    }

    if (found) 
      return (*best);
    else {
      TRACE(3,L"    NO matching candidates found, applying default rule.");    
      // Default rule: top_left, no relabel
      return completer_rule(L"-",L"-",L"top_left");
    }
  
  }


  ///////////////////////////////////////////////////////////////
  /// apply a tree completion rule
  ///////////////////////////////////////////////////////////////

  parse_tree * dep_txala::applyRule(const completer_rule & r, int chk, parse_tree * chunkLeft, parse_tree * chunkRight, dep_txala_status *st) const {
  
    // toggle necessary flags on/off
    set<wstring>::const_iterator x;
    for (x=r.flags_toggle_on.begin(); x!=r.flags_toggle_on.end(); x++) 
      st->active_flags.insert(*x);
    for (x=r.flags_toggle_off.begin(); x!=r.flags_toggle_off.end(); x++) 
      st->active_flags.erase(*x);

    // build id to retrieve that this is the application of rule num "r.line" to chunk number "chk"
    wstring aid = r.line+L":"+util::int2wstring(chk);

    // hang left tree under right tree root
    if (r.operation==L"top_right") {
      TRACE(3,L"Applying rule: Insert left chunk under top_right");
      // Right is head
      chunkLeft->begin()->set_head(false);      
      chunkRight->begin()->set_head(true);

      // change node labels if required
      if (r.newNode1!=L"-") {
        TRACE(3,L"    ... and relabel left chunk (child) as "+r.newNode1);
        chunkLeft->begin()->set_label(r.newNode1);      
      }
      if (r.newNode2!=L"-") {
        TRACE(3,L"    ... and relabel right chunk (parent) as "+r.newNode2);
        chunkRight->begin()->set_label(r.newNode2);   
      }   

      // insert Left tree under top node in Right
      chunkRight->hang_child(*chunkLeft,chunkRight->sibling_begin());
      return chunkRight;
    }
  
    // hang right tree under left tree root and relabel root
    else if (r.operation==L"top_left") {
      TRACE(3,L"Applying rule: Insert right chunk under top_left");
      // Left is head
      chunkRight->begin()->set_head(false);
      chunkLeft->begin()->set_head(true);

      // change node labels if required
      if (r.newNode1!=L"-") {
        TRACE(3,L"    ... and relabel left chunk (parent) as "+r.newNode1);
        chunkLeft->begin()->set_label(r.newNode1);      
      }   
      if (r.newNode2!=L"-") {
        TRACE(3,L"    ... and relabel right chunk (child) as "+r.newNode2);
        chunkRight->begin()->set_label(r.newNode2);   
      }

      // insert Right tree under top node in Left
      chunkLeft->hang_child(*chunkRight);
      return chunkLeft;
    }

    // hang right tree under last node in left tree
    else if (r.operation==L"last_left") {
      TRACE(3,L"Applying rule: Insert right chunk under last_left with label "+r.matchingCond.label);
      // Left is head, so unmark Right as head.
      chunkRight->begin()->set_head(false); 
      TRACE(4,L"recovering last right match for rule "+r.line+L" application "+aid);
      // obtain last node with given label in Left tree
      // We stored it in the rule when checking for its applicability.
      parse_tree::iterator p=st->last[aid];
      TRACE(4,L"   node recovered is: ");
      TRACE(4,L"     "+p->get_label());    // hang Right tree under last node in Left tree
      p.hang_child(*chunkRight); 
      return chunkLeft;
    }
  

    // hang right tree where last node in left tree is, and put the later under the former
    else if (r.operation==L"cover_last_left") {
      TRACE(3,L"Applying rule: Insert right chunk to cover_last_left with label "+r.newNode1);
      // Right will be the new head
      chunkRight->begin()->set_head(true); 
      chunkLeft->begin()->set_head(false); 
      // obtain last node with given label in Left tree
      // We stored it in the rule when checking for its applicability.
      parse_tree::iterator last=st->last[aid];
      parse_tree::iterator parent = last.get_parent();
      // put last_left tree under Right tree (removing it from its original place)
      chunkRight->begin().hang_child(last); 
      // put chunkRight under the same parent that last_left
      parent.hang_child(*chunkRight);
      return chunkLeft;
    }

    else {
      ERROR_CRASH(L"Internal Error unknown rule operation type: "+r.operation);
      return NULL; // avoid compiler warnings
    }
  
  }



  ///////////////////////////////////////////////////////////////
  /// Label nodes in a depencendy tree. (Initial call)
  ///////////////////////////////////////////////////////////////

  void dep_txala::label(dep_tree * dependency) const {
    TRACE(2,L"------ LABELING Dependences ------");
    dep_tree::iterator d=dependency->begin(); 
    d->set_label(L"top");
    label(dependency, d);
  }


  ///////////////////////////////////////////////////////////////
  /// Label nodes in a depencendy tree. (recursive)
  ///////////////////////////////////////////////////////////////

  void dep_txala::label(dep_tree* dependency, dep_tree::iterator ancestor) const {

    dep_tree::sibling_iterator d,d1;

    // there must be only one top 
    for (d=ancestor.sibling_begin(); d!=ancestor.sibling_end(); ++d) {
          
      ///const string ancestorLabel = d->get_dep_result();
      const wstring ancestorLabel = ancestor->get_link()->get_label();
      TRACE(2,L"Labeling dependency: "+d->get_link()->get_label()+L" --> "+ancestorLabel);
    
      map<wstring, list <labeler_rule> >::const_iterator frule=rules.find(ancestorLabel);
      if (frule!=rules.end()) {
        list<labeler_rule>::const_iterator rl=frule->second.begin();
        bool found=false;

        while (rl!=frule->second.end() && !found) {

          TRACE(3,L"  Trying rule: [line "+util::int2wstring(rl->line)+L"] ");
          bool skip=false;
          // if the label is declared as unique and a sibling already has it, skip the rule.
          if (unique.find(rl->label)!=unique.end())
            for (d1=ancestor.sibling_begin(); d1!=ancestor.sibling_end() && !skip; ++d1)
              skip = (rl->label == d1->get_label());
        
          if (!skip) {
            found = (rl->check(ancestor,d)); 
            if (found) { 
              d->set_label(rl->label);
              TRACE(3,L"      [line "+util::int2wstring(rl->line)+L"] "+rl->ancestorLabel+L" "+rl->label+L" -- rule matches!");
              TRACE(2,L"      RULE APPLIED. Dependence labeled as "+rl->label);
            }
            else {
              TRACE(3,L"      [line "+util::int2wstring(rl->line)+L"] "+rl->ancestorLabel+L" "+rl->label+L" -- no match");
            }
          }
          else {
            TRACE(3,L"      RULE SKIPPED -- Unique label "+rl->label+L" already present.");
          }

          ++rl;
        }
       
        if (!found) {d->set_label(L"modnomatch");}
      }
      else { 
        d->set_label(L"modnorule");
      }     
   
      // Recursive call
      label(dependency, d);
    }
  }


  ///////////////////////////////////////////////////////////////
  /// Apply completion rules to get a full parse tree
  ///////////////////////////////////////////////////////////////

  void dep_txala::complete_parse_tree(sentence &s) const {

    // parse each of k-best tag sequences
    for (unsigned int k=0; k<s.num_kbest(); k++) {

      dep_txala_status *st = new dep_txala_status();
      st->active_flags.insert(L"INIT");
      s.set_processing_status((processor_status*)st);

      // get chunker output
      parse_tree buff = s.get_parse_tree(k);
      // complete it into a full parsing tree
      parse_tree ntr = complete(buff,start,st);
      // store complete tree in the sentence
      s.set_parse_tree(ntr,k);    

      // parse_tree::PrintTree(ntr.begin(),k,0); // debugging

      s.clear_processing_status();
    }
  }

  ///////////////////////////////////////////////////////////////
  /// Apply completion rules to get a full parse tree
  ///////////////////////////////////////////////////////////////

  void dep_txala::complete_parse_tree(list<sentence> &ls) const {
    for (list<sentence>::iterator s=ls.begin(); s!=ls.end(); s++) 
      complete_parse_tree(*s);
  }

  ///////////////////////////////////////////////////////////////
  /// Apply completion rules to get a full parse tree
  ///////////////////////////////////////////////////////////////

  void dep_txala::complete_parse_tree(document &doc) const {
    for (document::iterator p=doc.begin(); p!=doc.end(); p++) 
      complete_parse_tree(*p);
  }


  ///////////////////////////////////////////////////////////////
  /// Enrich given sentence with a depenceny tree.
  ///////////////////////////////////////////////////////////////

  void dep_txala::analyze(sentence &s) const {

    // complete parse trees using completion rules
    complete_parse_tree(s);

    // convert to dependencies and label dep tree
    for (unsigned int k=0; k<s.num_kbest(); k++) {    
      // Convert full parse tree to dependencies tree
      dep_tree* deps = dependencies(s.get_parse_tree(k).begin(),s.get_parse_tree(k).begin());
      // Set labels on the dependencies
      label(deps);
    
      // store the tree in the sentence
      s.set_dep_tree(*deps,k);

      // PrintDepTree(s.get_dep_tree().begin(),0); // debugging
      delete(deps);
    }

  }


  //---------- dep_txala private functions 

  ///////////////////////////////////////////////////////////////
  /// Obtain a depencendy tree from a parse tree.
  ///////////////////////////////////////////////////////////////

  dep_tree * dep_txala::dependencies(parse_tree::iterator tr, parse_tree::iterator link) {

    dep_tree * result;

    if (tr.num_children() == 0) { 
      // direct case. Leaf node, just build and return a one-node dep_tree.
      depnode d(*tr); 
      d.set_link(link);
      result = new dep_tree(d);
    }
    else {
      // Recursive case. Non-leaf node. build trees 
      // for all children and hang them below the head

      // locate head child
      parse_tree::sibling_iterator head;
      parse_tree::sibling_iterator k;
      for (k=tr.sibling_begin(); k!=tr.sibling_end() && !k->is_head(); ++k);
      if (k==tr.sibling_end()) {
        WARNING(L"NO HEAD Found!!! Check your chunking grammar and your dependency-building rules.");
        k=tr.sibling_begin();
      }
      head = k;
    
      // build dep tree for head child
      if (!tr->is_head()) link=tr;
      result = dependencies(head,link);
    
      // Build tree for each non-head child and hang it under the head.
      // We maintain the original sentence order (not really necessary,
      // but trees are cuter this way...)

      // children to the left of the head
      k=head; ++k;
      while (k!=tr.sibling_end()) { 
        if (k->is_head()) WARNING(L"More than one HEAD detected. Only first prevails.");
        dep_tree *dt = dependencies(k,k);
        result->hang_child(*dt);  // hang it as last child
        ++k;
      }
      // children to the right of the head
      k=head; --k;
      while (k!=tr.sibling_rend()) { 
        if (k->is_head()) WARNING(L"More than one HEAD detected. Only first prevails.");
        dep_tree *dt = dependencies(k,k);
        result->hang_child(*dt,result->sibling_begin());   // hang it as first child
        --k;
      }
    }

    // copy chunk information from parse tree
    result->begin()->set_chunk(tr->get_chunk_ord());  
    return (result);
  }

  ///////////////////////////////////////////////////////////////
  /// Obtain a depencendy tree from a parse tree.
  /// Public acces to underlying recursive function above
  ///////////////////////////////////////////////////////////////

  dep_tree* dep_txala::parse2dep(parse_tree &t) {
    return dependencies(t.begin(),t.begin());
  }


} // namespace
