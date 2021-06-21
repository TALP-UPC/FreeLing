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
#include "freeling/morfo/relax_tagger.h"
#include "freeling/morfo/relax.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAX_TAGGER"
#define MOD_TRACECODE TAGGER_TRACE


  //---------- Class relax_tagger ----------------------------------

  ///////////////////////////////////////////////////////////////
  ///  Constructor: Build a relax PoS tagger
  ///////////////////////////////////////////////////////////////

  relax_tagger::relax_tagger(const analyzer_config &opt) : POS_tagger(opt),
							   solver(opt.config_opt.TAGGER_RelaxMaxIter,
								  opt.config_opt.TAGGER_RelaxScaleFactor,
								  opt.config_opt.TAGGER_RelaxEpsilon),
							   c_gram(opt.config_opt.TAGGER_RelaxFile),
							   RE_user(USER_RE) {}
  
  ////////////////////////////////////////////////
  ///  Perform PoS tagging on given sentences
  ////////////////////////////////////////////////

  void relax_tagger::annotate(sentence &se, const analyzer_invoke_options &opt) const {
    sentence::iterator w;
    word::iterator tag;
    list<ruleCG> cand;
    list<ruleCG>::const_iterator cs;
    unsigned int lb,i;
    int v;
  
    
    TRACE(1, L"Creating CLP");
    // Create a variable for each word, and a label foreach analysis.
    // Instantiate applicable constraints and add them to CLP
  
    // add an OUT_OF_BOUNDS mark at the begining of the sequence
    analysis an(L"OUT_OF_BOUNDS",L"OUT_OF_BOUNDS"); an.set_prob(1.0);
    word outofbounds(L"OUT_OF_BOUNDS"); outofbounds.add_analysis(an);
    se.insert(se.begin(),outofbounds);
    se.insert(se.end(),outofbounds);
  
    // build a RL problem for the current sentence (basically a table Vars x Labels)
    // and add appropriate labels (pos tags) to each variable (word)
    problem prb(se.size());
    for (v=0,w=se.begin();  w!=se.end();  v++,w++) {
      prb.set_var_name(v,w->get_form());
      for (tag=w->selected_begin(); tag!=w->selected_end(); tag++) {
        prb.add_label(v,tag->get_prob(),tag->get_tag());
      }
    }      
  
    // precompute which contraints affect each analysis for each word, and add them to the CLP
    for (v=0,w=se.begin(); w!=se.end(); v++,w++) {
    
      TRACE(2, L"   Adding constraints for word "+w->get_form());
    
      // constraints are needed only for ambiguous words
      if (w->get_n_selected()>1) {
      
        for (lb=0,tag=w->selected_begin(); tag!=w->selected_end(); lb++,tag++) {
        
          TRACE(2, L"     Adding label "+tag->get_tag());
        
          /// look for suitable candidate rules for this word-analysis
          wstring t=tag->get_tag(); wstring lm=L"<"+tag->get_lemma()+L">";
          cand.clear();
          c_gram.get_rules_head(t, cand);    // constraints for the TAG of the analysis.
          c_gram.get_rules_head(lm, cand);   // add constraints for the <lemma>
          c_gram.get_rules_head(t+lm, cand); // add constraints for the TAG<lemma> 
          c_gram.get_rules_head(t+L"("+w->get_form()+L")", cand); // add constraints for the TAG(form)
        
          wstring sen=L"";
          const list<pair<wstring,double> > & lsen=tag->get_senses();
          if (c_gram.senses_used && lsen.size()>1) {
            ERROR_CRASH(L"Conditions on 'sense' field used in constraint grammar, but 'DuplicateAnalysis' option was off during sense annotation. "); 
          }
          else if (lsen.size()>0) {
            sen=L"[" + lsen.begin()->first + L"]";
            c_gram.get_rules_head(sen, cand);    // constraints for [sense]
            c_gram.get_rules_head(t+sen, cand);  // constraints for TAG[sense]
          }
        
          // add constraints for user[i] fields:  u.0=stuff, u.1=foo, etc
          vector<wstring>::const_iterator uit; int u;
          for (u=0,uit=tag->user.begin(); uit!=tag->user.end(); u++,uit++) {
            wstring us= L"u."+util::int2wstring(u)+L"="+(*uit);
            c_gram.get_rules_head(us, cand);
          }
        
          // for all possible prefixes of the tag, look for constraints 
          // for TAGPREF*, TAGPREF*<lemma>, and TAGPREF*(form)
          for (i=1; i<t.size(); i++) {
            wstring pref=t.substr(0,i)+L"*";
            c_gram.get_rules_head(pref, cand);                         // TAGPREF*
            c_gram.get_rules_head(pref+lm, cand);                      // TAGPREF*<lemma>
            c_gram.get_rules_head(pref+L"("+w->get_form()+L")", cand);   // TAGPREF*(form)
            if (lsen.size()>0) c_gram.get_rules_head(t+sen, cand);     // TAGPREF*[sense]
          }
        
          TRACE(2, L"    Found "+util::int2wstring(cand.size()) +L" candidate rules.");
        
          // see whether each candidate constraint applies to the current context.
          for (cs=cand.begin(); cs!=cand.end(); cs++) {
          
            // The constraint applies if all its conditions match
            TRACE(3, L"---- Checking candidate for "+cs->get_head()+L" -----------");
            ruleCG::const_iterator x;
            list<list<pair<int,int> > > cnstr;
            bool applies=true;
            for (x=cs->begin(); applies && x!=cs->end(); x++) 
              applies = CheckCondition(se, w, v, *x, cnstr);
          
            // if the constraint applies, create a constraint to add to the label
            TRACE(3, (applies? L"Conditions match." : L"Conditions do not match."));
            if (applies) prb.add_constraint(v, lb, cnstr, cs->get_weight());
          }
        }
      }
    }
  
    TRACE(1, L"Solving CLP");
    // Solve CLP
    solver.solve(prb);
  
    TRACE(1, L"Interpreting CLP solution");
    // Fetch results and apply them to our sentence
  
    se.erase(se.begin());  // remove the OUT_OF_BOUNDS marks.
    w=se.end(); w--; se.erase(w);
  
    v=1;   // skip first mark in the table
    for (w=se.begin(); w!=se.end(); w++) {
      if (w->get_n_selected()>1) {
        // erase any previous selection 
        w->unselect_all_analysis();
      
        // get best labels (may be more than one) for the ambiguous word. 
        // they maintain the original order in which they were inserted.
        list<int> bests=prb.best_label(v);
        word::iterator a=w->begin();
        int pa=0;      
        // for each selected label
        for(list<int>::iterator b=bests.begin(); b!=bests.end(); b++) {
          // advance up to the analysis corresponding to that label   
          while (pa<(*b)) {
            a++;
            pa++;
          }
          // and mark it.
          w->select_analysis(a);
        }

        // // old version to do the same
        // list<int> bests=solver.best_label(v);
        // list<word::iterator> bi;
        // for(list<int>::iterator b=bests.begin(); b!=bests.end(); b++) {
        //   // locate analysis corresponding to that label
        //   word::iterator a=w->unselected_begin();
        //   for (int j=0; j!=(*b); j++) a++;
        //   bi.push_back(a);
        // }
        // //mark them as selected analysis for that word
        // for (list<word::iterator>::iterator k=bi.begin(); k!=bi.end(); k++)
        //   w->select_analysis(*k);      
      }
    
      v++;  // next variable
    }
  }


  //--- private methods --

  ////////////////////////////////////////////////////////////////
  /// auxiliary macro
  ////////////////////////////////////////////////////////////////

