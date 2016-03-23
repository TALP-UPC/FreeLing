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
#include <fstream>
#include <string>
#include <map>

#include "freeling/regexp.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/phonetics.h"

#define MOD_TRACENAME L"PHONETICS"
#define MOD_TRACECODE PHONETICS_TRACE

using namespace std;

namespace freeling {

  ///////////////////////////////////////////////////////////////
  ///  Constructor of a phonetic trasncoding rule
  ///////////////////////////////////////////////////////////////

  ph_rule::ph_rule() : re_env(L"") {}

  ///////////////////////////////////////////////////////////////
  ///  Destructor of a phonetic trasncoding rule
  ///////////////////////////////////////////////////////////////

  ph_rule::~ph_rule() {}


  ///////////////////////////////////////////////////////////////
  ///  Create the phonetic translator
  ///////////////////////////////////////////////////////////////

  phonetics::phonetics(const wstring &phFile) {

    /// regexps to parse rules
    freeling::regexp assign(L"^(.)=(.+)$");
    freeling::regexp replace(L"^([^/]+)/([^/]*)/(.+)$");

    // loading sound rules
    TRACE(3,L"loading rules from "+phFile);
    // read probabilities file and store information   
    enum sections {RULES,EXCEPTIONS};
    config_file cfg(false,L"#");  
    cfg.add_section(L"Rules",RULES);
    cfg.add_section(L"Exceptions",EXCEPTIONS);

    if (not cfg.open(phFile))
      ERROR_CRASH(L"Error opening file "+phFile);

    wstring line;
    vector<wstring> rem;  
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {

      case RULES: {
        if (cfg.at_section_start()) { 
          // starting new <Rules> section, create new ruleset
          RuleSets.push_back(rule_set());
          Cache.push_back(new safe_map<wstring,wstring>());
        }

        // add rule to current rule set
        int rs = RuleSets.size()-1;
        if (assign.search(line,rem)) {
          RuleSets[rs].Vars[rem[1]] = rem[2];
          TRACE(4,L"added var:"+rem[1]+L" value="+rem[2]);
        }
        else if (replace.search(line,rem)) 
          add_rule(rem[1],rem[2],rem[3]);
        else 
          WARNING(L"Wrongly formatted phonetic rule '"+line+L"' ignored.");
        break;
      }

      case EXCEPTIONS: {
        // Reading exception list
        wistringstream sin; sin.str(line);
        wstring word,sound;
        sin>>word>>sound;      
        Exceptions.insert(make_pair(word,sound));
        break;
      }

      default: break;
      }
    }
    cfg.close();

