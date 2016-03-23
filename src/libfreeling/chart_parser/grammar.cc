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

#include <fstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/lexer.h"
#include "freeling/morfo/grammar.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"GRAMMAR"
#define MOD_TRACECODE GRAMMAR_TRACE


  //-------- Class rule implementation -----------//
 
  ////////////////////////////////////////////////////////////////
  /// Default constructor of rule.
  ////////////////////////////////////////////////////////////////

  rule::rule(const wstring &s, const list<wstring> &ls, const int p): head(s),right(ls),gov(p) {}

  ////////////////////////////////////////////////////////////////
  /// constructor
  ////////////////////////////////////////////////////////////////

  rule::rule() { gov=0;}

  ////////////////////////////////////////////////////////////////
  /// copy constructor
  ////////////////////////////////////////////////////////////////

  rule::rule(const rule & r) { 
    gov=r.gov;
    head=r.head;
    right=r.right;
  }

  ////////////////////////////////////////////////////////////////
  /// assignment constructor
  ////////////////////////////////////////////////////////////////

  rule &  rule::operator=(const rule & r) {
    gov=r.gov;
    head=r.head;
    right=r.right;
    return *this;
  }


  ////////////////////////////////////////////////////////////////
  /// Set rule governor
  ////////////////////////////////////////////////////////////////

  void rule::set_governor(const int p) { 
    gov=p;
  }

  ////////////////////////////////////////////////////////////////
  /// Get rule governor
  ////////////////////////////////////////////////////////////////

  unsigned int rule::get_governor(void) const {
    return gov;
  }

  ////////////////////////////////////////////////////////////////
  /// Get head for  rule.
  ////////////////////////////////////////////////////////////////

  wstring rule::get_head() const {
    return(head);
  }

  ////////////////////////////////////////////////////////////////
  /// get right part of the rule.
  ////////////////////////////////////////////////////////////////

  list<wstring> rule::get_right() const {
    return(right);
  }


  //-------- Class grammar implementation -----------//

  //------ TOKENS used by the lexer to parse a grammar file
#define CATEGORY 1
#define FORM     2
#define LEMMA    3
#define COMMENT  4
#define HEAD     5
#define ARROW    6
#define BAR      7
#define COMMA    8
#define DOT      9
#define FLAT     10
#define HIDDEN   11
#define NOTOP    12
#define ONLYTOP  13
#define PRIOR    14
#define START    15
#define FILENAME 16