#define AdvanceNext(x,w,v)  {if (x.get_pos()>0) {w++; v++;} else if (x.get_pos()<0) {w--; v--;}}


  ////////////////////////////////////////////////////////////////
  /// Find the match of a condition against the context
  /// and add to the given constraint the list of terms to evaluate.
  ////////////////////////////////////////////////////////////////

  bool relax_tagger::CheckCondition(const sentence &s, sentence::const_iterator w, int v, 
                                    const condition &x, list<list<pair<int,int> > > &res) const {
    bool b;
    int nv, bv;
    sentence::const_iterator nw,bw;
    list<pair<int,int> > lsum;

    // position before first in the sentence is controlled with nv.
    // it would be nicer to use s.rend(), but that's not so easy with a single iterator

    // locate the position where to start matching
    TRACE(3, L"    Locating position "+util::int2wstring(x.get_pos())+L" relative to "+util::int2wstring(v));
    nw=w; nv=v; 
    while (nv-v!=x.get_pos() && nw!=s.end() && nv>=0) {
      AdvanceNext(x,nw,nv);    
    }
    TRACE(3, L"    Stopped at relative: "+util::int2wstring(nv-v)+L" absolute: "+util::int2wstring(nv));

    b=false;
    if (nw!=s.end() && nv>=0) {
      // it is a valid sentece position, let's check the word.
      do { 
        b = CheckWordMatchCondition(x.get_terms(), x.is_neg(), nv, nw, lsum);

        if (!b && x.has_star()) AdvanceNext(x,nw,nv);     // if it didn't match but the position 
      }                                                   // had a star, try the next word until
      while (!b && x.has_star() && nw!=s.end() && nv>=0); // one matches or the sentence ends.
    }
    else if (x.is_neg()) {
      // the position is outside the sentence, but the condition had a NOT, thus, it is a match.
      TRACE(3,L"    Out of of bounds, but negative condition: match");
      b=true;
      lsum.push_back(make_pair(0,0));  // word 0, tag 0 is the OUT_OF_BOUNDS mark
    }

    // add result for the matching word to the constraint
    if (b) res.push_back(lsum);

    // if there was a barrier, compute its weight, and add them to the result
    if (b && x.has_barrier()) {

      TRACE(3,L"  Checking BARRIER");
      list<list<pair<int,int> > > rbar;

      bool mayapply=true;
      bw=w; bv=v;  AdvanceNext(x,bw,bv);
      while (bw!=nw && mayapply) {
        if (CheckWordMatchCondition(x.get_barrier(), true, bv, bw, lsum)) 
          rbar.push_back(lsum);
        else 
          mayapply = false;  // found a word such that *all* analysis violate the barrier

        AdvanceNext(x,bw,bv);
      }

      if (!mayapply) {  
        // if there was any word such that *all* its analysis 
        // violated the barrier, the rule does not apply
        b=false;
        TRACE(3,L"  Barrier violated, rule not applying");
      }
      else if (!rbar.empty()) {
        // if there were words in between, add the weights 
        // of their analysis that do not violate the barrier
        for (list<list<pair<int,int> > >::iterator k=rbar.begin(); k!=rbar.end(); k++) 
          res.push_back(*k);
        TRACE(3,L"  Barrier only partially violated, rule may apply");
      }
      else {
        // else there were no words in between, the rule applies normally.
        TRACE(3,L"  No words in between, barrier not violated, rule may apply");
      }
    }
  
    return b;
  }



  ////////////////////////////////////////////////////////////////
  /// check whether a word matches a simple condition
  ////////////////////////////////////////////////////////////////

  bool relax_tagger::CheckWordMatchCondition(const list<wstring> &terms, bool is_neg, int nv,
                                             sentence::const_iterator nw,
                                             list<pair<int,int> > &lsum) const {
    bool amatch,b;
    int lb;
    list<wstring>::const_iterator t;
    word::const_iterator a;

    lsum.clear();
    b=false;
    for (a=nw->analysis_begin(),lb=0;  a!=nw->analysis_end();  a++,lb++) { // check each analysis of the word
  
      TRACE(3,L"    Checking condition for ("+nw->get_form()+L","+a->get_tag()+L")"); 
      amatch=false;
      for (t=terms.begin(); !amatch && t!=terms.end(); t++) { // check each term in the condition
        TRACE(3,L"    -- Checking term "+(*t)); 
        amatch = check_possible_matching(*t, a, nw);
      }
   
      // If the analysis matches (or if it doesn't and the condition was negated),
      // put the pair var-label in the result list.
      if ((amatch && !is_neg) || (!amatch && is_neg)) {
        lsum.push_back(make_pair(nv,lb));
        b = true;
        TRACE(3,L"  word matches condition");
      }

      // Note: Although one matching analysis is enough to satisfy the condition, 
      // we go through all of them, because we want to add the weights of all matching analysis.
    }

    return(b);
  }


  ////////////////////////////////////////////////////////////////
  /// check whether a word matches one of all possible condition patterns
  ////////////////////////////////////////////////////////////////

  bool relax_tagger::check_possible_matching(const wstring &s, word::const_iterator a,
                                             sentence::const_iterator w) const {
    bool b=false;
    vector<wstring> rem;  // to store regex matches
    const list<pair<wstring,double> > & lsen=a->get_senses();


    // If the condition has a "<>" pair, check lemma stuff
    if (s.find_first_of(L"<")!=wstring::npos && s[s.size()-1]==L'>')
      b= check_match(s, L"<"+a->get_lemma()+L">") || 
        check_match(s, a->get_tag()+L"<"+a->get_lemma()+L">");

    // If the condition has a "()" pair, check form stuff
    else if (s.find_first_of(L"(")!=wstring::npos  && s[s.size()-1]==L')') {
      wstring form=w->get_lc_form();
      b= check_match(s, L"("+form+L")") || 
        check_match(s, a->get_tag()+L"("+form+L")"); 
    }

    // If the condition has a "[]" pair, check sense stuff
    else if (s.find_first_of(L"[")!=wstring::npos  && s[s.size()-1]==L']') {
      if (lsen.size()>1) {
        ERROR_CRASH(L"Conditions on 'sense' field used in constraint grammar, but 'DuplicateAnalysis' option was off during sense annotation. "); 
      }
      else if (lsen.size()>0) {
        wstring sens= L"[" + lsen.begin()->first + L"]";
        b= check_match(s,sens) || check_match(s, a->get_tag() + sens); 
      }
      else 
        b=false;
    }

    // If the condition has a "{}" pair, look in the sets map
    else if (s.find_first_of(L"{")!=wstring::npos  && s[s.size()-1]==L'}') {
      map<wstring,setCG>::const_iterator p = c_gram.sets.find(s.substr(1,s.size()-2));
      wstring search;
      switch (p->second.type) {
      case setCG::FORM_SET:  search= L"("+w->get_lc_form()+L")"; 
        break;
      case setCG::LEMMA_SET: search= L"<"+util::lowercase(a->get_lemma())+L">"; 
        break;
      case setCG::SENSE_SET: if (lsen.size()>1) {
          ERROR_CRASH(L"Conditions on 'sense' field used in constraint grammar, but 'DuplicateAnalysis' option was off during sense annotation. "); 
        }
        else 
          search= L"["+ (lsen.size()>0 ? lsen.begin()->first : L"") +L"]"; 
        break;
      case setCG::CATEGORY_SET: search= a->get_tag(); 
        break;
      }
      b= (p->second.find(search) != p->second.end());
    }

    // If the condition has pattern "u.9=XXXXXX", check user stuff
    else if (RE_user.search(s,rem)) {
      // find position indexed in the rule
      wstring ps=rem[1];  //RE_user.Match(1);
      wstring::size_type p= util::wstring2int(ps); 
      // check condition if position exists
      b = (a->user.size()>p) && check_match(s, L"u."+ps+L"="+a->user[p]);  
    }

    // otherwise, check tag
    else 
      b = check_match(s, a->get_tag());

    return (b);
  }


  ////////////////////////////////////////////////////////////////
  /// check match between a (possibly) wildcarded string and a literal.
  /// ------------------------------------------------------------
  /// This method is identical to chart::check_match. 
  /// Maybe some kind of generalization into a new class would be a good idea ?
  /// Maybe it should be moved to util.cc or like ?
  ////////////////////////////////////////////////////////////////

  bool relax_tagger::check_match(const wstring &searched, const wstring &found) const {
    wstring s,m;
    wstring::size_type n;

    TRACE(3,L"       matching "+searched+L" and "+found);

    if (searched==found) return true;

    // not equal, check for a wildcard
    n = searched.find_first_of(L"*");
    if (n == wstring::npos)  return false;  // no wildcard, forget it.

    // check for wildcard match 
    if ( found.find(searched.substr(0,n)) != 0 ) return false;  //no match, forget it.

    // the start of the wildcard expression matches found string (e.g. VMI* matches VMI3SP0) 
    // Now, make sure the whole conditions hold (may be VMI*<lemma> )

    // check for lemma or form conditions: Actual searched is the expanded 
    // wildcard plus the original lemma/form condition (if any)
    n=found.find_first_of(L"(<[");
    if (n==wstring::npos) s=found; else s=found.substr(0,n);  
    n=searched.find_first_of(L"(<[");
    if (n==wstring::npos) m=L""; else m=searched.substr(n);

    return (s+m == found);
  }





} // namespace
