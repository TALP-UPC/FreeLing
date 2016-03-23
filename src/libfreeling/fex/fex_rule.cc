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
#include "freeling/morfo/fex_rule.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"FEX_RULE"
#define MOD_TRACECODE FEX_TRACE

  /// static members

  const freeling::regexp fex_condition::split_re(L"^([tTwWl]|p[Ttl]|na)\\.split\\(\\\"(.*)\\\"\\)$");
  const freeling::regexp fex_rule::rulepat(L"^(.*)(\\$([tTwWlAa]|p[TtlAa]|na|u\\.[[:digit:]]+))(\\((-?[[:digit:]]+)\\))(.*)$");
  const freeling::regexp fex_rule::subexpr(L"^(.*)\\{\\$([[:digit:]]+)\\}(.*)$");
  const freeling::regexp fex_rule::featfun(L"^(.*)\\{([[:alpha:]]+)\\((-?[[:digit:]]+)\\)\\}(.*)$");
  const freeling::regexp fex_rule::rulepat_anch(L"^(.*)(\\$([tTwWlAa]|p[TtlAa]|na|u\\.[[:digit:]]+))(.*)$");

  ////////////////////////////////////////////////////////////////
  ///  Empty constructor
  ////////////////////////////////////////////////////////////////

  fex_condition::fex_condition() : match_re(L"") {}

  ////////////////////////////////////////////////////////////////
  /// Copy constructor.
  ////////////////////////////////////////////////////////////////

  fex_condition::fex_condition(const fex_condition &c) : match_re(c.match_re) {
    cid = c.cid;
    function = c.function;  focus = c.focus;     split = c.split;
    literal = c.literal;    fileset = c.fileset; 
    negated = c.negated;    cond_true = c.cond_true;
  }

  ////////////////////////////////////////////////////////////////
  /// assignment
  ////////////////////////////////////////////////////////////////

  fex_condition& fex_condition::operator=(const fex_condition& c){
    if (this!=&c) {
      cid = c.cid;
      function = c.function;  focus = c.focus;     split = c.split;
      literal = c.literal;    fileset = c.fileset; 
      match_re = c.match_re; 
      negated = c.negated;    cond_true = c.cond_true;
    }
    return *this;
  }


  ////////////////////////////////////////////////////////////////
  ///  fex_condition constructor, given function,
  ///  focus, and filename/regex.
  ////////////////////////////////////////////////////////////////

  fex_condition::fex_condition(const wstring &id, const wstring &foc, 
                               const wstring &fun, 
                               const wstring &param,
                               map<wstring,set<wstring> > &set_files) : match_re(L"") {
    // store condition id
    cid = id;

    // remember if the operation was negated
    negated = (fun[0]==L'!');
    if (negated) function=fun.substr(1);
    else function=fun;

    // extract operation target.
    vector<wstring> rem;
    if (split_re.search(foc,rem)) {
      focus=rem[1];
      split=rem[2];
    }
    else {
      focus = foc;
      split = L"";
    }

    cond_true = false;
    // if condition is trivially true, just remember it
    if (focus==L"ALL") 
      cond_true=true;

    // otherwise, get condition parameters, depending on operation
    else if (function.find(L"in_set")!=wstring::npos) {
      // param is a set file. (warn: file name must be absolute, the caller must ensure that)
      // Check if already in set_files, and load it if necessary

      map<wstring, set<wstring> >::iterator p=set_files.find(param);
      if (p==set_files.end()) { 
        // new file, create new entry in set_files
        p=set_files.insert(set_files.begin(),make_pair(param,set<wstring>()));

        // loading file into set
        wifstream fabr;
        util::open_utf8_file(fabr, param);
        if (fabr.fail()) { ERROR_CRASH(L"Error opening file "+param); }
      
        wstring line;
        while (getline(fabr, line))
          p->second.insert(line);
        fabr.close(); 
      }
      // remember which set this condition is using
      fileset = &p->second;
      // remember file name, for tracing
      literal = param;
    }
    else if (function==L"matches") {
      // param is a regex. Build and store it.
      match_re = freeling::regexp(param);
      // remember regex in string form, for tracing
      literal = param;
    }
    else if (function==L"is") 
      // param is a string, just store it
      literal = param;
    else {
      WARNING(L"Ignored invalid feature extraction condition: "+foc+L" "+fun+L" "+param);
    }

    TRACE(3,L"created new condition "+focus+L" "+wstring(negated?L"!":L"")+function+L" "+param);
  
  }


  ////////////////////////////////////////////////////////////////
  ///  Check if given word satisfies the condition
  ////////////////////////////////////////////////////////////////

  bool fex_condition::check(const word &w, const tagset &tags, fex_status *st) const {

    TRACE(4,L"   Check condition: "+focus+L" "+wstring(negated? L"!" : L"")+function+L" "+literal);
    if (cond_true) return true;  // condition is a literal "true". speed it up.

    list<wstring> target = get_target(w,tags);

    bool t =false;
    list<wstring>::iterator s;

    if (function==L"any_in_set") {
      // Check any target is in the set
      for (s=target.begin(); s!=target.end() and not t; s++)
        t = (fileset->find(*s)!=fileset->end());
    }
    else if (function==L"all_in_set") {
      // Check all targets are in the set
      t=true;
      for (s=target.begin(); s!=target.end() and t; s++)
        t = (fileset->find(*s)!=fileset->end());
    }
    else if (function==L"some_in_set") { 
      // check "some" targets (more than one) are in the set
      int n=0;
      for (s=target.begin(); s!=target.end() and n<1; s++)
        if (fileset->find(*s)!=fileset->end()) n++;

      t = (n>1);
    }
    else if (function==L"is") {
      // check identity
      for (s=target.begin(); s!=target.end() and not t; s++) 
        t = ((*s)==literal);
    }
    else if (function==L"matches") {
      // make sure status contains a vector for regexp matches for this condition
      if (st->re_result.find(cid)==st->re_result.end())
        st->re_result.insert(make_pair(cid,vector<wstring>()));
      // check regexp match, store result
      for (s=target.begin(); s!=target.end() and not t; s++) 
        t = match_re.search(*s,st->re_result[cid]);
    }

    TRACE(4,L"   -- result is "+wstring(t!=negated? L"true" : L"false"));
    /// if negated==true, invert the result (boolean xor)
    return (t!=negated);
  }

  ////////////////////////////////////////////////////////////////
  ///  Obtain the target(s) of a condition 
  ////////////////////////////////////////////////////////////////

  list<wstring> fex_condition::get_target(const word &w, const tagset &tags) const {

    TRACE(5,L"      get target for focus="+focus);
    list<wstring> target;
    if (focus==L"w") target.push_back(w.get_lc_form());
    else if (focus==L"W") target.push_back(w.get_form());
    else if (focus==L"l" and w.get_n_analysis()>0) target.push_back(w.get_lemma());
    else if (focus==L"T" and w.get_n_analysis()>0) target.push_back(w.get_tag());
    else if (focus==L"t" and w.get_n_analysis()>0) target.push_back(tags.get_short_tag(w.get_tag()));
    else if (focus.substr(0,2)==L"u.") {
      unsigned int i = util::wstring2int(focus.substr(2));
      if (i >= w.user.size()) {
        ERROR_CRASH(L"RGF file refers to unexisting user field: "+focus);
      }
      target.push_back(w.user[i]);
    }
    else if (focus==L"pT") { // seek all possible tags, no duplicates
      set<wstring> aux;
      for (word::const_iterator an=w.begin(); an!=w.end(); an++)
        aux.insert(an->get_tag());
      target.insert(target.end(),aux.begin(),aux.end());
    }
    else if (focus==L"pt") { // seek all possible tags, no duplicates
      set<wstring> aux;
      for (word::const_iterator an=w.begin(); an!=w.end(); an++)
        aux.insert(tags.get_short_tag(an->get_tag()));
      target.insert(target.end(),aux.begin(),aux.end());
    }
    else if (focus==L"pl") { // seek all possible lemmas, no duplicates
      set<wstring> aux;
      for (word::const_iterator an=w.begin(); an!=w.end(); an++)
        aux.insert(an->get_lemma());
      target.insert(target.end(),aux.begin(),aux.end());
    }
    else if (focus==L"na") { // seek number of possible analysis
      target.push_back(util::int2wstring(w.size()));
    }

    TRACE(5,L"      got target for focus="+focus+L" split is '"+split+L"'  target="+util::list2wstring(target,L"/"));

    if (not split.empty()) {
      list<wstring> target2;
      for (list<wstring>::iterator t=target.begin(); t!=target.end(); t++) {
        list<wstring> spl=util::wstring2list(*t,split);
        target2.insert(target2.end(),spl.begin(),spl.end());
      }
      TRACE(4,L"     target splitted: ("+util::list2wstring(target2,L"/")+L")");
      return target2;
    }
    else
      return target;
  }


  ////////////////////////////////////////////////////////////////
  /// check whether the condition is "true" (literally) and will match any words.
  ////////////////////////////////////////////////////////////////

  bool fex_condition::is_true() const {
    return cond_true;
  }

  ////////////////////////////////////////////////////////////////
  /// get i-th subexpression match of last RE application
  ////////////////////////////////////////////////////////////////

  wstring fex_condition::get_match(int i, fex_status *st) const {
    if (function!=L"matches") {
      ERROR_CRASH(L"Wrong use of subexpression in rule with no regex matching.");
    }
    return st->re_result[cid][i];
  }

  ////////////////////////////////////////////////////////////////
  /// trace - debugging purposes only
  ////////////////////////////////////////////////////////////////

  void fex_condition::trace(int level) const {
    TRACE(level,L"        "+focus+L" "+wstring(negated?L"!":L"")+function+L" "+literal);
  }

  ////////////////////////////////////////////////////////////////
  /// Constructor, given pattern, rang, and condition
  ////////////////////////////////////////////////////////////////

