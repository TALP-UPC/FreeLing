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
#include <sstream>

#include "freeling/morfo/suffixes.h"
#include "freeling/morfo/accents.h"
#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"AFFIXES"
#define MOD_TRACECODE AFF_TRACE


  ///////////////////////////////////////////////////////////////
  ///     Create a suffixed words analyzer. 
  ///////////////////////////////////////////////////////////////

  affixes::affixes(const std::wstring &Lang, const std::wstring &sufFile, const dictionary &d): accen(Lang), dic(d) {

    wstring line,key;
    wstring cond,term,output,retok,lema;
    int acc,enc,nomore,always;

    // open suffix file
    wifstream fabr;
    util::open_utf8_file(fabr, sufFile);
    if (fabr.fail()) ERROR_CRASH(L"Error opening file "+sufFile);
  
    int kind= -1;
    Longest[SUF]=0;  Longest[PREF]=0;    
    // load suffix rules
    while (getline(fabr, line)) {
      // skip comments and empty lines       
      if (!line.empty() && line[0]!=L'#') {

        if (line==L"<Suffixes>") kind=SUF;
        else if (line==L"</Suffixes>") kind= -1;
        else if (line==L"<Prefixes>") kind=PREF;
        else if (line==L"</Prefixes>") kind= -1;
        else if (kind==SUF or kind==PREF) {
          // read affix info from line
          wistringstream sin;
          sin.str(line);
          sin>>key>>term>>cond>>output>>acc>>enc>>nomore>>lema>>always>>retok;

          TRACE(4,L"Loading rule: "+key+L" "+term+L" "+cond+L" "+output+L" "
                +util::int2wstring(acc)+L" "+util::int2wstring(enc)+L" "
                +util::int2wstring(nomore)+L" "+lema+L" "+util::int2wstring(always)+L" "+retok);
          // create rule.
          sufrule suf(cond);
          suf.term=term; suf.output=output; suf.acc=acc; suf.enc=enc; 
          suf.nomore=nomore; suf.lema=lema; suf.always=always; 
          suf.retok=retok;
          if (suf.retok==L"-") suf.retok.clear();
          // Insert rule in appropriate affix multimaps.
          affix[kind].insert(make_pair(key,suf));
          if (suf.always) affix_always[kind].insert(make_pair(key,suf));  
          ExistingLength[kind].insert(key.size());
          if (key.size()>Longest[kind]) Longest[kind]=key.size();
        }
      }
    }
  
    fabr.close();

    TRACE(3,L"analyzer succesfully created");
  }


  //////////////////////////////////////////////////////////////////////////////////////////
  /// Look up possible roots of a suffixed form.
  /// Words already analyzed are only applied the "always"-marked suffix rules.
  /// So-far unrecognized words, are applied all the sufix rules.
  //////////////////////////////////////////////////////////////////////////////////////////

  void affixes::look_for_affixes(word &w) const {

    if (w.get_n_analysis()>0) {
      // word with analysys already. Check only "always-checkable" affixes
      TRACE(2,L"=== Known word '"+w.get_form()+L"', with "+util::int2wstring(w.get_n_analysis())+L" analysis. Looking only for 'always' affixes");
      TRACE(3,L" --- Cheking SUF ---");
      look_for_affixes_in_list(SUF,affix_always[SUF],w);
      TRACE(3,L" --- Cheking PREF ---");
      look_for_affixes_in_list(PREF,affix_always[PREF],w);
      TRACE(3,L" --- Cheking PREF+SUF ---");
      look_for_combined_affixes(affix_always[SUF],affix_always[PREF],w);
    }
    else {
      // word not in dictionary. Check all affixes
      TRACE(2,L"===Unknown word '"+w.get_form()+L"'. Looking for any affix");
      TRACE(3,L" --- Cheking SUF ---");
      look_for_affixes_in_list(SUF,affix[SUF],w);
      TRACE(3,L" --- Cheking PREF ---");
      look_for_affixes_in_list(PREF,affix[PREF],w);
      TRACE(3,L" --- Cheking PREF+SUF ---");
      look_for_combined_affixes(affix[SUF],affix[PREF],w);
    }
  }


  //////////////////////////////////////////////////////////////////////////////////////////
  // Look for suffixes of the word w, using the given multimap.
  // The word is annotated with new analysis, if any.
  //////////////////////////////////////////////////////////////////////////////////////////

  void affixes::look_for_affixes_in_list(int kind, const std::multimap<std::wstring,sufrule> &suff, word &w) const {
    typedef multimap<wstring,sufrule>::const_iterator T1;
    T1 sufit;
    pair<T1,T1> rules;
    set<wstring> candidates;
    wstring lws,form_term,form_root;
    unsigned int i,j, len;

    lws=w.get_lc_form();
    len=lws.length();
    for (i=1; i<=Longest[kind] && i<len; i++) {
      // advance backwards in form
      j=len-i;

      // check whether there are any affixes of that size
      if (ExistingLength[kind].find(i)==ExistingLength[kind].end()) {
        TRACE(4,L"No affixes of size "+util::int2wstring(i));
      }
      else {
        // get the termination/beggining
        if (kind==SUF) form_term = lws.substr(j);
        else if (kind==PREF) form_term = lws.substr(0,i);

        // get all rules for that suffix
        rules=suff.equal_range(form_term);
        if (rules.first==suff.end() || rules.first->first!=form_term) {
          TRACE(3,L"No rules for affix "+form_term+L" (size "+util::int2wstring(i)+L")");
        }
        else {
          TRACE(3,L"Found "+util::int2wstring(suff.count(form_term))+L" rules for affix "+form_term+L" (size "+util::int2wstring(i)+L")");
          for (sufit=rules.first; sufit!=rules.second; sufit++) {
            // get the stem, after removing the affix
            if (kind==SUF) form_root = lws.substr(0,j);
            else if (kind==PREF) form_root = lws.substr(i);
            TRACE(3,L"Trying rule ["+form_term+L" "+sufit->second.term+L" "+sufit->second.expression+L" "+sufit->second.output+L"] on root "+form_root);
          
            // complete all possible roots, using terminations/begginings provided in suffix rule
            candidates = GenerateRoots(kind, sufit->second, form_root);
            // fix accentuation patterns of obtained roots
            accen.fix_accentuation(candidates, sufit->second);
            // enrich word analysis list with dictionary entries for valid roots
            SearchRootsList(candidates, form_term, sufit->second, w);
          }
        }
      }
    }
  }


  //////////////////////////////////////////////////////////////////////////////////////////
  // Check if the word w is decomposable with one prefix+one suffix
  // The word is annotated with new analysis, if any.
  //////////////////////////////////////////////////////////////////////////////////////////

  void affixes::look_for_combined_affixes(const std::multimap<std::wstring,sufrule> &suff, const std::multimap<std::wstring,sufrule> &pref, word &w) const
  {
    typedef multimap<wstring,sufrule>::const_iterator T1;
    T1 sufit,prefit;
    pair<T1,T1> rules_S,rules_P;
    set<wstring> candidates,cand1,cand2;
    wstring lws,form_suf,form_pref,form_root;
    unsigned int i,j,len;

    lws=w.get_lc_form();
    len=lws.length();
    for (i=1; i<=Longest[SUF] && i<len; i++) {
      // check whether there are any suffixes of that size. 
      if (ExistingLength[SUF].find(i)==ExistingLength[SUF].end()) {
        TRACE(4,L"No suffixes of size "+util::int2wstring(i));
        continue; // Skip to next size if none found
      }
 
      // check prefixes, only to len-i, since i+j>=len leaves no space for a root.
      for (j=1; j<=Longest[PREF] && j<len-i; j++) {  
        // check whether there are any suffixes of that size. Skip to next if none found
        if (ExistingLength[PREF].find(j)==ExistingLength[PREF].end()) {
          TRACE(4,L"No prefixes of size "+util::int2wstring(j));
          continue; // Skip to next size if none found
        }
      
        // get sufix and prefix of current lengths
        form_suf = lws.substr(len-i);
        form_pref = lws.substr(0,j);
        // get all rules for those affixes
        rules_S = suff.equal_range(form_suf);
        if (rules_S.first==suff.end() || rules_S.first->first!=form_suf) {
          TRACE(3,L"No rules for suffix "+form_suf+L" (size "+util::int2wstring(i)+L")");
          continue; // Skip to next size if no rules found
        }
        rules_P = pref.equal_range(form_pref);
        if (rules_P.first==pref.end() || rules_P.first->first!=form_pref) {
          TRACE(3,L"No rules for prefix "+form_pref+L" (size "+util::int2wstring(j)+L")");
          continue; // Skip to next size if no rules found
        }

        // get the stem, after removing the suffix and prefix
        form_root = lws.substr(0,len-i).substr(j);
        TRACE(3,L"Trying a decomposition: "+form_pref+L"+"+form_root+L"+"+form_suf);

        // if we reach here, there are rules for a sufix and a prefix of the word.      
        TRACE(3,L"Found "+util::int2wstring(suff.count(form_suf))+L" rules for suffix "+form_suf+L" (size "+util::int2wstring(i)+L")");
        TRACE(3,L"Found "+util::int2wstring(pref.count(form_pref))+L" rules for prefix "+form_pref+L" (size "+util::int2wstring(j)+L")");
      
        bool wfid=w.found_in_dict();

        for (sufit=rules_S.first; sufit!=rules_S.second; sufit++) {
          for (prefit=rules_P.first; prefit!=rules_P.second; prefit++) {
          
            candidates.clear();
            // cand1: all possible completions with suffix rule
            cand1 = GenerateRoots(SUF, sufit->second, form_root);
            // fix accentuation patterns of obtained roots
            accen.fix_accentuation(cand1, sufit->second);
            for (set<wstring>::iterator s=cand1.begin(); s!=cand1.end(); s++) {
              // cand2: for each cand1, generate all possible completions with pref rule 
              cand2 = GenerateRoots(PREF, prefit->second, (*s));
              // fix accentuation patterns of obtained roots
              accen.fix_accentuation(cand2, prefit->second);
              // accumulate cand2 to candidate list.
              candidates.insert(cand2.begin(),cand2.end());
            }

            // enrich word analysis list with dictionary entries for valid roots
            word waux=w;
            // apply prefix rules and generate analysis in waux.
            SearchRootsList(candidates, form_pref, prefit->second, waux);
            // use analysis in waux as base to apply suffix rule
            for (set<wstring>::iterator s=candidates.begin(); s!=candidates.end(); s++) 
              ApplyRule(form_pref+(*s), waux, form_suf, sufit->second, w);

            // unless both rules stated nomore, leave everything as it was
            if (not (sufit->second.nomore and prefit->second.nomore))
              w.set_found_in_dict(wfid);  
          }
        }
      }
    }
  }


  //////////////////////////////////////////////////////////////////////////////////////////
  /// Generate all possible forms expanding root rt with all possible terminations
  /// according to the given suffix rule.
  //////////////////////////////////////////////////////////////////////////////////////////

  set<wstring> affixes::GenerateRoots(int kind, const sufrule &suf, const std::wstring &rt) const
  {
    set<wstring> cand;
    wstring term,r;
    size_t pe;

    cand.clear();
    term=suf.term;
    TRACE(3,L"possible terminations/begginings: "+term);

    // fill the set of completed roots
    pe=term.find_first_of(L"|");
    while (pe!=wstring::npos) {
      r=term.substr(0,pe);
      if (r==L"*")  r=L""; // null termination

      if (kind==SUF) {
        TRACE(3,L"Adding to t_roots the element: "+rt+r);
        cand.insert(rt+r);
      }
      else if (kind==PREF) {
        TRACE(3,L"Adding to t_roots the element: "+r+rt);
        cand.insert(r+rt);
      }

      term=term.substr(pe+1);
      pe=term.find_first_of(L"|");
    }

    // adding the last termination/beggining in the list
    if (term==L"*") term=L"";
    if (kind==SUF) {
      TRACE(3,L"Adding to t_roots the element: "+rt+term);
      cand.insert(rt+term);
    }
    else if (kind==PREF) {
      TRACE(3,L"Adding to t_roots the element: "+term+rt);
      cand.insert(term+rt);
    }

    return(cand);
  }


  //////////////////////////////////////////////////////////////////////////////////////////
  /// Search candidate forms in dictionary, discarding invalid forms
  /// and annotating the valid ones.
  //////////////////////////////////////////////////////////////////////////////////////////

  void affixes::SearchRootsList(set<wstring> &roots, const wstring &aff, const sufrule &suf, word &wd) const
  {
    set<wstring> remain;
    set<wstring>::iterator r;
    list<analysis> la;

    TRACE(3,L"Checking a list of "+util::int2wstring(roots.size())+L" roots.");
#ifdef VERBOSE
    for (r=roots.begin(); r!=roots.end(); r++) TRACE(3,L"        "+(*r));
#endif

    remain=roots;

    while (not remain.empty()) {

      r=remain.begin();

      // look into the dictionary for that root
      la.clear();
      dic.search_form(*r,la);

      // if found, we must construct the analysis for the suffix
      if (la.empty()) {
        TRACE(3,L"Root "+(*r)+L" not found.");
        roots.erase(*r);  // useless root, remove from list.
      }
      else {
        TRACE(3,L"Root "+(*r)+L" found in dictionary.");      
        // apply the rule -if matching- and enrich the word analysis list.
        ApplyRule(*r,la,aff,suf,wd);
      }

      // Root procesed. Remove from remaining candidates list.
      remain.erase(*r);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  /// Actually apply a rule
  //////////////////////////////////////////////////////////////////////////////////////////

  void affixes::ApplyRule(const wstring &r, const list<analysis> &la, const wstring &aff, const sufrule &suf, word &wd) const 
  {
    list<analysis>::const_iterator pos;
    analysis a;
    wstring tag,lem;

    for (pos=la.begin(); pos!=la.end(); pos++) {
      // test condition over tag
      if (not suf.cond.search(pos->get_tag())) {
        ///if (not suf.cond.Search(pos->get_tag()) ) {
        TRACE(3,L" Tag "+pos->get_tag()+L" fails input conditon "+suf.expression);
      }
      else {
        TRACE(3,L" Tag "+pos->get_tag()+L" satisfies input conditon "+suf.expression);
        // We're applying the rule. If it says so, avoid assignation 
        // of more tags by later modules (e.g. probabilities).
        if (suf.nomore) wd.set_found_in_dict(true); 
        
        if (suf.output==L"*") {
          // we have to keep tag untouched
          TRACE(3,L"Output tag as found in dictionary");
          tag = pos->get_tag();
        }
        else {
          TRACE(3,L"Output tag provided by suffix rule");
          tag = suf.output;
        }

        // process suf.lema to build output lema
        list<wstring> suflem=util::wstring2list(suf.lema,L"+");
        lem = L"";
        for (list<wstring>::iterator s=suflem.begin(); s!=suflem.end(); s++) {
          if (*s==L"F") {
            TRACE(3,L"Output lemma: add original word form");
            lem = lem + wd.get_lc_form();
          }
          else if ( *s==L"R") {
            TRACE(3,L"Output lemma: add root found in dictionary");
            lem = lem + r;
          }
          else if ( *s==L"L") {
            TRACE(3,L"Output lemma: add lemma found in dictionary");
            lem = lem + pos->get_lemma();
          }
          else if ( *s==L"A") {
            TRACE(3,L"Output lemma: add affix");
            lem = lem + aff;
          }
          else {
            TRACE(3,L"Output lemma: add wstring '"+(*s)+L"'");
            lem = lem + (*s);
          }
        }

        TRACE(3,L"Analysis for the affixed form "+r+L" ("+lem+L","+tag+L")");
        
        list<word> rtk;
        CheckRetokenizable(suf,r,lem,tag,rtk,util::capitalization(wd.get_form()));
        
        word::iterator p;
        // check whether the found analysis was already there
        for (p=wd.begin(); p!=wd.end() && !(p->get_lemma()==lem && p->get_tag()==tag); p++);
        
        // new analysis, analysis to the word, with retokenization info.
        if (p==wd.end()) {
          a.init(lem,tag);
          a.set_retokenizable(rtk);
          wd.add_analysis(a);
        }
        else { // if the analysis was already there, make sure it gets the retokenization
          // info, unless it already has some.
          TRACE(3,L"Analysis was already there, adding RTK info");
          if (!rtk.empty() && !p->is_retokenizable()) 
            p->set_retokenizable(rtk);
        }
      }
    }
  }
 
  //////////////////////////////////////////////////////////////////////////////////////////
  /// Check whether the suffix carries retokenization information, and create alternative
  /// word list if necessary.
  //////////////////////////////////////////////////////////////////////////////////////////

  void affixes::CheckRetokenizable(const sufrule &suf, const std::wstring &form, const std::wstring &lem, const std::wstring &tag, std::list<word> &rtk, int caps) const
  {
    wstring::size_type i;
    analysis a;

    TRACE(3,L"Check retokenizable.");
    if (!suf.retok.empty()) {

      TRACE(3,L" - sufrule has RTK: "+suf.retok);
      i=suf.retok.find_first_of(L":");
    
      list<wstring> forms=util::wstring2list(suf.retok.substr(0,i),L"+");
      list<wstring> tags=util::wstring2list(suf.retok.substr(i+1),L"+");
      if (forms.size()!=tags.size()) 
	ERROR_CRASH(L"Size mismatch for lemma/tag retokenization patterns "+suf.retok);      

      list<wstring>::iterator k,j;
      bool first=true;
      for (k=forms.begin(),j=tags.begin(); k!=forms.end(); k++,j++) {
        // create a word for each pair form/tag in retok 
        word w(L"");
        if ((*k)==L"$$") {  // base form (referred to as "$$" in retok)
          w.set_form(util::capitalize(form,caps,first));
          a.init(lem,tag);
          w.add_analysis(a);
        }
        else { // other retok forms
          w.set_form(util::capitalize(*k,caps,first));

	  if ((*j)[0]==L'!') {
	    // tag has a leading "!", meaning no dictionary check. Just add it
	    a.init(*k,(*j).substr(1));
	    w.add_analysis(a);
	  }
	  else {
            // tag must be checked. Search each form in dict
	    list<analysis> la;
	    list<analysis>::iterator a;
            dic.search_form(*k,la);
            // use only analysis matching the appropriate tag in retok 
            for (a=la.begin(); a!=la.end(); a++) {
              if (a->get_tag().find(*j)==0)
                w.add_analysis(*a);
            }
	  } 
        }
        rtk.push_back(w);
        TRACE(3,L"    word "+w.get_form()+L"("+w.get_lemma()+L","+w.get_tag()+L") added to decomposition list");
        first=false;
      }
    }
  }
} // namespace
