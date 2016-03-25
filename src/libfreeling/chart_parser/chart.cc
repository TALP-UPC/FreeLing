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

#include "freeling/morfo/chart.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"CHART"
#define MOD_TRACECODE CHART_TRACE

  //-------- Class edge implementation ----------//

  ////////////////////////////////////////////////////////////////
  /// Default constructor of edge.
  ////////////////////////////////////////////////////////////////

  edge::edge(): rule() {}

  ////////////////////////////////////////////////////////////////
  /// Default destructor of edge.
  ////////////////////////////////////////////////////////////////

  edge::~edge() {}

  ////////////////////////////////////////////////////////////////
  /// Constructor from head + right list.
  ////////////////////////////////////////////////////////////////

  edge::edge(const wstring &s, const list<wstring> &ls, const int pgov): rule(s,ls,pgov) {
    matched.clear();
    backpath.clear();
  }

  ////////////////////////////////////////////////////////////////
  /// get matched part of the edge.
  ////////////////////////////////////////////////////////////////

  const list<wstring> edge::get_matched() const {
    return(matched);
  }

  /**
////////////////////////////////////////////////////////////////
///  copy operator
////////////////////////////////////////////////////////////////

edge::edge(const edge &e) {
matched=e.matched;
backpath=e.backpath;
}

////////////////////////////////////////////////////////////////
///  copy operator
////////////////////////////////////////////////////////////////

edge & edge::operator=(const edge &e) {
matched=e.matched;
backpath=e.backpath;
return *this;
}
  **/


  ////////////////////////////////////////////////////////////////
  /// get backpath list of the edge.
  ////////////////////////////////////////////////////////////////

  const list<pair<int,int> > edge::get_backpath() const {
    return(backpath);
  }

  ////////////////////////////////////////////////////////////////
  /// Check whether the edge is complete (inactive).
  ////////////////////////////////////////////////////////////////

  bool edge::active(void) const {
    return(!right.empty());
  }

  ////////////////////////////////////////////////////////////////
  /// Advance the edge one position, storing in backpath the
  /// coordinates of the cell that caused the advance.
  ////////////////////////////////////////////////////////////////

  void edge::shift(int a, int b) {
    matched.splice(matched.end(), right, right.begin());
    backpath.push_back(make_pair(a,b));
  }

  //-------- Class chart implementation ----------//


  ////////////////////////////////////////////////////////////////
  /// Constructor.
  ////////////////////////////////////////////////////////////////

  chart::chart(const grammar &g) : gram(g) {}

  ////////////////////////////////////////////////////////////////
  /// Destructor
  ////////////////////////////////////////////////////////////////

  chart::~chart() {}

  ////////////////////////////////////////////////////////////////
  /// Get size of the table.
  ////////////////////////////////////////////////////////////////

  int chart::get_size() const { return size; }


  ////////////////////////////////////////////////////////////////
  /// load sentece and init parsing (fill up first row of chart).
  ////////////////////////////////////////////////////////////////

  void chart::load_sentence(const sentence &s, int k) {
    int j,n;
    list<wstring> l;
    sentence::const_iterator w;
    word::const_iterator a;
    TRACE(2,L"Loading sentence");

    // allocate all cells we'll need for this sentence
    n=s.size();
    table.clear();  
    table.assign((1+n)*n/2,cell());

    TRACE(2,L"Table allocated");
    // load sentence words in lower row of the chart
    j=0; l.clear();
    for (w=s.begin(); w!=s.end(); w++) {
      cell ce;

      for (a=w->selected_begin(k); a!=w->selected_end(k); a++) {
 
        TRACE(3,L"selected tags");
        edge e(a->get_tag(),l,0); 
        ce.push_back(e); 
        TRACE(3,L" created edge "+a->get_tag()+L" in cell (0,"+util::int2wstring(j)+L")");
        find_all_rules(e,ce,0,j);
      
        edge e1(a->get_tag()+L"("+w->get_lc_form()+L")",l,0); 
        ce.push_back(e1); 
        TRACE(3,L" created edge "+a->get_tag()+L"("+w->get_lc_form()+L") in cell (0,"+util::int2wstring(j)+L")");
        find_all_rules(e1,ce,0,j);
      
        edge e2(a->get_tag()+L"<"+a->get_lemma()+L">",l,0); 
        ce.push_back(e2);
        TRACE(3,L" created edge "+a->get_tag()+L"<"+a->get_lemma()+L"> in cell (0,"+util::int2wstring(j)+L")");
        find_all_rules(e2,ce,0,j);
      }
    
      table[index(0,j)] = ce;
      j++;  
    }

    size=j;

    TRACE(3,L"Sentence loaded.");
  }


  ////////////////////////////////////////////////////////////////
  /// Do the parsing of loaded sentence using current grammar.
  ////////////////////////////////////////////////////////////////

  void chart::parse() {
    list<wstring> ls;
    list<pair<int,int> > lp;
    list<pair<int,int> >::const_iterator p;
    list<edge>::const_iterator ed;
    int a,i,k;
    bool gotroot;
    cell ce;
    edge e;

    // Cycle through lengths
    for (k=1; k<size; k++) {
      // Visit all cells
      for (i=0; i<size-k; i++) {
        ce.clear();
        for (a=0; a<k; a++) {
          TRACE(3,L"Visiting cell ("+util::int2wstring(a)+L","+util::int2wstring(i)+L")");
          for (ed=table[index(a,i)].begin(); ed!=table[index(a,i)].end(); ed++) {     
            if (ed->active()) {
              TRACE(3,L"   Active edge for "+ed->get_head());
              ls=ed->get_right();
              if (can_extend(*ls.begin(),k-a-1,i+a+1)) {
                TRACE(3,L"      it can be extended with "+*ls.begin()+L" at "+util::int2wstring(k-a-1)+L" "+util::int2wstring(i+a+1));
                e = (*ed);
                e.shift(k-a-1,i+a+1);
                ce.push_back(e);
                if (!e.active()) 
                  find_all_rules(e,ce,k,i);
              }
              else
                TRACE(3,L"      it can NOT be extended with "+*ls.begin()+L" at "+util::int2wstring(k-a-1)+L" "+util::int2wstring(i+a+1));
            }
          }
        }
        table[index(k,i)] = ce;
      }
    }


    // search for valid roots covering all the sentence.  Valid roots are inactive 
    // edges at cell (size-1,0) which are not marked as @NOTOP
    edge best; gotroot=false;
    for (ed=table[index(size-1,0)].begin(); ed!=table[index(size-1,0)].end(); ed++) { 
      if (!ed->active() && !gram.is_notop(ed->get_head()) && better_edge(*ed,best)) {
        gotroot=true;
        best=(*ed);
      }
    }
 
    // if no valid root at top level, force the result to be a tree 
    // inserting a fictitious root with label @START
    if (!gotroot) {

      TRACE(3,L"Adding fictitious root at ["+util::int2wstring(size-1)+L",0]");
      lp = cover (size-1, 0);

      ls.clear();
      for (p=lp.begin(); p!=lp.end(); p++) {
        edge best; 
        for (ed=table[index(p->first,p->second)].begin(); ed!=table[index(p->first,p->second)].end(); ed++) { 
          if (!ed->active() && better_edge(*ed,best)) 
            best=(*ed);
        } 
        // there must be some inactive edge, otherwise the cell wouldn't be in the list     
        ls.push_back(best.get_head());
        TRACE(3,L"Inactive edge selected for ("+util::int2wstring(p->first)+L","+util::int2wstring(p->second)+L") is "+best.get_head());     
      }
      // create fictitious edge with the appropriate fictitious rule, with no governor (-1)
      edge e1(gram.get_start_symbol(),ls,grammar::NOGOV);
      TRACE(3, L"created fictitious "+e1.get_head());
      // fill backpath of the fictitious rule
      for (p=lp.begin(); p!=lp.end(); p++) {
        e1.shift(p->first,p->second);
      }
      // insert the edge in the highest left cell
      table[index(size-1,0)].push_back(e1);
    }

    //dump();
  }


  ////////////////////////////////////////////////////////////////
  /// navigate through the chart and obtain a parse tree.
  ////////////////////////////////////////////////////////////////

  parse_tree chart::get_tree(int x, int y, const wstring &lab) const {
    list<edge>::const_iterator ed;
    list<wstring>::const_iterator s;
    list<wstring> r;
    list<pair<int,int> > bp;
    list<pair<int,int> >::const_iterator p;
    parse_tree child;

    wstring label=lab;
    // if no label specified, select best edge at given cell
    // (this is typically the tree root call.)
    if (label==L"") {
      edge best;
      for (ed=table[index(x,y)].begin(); ed!=table[index(x,y)].end(); ed++) {
        if (!ed->active() && !gram.is_hidden(ed->get_head()) && better_edge(*ed,best)) {
          label=ed->get_head();
          best=(*ed);
        }
      }
    }

    node nod(label);
    parse_tree tr(nod);

    TRACE(3, L"  building tree for ("+util::int2wstring(x)+L","+util::int2wstring(y)+L"): "+label);
    if (label==gram.get_start_symbol() || !gram.is_terminal(label)) {

      TRACE(3, L"  Checking: "+label);
      // select edge to expand
      edge best;
      for (ed=table[index(x,y)].begin(); ed!=table[index(x,y)].end(); ed++) {
        if (!ed->active() && label==ed->get_head() && better_edge(*ed,best)) {

          best=(*ed);
          TRACE(3, L"  selected best: "+best.get_head());
        }
      }
      TRACE(3, L"    expanding..");

      r = best.get_matched(); 
      bp = best.get_backpath();
      unsigned int g = best.get_governor();
      unsigned int ch;
      bool headset=false;
      parse_tree::sibling_iterator newh1,newh2;
      for (ch=0,s=r.begin(),p=bp.begin();  s!=r.end() && p!=bp.end();  ch++,s++,p++) {

        // recursive call to process child
        TRACE(3, L"    Entering down to child "+(*s));
        child = get_tree(p->first, p->second, *s);

        // see if we have to skip the root in child tree (hidden, onlytop, or recursive flat label).
        wstring childlabel = child.begin()->get_label();
        if (gram.is_hidden(childlabel) ||
            gram.is_onlytop(childlabel) ||
            (gram.is_flat(childlabel) && label==childlabel)) { 
          TRACE(3, L"    -Child is hidden or flat "+label+L" "+childlabel);
          // skip 'child' and append its daughters
          for (parse_tree::sibling_iterator x=child.sibling_begin(); x!=child.sibling_end(); ++x) {
            // if the skipped child was the head, preserve its head as new head for the father.
            // otherwise, make sure the child's head won't interfere
            if (ch == g) headset=true; 
            else x->set_head(false);
            tr.add_child(*x);
          }
          TRACE(3, L"     skipped, sons raised. Headset="+wstring(headset?L"YES":L"NO"));
        }
        else { //  normal node, append it as a child
          TRACE(3, L"    -Child is NOT hidden or flat "+label+L" "+childlabel);
          // if the child was the head, mark it
          if (ch == g) {
            child.begin()->set_head(true);
            headset=true;
          }
          tr.add_child(child);
          TRACE(3, L"     added. Headset="+wstring(headset?L"YES":L"NO"));
        }      
      }

      if (!headset && label!=gram.get_start_symbol()) {
        WARNING(L"  Unset rule governor for "+label +L" at ("+util::int2wstring(x)+L","+util::int2wstring(y)+L")");
      }
    }

    return tr;
  }


  //------------- Private methods ----------------

  ////////////////////////////////////////////////////////////////
  /// obtain a list of cells that cover the subtree under cell (a,b).
  ////////////////////////////////////////////////////////////////

  list<pair<int,int> > chart::cover (int a, int b) const {
    int x=0,y=0;
    int i,j;
    bool f;
    list<edge>::const_iterator ed;
    list<pair<int,int> > lp,lr;
  
    // if out of range, return empty list
    if (a<0 || b<0 || a+b>=size) {
      lp.clear();
      return(lp);
    }
    TRACE(3,L"Covering under ("+util::int2wstring(a)+L","+util::int2wstring(b)+L")");

    // find highest cell with one inactive edge. Select best edge with the same height.
    f=false; 
    edge best;
    for (i=a; !f && i>=0; i--) {
      for (j=b; j<b+(a-i)+1; j++) {
        for (ed=table[index(i,j)].begin(); ed!=table[index(i,j)].end(); ed++) {
          if (!ed->active() && better_edge(*ed,best) ) {
            x=i; y=j; 
            best=(*ed);
            f=true;
          }
        }
      }
    }
    TRACE(3,L"  Highest cell found is ("+util::int2wstring(x)+L","+util::int2wstring(y)+L")");
    TRACE(3,L"    Pending ("+util::int2wstring(y-b-1)+L","+util::int2wstring(b)+L") ("+util::int2wstring((a+b)-(x+y+1))+L","+util::int2wstring(x+y+1)+L")");

    // we should always find inactive edges in the lowest row of the chart
    if (!f) ERROR_CRASH(L"Inconsistent chart or wrongly loaded sentence.");

    // cover subtrees to left and right
    lp=cover(y-b-1,b);  lr=cover((a+b)-(x+y+1),x+y+1);

    // build the whole sequence and return it
    lp.push_back(make_pair(x,y));
    lp.insert(lp.end(),lr.begin(),lr.end());
    return(lp);

  } 


  ////////////////////////////////////////////////////////////////
  /// Compare two inactive edges. Return true iff first is better.
  /// This is used to build the tree that
  /// is returned by get_tree. The behaviour of this function
  /// is affected by @PRIOR directives in the grammar.
  ////////////////////////////////////////////////////////////////

  bool chart::better_edge(const edge &e1, const edge &e2) const {
 
    wstring h1=e1.get_head();
    wstring h2=e2.get_head();
    wstring start=gram.get_start_symbol();

    // @START symbol is always better
    if (h1==start && h2!=start) return(true);
    if (h1!=start && h2==start) return(false);

    // if both are terminals, the more specific, the better (form is more specific 
    // than lemma, and lemma more than PoS). Lower value, higher specificity.
    if (gram.is_terminal(h1) && gram.is_terminal(h2)) 
      return (gram.get_specificity(h1)<gram.get_specificity(h2));
 
    // if both are non-terminals. Decide according to @PRIOR:
    //  the lower value, the higher priority.
    if (!gram.is_terminal(h1) && !gram.is_terminal(h2)) {
      if (gram.get_priority(h1)<gram.get_priority(h2)) return(true);
      if (gram.get_priority(h1)>gram.get_priority(h2)) return(false);
      // if equal priority, the longer rule, the better. 
      return (e1.get_matched().size()>e2.get_matched().size());
    }

    // non-terminals before terminals.
    return(!gram.is_terminal(h1) && gram.is_terminal(h2));
  }


  ////////////////////////////////////////////////////////////////
  /// compute position of the cell inside the vector.
  ////////////////////////////////////////////////////////////////

  int chart::index(int i, int j) const {
    return j + i*(size+1) - (i+1)*i/2;
  }

  ////////////////////////////////////////////////////////////////
  /// find out whether the cell (i,j) has some inactive 
  /// edge whose head is the given category.
  ////////////////////////////////////////////////////////////////

  bool chart::can_extend(const wstring &hd, int i, int j) const {
    list<edge>::const_iterator ed;
    bool b;

    b = false;
    for (ed=table[index(i,j)].begin(); !b && ed!=table[index(i,j)].end(); ed++) {
      TRACE(4,L"   rule head: "+ed->get_head()+L", active: "+util::int2wstring(ed->active()));
      b = (!ed->active() && check_match(hd,ed->get_head()));
    }

    return b;
  }

  ////////////////////////////////////////////////////////////////
  /// check match between a (possibly) wildcarded string and a literal.
  ////////////////////////////////////////////////////////////////

  bool chart::check_match(const wstring &searched, const wstring &found) const {
    wstring s,m,t;
    wstring::size_type n;
    bool file;

    if (searched==found) return true;

    // not equal, check for a wildcard
    n = searched.find_first_of(L"*");
    if (n == wstring::npos)  return false;  // no wildcard, forget it.

    // check for wildcard match 
    if ( found.find(searched.substr(0,n)) != 0 ) return false;  //no match, forget it.

    // the start of the wildcard expression matches found wstring (e.g. VMI* matches VMI3SP0) 
    // Now, make sure the whole conditions hold (may be VMI*<lemma> )

    // check for lemma or form conditions: Actual searched is the expanded 
    // wildcard plus the original lemma/form condition (if any)
    n=found.find_first_of(L"(<");
    if (n==wstring::npos) {s=found; t=L"";} else {s=found.substr(0,n); t=found.substr(n);}
    n=searched.find_first_of(L"(<");
    if (n==wstring::npos) m=L""; else m=searched.substr(n);

    // if the lemma/form contains quotes, assume it's a filename
    file = (m.find_first_of(L"\"") != wstring::npos);

    if (!file) {
      // normal case, straight check of form/lemma match.
      return (s+m == found);
    }
    else {
      // filename appears in grammar rule. We must look for form/lemma match in file map
      return (gram.in_filemap(t,m));
    }
  }


  ////////////////////////////////////////////////////////////////
  /// Complete edges in a cell (ce) after inserting a terminal, using rules
  /// whose right part starts with wildcard token if appropriate.
  ////////////////////////////////////////////////////////////////

  void chart::find_all_rules(const edge &e, cell &ce, int k, int i) const {
    list<rule> lr;
    list<rule>::iterator r;
    list<wstring> d;
    edge ed;

    // find  rules applicable via wildcards (only for terminals)
    if (gram.is_terminal(e.get_head())) {
      lr = gram.get_rules_right_wildcard(e.get_head().substr(0,1));
      for (r=lr.begin(); r!=lr.end(); r++) {
        if (check_match(*r->get_right().begin(),e.get_head())) {
          TRACE(3,L"    Match for "+e.get_head()+L". adding WILDCARD rule ["+r->get_head()+L"==>"+*r->get_right().begin()+L"..etc");
          edge ed(r->get_head(),r->get_right(),r->get_governor());
          ed.shift(k,i);
          ce.push_back(ed);
          if (!ed.active()) {
            d.push_back(ed.get_head());
          }
        }
      }
    }

    // find normal rules
    d.push_back(e.get_head()); 
    while (!d.empty()) {
      lr=gram.get_rules_right(*d.begin());
      for (r=lr.begin(); r!=lr.end(); r++) {
        TRACE(3,L"    adding rule ["+r->get_head()+L"==>"+*r->get_right().begin()+L"..etc]  with gov="+util::int2wstring(r->get_governor()));
        edge ed(r->get_head(),r->get_right(),r->get_governor());
        ed.shift(k,i);
        ce.push_back(ed);
        if (!ed.active()) {
          d.push_back(ed.get_head());
        }
      }
      d.pop_front();
    }

  }

  ////////////////////////////////////////////////////////////////
  /// output chart contents (debugging purposes only).
  ////////////////////////////////////////////////////////////////

  void chart::dump() const {
    int a,i;
    list<edge>::const_iterator ed;
    list<wstring> ls;
    list<wstring>::const_iterator s;
    list<pair<int,int> > lp;
    list<pair<int,int> >::const_iterator p;

    for (a=0; a<size; a++) {
      for (i=0; i<size-a; i++) {
        if (not table[index(a,i)].empty()) {
          wcout<<L"Cell ("<<a<<L","<<i<<L")"<<endl;
          for (ed=table[index(a,i)].begin(); ed!=table[index(a,i)].end(); ed++) {
            wcout<<L"   "<<ed->get_head()<<L" ==>";
            ls=ed->get_matched();
            for (s=ls.begin(); s!=ls.end(); s++) wcout<<L" "<<*s;
            wcout<<L" .";
            ls=ed->get_right();
            for (s=ls.begin(); s!=ls.end(); s++) wcout<<L" "<<*s;
            lp=ed->get_backpath();
            wcout<<L"   Backpath:";
            for (p=lp.begin(); p!=lp.end(); p++) wcout<<L" ("<<p->first<<L","<<p->second<<L")";
            wcout<<endl;
          }
        }
      }
    }
  }
} // namespace