#define MAX_RANGE 9999
  fex_rule::fex_rule (const wstring &id, const wstring &pat, const wstring &rang,
                      int op, const list<fex_condition> &lcd,
                      const map<wstring,const feature_function*> &custom_feat) : conds(lcd), feat_functs(custom_feat) {

    rid = id;
    operation = op;
    pattern = pat;

    wstring x=rang.substr(1,rang.size()-2);  // remove "[" and "]"
    vector<wstring> v = util::wstring2vector(x,L",");  // split
    left= (v[0]==L"*" ? -MAX_RANGE: util::wstring2int(v[0])); 
    right= (v[1]==L"*" ? MAX_RANGE: util::wstring2int(v[1]));

    if (left>right) {
      WARNING(L"Ignored empty range "+rang+L" in feature extraction rule.");
      left = -MAX_RANGE;
      right = MAX_RANGE;
    }
  }

  ////////////////////////////////////////////////////////////////
  /// Copy constructor.
  ////////////////////////////////////////////////////////////////

  fex_rule::fex_rule(const fex_rule &r) : feat_functs(r.feat_functs) {
    rid = r.rid;
    pattern = r.pattern;
    left = r.left; right = r.right;
    conds = r.conds;
    operation = r.operation;
  }

  ////////////////////////////////////////////////////////////////
  /// assignment
  ////////////////////////////////////////////////////////////////

  fex_rule& fex_rule::operator=(const fex_rule& r) {
    if (this!=&r) {
      rid = r.rid;
      pattern = r.pattern;
      left = r.left; right = r.right;
      conds = r.conds;
      operation = r.operation;
    }
    return *this;
  }

  ////////////////////////////////////////////////////////////////
  /// get rule id
  ////////////////////////////////////////////////////////////////

  wstring fex_rule::get_id() const {
    return rid;
  }

  ////////////////////////////////////////////////////////////////
  /// get left limit of range
  ////////////////////////////////////////////////////////////////

  int fex_rule::get_left() const {
    return left;
  }

  ////////////////////////////////////////////////////////////////
  /// get right limit of range
  ////////////////////////////////////////////////////////////////

  int fex_rule::get_right() const {
    return right;
  }

  ////////////////////////////////////////////////////////////////
  /// check whether a word matches the rule, precompute the 
  /// feature, and store it.
  ////////////////////////////////////////////////////////////////

  void fex_rule::precompute(const sentence& sent, int i, const tagset &tags) const {

    // get status. It already should contain one entry for this feature
    fex_status * st = (fex_status *)sent.get_processing_status();
    map<int,list<wstring> > &rfeats = st->features.find(rid)->second;

    // if feature is already computed for this word, do nothing
    if (rfeats.find(i)!=rfeats.end()) return; 

    /// check if word matches rule conditions
    if (fex_rule::check_conds(conds, operation, sent[i], tags, st)) {
      // create feature, instantiating the feature pattern (except position info)
      list<wstring> feat;
      pattern_instance(sent,i,tags,feat);
      // remember features for this word, even if empty list (so we will 
      // not try (and fail) again to compute them)
      rfeats.insert(make_pair(i,feat));
    } 
  }

  ////////////////////////////////////////////////////////////////
  /// Use precomputed features to extract actual values, 
  /// including position information (if any).
  /// Extract the feature for word "i" as seen from word "anch"
  ////////////////////////////////////////////////////////////////

  void fex_rule::extract(const sentence &sent, int i, int anch, const tagset &tags, list<wstring> &result) const {

    result.clear();

    // get status. It already should contain one entry for this feature
    fex_status * st = (fex_status *)sent.get_processing_status();
    map<int,list<wstring> > &rfeats = st->features.find(rid)->second;

    // A precomputed feature is not available for this word, forget it.
    map<int,list<wstring> >::const_iterator p=rfeats.find(i);
    if (p==rfeats.end() or p->second.empty()) {
      TRACE(4, L"  No precomputed feature available for word "+sent[i].get_form());
      return; 
    }

    list<wstring>::const_iterator f;
    for (f=p->second.begin(); f!=p->second.end(); f++) {

      TRACE(4,L"  extract feature pattern: "+*f); 

      // replace any "@" with the relative position,
      wstring nf=*f;
      size_t c=nf.find(L"@");
      while (c!=wstring::npos) {
        c++;
        nf.insert(c,util::int2wstring(i-anch));
        c=nf.find(L"@",c);
      }
    
      // relace any unindexed $w,$t, etc with the right property of the anchor word.
      list<wstring> res;
      res.push_back(nf);

      vector<wstring> rem;
      list<wstring> extr;
      wstring pref,suf;
      list<wstring>::iterator s=res.begin();
      while (s!=res.end()) {
        rem.clear(); 
        extr.clear();

        TRACE(4,L"    instance "+*s); 
    
        if (rulepat_anch.search(*s,rem)) { 
          // instantiable pattern, perform substitutions
          TRACE(4,L"  pattern anchor instance matches s="+*s); 
          // unchanged parts
          pref=rem[1]; suf=rem[4]; 
          // get replacements for 'info' in current anchor word.
          get_replacements(rem[2], sent[anch], tags, extr);
        }

        // replace chunck with extracted values (if any), add them to pending list.
        for (list<wstring>::iterator e=extr.begin(); e!=extr.end(); e++) {
          TRACE(4,L"    adding replacement "+pref+L"+"+(*e)+L"+"+suf); 
          res.push_back(pref + (*e) + suf);
        }
      
        // Move to next
        list<wstring>::iterator s1=s;
        s++; 
        // if substitutions made, erase processed pattern. 
        if (not extr.empty()) {
          TRACE(3,L"Erasing");
          res.erase(s1);
        }
      }

      result.insert(result.end(),res.begin(),res.end());
    }
  }

  ////////////////////////////////////////////////////////////////
  /// Instantiate the feature pattern (excluding position info)
  /// using current word information.
  ////////////////////////////////////////////////////////////////

  void fex_rule::pattern_instance(const sentence &sent, int i, const tagset &tags, list<wstring> &res) const {

    // get status. 
    fex_status * st = (fex_status *)sent.get_processing_status();

    // start with the pattern as result, and replace its parts
    res.clear();
    res.push_back(pattern);

    vector<wstring> rem,mch;
    list<wstring> extr;
    wstring pref,suf;

    // while the list is not completely processed
    list<wstring>::iterator s=res.begin(); 
    while (s!=res.end()) {

      bool erase=false;
      rem.clear(); mch.clear();
      extr.clear();

      // find first instantiable pattern for current list element
      if (rulepat.search(*s,rem)) {  // instantiable pattern, perform substitutions
        TRACE(4,L"  pattern instance matches s="+*s); 
      
        // look for first replaceable chunks in rule pattern (e.g: $t(0), $l(-1), $1, etc.)
        wstring info = rem[2];

        // convert relative word position to absolute in sentence
        int pos = i + util::wstring2int(rem[5]);        

        // unchanged parts
        pref=rem[1]; suf=rem[6]; 

        TRACE(4,L"  info = "+info+L" rem[5]="+rem[5]+L"  pos = "+util::int2wstring(pos));

        /// pattern requires out of bounds info. Ignore feature.
        if (pos<0 or pos>=(int)sent.size()) {
          res.clear();
          return;
        }

        // get replacements for 'info' in current word.
        get_replacements(info, sent[pos], tags, extr);

        // If replacements where not found where expected, the word 
        // had no analysis: Do not keep feature.
        // If replacements where added, erase original incomplete feature.
        erase = true;
      }

      // no instantiable pattern found, look for subexpressions.
      else if (subexpr.search(*s,rem)) {
        TRACE(4,L"  subexpression matches s="+*s); 
        // unchanged parts
        pref=rem[1]; suf=rem[3]; 
        // get subexpt number
        int i = util::wstring2int(rem[2]);
        // recover subexpr match
        extr.push_back(conds.begin()->get_match(i,st));
        // one replacement added. Erase original incomplete feature
        erase = true;
      }

      // no instantiable pattern found, no subexpression, look for user-defined functions
      else if (featfun.search(*s,rem)) {
        TRACE(4,L"  custom function matches s="+*s); 
        // unchanged parts
        pref=rem[1]; suf=rem[4]; 
        // get function name
        wstring fname = rem[2];
      
        map<wstring,const feature_function*>::const_iterator f=feat_functs.find(fname);
        if (f==feat_functs.end()) {
          WARNING(L"WARNING: Ignoring undefined feature function '"+fname+L"' called from RGF file.");
        }
        else {
          // get parameter number
          int pos = i + util::wstring2int(rem[3]);
          // call user-defined function
          f->second->extract(sent,pos,extr);      
          // replacements added. Erase original incomplete feature
          erase = true;
        }
      }

      // replace chunck with extracted values (if any), add them to pending list.
      for (list<wstring>::iterator e=extr.begin(); e!=extr.end(); e++)
        res.push_back(pref + (*e) + suf);
    
      // Move to next
      list<wstring>::iterator s1=s;
      s++; 
      // if substitutions made, erase processed pattern. 
      if (erase) res.erase(s1);
    }
  }


  ////////////////////////////////////////////////////////////////
  /// Return string matching a simple replace pattern on a fex_rule
  ////////////////////////////////////////////////////////////////

  void fex_rule::get_replacements(const wstring &info, const word &w, const tagset &tags, list<wstring> &extr) const {

    // if information required for substitutions is not available, give up
    if (info!=L"$na" and info!=L"$w" 
        and info!=L"$W" and w.get_n_analysis()==0) 
      return; 

    TRACE(4,L"    replacing "+info);
    // if there is information to substitute, get possible replacements
    if (info==L"$t" and w.get_n_analysis()>0) extr.push_back(tags.get_short_tag(w.get_tag()));
    else if (info==L"$T" and w.get_n_analysis()>0) extr.push_back(w.get_tag());
    else if (info==L"$l" and w.get_n_analysis()>0) extr.push_back(w.get_lemma());
    else if (info==L"$a" and w.get_n_analysis()>0) extr.push_back(w.get_lemma()+L"/"+tags.get_short_tag(w.get_tag()));
    else if (info==L"$A" and w.get_n_analysis()>0) extr.push_back(w.get_lemma()+L"/"+w.get_tag());
    else if (info==L"$w") extr.push_back(w.get_lc_form());
    else if (info==L"$W") extr.push_back(w.get_form());
    else if (info==L"$na") extr.push_back(util::int2wstring(w.get_n_analysis()));
    else if (info.substr(0,3)==L"$u.") extr.push_back(w.user[util::wstring2int(info.substr(3))]);
    else if (info==L"$pt") {
      set<wstring> aux; // avoid duplicates
      for (word::const_iterator a=w.begin(); a!=w.end(); a++)
        aux.insert(tags.get_short_tag(a->get_tag()));
      extr.insert(extr.end(),aux.begin(),aux.end());
    }
    else if (info==L"$pT") {
      set<wstring> aux; // avoid duplicates
      for (word::const_iterator a=w.begin(); a!=w.end(); a++)
        aux.insert(a->get_tag());
      extr.insert(extr.end(),aux.begin(),aux.end());
    }
    else if (info==L"$pl") {
      set<wstring> aux; // avoid duplicates
      for (word::const_iterator a=w.begin(); a!=w.end(); a++)
        aux.insert(a->get_lemma());
      extr.insert(extr.end(),aux.begin(),aux.end());
    }
    else if (info==L"$pa") {
      set<wstring> aux; // avoid duplicates
      for (word::const_iterator a=w.begin(); a!=w.end(); a++)
        aux.insert(a->get_lemma()+L"/"+tags.get_short_tag(a->get_tag()));
      extr.insert(extr.end(),aux.begin(),aux.end());
    }
    else if (info==L"$pA") {
      set<wstring> aux; // avoid duplicates
      for (word::const_iterator a=w.begin(); a!=w.end(); a++)
        aux.insert(a->get_lemma()+L"/"+a->get_tag());
      extr.insert(extr.end(),aux.begin(),aux.end());
    }

  }

  ////////////////////////////////////////////////////////////////
  /// Check a word to find out if it matches a list od conditions
  /// joined with the given and/or operation
  ////////////////////////////////////////////////////////////////

  bool fex_rule::check_conds(const list<fex_condition> &conds, int op, const word &w, const tagset &tags, fex_status *st) {
    /// check if word matches rule conditions
    TRACE(4,L" Checking rule conditions on word: "+w.get_form());
    bool do_and=(op==OP_AND);
    bool chk = do_and;
    list<fex_condition>::const_iterator c;
    for (c=conds.begin(); c!=conds.end() and chk==do_and; c++)
      chk= c->check(w,tags,st);
    return chk;
  }


  ////////////////////////////////////////////////////////////////
  /// trace - debugging purposes only
  ////////////////////////////////////////////////////////////////

  void fex_rule::trace(int level) const {
    TRACE(level,L"   rule: "+pattern);
    TRACE(level,L"     "+wstring(operation==OP_AND?L"AND":(operation==OP_OR?L"OR":L"none"))+L" :");
    for (list<fex_condition>::const_iterator c=conds.begin(); c!=conds.end(); c++)
      c->trace(level);
  }


  ////////////////////////////////////////////////////////////////
  /// Constructor
  ////////////////////////////////////////////////////////////////

  fex_rulepack::fex_rulepack () {}

  ////////////////////////////////////////////////////////////////
  /// Copy constructor
  ////////////////////////////////////////////////////////////////

  fex_rulepack::fex_rulepack(const fex_rulepack &f) {
    rules=f.rules;
    conds=f.conds;
    operation=f.operation;
  }

  ////////////////////////////////////////////////////////////////
  /// assignment
  ////////////////////////////////////////////////////////////////

  fex_rulepack& fex_rulepack::operator=(const fex_rulepack &f) {
    if (this!=&f) {
      rules=f.rules;
      conds=f.conds;
      operation=f.operation;
    }
    return *this;
  }

  ////////////////////////////////////////////////////////////////
  /// trace - debugging purposes only
  ////////////////////////////////////////////////////////////////

  void fex_rulepack::trace(int level) const {
    TRACE(level,L"rulepack:");
    TRACE(level,L"     "+wstring(operation==OP_AND?L"AND":(operation==OP_OR?L"OR":L"none"))+L":");
    for (list<fex_condition>::const_iterator c=conds.begin(); c!=conds.end(); c++)
      c->trace(level);
    TRACE(2,L" --rules:");
    for (list<fex_rule>::const_iterator r=rules.begin(); r!=rules.end(); r++)
      r->trace(level);
  }

} // namespace