#define ParseError(x) WARNING(L"File "+fname+L", line "+util::int2wstring(fl.lineno())+L": "+x)

  /// no governor mark
  const unsigned int grammar::NOGOV=99999;  
  /// default governor is first element in rule
  const unsigned int grammar::DEFGOV=0;

  ////////////////////////////////////////////////////////////////
  /// Create a grammar, loading a file.
  ////////////////////////////////////////////////////////////////

  grammar::grammar(const wstring &fname) {

    const int MAX= 32;
    int tok,stat,newstat,i,j;
    int what=0;
    int trans[MAX][MAX];
    bool first=false, wildcard=false;
    wstring head, err, categ, name;
    list<wstring> ls;
    int prior_val;

    // We use a FSA to read grammar rules. Fill transition tables
    for (i=0; i<MAX; i++) for (j=0; j<MAX; j++) trans[i][j]=0;
    // State 1. Initial state. Any line may come
    trans[1][COMMENT]=1; trans[1][CATEGORY]=2; trans[1][PRIOR]=6;
    trans[1][START]=8;   trans[1][HIDDEN]=6;   trans[1][FLAT]=6;
    trans[1][NOTOP]=6;   trans[1][ONLYTOP]=6;
    // State 2. A rule has started
    trans[2][ARROW]=3; 
    // State 3. Right side of a rule
    trans[3][CATEGORY]=4; 
    trans[3][HEAD]=10;
    // State 4. Right side of a rule, category found
    trans[4][COMMA]=3;  trans[4][BAR]=3;  trans[4][DOT]=1; 
    trans[4][LEMMA]=5;  trans[4][FORM]=5; trans[4][FILENAME]=5;
    // State 5. Right side of a rule, specificied category found
    trans[5][COMMA]=3;  trans[5][BAR]=3;  trans[5][DOT]=1; 
    // State 6. List directive found (@PRIOR, @HIDDEN, etc)
    trans[6][CATEGORY]=7;
    // State 7. Processing list directive
    trans[7][CATEGORY]=7;  trans[7][DOT]=1;
    // State 8. @START directive found 
    trans[8][CATEGORY]=9;
    // State 9. Processing list directive
    trans[9][DOT]=1;
    // State 10. Processing governor tag
    trans[10][CATEGORY]=4; 

    vector<pair<freeling::regexp,int> > rules;
    rules.push_back(make_pair(freeling::regexp(L"[ \\t\\n\\r]+"),0));
    rules.push_back(make_pair(freeling::regexp(L"%.*"),COMMENT));
    rules.push_back(make_pair(freeling::regexp(L"==>"),ARROW));
    rules.push_back(make_pair(freeling::regexp(L"\\([[:alpha:]_'\\-\\·]+\\)"),FORM));
    rules.push_back(make_pair(freeling::regexp(L"<[[:lower:]_'\\-\\·]+>"),LEMMA));
    rules.push_back(make_pair(freeling::regexp(L"\\(\\\"([A-Za-z]:)?[[:alnum:]_\\-\\./\\\\]+\\\"\\)"),FILENAME));
    rules.push_back(make_pair(freeling::regexp(L"<\\\"([A-Za-z]:)?[[:alnum:]_\\-\\./\\\\]+\\\">"),FILENAME));
    rules.push_back(make_pair(freeling::regexp(L"[A-Za-z][\\-A-Za-z0-9]*[*]?"),CATEGORY));
    rules.push_back(make_pair(freeling::regexp(L"@PRIOR"),PRIOR));
    rules.push_back(make_pair(freeling::regexp(L"@START"),START));
    rules.push_back(make_pair(freeling::regexp(L"@HIDDEN"),HIDDEN));
    rules.push_back(make_pair(freeling::regexp(L"@FLAT"),FLAT));
    rules.push_back(make_pair(freeling::regexp(L"@NOTOP"),NOTOP));
    rules.push_back(make_pair(freeling::regexp(L"@ONLYTOP"),ONLYTOP));
    rules.push_back(make_pair(freeling::regexp(L"\\|"),BAR));
    rules.push_back(make_pair(freeling::regexp(L"\\."),DOT));
    rules.push_back(make_pair(freeling::regexp(L","),COMMA));
    rules.push_back(make_pair(freeling::regexp(L"\\+"),HEAD));
    lexer fl(rules);

    wifstream gf;
    util::open_utf8_file(gf,fname);
    if (gf.fail()) ERROR_CRASH(L"Error opening file '"+fname+L"'");

    // Governor default 0 (for unary rules. All others must have an explicit governor)
    int gov=0; 
    bool havegov=false;

    // FS automata to check rule file syntax and load rules.
    stat=1; prior_val=1; err=L"";
    while( (tok=fl.getToken(gf)) ) {

      newstat = trans[stat][tok];

      // do appropriate processing depending on the state
      switch(newstat){
      case 0:
        // error state
        if (tok==COMMENT) err=L"Unexpected comment. Missing dot ending previous rule/directive ?";
        if (err==L"") err=L"Unexpected '"+fl.getText()+L"' found.";
        ParseError(err);
        err=L"";
          
        // skip until first end_of_rule/directive or EOF, and continue from there.
        while (tok && tok!=DOT) tok=fl.getToken(gf);
        newstat=1;
        break;

      case 1:
        if (tok==DOT && (stat==4 || stat==5)) { 
          // a rule has just finished. Store it.
          ls.insert(ls.end(),categ);

          // If a governor has not been defined, use default.
          // If the rule was not unary, issue a parsing warning
          if (!havegov) {
            gov=DEFGOV;
            if (ls.size()!=1)
              ParseError(L"Non-unary rule with no governor. First component taken as governor.");
          }

          // store rule
          new_rule(head,ls,wildcard,gov);

          // prepare for next rule
          gov=NOGOV; havegov=false;
        }
        break;

      case 2:
        // start of a rule, since there is a rule to rewrite 
        // this symbol, assume it is  non-terminal. Remember the head
        head=fl.getText();
        nonterminal.insert(head); 
        break;

      case 3:
        if (tok==ARROW) {
          // the right-hand side of a rule is starting. Initialize
          ls.clear();
          first=true; wildcard=false;
        }
        else if (tok==COMMA) { 
          // new category for the same rule, add the category to the rule right part list
          ls.insert(ls.end(),categ);
        }
        else if (tok==BAR) { 
          // rule finishes here
          ls.insert(ls.end(),categ);

          // If a governor has not been defined, use default.
          // If the rule was not unary, issue a parsing warning
          if (!havegov) {
            gov=DEFGOV;
            if (ls.size()!=1)
              ParseError(L"Non-unary rule with no governor. First component taken as governor.");
          }

          // store rule
          new_rule(head,ls,wildcard, gov);

          // go for a new rule, with the same parent.
          gov=NOGOV; havegov=false;

          ls.clear();
        }
        break;

      case 4:
        // store right-hand part of the rule
        categ= fl.getText();
        // if first token in right part, check for wildcards
        if (first && categ.find_first_of(L"*")!=wstring::npos) wildcard = true;
        // next will no longer be first
        first = false; 
        break;

      case 5:
        name=fl.getText();
        // category is specified with lemma or form. Remember that.
        categ = categ+name;

        // if a file name is given, load it into filemap
        if (tok==FILENAME) {

          wstring sname;
          // remove brackets
          sname=name.substr(1,name.size()-2); 
          // convert relative file name to absolute, usig directory of grammar file as base.
          sname = util::absolute(sname, fname.substr(0,fname.find_last_of(L"/\\")+1) );

          wifstream fs;
          util::open_utf8_file(fs,sname);
          if (fs.fail()) ERROR_CRASH(L"Error opening file "+sname);

          wstring op, clo;
          if (name[0]=='<') {op=L"<"; clo=L">";}
          else if (name[0]=='(') {op=L"("; clo=L")";}

          wstring line;
          while (getline(fs,line))
            filemap.insert(make_pair(op+line+clo,name));

          fs.close();
        }

        break;

      case 6:
        // list directive start. Remember which directive.
        what=tok;
        break;

      case 7:
        categ=fl.getText();
        if (nonterminal.find(categ)!=nonterminal.end()) {
          switch (what) {
          case PRIOR:   prior.insert(make_pair(categ,prior_val++)); break;
          case HIDDEN:  hidden.insert(categ); break;
          case FLAT:    flat.insert(categ); break;
          case NOTOP:   notop.insert(categ); break;
          case ONLYTOP: onlytop.insert(categ); break;
          }
        }
        else  {
          err=L"Terminal symbol '"+fl.getText()+L"' not allowed in directive.";
          newstat=0;
        }
        break;

      case 8:
        if (start!=L"") {
          err=L"@START specified more than once.";
          newstat=0;
        }
        break;

      case 9:
        start=fl.getText();
        if (nonterminal.find(start)==nonterminal.end()) nonterminal.insert(start);
        break;

      case 10:

        // Found governor mark, store current position
        gov=ls.size();
        havegov=true;
        break;

      } // End switch

      stat=newstat;
    }  

    if (start==L"") ParseError(L"@START symbol not specified.");
    if (hidden.find(start)!=hidden.end()) ParseError(L"@START symbol cannot be @HIDDEN.");
    if (notop.find(start)!=notop.end()) ParseError(L"@START symbol cannot be @NOTOP.");

    set<wstring>::const_iterator x;
    for (x=onlytop.begin(); x!=onlytop.end(); x++) {
      if (hidden.find(*x)!=hidden.end()) {
        ParseError(L"@HIDDEN directive for '"+(*x)+L"' overrides @ONLYTOP.");
      }
    }

    gf.close();

    TRACE(2,L" Grammar loaded.");
  }


  ////////////////////////////////////////////////////////////////
  /// Create and store a new rule, indexed by 1st category in its right part.
  ////////////////////////////////////////////////////////////////

  void grammar::new_rule(const wstring &h, const list<wstring> &ls, bool w, const int ngov) {
  
    rule r(h,ls,ngov);    // create
    //  r.set_governor(ngov);
    this->insert(make_pair(*ls.begin(),r)); // store

    // if appropriate, insert rule in wildcarded rules list
    // indexed by 1st char in wildcarded category
    if (w) wild.insert(make_pair(ls.begin()->substr(0,1),r));
  }


  ////////////////////////////////////////////////////////////////
  /// Obtain specificity for a terminal symbol.
  /// Lower value, higher specificity.
  /// highest is "TAG(form)", then "TAG<lemma>", then "TAG"
  ////////////////////////////////////////////////////////////////

  int grammar::get_specificity(const wstring &s) const {

    // Form: most specific (spec=0)
    if (s.find_first_of(L"(")!=wstring::npos &&  s.find_first_of(L")")==s.size()-1) return (0);
    // Lemma: second most specific (spec=1)
    if (s.find_first_of(L"<")!=wstring::npos && s.find_first_of(L">")==s.size()-1) return (1);
    // Other (single tags): less specific (spec=2)
    return (2);
  }

  ////////////////////////////////////////////////////////////////
  /// Obtain priority for a non-terminal symbol.
  /// Lower value, higher priority.
  ////////////////////////////////////////////////////////////////

  int grammar::get_priority(const wstring &sym) const {
    map<wstring,int>::const_iterator x;

    x=prior.find(sym);
    if (x!=prior.end()) return(x->second);
    else return 9999;
  }


  ////////////////////////////////////////////////////////////////
  /// Check whether a symbol is terminal or not.
  ////////////////////////////////////////////////////////////////

  bool grammar::is_terminal(const wstring &sym) const {
    return(nonterminal.find(sym)==nonterminal.end());
  }

  ////////////////////////////////////////////////////////////////
  /// Check whether a symbol must disappear of final tree
  ////////////////////////////////////////////////////////////////

  bool grammar::is_hidden(const wstring &sym) const {
    return(hidden.find(sym)!=hidden.end());
  }

  ////////////////////////////////////////////////////////////////
  /// Check whether a symbol must be flattened when recursive
  ////////////////////////////////////////////////////////////////

  bool grammar::is_flat(const wstring &sym) const {
    return(flat.find(sym)!=flat.end());
  }

  ////////////////////////////////////////////////////////////////
  /// Check whether a symbol can not be used as a tree root
  ////////////////////////////////////////////////////////////////

  bool grammar::is_notop(const wstring &sym) const {
    return(notop.find(sym)!=notop.end());
  }

  ////////////////////////////////////////////////////////////////
  /// Check whether a symbol is hidden unless when at tree root
  ////////////////////////////////////////////////////////////////

  bool grammar::is_onlytop(const wstring &sym) const {
    return(onlytop.find(sym)!=onlytop.end());
  }



  ////////////////////////////////////////////////////////////////
  /// Return the grammar start symbol.
  ////////////////////////////////////////////////////////////////

  wstring grammar::get_start_symbol() const {
    return(start);
  }


  ////////////////////////////////////////////////////////////////
  /// Get all rules with a right part beggining with the given category.
  ////////////////////////////////////////////////////////////////

  list<rule> grammar::get_rules_right(const wstring &cat) const {
    typedef multimap<wstring,rule>::const_iterator T1;
    pair<T1,T1> i;
    T1 j;
    list<rule> lr;

    // find appropriate rules
    i=this->equal_range(cat);
    // If any rules found, return them in a list
    if (i.first!=this->end() && i.first->first==cat) {
      for (j=i.first; j!=i.second; j++) {
        lr.push_back(j->second);
      }
    }
    return lr;
  }


  ////////////////////////////////////////////////////////////////
  /// Get all rules with a right part beggining with a wilcarded category.
  ////////////////////////////////////////////////////////////////

  list<rule> grammar::get_rules_right_wildcard(const wstring &c) const {
    typedef multimap<wstring,rule>::const_iterator T1;
    pair<T1,T1> i;
    T1 j;
    list<rule> lr;

    // find appropriate rules
    i=wild.equal_range(c);
    // If any rules found, return them in a list
    if (i.first!=wild.end() && i.first->first==c) {
      for (j=i.first; j!=i.second; j++) {
        lr.push_back(j->second);
      }
    }
    return lr;
  }

  ////////////////////////////////////////////////////////////////
  /// search given string in filemap, and check whether it maps to the second
  ////////////////////////////////////////////////////////////////

  bool grammar::in_filemap(const wstring &key, const wstring &val) const {
    typedef multimap<wstring,wstring>::const_iterator T1;
    pair<T1,T1> i;
    T1 j;

    // search in multimap
    i=filemap.equal_range(key);  

    if (i.first==filemap.end() || i.first->first!=key)
      return false;     // pair not found

    // key found, check pairs.
    bool b=false;
    for (j=i.first; j!=i.second && !b; j++) b=(j->second==val);  
    return b;
  }
} // namespace