    TRACE(3,L"Analyzer sucessfully created.");
  
  }

  ///////////////////////////////////////////////////////////////
  /// Destructor
  ///////////////////////////////////////////////////////////////

  phonetics::~phonetics() {
    vector<safe_map<wstring,wstring>*>::iterator p;
    for (p=Cache.begin(); p!=Cache.end(); p++) delete (*p);
  }

  ///////////////////////////////////////////////////////////////
  ///  Insert a new rule in rules list
  ///////////////////////////////////////////////////////////////

  void phonetics::add_rule(const wstring &from, const wstring &to, const wstring &env) {

    ph_rule newrule;
    newrule.from=from;
    newrule.to=to;
    newrule.env=env;

    int rs = RuleSets.size()-1;
    const map<wstring,wstring> & vars = RuleSets[rs].Vars;
  
    // check whether the rule is variable-type (A/B/x)
    map<wstring,wstring>::const_iterator f=vars.find(newrule.from);
    map<wstring,wstring>::const_iterator t=vars.find(newrule.to);
    if (f!=vars.end() and t!=vars.end()) {
      newrule.from=f->second;
      newrule.to=t->second;
    
      // shorten/widen "to" to the same length than "from"
      if (newrule.from.size()<newrule.to.size()) 
        newrule.to=newrule.to.substr(0,newrule.from.size());
      else if (newrule.from.size()>newrule.to.size()) 
        newrule.to = newrule.to + wstring(newrule.from.size()-newrule.to.size(),
                                          newrule.to[newrule.to.size()-1]);
      // store rule with marks to remember it is a set-replacement rule
      newrule.to=L"["+newrule.to+L"]";
      newrule.from=L"["+newrule.from+L"]";
    }
    else if (f!=vars.end() or t!=vars.end()) {
      WARNING(L"Sound conversion rules require variables either in both or none for 'from' and 'to'. Rule "+from+L"/"+to+L"/"+env+L" ignored.");
      return;
    }
  
    // replace any variables in context condition
    size_t e=0; 
    while (e<newrule.env.size()) {
      map<wstring,wstring>::const_iterator k=vars.find(newrule.env.substr(e,1));
      if (k!=vars.end()) {
        newrule.env.replace(e,1,L"["+k->second+L"]");
        e += k->second.size()+2;
      }
      else e++;
    }

    // replace "_" with "(from)" in context
    e = newrule.env.find(L"_");
    if (e==wstring::npos) {
      WARNING(L"Context component of sond conversion rule requires one placeholder '_'. Rule "+from+L"/"+to+L"/"+env+L" ignored.");
      return;
    }
    newrule.env.replace(e,1,L"("+newrule.from+L")");
    newrule.re_env = freeling::regexp(newrule.env);

    TRACE(4,L"Loaded rule ("+newrule.from+L","+newrule.to+L","+newrule.env+L")");
    // store the resulting rule.
    RuleSets[rs].Rules.push_back(newrule);
  }


  /////////////////////////////////////////////////////////////////////////////
  /// getSound return the phonetic translation of a word encoded in SAMPA
  /////////////////////////////////////////////////////////////////////////////

  wstring phonetics::get_sound(const wstring &word) const {

    // avoid working in vain
    if (word.empty()) return word;

    wstring sound;

    // if word in exception list, return sound.
    map<wstring,wstring>::const_iterator ex = Exceptions.find(word);
    if (ex!=Exceptions.end()) {
      sound = ex->second;
      TRACE(4,L"Word "+word+L" found in exception list. sound="+sound);
    }

    else {  // word not found in exception list, use regular rules
      // apply each ruleset, in order
      sound=word;
      for (size_t rs=0; rs<RuleSets.size(); rs++) {
        TRACE(4,L"Start rule set application "+freeling::util::int2wstring(rs));
        // if word found in rule set cache, don't do the work again
        wstring ch;
        if (Cache[rs]->find_safe(sound,ch)) {
          TRACE(4,L"  word "+sound+L" found in cache as "+ch);
          sound = ch;
        }
        else {  // word not in cache. Compute sound and store it in the cache
          vector<ph_rule>::const_iterator r;
          for (r=RuleSets[rs].Rules.begin(); r!=RuleSets[rs].Rules.end(); r++) {
            TRACE(4,L"Appling rule ("+r->from+L"/"+r->to+L"/"+r->env+L") to word '"+word+L"'");
            apply_rule(*r,sound);
            TRACE(4,L"  result: "+sound);
          }
          Cache[rs]->insert_safe(word,sound);
        }
        TRACE(4,L"End rule set application");
      }
    }

    TRACE(3,L"get_sound returns: "+sound+L" for word "+word);
    return sound;
  }


  ///////////////////////////////////////////////////////////////
  ///  Replace all ocurrences in text of "findStr" with replaceStr, 
  /// if "[findStr]" then replaces every char(i) of findStr for replaceStr(i)
  ///////////////////////////////////////////////////////////////

  void phonetics::apply_rule(const ph_rule &rul, wstring &text) const {
  
    if (rul.from==rul.to) return;  // nothing to do.

    vector<wstring> mch;
    vector<int> pos;

    if (rul.from.substr(0,1)!=L"[") { // string-to-string replacing rule 

      // locate each match of "from" in the right context, and replace it with "to"
      size_t p=0;
      wstring::iterator ch=text.begin();
      while (rul.re_env.search(ch, text.end(), mch, pos)) {
        // replace found "from" match with "to"
        text.replace(p+pos[1], rul.from.size(), rul.to); 
        // skip inserted text and look for next match
        p = p + pos[1] + rul.to.size();
        ch = text.begin() + p;
      }
    }

    else { // char-to-char replacing rule: [abcd]/[edfg]/x_y

      // locate each match of a char in "from" set, and replace with pairing char in "to"
      size_t p=0;
      wstring::iterator ch=text.begin();
      while (rul.re_env.search(ch, text.end(), mch, pos)) {
        // find out which was the character, and which position it takes in "from"
        size_t k = rul.from.find(text[p+pos[1]]);
        // replace found match with corresponding character in "to"
        text[p+pos[1]] = rul.to[k];
        // skip it and look for next match
        p = p + pos[1] + 1;
        ch = text.begin() + p;
      }
    }
  }

  ///////////////////////////////////////////////////////////////
  ///  Add phonetic encoding to words in given sentence.
  ///////////////////////////////////////////////////////////////  

  void phonetics::analyze(sentence &s) const {
    for (sentence::iterator w=s.begin(); w!=s.end(); w++)
      w->set_ph_form(get_sound(w->get_lc_form()));
  }


} // namespace
