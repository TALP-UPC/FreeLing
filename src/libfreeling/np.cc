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
#include <iostream>  

#include "freeling/morfo/np.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NP"
#define MOD_TRACECODE NP_TRACE

  //-----------------------------------------------//
  //       Proper noun recognizer                  //
  //-----------------------------------------------//

  // State codes
  // recognize named entities (NE).
#define ST_IN 1   // initial
#define ST_NP 2   // Capitalized word, much likely a NE component
#define ST_FUN 3  // Functional word inside a NE
#define ST_PREF 4 // a possible NE prefix was found
#define ST_SUF  5 // a possible NE suffix was found
#define ST_STOP 6

  // Token codes
#define TK_sUnkUpp  1   // non-functional, unknown word, begining of sentence, capitalized, with no analysis yet
#define TK_sNounUpp 2   // non-functional, known as noun, begining of sentence, capitalized, with no analysis yet
#define TK_mUpper 3     // capitalized, not at beggining of sentence
#define TK_mFun  4   // functional word, non-capitalized, not at beggining of sentence
#define TK_mPref 5   // lowercase word in the list of proper noun prefixes (mr., dr.) 
#define TK_mSuf  6   // lowercase word in the list of proper noun suffixes (jr., sr.) 
#define TK_other 7

  ///////////////////////////////////////////////////////////////
  ///     Create a proper noun recognizer
  ///////////////////////////////////////////////////////////////

  np::np(const std::wstring &npFile): ner_module(npFile), RE_NounAdj(RE_NA), 
                                      RE_Closed(RE_CLO), RE_DateNumPunct(RE_DNP) {

    enum sections {NER_TYPE,FUNCTION,SPECIAL,NAMES,NE_IGNORE,
                   REX_NOUNADJ,REX_CLOSED,REX_DATNUMPUNT,AFFIXES};
    config_file cfg(true);
    cfg.add_section(L"Type",NER_TYPE);
    cfg.add_section(L"FunctionWords",FUNCTION);
    cfg.add_section(L"SpecialPunct",SPECIAL);
    cfg.add_section(L"Names",NAMES);
    cfg.add_section(L"Ignore",NE_IGNORE);
    cfg.add_section(L"RE_NounAdj",REX_NOUNADJ);
    cfg.add_section(L"RE_Closed",REX_CLOSED);
    cfg.add_section(L"RE_DateNumPunct",REX_DATNUMPUNT);
    cfg.add_section(L"Affixes",AFFIXES);

    if (not cfg.open(npFile))
      ERROR_CRASH(L"Error opening file "+npFile);

    wstring line; 
    while (cfg.get_content_line(line)) {

      // process each content line according to the section where it is found
      switch (cfg.get_section()) {

      case NER_TYPE: {
        if (util::lowercase(line)!=L"basic")
          ERROR_CRASH(L"Invalid configuration file for 'basic' NER, "+npFile);
        break;
      }

      case FUNCTION: {  // reading Function words
        func.insert(line);
        break;
      }
 
      case SPECIAL: {   // reading special punctuation tags
        punct.insert(line); 
        break;
      } 

      case NAMES: {  // reading list of words to consider names when at line beggining
        names.insert(line);
        break;
      }

      case NE_IGNORE: { // reading list of words to ignore as possible NEs, even if they are capitalized
        wistringstream sin;
        sin.str(line);
        wstring key; int type;
        sin>>key>>type;
        if (util::RE_is_capitalized.search(key)) ignore_tags.insert(make_pair(key,type));
        else ignore_words.insert(make_pair(key,type));
        break;
      }

      case REX_NOUNADJ: {
        RE_NounAdj=freeling::regexp(line);
        break;
      }

      case REX_CLOSED: {
        RE_Closed=freeling::regexp(line);
        break;
      }

      case REX_DATNUMPUNT: {
        RE_DateNumPunct=freeling::regexp(line);
        break;
      }
  
      case AFFIXES: { // list of suffixes/prefixes
        wistringstream sin;
        sin.str(line);
        wstring word, type;
        sin>>word>>type;
        if (type==L"SUF")         
          suffixes.insert(word);
        else if (type==L"PRE") 
          prefixes.insert(word);
        else 
          WARNING(L"Ignored affix with unknown type '"+type+L"' in file "+npFile);
        break;
      }

      default: break;
      }
    }
  
    cfg.close(); 
  
    // Initialize special state attributes
    initialState=ST_IN; stopState=ST_STOP;
    // Initialize Final state set 
    Final.insert(ST_NP); 
    Final.insert(ST_SUF); 
  
    // Initialize transitions table. By default, stop state
    int s,t;
    for(s=0;s<MAX_STATES;s++) for(t=0;t<MAX_TOKENS;t++) trans[s][t]=ST_STOP;
  
    // Initializing transitions table
    // State IN
    trans[ST_IN][TK_sUnkUpp]=ST_NP; trans[ST_IN][TK_sNounUpp]=ST_NP; trans[ST_IN][TK_mUpper]=ST_NP;
    trans[ST_IN][TK_mPref]=ST_PREF; 
    // State PREF
    trans[ST_PREF][TK_mPref]=ST_PREF; 
    trans[ST_PREF][TK_mUpper]=ST_NP;
    // State NP
    trans[ST_NP][TK_mUpper]=ST_NP;
    trans[ST_NP][TK_mFun]=ST_FUN;
    trans[ST_NP][TK_mSuf]=ST_SUF; 
    // State FUN
    // trans[ST_FUN][TK_sUnkUpp]=ST_NP; trans[ST_FUN][TK_sNounUpp]=ST_NP;   // shouldn't be necessary... 
    trans[ST_FUN][TK_mUpper]=ST_NP;
    trans[ST_FUN][TK_mFun]=ST_FUN;
    // State SUF
    trans[ST_SUF][TK_mSuf]=ST_SUF; 

    TRACE(3,L"analyzer succesfully created");
  }


  //-- Implementation of virtual functions from class automat --//

  ///////////////////////////////////////////////////////////////
  ///  Compute the right token code for word j from given state.
  ///////////////////////////////////////////////////////////////

  int np::ComputeToken(int state, sentence::iterator &j, sentence &se) const {
    wstring form, formU;
    int token;
    bool sbegin;
  
    formU = j->get_form();
    form = j->get_lc_form();
    TRACE(5,L">>> Computing token for "+form);
  
    token = TK_other;
  
    if (j==se.begin()) {
      // we are the first word in sentence
      sbegin=true;
    }
    else {
      // not the first, locate previous word...
      sentence::const_iterator ant=j; ant--; 
      // ...and check if it has any of the listed punctuation tags 
      sbegin=false;    
      for (word::const_iterator a=ant->begin(); a!=ant->end() and not sbegin; a++)
        sbegin = (punct.find(a->get_tag())!=punct.end());    
    }
  
    // set ignore flag:  
    //   0= non-ignorable; 1= ignore only if no capitalized neighbours; 2= ignore always
    int ignore=0;
    // check if ignorable word
    map<wstring,int>::const_iterator it=ignore_tags.end();
    map<wstring,int>::const_iterator iw=ignore_words.find(form);
    if (iw!=ignore_words.end()) 
      ignore=iw->second+1;
    else {
      // check if any of word tags are ignorable
      bool found=false;
      for (word::const_iterator an=j->begin(); an!=j->end() and not found; an++) {
        it = ignore_tags.find(an->get_tag());
        found = (it!=ignore_tags.end());
      }    
      if (found) ignore=it->second+1;
    }

    if (ignore==1) {
      TRACE(3,L"Ignorable word ("+form+(it!=ignore_tags.end()?L","+it->first+L")":L")")+L". Ignore=0");
      if (state==ST_NP) 
        token=TK_mUpper;  // if inside a NE, do not ignore
      else {
        // if not inside, only form NE if followed by another capitalized word
        sentence::iterator nxt=j; nxt++;
        if (nxt!=se.end() && util::RE_is_capitalized.search(nxt->get_form()))
          // set token depending on if it's first word in sentence
          token= (sbegin? TK_sNounUpp: TK_mUpper);
      }
    }
    else if (ignore==2) {
      // leave it be as TK_other (so it will be ignored)
      TRACE(3,L"Ignorable word ("+form+(it!=ignore_tags.end()?L","+it->first+L")":L")")+L". Ignore=1");
    }
    // non-ignorable
    else if (sbegin) { 
      TRACE(3,L"non-ignorable word, sbegin ("+form+L")");

      // if all caps (and longer than one-letter word), treat as possible
      // multiword part, regardless of other things (such as PoS tag).
      if (formU.size()>1 and util::RE_all_caps.search(formU)) 
        token = TK_sNounUpp; 

      // first word in sentence (or word preceded by special punctuation sign), and not locked
      else if (not j->is_locked_multiwords() and util::RE_is_capitalized.search(formU) and
               func.find(form)==func.end() and
               not j->is_multiword() and
               not j->find_tag_match(RE_DateNumPunct)) {
        // capitalized, not in function word list, no analysis except dictionary.

        // check for unknown/known word
        if (j->get_n_analysis()==0) {
          // not found in dictionary
          token = TK_sUnkUpp;
        }
        else if ( not j->find_tag_match(RE_Closed) and (j->find_tag_match(RE_NounAdj) or names.find(form)!=names.end() ) ) {
          // found as noun with no closed category
          // (prep, determiner, conjunction...) readings
          token = TK_sNounUpp;
        }
      }
    }
    else if (not j->is_locked_multiwords()) {
      TRACE(3,L"non-ignorable word, non-locked ("+form+L")");
      // non-ignorable, not at sentence beggining, non-locked word
      if (util::RE_is_capitalized.search(formU) && not j->find_tag_match(RE_DateNumPunct))
        // Capitalized and not number/date
        token=TK_mUpper;
      else if (func.find(form)!=func.end())
        // in list of functional words
        token=TK_mFun;
      else if (prefixes.find(form)!=prefixes.end())
        token=TK_mPref;
      else if (suffixes.find(form)!=suffixes.end())
        token=TK_mSuf;
    }
    else {
      TRACE(3,L"non-ignorable word, locked ("+form+L")");
    }
 
    TRACE(3,L"Next word form is: ["+formU+L"] token="+util::int2wstring(token));
    TRACE(3,L"Leaving state "+util::int2wstring(state)+L" with token "+util::int2wstring(token)); 
    return (token);
  }


  ///////////////////////////////////////////////////////////////
  ///   Reset flag about capitalized noun at sentence start.
  ///////////////////////////////////////////////////////////////

  void np::ResetActions(ner_status *st) const {
    st->initialNoun=false;
  }



  ///////////////////////////////////////////////////////////////
  ///  Perform necessary actions in "state" reached from state 
  ///  "origin" via word j interpreted as code "token":
  ///  Basically, set flag about capitalized noun at sentence start.
  ///////////////////////////////////////////////////////////////

  void np::StateActions(int origin, int state, int token, sentence::const_iterator j, ner_status *st) const {

    // if we found a capitalized noun at sentence beginning, remember it.
    if (state==ST_NP) {
      TRACE(3,L"actions for state NP");
      st->initialNoun = (token==TK_sNounUpp);
    }

    TRACE(3,L"State actions completed. initialNoun="+util::int2wstring(st->initialNoun));
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void np::SetMultiwordAnalysis(sentence::iterator i, int fstate, const ner_status *st) const {

    // Setting the analysis for the Named Entity. 
    // The new MW is just created, so its list is empty.
  
    if (st->initialNoun and i->get_n_words_mw()==1) {
      // if the MW has only one word, and is an initial noun, then:
      // 1. copy its possible analysis.
      TRACE(3,L"copying first word analysis list");
      i->copy_analysis(i->get_words_mw().front());
      // 1. copy the modules that analyzed the word.
      i->set_analyzed_by(i->get_words_mw().front().get_analyzed_by());
    }
  
    // Add an NP analysis, with the compound form as lemma.
    ner_module::SetMultiwordAnalysis(i,fstate,st);
  }

} // namespace
