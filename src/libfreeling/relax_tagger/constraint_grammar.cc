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

#include "freeling/morfo/lexer.h"
#include "freeling/morfo/constraint_grammar.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"CONST_GRAMMAR"
#define MOD_TRACECODE TAGGER_TRACE

  //-------- Class condition implementation -----------//
 
  ////////////////////////////////////////////////////////////////
  /// Default constructor of condition
  ////////////////////////////////////////////////////////////////

  condition::condition() {}

  ////////////////////////////////////////////////////////////////
  /// clear condition contents
  ////////////////////////////////////////////////////////////////

  void condition::clear() {
    neg = false;
    pos = 0;
    starpos = false;
    terms.clear();
    barrier.clear();
  }

  ////////////////////////////////////////////////////////////////
  /// Set negation value
  ////////////////////////////////////////////////////////////////

  void condition::set_neg(bool n) {neg = n;}

  ////////////////////////////////////////////////////////////////
  /// Set position value (and star condition)
  ////////////////////////////////////////////////////////////////

  void condition::set_pos(int p, bool s) {pos = p; starpos = s;}

  ////////////////////////////////////////////////////////////////
  /// Set list of condition terms
  ////////////////////////////////////////////////////////////////

  void condition::set_terms(const list<wstring>& t) {terms = t;}

  ////////////////////////////////////////////////////////////////
  /// Set list of barrier terms. Empty list means no barrier.
  ////////////////////////////////////////////////////////////////

  void condition::set_barrier(const list<wstring>& b) {barrier = b;}

  ////////////////////////////////////////////////////////////////
  /// Get negation value
  ////////////////////////////////////////////////////////////////

  bool condition::is_neg() const {return(neg);}

  ////////////////////////////////////////////////////////////////
  /// Get position value 
  ////////////////////////////////////////////////////////////////

  int condition::get_pos() const {return(pos);}

  ////////////////////////////////////////////////////////////////
  /// Get star value for position
  ////////////////////////////////////////////////////////////////

  bool condition::has_star() const {return(starpos);}

  ////////////////////////////////////////////////////////////////
  /// Get list of condition terms
  ////////////////////////////////////////////////////////////////

  list<wstring> condition::get_terms() const {return(terms);}

  ////////////////////////////////////////////////////////////////
  /// Find out whether there ara barrier conditions
  ////////////////////////////////////////////////////////////////

  bool condition::has_barrier() const {return(!barrier.empty());}

  ////////////////////////////////////////////////////////////////
  /// Get list of barrier terms. Empty list means no barrier.
  ////////////////////////////////////////////////////////////////

  list<wstring> condition::get_barrier() const {return(barrier);}

  //-------- Class ruleCG implementation -----------//
 
  ////////////////////////////////////////////////////////////////
  /// Default constructor of ruleCG.
  ////////////////////////////////////////////////////////////////

  ruleCG::ruleCG() {}

  ////////////////////////////////////////////////////////////////
  /// Get head for  rule.
  ////////////////////////////////////////////////////////////////

  wstring ruleCG::get_head() const {return(head);}

  ////////////////////////////////////////////////////////////////
  /// get rule compatibility value
  ////////////////////////////////////////////////////////////////

  double ruleCG::get_weight() const {return(weight);}

  ////////////////////////////////////////////////////////////////
  /// set head for  rule.
  ////////////////////////////////////////////////////////////////

  void ruleCG::set_head(wstring const &h) {head=h;}

  ////////////////////////////////////////////////////////////////
  /// set rule compatibility value
  ////////////////////////////////////////////////////////////////

  void ruleCG::set_weight(double w) {weight = w;}


  //-------- Class constraint_grammar implementation -----------//

  //---- STATES used to parse constraint_grammar files.
