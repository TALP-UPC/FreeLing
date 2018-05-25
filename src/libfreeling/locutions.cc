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

#include <sstream>
#include <fstream>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/locutions.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"LOCUTIONS"
#define MOD_TRACECODE LOCUT_TRACE

  //-----------------------------------------------//
  //        Multiword recognizer                   //
  //-----------------------------------------------//

  // State codes 
  // states for a multiword recognizer
#define ST_P 1  // initial state. A mw prefix is found
#define ST_M 2  // Complete multiword found
  // stop state
#define ST_STOP 3

  // Token codes
#define TK_pref   1 // the arriving form with the accumulated mw forms a valid mw prefix
#define TK_mw     2 // the arriving form with the accumulated mw forms a valid complete mw
#define TK_prefL  3 // the arriving lemma with the accumulated mw forms a valid mw prefix
#define TK_mwL    4 // the arriving lemma with the accumulated mw forms a valid complete mw
#define TK_prefP  5 // the arriving tag with the accumulated mw forms a valid mw prefix
#define TK_mwP    6 // the arriving tag with the accumulated mw forms a valid complete mw
#define TK_other  7


  ///////////////////////////////////////////////////////////////
  ///  Create a multiword recognizer, loading multiword file.
  ///////////////////////////////////////////////////////////////

  locutions::locutions(const std::wstring &locFile): automat<locutions_status>() {

    Tags = NULL;
    OnlySelected = false;

    if (not locFile.empty()) { // if no file given, wait for later manual locution loading

      enum sections {TAGSET, MULTIWORDS, ONLYSELECTED};
      config_file cfg;  
      cfg.add_section(L"TagSetFile",TAGSET);
      cfg.add_section(L"Multiwords",MULTIWORDS);
      cfg.add_section(L"OnlySelected",ONLYSELECTED);
      
      if (not cfg.open(locFile))
        ERROR_CRASH(L"Error opening file "+locFile);

      wstring line;
      while (cfg.get_content_line(line)) {
        
        switch (cfg.get_section()) {

        case MULTIWORDS: {
          // read locution entry
          add_locution(line);
          break;
        }
        
        case TAGSET: {
          // load tagset description
          wstring path=locFile.substr(0,locFile.find_last_of(L"/\\")+1);
          Tags = new tagset(util::absolute(line,path));
          break;
        }

        case ONLYSELECTED: {
          // Read status of "OnlySelected"
          OnlySelected = (line==L"yes" or line==L"true");
          break;
        }

        default: break;
        }
      }        
    
      cfg.close(); 
    }

    // Initialize special state attributes
    initialState=ST_P; stopState=ST_STOP;
  
    // Initialize Final state set 
    Final.insert(ST_M);
  
    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // Initializing transitions table
    // State ST_P
    trans[ST_P][TK_pref]=ST_P; trans[ST_P][TK_mw]=ST_M;   
    trans[ST_P][TK_prefL]=ST_P; trans[ST_P][TK_mwL]=ST_M;   
    trans[ST_P][TK_prefP]=ST_P; trans[ST_P][TK_mwP]=ST_M;   
    // State ST_M
    trans[ST_M][TK_pref]=ST_P; trans[ST_M][TK_mw]=ST_M; 
    trans[ST_M][TK_prefL]=ST_P; trans[ST_M][TK_mwL]=ST_M; 
    trans[ST_M][TK_prefP]=ST_P; trans[ST_M][TK_mwP]=ST_M; 

    TRACE(3,L"analyzer succesfully created");
  }


  ///////////////////////////////////////////////////////////////
  ///  Destructor
  ///////////////////////////////////////////////////////////////

  locutions::~locutions() {
    delete Tags;
  }


  ///////////////////////////////////////////////////////////////
  ///  Add a locution rule to the multiword recognizer
  ///////////////////////////////////////////////////////////////

  void locutions::add_locution(const std::wstring &line) {

    wstring prefix, key, lemma, tag;
    wstring::size_type p;   

    wistringstream sin;
    sin.str(line);
    sin>>key;

    // read first pair lemma-tag
    sin>>lemma>>tag;  wstring data = lemma+L" "+tag;
    // read any other pair
    wstring t[2];  int i=0;
    while (sin>>t[i]) {
      if (i==1) {
        data += L"#"+t[0]+L" "+t[1];
        t[0].clear(); t[1].clear();
      }
      i=1-i;
    }
    // store last unpaired field (A/I), if any
    if (t[0].empty()) t[0]=L"I";
    data += L"|"+t[0];
  
    // store multiword in multiwords map
    locut.insert(make_pair(key,data));
  
    // store all its prefixes (xxx_, xxx_yyy_, xxx_yyy_zzz_, ...) in prefixes map 
    prefix=L"";
    p = key.find_first_of(L"_");
    while (p!=wstring::npos) {
    
      prefix += key.substr(0,p+1);                                 
      prefixes.insert(prefix);  
    
      key = key.substr(p+1);
      p = key.find_first_of(L"_");
    }  
  }


  ///////////////////////////////////////////////////////////////
  ///  Chancge behaviour of the module regarding to which analysis
  /// are considered when checking lemma & PoS patterns.
  ///////////////////////////////////////////////////////////////

  void locutions::set_OnlySelected(bool b) {
    OnlySelected=b;
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Auxiliar for ComputeToken
  ///////////////////////////////////////////////////////////////

  bool locutions::check(const wstring &s, set<wstring> &acc, bool &mw, bool &pref, locutions_status *st) const {  
    if (locut.find(s)!=locut.end()) {
      acc.insert(s); 
      st->longest_mw=acc;
      st->over_longest=0;
      TRACE(3,L"  Added MW: "+s);
      mw=true;
    }
    else if (prefixes.find(s+L"_")!=prefixes.end()) {
      acc.insert(s); 
      TRACE(3,L"  Added PRF: "+s);
      pref=true;
    }
    return (mw or pref);
  }


  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int locutions::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
 
    locutions_status *st = (locutions_status *)se.get_processing_status();

    // store component
    st->components.push_back(j);

    wstring form, lem, tag;
    form = j->get_lc_form();
  
    int token = TK_other;

    // look for first analysis matching some locution or prefix
    set<wstring> acc;
    bool mw=false; bool pref=false;

    if (j->size() == 0) {
      // if no analysis, check only the form
      TRACE(3,L"checking ("+form+L")");
      if (st->acc_mw.empty()) {
        check(form,acc,mw,pref,st); 
      }
      else {
        for (set<wstring>::const_iterator i=st->acc_mw.begin(); i!=st->acc_mw.end(); i++) {
          TRACE(3,L"   acc_mw: ["+(*i)+L"]");
          check((*i)+L"_"+form,acc,mw,pref,st); 
        }
      }
    }
    else { 
      // there are analysis, check all multiword patterns
      word::iterator first=j->begin(); 
      word::iterator last=j->end();
      if (OnlySelected) {
        // if option "OnlySelected" is set, check only selected analysis. 
        first=j->selected_begin();
        last=j->selected_end();
        TRACE(4,L"Only selected is set.");
      }
      for (word::iterator a=first; a!=last; a++) {
        bool bm=false,bp=false;
        lem = L"<"+a->get_lemma()+L">";
        tag = a->get_tag();
        if (Tags!=NULL) tag=Tags->get_short_tag(tag);
        TRACE(3,L"checking ("+form+L","+lem+L","+tag+L")");
      
        if (st->acc_mw.empty()) {
          check(form,acc,bm,bp,st); 
          check(lem,acc,bm,bp,st);  
          if (check(tag,acc,bm,bp,st)) {
            j->unselect_all_analysis(); 
            a->mark_selected(); 
          }
          mw=mw||bm; pref=pref||bp; 
        }
        else {
          for (set<wstring>::const_iterator i=st->acc_mw.begin(); i!=st->acc_mw.end(); i++) {
            TRACE(3,L"   acc_mw: ["+(*i)+L"]");
            check((*i)+L"_"+form,acc,bm,bp,st); 
            check((*i)+L"_"+lem,acc,bm,bp,st);  
            if (check((*i)+L"_"+tag,acc,bm,bp,st))  {
              j->unselect_all_analysis(); 
              a->mark_selected(); 
            }
            mw=mw||bm; pref=pref||bp; 
          }
        }
      }
    }

    TRACE(3,L" fora: "+wstring(mw?L"MW":L"noMW")+L","+wstring(pref?L"PREF":L"noPREF"));

    if (mw) token=TK_mw;
    else if (pref) token=TK_pref;

    st->over_longest++;
    st->acc_mw = acc;

    TRACE(3,L"Encoded word: ["+form+L","+lem+L","+tag+L"] token="+util::int2wstring(token));
    return (token);
  }

  ///////////////////////////////////////////////////////////////
  ///   Reset current multiword acumulator.
  ///////////////////////////////////////////////////////////////

  void locutions::ResetActions(locutions_status *st) const {
    st->longest_mw.clear();
    st->acc_mw.clear();
    st->components.clear();
    st->mw_analysis.clear();
  }



  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, when reaching a state, update accumulated multiword.
  ///////////////////////////////////////////////////////////////

  void locutions::StateActions(int origin, int state, int token, sentence::const_iterator j, locutions_status *st) const {

#ifdef VERBOSE
    TRACE(3,L"State actions completed. LMWs are:");
    for (set<wstring>::iterator m=st->longest_mw.begin(); m!=st->longest_mw.end(); m++)
      TRACE(3,L"                                "+*m);
#endif

  }


  ///////////////////////////////////////////////////////////////
  /// Perform last minute validation before effectively building multiword.
  ///////////////////////////////////////////////////////////////

  bool locutions::ValidMultiWord(const word & w, locutions_status *st) const {

    wstring lemma,tag,check,par;
    map<wstring,wstring>::const_iterator mw_data;
    unsigned int nc;
    list<analysis> la;
    bool valid=false;
    bool ambiguous=false;

    // Setting the analysis for the multiword
  
    TRACE(3,L" Form MW: ("+w.get_lc_form()+L") "+L" comp="+util::int2wstring(st->components.size()-st->over_longest+1));
  
    TRACE(3,L" longest_mw #candidates: ("+util::int2wstring(st->longest_mw.size())+L")");

    // consider all possible matching MWs
    for (set<wstring>::iterator m=st->longest_mw.begin(); m!=st->longest_mw.end(); m++ ) {

      wstring form=*m;

      if (locut.find(form)!=locut.end()) {  // only bother if it's a real MW, not a prefix.
    
        TRACE(3,L" matched locution: ("+form+L")");
      
        // MW matched, recover its tags and add them to the list.
        mw_data=locut.find(form);
        wstring::size_type p= mw_data->second.find(L"|");
        wstring tags=mw_data->second.substr(0,p);
        list<wstring> ldata = util::wstring2list(tags,L"#");
        wstring amb=mw_data->second.substr(p+1);
        ambiguous = ambiguous or (amb==L"A");

        TRACE(4,L"   found entry  ("+tags+L")");
      
        for (list<wstring>::const_iterator k=ldata.begin(); k!=ldata.end(); k++) {
        
          TRACE(4,L"   process item ("+*k+L")");
          wistringstream sin;
          sin.str(*k);
          sin>>lemma>>tag;
          TRACE(4,L"   yields pair ("+lemma+L","+tag+L")");
        
          // if lemma has variables, replace them
          p = lemma.find(L"$",0);
          while (p!=wstring::npos and p<lemma.length()-2) {
            // check variable ($F1, $L1, $F2, $L2, etc)
            wstring lf=lemma.substr(p+1,1);
            wstring num=lemma.substr(p+2,1);
            if ((lf==L"F" or lf==L"L") and (num>=L"0" and num<=L"9")) {
              int pos=util::wstring2int(num);
              // get replacing value (form, or first lemma --> to be improved)
              wstring repl;
              TRACE(3,L"n_selected="+util::int2wstring(st->components[pos-1]->get_n_selected()));
              if (lf==L"F") repl=st->components[pos-1]->get_lc_form();
              else if (lf==L"L") repl=st->components[pos-1]->get_lemma();
              // replace variable and repeat, util no more replacements
              lemma.replace(p,3,repl);
            }
            else {
              // we found a '$' but not followed by [FL][0-9], skip it
              ++p;
            }
              
            p = lemma.find(L"$",p);
          }

          // the tag is straighforward, use as is.
          if (tag[0]!=L'$') {
            la.push_back(analysis(lemma,tag));    
            valid = true;
          }
          else {
            // the tag is NOT straighforward, must be recovered from the components
          
            // locate end of component number, and extract the number
            p = tag.find(L":",0);
            if (p==wstring::npos) ERROR_CRASH(L"Invalid tag in locution entry: "+form+L" "+lemma+L" "+tag);
            check=tag.substr(p+1);
            // get the condition on the PoS after the component number (e.g. $1:NC)
            nc=util::wstring2int(tag.substr(1,p-1));
            TRACE(3,L"Getting tag from word $"+util::int2wstring(nc)+L", constraint: "+check);
          
            // search all analysis in the given component that match the PoS condition,
            bool found=false;
            for (word::const_iterator a=st->components[nc-1]->begin(); a!=st->components[nc-1]->end(); a++) { 
              TRACE(4,L"  checking analysis: "+a->get_lemma()+L" "+a->get_tag());
              par=a->get_tag();
              if (par.find(check)==0) {
                found=true;
                la.push_back(analysis(lemma,par));
              }
            }
          
            if (!found) {
              TRACE(2,L"Validation Failed: Tag "+tag+L" not found in word. Locution entry: "+form+L" "+lemma+L" "+tag);
            }
            valid = found;
          }
        }
      }
    }

    st->mw_analysis = la;
    st->mw_ambiguous = ambiguous;
    return(valid);
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void locutions::SetMultiwordAnalysis(sentence::iterator i, int fstate, const locutions_status *st) const {

    i->set_analysis(st->mw_analysis);
    i->set_ambiguous_mw(st->mw_ambiguous);
    TRACE(3,L"Analysis set to: ("+i->get_lemma()+L","+i->get_tag()+L") "+(st->mw_ambiguous?L"[A]":L"[I]"));

    // record this word was analyzed by this module
    i->set_analyzed_by(word::MULTIWORDS);    
  }

} // namespace
