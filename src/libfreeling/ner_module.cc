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

#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"
#include "freeling/morfo/ner_module.h"
#include "freeling/morfo/configfile.h"

using namespace std;

namespace freeling {

#undef MOD_TRACENAME
#undef MOD_TRACECODE
#define MOD_TRACENAME L"NP"
#define MOD_TRACECODE NP_TRACE

  //-----------------------------------------------//
  //       Generic proper noun recognizer                  //
  //-----------------------------------------------//

  /// Allow classes under ner_module to be incomplete automata
  int ner_module::ComputeToken(int, sentence::iterator &, sentence &) const {return 0;}
  void ner_module::ResetActions(ner_status *) const {}
  void ner_module::StateActions(int, int, int, sentence::const_iterator,ner_status *) const {}

  ner_module::ner_module(const wstring &npFile) {

    // default
    Title_length = 0;
    AllCaps_Title_length = 0;
    splitNPs=false;

    enum sections {NE_TAG,TITLE_LIMIT,AC_TITLE_LIMIT,SPLIT_MW};
    config_file cfg(true);
    cfg.add_section(L"NE_Tag",NE_TAG);
    cfg.add_section(L"TitleLimit",TITLE_LIMIT);
    cfg.add_section(L"AllCapsTitleLimit",AC_TITLE_LIMIT);
    cfg.add_section(L"SplitMultiwords",SPLIT_MW);

    if (not cfg.open(npFile))
      ERROR_CRASH(L"Error opening file "+npFile);
  
    // load list of functional words that may be included in a NE.
    wstring line; 
    while (cfg.get_content_line(line)) {

      switch (cfg.get_section()) {
      case NE_TAG: { 
        NE_tag=line;
        break; 
      }

      case TITLE_LIMIT: { 
        Title_length = util::wstring2int(line);
        break; 
      }

      case AC_TITLE_LIMIT: { 
        AllCaps_Title_length = util::wstring2int(line);
        break; 
      }

      case SPLIT_MW: { 
        splitNPs = (line==L"yes");
        break; 
      }

      default: break;
      }
    }
    cfg.close(); 
  }


  ///////////////////////////////////////////////////////////////
  ///  Effectively build multiword, altering sentence
  ///////////////////////////////////////////////////////////////

  sentence::iterator ner_module::BuildMultiword(sentence &se, sentence::iterator start, sentence::iterator end, int fs, bool &built, ner_status *st) const {
    sentence::iterator i;
    list<word> mw;
    wstring form;
  
    TRACE(3,L"  Building multiword");
    for (i=start; i!=end; i++){
      mw.push_back(*i);           
      form += i->get_form()+L"_";
      TRACE(3,L"   added next ["+form+L"]");
    }
    // don't forget last word
    mw.push_back(*i);           
    form += i->get_form();
    TRACE(3,L"   added last ["+form+L"]");
  
    // build new word with the mw list, and check whether it is acceptable
    word w(form,mw);

    sentence::iterator end1=end; end1++;
    if (ValidMultiWord(w,st)) {  
      // if split is set and the MW has more than one word, split it.
      if (splitNPs and start!=end) {
        TRACE(3,L"Valid Multiword. Split NP is on: keeping separate words");
        for (sentence::iterator j=start; j!=se.end() && j!=end1; j++) {
          if (util::RE_is_capitalized.search(j->get_form())) {
            TRACE(4,L"   Splitting. Set "+NE_tag+L" for "+j->get_form());
            // set a unique NP analysis
            j->set_analysis(analysis(j->get_lc_form(),NE_tag));
            // prevent guesser from changing that
            j->set_found_in_dict(true);
          }
        }
      
        ResetActions(st);
        i=end;
        built=true;
      }
      else {
        TRACE(3,L"Valid Multiword. Modifying the sentence");
        // erasing from the sentence the words that composed the multiword
        end++;
        i=se.erase(start, end);
        // insert new multiword it into the sentence
        i=se.insert(i,w); 
        TRACE(3,L"New word inserted");
        // Set morphological info for new MW
        SetMultiwordAnalysis(i,fs,st);
        built=true;
      }
    }
    else {
      TRACE(3,L"Multiword found, but rejected. Sentence untouched");
      ResetActions(st);
      i=end;
      built=false;
    }
  
    return(i);
  }

  ///////////////////////////////////////////////////////////////
  ///  Perform last minute validation before effectively building multiword
  ///////////////////////////////////////////////////////////////

  bool ner_module::ValidMultiWord(const word &w, ner_status *st) const {
        
    // We do not consider a valid proper noun if all words are capitalized 
    // and there are more than Title_length words (it's probably a news
    // title, e.g. "TITANIC WRECKS IN ARTIC SEAS!!").
    // Title_length==0 deactivates this feature

    const list<word> & mw = w.get_words_mw();
    if (Title_length>0 && mw.size() >= Title_length) {
      list<word>::const_iterator p;
      bool lw=false;
      for (p=mw.begin(); p!=mw.end() && !lw; p++) lw=util::RE_has_lowercase.search(p->get_form());
      // if a word with lowercase chars is found, it is not a title, so it is a valid proper noun.
      return (lw);
    }
    else
      return(true);
  }


  ///////////////////////////////////////////////////////////////
  ///   Set the appropriate lemma and tag for the
  ///   new multiword.
  ///////////////////////////////////////////////////////////////

  void ner_module::SetMultiwordAnalysis(sentence::iterator i, int fstate, const ner_status *st) const {

    // Setting the analysis for the Named Entity. 
    // The new MW is just created, so its list is empty.
  
    // Add an NP analysis, with the compound form as lemma.
    TRACE(3,L"   adding NP analysis");
    i->add_analysis(analysis(i->get_lc_form(),NE_tag));
  }

} // namespace
