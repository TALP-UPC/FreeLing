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

#include "freeling/morfo/configfile.h"
#include "freeling/morfo/compounds.h"
#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"COMPOUNDS"
#define MOD_TRACECODE AFF_TRACE

  ///////////////////////////////////////////////////////////////
  ///   Auxiliary class to store valid compound patterns
  ///////////////////////////////////////////////////////////////

  compounds::pattern::pattern(const std::wstring &p, int h, const std::wstring &t) : patr(p), head(h), tag(t) {}
  compounds::pattern::~pattern() {}


  ///////////////////////////////////////////////////////////////
  ///     Constructor
  ///////////////////////////////////////////////////////////////

  compounds::compounds(const wstring &CompFile, const dictionary &d) : dic(d) {

    enum sections {UNK_ONLY, JOIN_CHARS, PATTERNS};
    config_file cfg(true);
    cfg.add_section(L"UnknownWordsOnly",UNK_ONLY);
    cfg.add_section(L"JoinChars",JOIN_CHARS);
    cfg.add_section(L"CompoundPatterns",PATTERNS);

    if (not cfg.open(CompFile)) {
      ERROR_CRASH(L"Error opening file "+CompFile);
    }

    // default value
    unknown_only = true;
    list<wstring> joins;
    
    wstring line; 
    while (cfg.get_content_line(line)) {

      // process each content line according to the section where it is found
      switch (cfg.get_section()) {

      case UNK_ONLY: {
        unknown_only = (line==L"yes");
        break;
      }

      case JOIN_CHARS: {
        if (line.length()>1) {
          WARNING(L"Join character lines must contain exactly one character. Ignoring line '"+line+L"' in file "+CompFile);
        }
        else 
          joins.push_back(line);
        break;
      }

      case PATTERNS: {
        wistringstream sin;
        sin.str(line);

        wstring patr;
        sin>>patr;

        list<wstring> p = util::wstring2list(patr,L"_");
        int head = -1;
        int i = 0;
        for (list<wstring>::iterator k=p.begin(); k!=p.end(); k++) {
          if ((*k)[0]==L'+') {
            k->erase(0,1);
            head=i;
          }
          if ((*k)==L"*") (*k)=L"";
          i++;
        }

        wstring tag;
        bool has_tag = bool(sin>>tag);
        if (head>=0 and has_tag) {
          WARNING(L"Ignoring head mark in pattern '"+line+L"' since tag was specified.");
          head = -1;
        }
        else if (head<0 and not has_tag) {
          WARNING(L"Ignoring compound pattern '"+line+L"' because neither head mark nor tag were specified.");
        }

        if (head>=0 or has_tag) {
          TRACE(3,L"Adding pattern "+util::list2wstring(p,L"_")+L" "+util::int2wstring(head)+L" ("+tag+L")");
          patterns.push_back(pattern(util::list2wstring(p,L"_"),head,tag));
        }
        break;
      }

      default: break;
      }
    }  
    cfg.close(); 

    TRACE(3,L"Dumping dicc");
    // dump dictionary keys into a text buffer
    wstringstream buff;
    dic.dump_dictionary(buff,true);

    TRACE(3,L"Creating FSM");
    // create automata for L(_L+)
    fsm = new foma_FSM(buff, L"", joins);

    TRACE(3,L"Module successfully created");
  }


  ///////////////////////////////////////////////////////////////
  ///     Destructor
  ///////////////////////////////////////////////////////////////

  compounds::~compounds() { 
    delete fsm;
  }


  ///////////////////////////////////////////////////////////////
  ///    Check whether a word is a compound. Add analysis if it is
  ///////////////////////////////////////////////////////////////

  bool compounds::check_compound(word &w) const {

    TRACE(2,L"Checking compound for "+w.get_lc_form());
    // see if word needs checking
    if (unknown_only and w.get_n_analysis()>0) return false;

    // so far, no evidence it is a compound
    bool compound = false;

    // obtain decompositions
    list<alternative> comps;
    fsm->get_similar_words(w.get_lc_form(), comps);
    TRACE(2,L"Obtained splittings");

    if ( comps.empty()) {
        return compound;
    }

    // process decompositions with minimal cost
    list<alternative>::iterator d = comps.begin();
    int cost = d->get_distance();
    while (d != comps.end() and cost == d->get_distance()) {
      TRACE(3,L"Processing splitting "+d->get_form());
      // see if decomposition matches any valid pattern
      list<wstring> dec = util::wstring2list(d->get_form(),L"_");
      list<word> wds;
      for (list<wstring>::iterator s=dec.begin(); s!=dec.end(); s++) {
        word t(*s);
        dic.annotate_word(t);
        wds.push_back(t);
      }

      TRACE(3,L"Splitting annotated");
      TRACE_WORD_LIST(4,wds);
      
      // check each possible pattern, to see if it matches the word
      set<wstring> seen;
      for (list<pattern>::const_iterator p=patterns.begin(); p!=patterns.end(); p++) {

        TRACE(4,L"Matching splitting against pattern "+p->patr+L" "+util::int2wstring(p->head)+L" "+p->tag);
        // for each word in the compound, build compound lemma and tag, 
        // as long as the pattern is satisfied.
        wstring lemma, tag;
        // list of tags in the pattern
        list<wstring> tags = util::wstring2list(p->patr,L"_"); 

        // if pattern and compound differ in size, pattern is not satisfied, skip it
        if (tags.size() != wds.size()) continue;

        // check if each word has the tag required by the pattern
        list<wstring>::iterator t = tags.begin();
        list<word>::iterator wd = wds.begin();
        int i=0;
        bool good=true;
        while (t != tags.end()) {
          // find analysis with tag matching the pattern
          TRACE(5,L"  check tag "+(*t));
          word::iterator a = wd->begin();
          while (a!=wd->end() and a->get_tag().find(*t)!=0) a++;
          if (a==wd->end()) {
            TRACE(5,L"       failed");
            // no tag matches, discard this pattern 
            good = false;
            break;
          }

          TRACE(5,L"       found ["<<a->get_lemma()<<L","<<a->get_tag()<<L"]");
          // store lemma, and tag if it is the head.
          lemma = lemma + L"_" + a->get_lemma();
          if (i == p->head) tag = a->get_tag();
          // next tag in pattern, next word in compound.
          t++;
          wd++;
          i++;
        }
        
        // add new compound analysis to word, if not already there.
        if (p->head<0) tag = p->tag;
        TRACE(5,L"pattern "<<p->patr<<L" good="<<good)
        if (good and seen.find(lemma+L"#"+tag)==seen.end()) {
	  TRACE(5, L"  Adding analysis "<<L"lemma="<<lemma.substr(1)<<L" tag="<<tag);
          w.add_analysis(analysis(lemma.substr(1),tag));
          // record this word was analyzed by this module
          w.set_analyzed_by(word::COMPOUNDS);
          seen.insert(lemma+L"#"+tag);
          compound = true;
        }
      }
    
      d++;  // next decomposition with best cost
    }

    return compound;
  }


}  // namespace

  