#define ERROR_ST   0
#define INIT       1
#define S_SETS     2
#define S_CONSTR   3
#define WAIT_IS    4
#define INILIST    5
#define F_LIST     6
#define L_LIST     7
#define C_LIST     8
#define S_LIST     9
#define INIRULE   10
#define PHEAD     11
#define FHEAD     12
#define INICOND   13
#define NOTCOND   14
#define TERMS     15
#define MORETERMS 16
#define TERMLIST  17
#define S_BARR    18
#define MOREBARR  19
#define ENDBARR   20
#define ENDRULE   21

  //----  TOKENS used to parse constraint_grammar files.
#define CATEGORY 1
#define FORM     2
#define LEMMA    3
#define COMMENT  4
#define CG_ERROR 5
#define BARRIER  6
#define CPAR     7
#define FLOATNUM 8
#define NOT      9
#define OR       10
#define OPAR     11
#define OUTBOUNDS 12
#define POSITION 13
#define SEMICOLON 14
#define USER      15
#define SETS      16
#define CONSTRAINTS 17
#define IS          18
#define SETREF      19
#define SENSE       20


  ////////////////////////////////////////////////////////////////
  /// Create a constraint_grammar, loading a file.
  ////////////////////////////////////////////////////////////////

  constraint_grammar::constraint_grammar(const wstring &fname) {
    const int MAX=50;
    int tok,stat,newstat,i,j;
    int section=0;
    int trans[MAX][MAX];
    bool star;
    wstring err;
    list<wstring> lt;
    list<wstring>::reverse_iterator x;
    map<wstring,setCG>::iterator p;
    ruleCG rul;
    condition cond;

    // We use a FSA to read grammar rules. Fill transition tables
    for (i=0; i<MAX; i++) for (j=0; j<MAX; j++) trans[i][j]=ERROR_ST;

    // State INIT. Initial state. SETS or CONSTRAINTS may come
    trans[INIT][COMMENT]=INIT;  trans[INIT][SETS]=S_SETS; trans[INIT][CONSTRAINTS]=S_CONSTR;

    // Sets section started, read SETS
    trans[S_SETS][COMMENT]=S_SETS; trans[S_SETS][CATEGORY]=WAIT_IS; trans[S_SETS][CONSTRAINTS]=S_CONSTR;
    // Set name read, expect "is"
    trans[WAIT_IS][IS]=INILIST;
    // start of list, see what it contains
    trans[INILIST][FORM]=F_LIST;  trans[INILIST][LEMMA]=L_LIST;  
    trans[INILIST][CATEGORY]=C_LIST; trans[INILIST][SENSE]=S_LIST; 
    // list of forms
    trans[F_LIST][FORM]=F_LIST;  trans[F_LIST][SEMICOLON]=S_SETS;
    // list of lemmas
    trans[L_LIST][LEMMA]=L_LIST;  trans[L_LIST][SEMICOLON]=S_SETS;
    // list of categories
    trans[C_LIST][CATEGORY]=C_LIST;  trans[C_LIST][SEMICOLON]=S_SETS;
    // list of senses
    trans[S_LIST][SENSE]=S_LIST;  trans[S_LIST][SEMICOLON]=S_SETS;

    // Constraints section started, expect first constraint
    trans[S_CONSTR][COMMENT]=S_CONSTR; trans[S_CONSTR][FLOATNUM]=INIRULE;
    // State INIRULE. A rule has started, the head is coming
    trans[INIRULE][CATEGORY]=PHEAD;   
    trans[INIRULE][LEMMA]=FHEAD;  trans[INIRULE][USER]=FHEAD; trans[INIRULE][SENSE]=FHEAD; 
    // State PHEAD. Partial head, it may be continuing
    trans[PHEAD][FORM]=PHEAD;  
    trans[PHEAD][LEMMA]=FHEAD;  trans[PHEAD][SENSE]=FHEAD;  trans[PHEAD][OPAR]=INICOND; 
    // State FHEAD. Full head readed, conditions coming.
    trans[FHEAD][OPAR]=INICOND; 
    // States INICOND, NOTCOND. start of a condition
    trans[INICOND][NOT]=NOTCOND; trans[INICOND][POSITION]=TERMS; 
    trans[NOTCOND][POSITION]=TERMS;  
    // State TERMS. OR-ed term list
    trans[TERMS][CATEGORY]=MORETERMS; trans[TERMS][FORM]=TERMLIST;  trans[TERMS][LEMMA]=TERMLIST; 
    trans[TERMS][SENSE]=TERMLIST;     trans[TERMS][USER]=TERMLIST;  trans[TERMS][OUTBOUNDS]=TERMLIST; 
    trans[TERMS][SETREF]=TERMLIST;
    // State MORETERMS. OR-ed term list
    trans[MORETERMS][FORM]=TERMLIST; trans[MORETERMS][LEMMA]=TERMLIST; trans[MORETERMS][SENSE]=TERMLIST;   
    trans[MORETERMS][OR]=TERMS;      trans[MORETERMS][BARRIER]=S_BARR; trans[MORETERMS][CPAR]=ENDRULE;
    // State TERMLIST. Processing term list
    trans[TERMLIST][OR]=TERMS;        trans[TERMLIST][BARRIER]=S_BARR;  trans[TERMLIST][CPAR]=ENDRULE;
    // State S_BARR. Barrier term list starting
    trans[S_BARR][CATEGORY]=MOREBARR; trans[S_BARR][FORM]=ENDBARR;    trans[S_BARR][LEMMA]=ENDBARR;
    trans[S_BARR][SENSE]=ENDBARR;     trans[S_BARR][USER]=ENDBARR;    trans[S_BARR][SETREF]=ENDBARR; 
    // State MOREBARR. Barrier term list continuing
    trans[MOREBARR][FORM]=ENDBARR;    trans[MOREBARR][LEMMA]=ENDBARR; trans[MOREBARR][SENSE]=ENDBARR;
    trans[MOREBARR][OR]=S_BARR;       trans[MOREBARR][CPAR]=ENDRULE;
    // State ENDBARR. Processing barrier term list
    trans[ENDBARR][OR]=S_BARR;  trans[ENDBARR][CPAR]=ENDRULE;
    // State ENDRULE. End of condition
    trans[ENDRULE][OPAR]=INICOND;  trans[ENDRULE][SEMICOLON]=S_CONSTR;

    // Create lexer for tokens in the grammar
    vector<pair<freeling::regexp,int> > rules;
    rules.push_back(make_pair(freeling::regexp(L"[ \\t\\n\\r]+"),0));
    rules.push_back(make_pair(freeling::regexp(L"%.*"),COMMENT));
    rules.push_back(make_pair(freeling::regexp(L"SETS"),SETS));
    rules.push_back(make_pair(freeling::regexp(L"CONSTRAINTS"),CONSTRAINTS));
    rules.push_back(make_pair(freeling::regexp(L"-?[0-9]+\\.[0-9]+"),FLOATNUM));
    rules.push_back(make_pair(freeling::regexp(L"-?[0-9]+\\*?"),POSITION));
    rules.push_back(make_pair(freeling::regexp(L"\\([[:alpha:]_'\\-\\·]+\\)"),FORM));
    rules.push_back(make_pair(freeling::regexp(L"<[[:lower:]_'\\-\\·]+>"),LEMMA));
    rules.push_back(make_pair(freeling::regexp(L"\\{[A-Z][A-Za-z0-9$]*[*]?\\}"),SETREF));
    rules.push_back(make_pair(freeling::regexp(L"\\[[A-Za-z0-9]+\\]"),SENSE));
    rules.push_back(make_pair(freeling::regexp(L"u\\.[0-9]+=([^\\s\\n]+)"),USER));
    rules.push_back(make_pair(freeling::regexp(L"or"),OR));
    rules.push_back(make_pair(freeling::regexp(L"not"),NOT));
    rules.push_back(make_pair(freeling::regexp(L"="),IS));
    rules.push_back(make_pair(freeling::regexp(L"OUT_OF_BOUNDS"),OUTBOUNDS));
    rules.push_back(make_pair(freeling::regexp(L"barrier"),BARRIER));
    rules.push_back(make_pair(freeling::regexp(L"[A-Z][A-Za-z0-9$:]*[*]?"),CATEGORY));
    rules.push_back(make_pair(freeling::regexp(L";"),SEMICOLON));
    rules.push_back(make_pair(freeling::regexp(L"\\("),OPAR));
    rules.push_back(make_pair(freeling::regexp(L"\\)"),CPAR));
    lexer fl(rules);

    // open grammar file
    wifstream fcg;
    util::open_utf8_file(fcg, fname);
    if (fcg.fail()) ERROR_CRASH(L"Error opening file "+fname);

    senses_used=false;

    // FS automata to check rule file syntax and load rules.
    stat=INIT; err=L"";
    while( (tok=fl.getToken(fcg)) ) {

      newstat = trans[stat][tok];

      // do appropriate processing depending on the state
      switch(newstat){
      case ERROR_ST:
        // error state
        if (tok==COMMENT) err=L"Unexpected comment. Missing semicolon ending previous rule?";
        if (err==L"") err=L"Unexpected '"+fl.getText()+L"' found.";
        WARNING(L"File "+fname+L", line "+util::int2wstring(fl.lineno())+L". Syntax error: "+err);
        err=L"";
          
        // skip until first end_of_rule or EOF, and continue from there.
        while (tok && tok!=SEMICOLON) tok=fl.getToken(fcg);
        newstat=section;
        break;

      case INIT:  // only to expect SETS or CONSTRAINTS
        break;

      case WAIT_IS: {
        // A new set is starting, create set associated to its name, keep iterator, wait for "="
        setCG myset;  
        pair<map<wstring,setCG>::iterator,bool> q=sets.insert(make_pair(fl.getText(),myset));
        p=q.first;
        break;
      }

      case S_SETS:
        section=S_SETS; // where to sync in case of error
        break;

      case INILIST:  //  just read "=", wait to see what are the set members
        break;

      case F_LIST:   // store type of set elements (FORM, LEMMA, CATEGORY)
      case L_LIST:
      case C_LIST:
      case S_LIST:
        switch (tok) {
        case FORM: p->second.type = setCG::FORM_SET; break;
        case LEMMA: p->second.type = setCG::LEMMA_SET; break;
        case SENSE: p->second.type = setCG::SENSE_SET; break;
        case CATEGORY: p->second.type = setCG::CATEGORY_SET; break;
        default: ERROR_CRASH(L"Unexpected token "+util::int2wstring(tok)+L" in list "+p->first);
        }

        p->second.insert(fl.getText());
        break;

      case S_CONSTR:
        section=S_CONSTR; // where to sync in case of error

        if (tok==SEMICOLON && stat==ENDRULE) { 
          // store last condition and tidy up
          rul.push_back(cond);
          cond.clear(); lt.clear();
          // a rule has just finished. Store it, indexed by first char in head if TAG, 
          // or by whole head if <lemma>.  (form) is not allowed in constraint head.
          TRACE(3,L"Inserted new rule:  ("+rul.get_head()+L", "+util::int2wstring(rul.size())+L" conditions)");
          this->insert(make_pair(rul.get_head(),rul)); 
          rul.clear();      
        }
        break;

      case INIRULE:
        // start of a rule, store weight
        rul.set_weight(util::wstring2double(fl.getText()));
        rul.set_head(L"");
        break;

      case PHEAD:
        // got head of a rule, store it
        rul.set_head(fl.getText());
        break;

      case FHEAD:
        // head is continuing, update.
        rul.set_head(rul.get_head()+fl.getText());
        if (tok==SENSE) senses_used=true;
        break;

      case INICOND:
        // If a condition has finished, store it
        if (stat==ENDRULE) rul.push_back(cond);
        // in any case, prepare to start a new condition
        cond.clear();
        lt.clear();
        break;

      case NOTCOND:
        // Negative condition
        cond.set_neg(true);
        break;

      case TERMS: 
        // condition is starting
        if (tok==POSITION && (stat==INICOND || stat==NOTCOND)) {
          // Separate position value from star, if any.
          wstring p = fl.getText();
          wstring::size_type s = p.find_first_of(L"*"); 
          if (s!=wstring::npos) {
            star=true;
            p=p.substr(0,s);
          }
          else star=false;
          s = util::wstring2int(p);
          cond.set_pos(s,star);
          // prepare to accumulate condition terms
          lt.clear();
        }
        break;

      case MORETERMS:
        // new term, accumulate
        lt.push_back(fl.getText());
        break;

      case TERMLIST:
        if (tok==SENSE) senses_used=true;

        if (stat==TERMS) {
          wstring tkform = fl.getText();
          // new term
          if (tok==SETREF && sets.find(tkform.substr(1,tkform.size()-2))==sets.end()) {
            err=L"Reference to undefined SET '"+tkform+L"'.--"+tkform.substr(1,tkform.size()-2)+L"--";
            newstat=ERROR_ST;
          }
          else
            lt.push_back(tkform);
        }
        else if (stat==MORETERMS)  { 
          // term is continuing, update last added element
          x = lt.rbegin(); 
          (*x)=(*x)+fl.getText();
        }
        break;

      case S_BARR:
        if (cond.has_star()) {
          if (tok==BARRIER && (stat==MORETERMS || stat==TERMLIST)) {
            // start of barrier, store accumulated terms
            cond.set_terms(lt);
            // start accumulating barrier terms.
            lt.clear();
          }
        }
        else  {
          err=L"BARRIER is nonsense in a fixed position condition.";
          newstat=ERROR_ST;
        }
        break;
      
      case MOREBARR:
        // new barrier term, accumulate
        lt.push_back(fl.getText());
        break;

      case ENDBARR:
        if (tok==SENSE) senses_used=true;

        if (stat==S_BARR) {
          wstring tkform = fl.getText();
          // new term
          if (tok==SETREF && sets.find(tkform.substr(1,tkform.size()-2))==sets.end()) {
            err=L"Reference to undefined SET '"+tkform+L"'.";
            newstat=ERROR_ST;
          }
          else
            lt.push_back(tkform);
        }
        else if (stat==MOREBARR)  { 
          // term is continuing, update last added element
          x = lt.rbegin();
          (*x)=(*x)+fl.getText();
        }
        break;

      case ENDRULE:
        // end of condition
        if (stat==MORETERMS || stat==TERMLIST) // we come from condition states, no barrier       
          cond.set_terms(lt);  

        else if (stat==S_BARR || stat==MOREBARR || stat==ENDBARR) // we come from barrier states  
          cond.set_barrier(lt);  
        break;

      default: // should never happen
        ERROR_CRASH(L"Bad internal state ("+util::int2wstring(newstat)+L")");

      } // End switch

      stat=newstat;
    }  

    fcg.close();

    TRACE(3,L" Constraint Grammar loaded.");
  }


  ////////////////////////////////////////////////////////////////
  /// Add to the given list all rules with a head starting with the given string.
  ////////////////////////////////////////////////////////////////

  void constraint_grammar::get_rules_head(const wstring &h, list<ruleCG> &lr) const {
    typedef multimap<wstring,ruleCG>::const_iterator T1;
    pair<T1,T1> i;
    T1 j;

    // find appropriate rules
    i=this->equal_range(h);
    // If any rules found, return them in a list
    if (i.first!=this->end() && i.first->first==h) {
      for (j=i.first; j!=i.second; j++) {
        lr.push_back(j->second);
      }
    }
  }


} // namespace
