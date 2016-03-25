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

#include "freeling/morfo/accents_modules.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/traces.h"

#define MOD_TRACENAME L"ACCENTS"
#define MOD_TRACECODE AFF_TRACE

using namespace std;

namespace freeling {

  //---------------------------------------------------------//
  //      Galician-specific accentuation                     //
  //---------------------------------------------------------//

  const freeling::regexp accents_gl::last_vowel_put_acc(L"(.*)([aeiou])([sn]?)$");
  const freeling::regexp accents_gl::last_vowel_not_acc(L"(.*)([aeiou])([^aeiou]*)$");
  const freeling::regexp accents_gl::any_vowel_acc(L"(.*)([áéíóú])(.*)");
  const freeling::regexp accents_gl::any_closed_vowel_acc(L"(.*)(([aeio]|[^qg]u)[íú])|([íú][aeiou])(.*)");
  const freeling::regexp accents_gl::oxytone_without_acc(L"(.*)([aeiou]+)([^aeiou]*)(([^iu][ae])|([^aeo]i)|([qg]u[ei]))([sn]?)$");
  const freeling::regexp accents_gl::oxytone_with_acc(L"(.*)[áéí]([sn]?)$");
  const freeling::regexp accents_gl::diacritic_acc(L"^(cómpren?|dás?|é|pór|sé|vé[ns])$");
  const freeling::regexp accents_gl::past_subj_with_acc(L"(.*)([áéí]se(mos|des))$");
  const freeling::regexp accents_gl::infinitive_r_accusative_allomorph(L"([^áéíóú]*)([ae])([rs])$");
  const freeling::regexp accents_gl::present_s_accusative_allomorph(L"([áéíóú]*)([ae])([rs])$");

  /// maps to store equivalences between accentued and not accentued chars.
  const map<wstring,wstring> accents_gl::with_acc = {{L"a",L"á"},{L"e",L"é"},{L"i",L"í"},{L"o",L"ó"},{L"u",L"ú"}};
  const map<wstring,wstring> accents_gl::without_acc = {{L"á",L"a"},{L"é",L"e"},{L"í",L"i"},{L"ó",L"o"},{L"ú",L"u"}};

  //////  Galician accent handling regexps ////////

  // check if a word has accents
#define is_accentued_gl(s) any_vowel_acc.search(s)
  // check if is needed to put accent for a second singular person or a third person of indicative future
#define check_add_gl(s)  oxytone_without_acc.match(s)
  // check if it is an diacritic accent
#define diacritic_acc_gl(s)  diacritic_acc.match(s)
  // check if it is a second singular person or a third person of indicative future or present with accent
#define oxytone_with_acc_gl(s)  oxytone_with_acc.match(s)
  // check if it is a fisrt or a second plural forms of past subjunctive
#define past_subj_with_acc_gl(s)  past_subj_with_acc.match(s)
  // check if it is an hiatus accentued
#define any_closed_vowel_acc_gl(s)  any_closed_vowel_acc.search(s)
  // check if it is a potential infinitive to disambiguate
#define infinitive_r_accusative_allomorph_gl(s)  infinitive_r_accusative_allomorph.match(s)
  // check if it is a potential second singular person of indicative present to disambiguate
#define present_s_accusative_allomorph_gl(s)  present_s_accusative_allomorph.match(s)
  // check an accentued form for a correct acentuation pattern
#define check_accent_gl(s) (diacritic_acc_gl(s)||oxytone_with_acc_gl(s)||past_subj_with_acc_gl(s)||any_closed_vowel_acc_gl(s))


  ///////////////////////////////////////////////////////////////
  /// Create an accent handler for Galician.
  ///////////////////////////////////////////////////////////////

  accents_gl::accents_gl(): accents_module() {
    TRACE(3,L"created Galician accent handler ");
  }

  ///////////////////////////////////////////////////////////////
  /// specific Galician behaviour: modify given roots according
  /// to Galician accentuation requirements. Roots are obtanined
  /// after suffix removal and may require accent fixing, which
  /// is done here.
  ///////////////////////////////////////////////////////////////

  void accents_gl::fix_accentuation(set<wstring> &candidates, const sufrule &suf) const {
    set<wstring> roots;
    set<wstring>::iterator r;
        
    // Disambiguates phonological accent for roots with accusative 
    // allomorph before cheking if accent is right or wrong in roots 
    // (e. g. comelos -> comer + o / cómelos -> comes + o)
    fix_accusative_allomorph_gl(candidates); 
  
    for (r=candidates.begin(); r!=candidates.end(); r++) {
    
      // first lowercase string (is needed here to consider accentued capitalization)
      wstring s=util::lowercase(*r);

      // TRACE(3,L"We store always the root in roots without any changes ");
      // roots.insert(s);
    
      if (suf.enc) {
        TRACE(3,L"enclitic suffix. root="+s);
      
        if (not is_accentued_gl(s)) { // non-accentued roots
          if (check_add_gl(s)){ 
            TRACE(3,L"Add accent second singular and third persons of some present and future indicative oxytone forms");
            s = put_accent_gl(s);
          }
        }
        else { // accentued roots
          if (not check_accent_gl(s)) {
            TRACE(3,L"Removing wrong accents ");
            s = remove_accent_gl(s);
          }
        }
        roots.insert(s); // append to roots this element
      }
    
      else if (suf.acc) {
      
        TRACE(3,L"We store always the root in roots without any changes ");
        roots.insert(s);
        TRACE(3,L"try without accent and with an accent in each sylabe ");
        // first remove all accents
        s = remove_accent_gl(s);
      
        // then construct all possible accentued roots
        wstring suf=L"";
        vector<wstring> rem;
        while (last_vowel_not_acc.match(s,rem)) {
          wstring r = rem[1] + with_acc.find(rem[2])->second + rem[3] + suf;
          roots.insert(r);
          suf = rem[2] + rem[3] + suf;
          s = rem[1];
        }
      }
    }
  
    candidates=roots;
  }

  ////////////////////////////////////////////////////////////
  /// given a non-accentued second singular person or third person 
  /// of indicative future forms, put an accent on the last vowel.
  ////////////////////////////////////////////////////////////

  std::wstring accents_gl::put_accent_gl(const std::wstring &s) {

    std::wstring res=s;
    vector<wstring> rem;
    if (last_vowel_put_acc.match(res,rem)) {
      res = rem[1] + with_acc.find(rem[2])->second + rem[3];
    }
    else 
      res = s;

    return res;
  }


  ////////////////////////////////////////////////////////////
  /// given an accentued form, remove the accent(s).
  ////////////////////////////////////////////////////////////

  std::wstring accents_gl::remove_accent_gl(const std::wstring &s)  {

    std::wstring res=s;
    vector<wstring> rem;
    while (any_vowel_acc.match(res,rem)) {
      res = rem[1] + without_acc.find(rem[2])->second + rem[3];
    }
    return res;
  }


  ///////////////////////////////////////////////////////////////
  /// Fix accent for Galician accusative allomorphs
  ///////////////////////////////////////////////////////////////

  void accents_gl::fix_accusative_allomorph_gl(std::set<std::wstring> &s) {
    std::set<std::wstring>::iterator r1, r2;

    if (s.empty()) return;

    r2=s.end(); r2--;
    r1=r2; r1--;
    while (r2!=s.begin()) {
      std::wstring root1=util::lowercase(*r1), root2=util::lowercase(*r2);
      // Compares actual candidate and next candidate to 
      // disambiguate (eg. comelo/cómelo).  Prevents removing second 
      // person singular of future indicative (comeralo = comerás + o)
      if (root1.length()==root2.length() && infinitive_r_accusative_allomorph_gl(root1) && 
          infinitive_r_accusative_allomorph_gl(root2) && !check_add_gl(root2)) {
        // it changes -s candidate to oxytone one: 'prevelo' = 'prevés' (second person 
        // singular of present indicative) + 'o' || 'prever' (infinitive) + 'o' && 
        // '*comelo' (second person singular of present indicative) is not 'comes' 
        // {'comés' is not into de dictionary} + 'o' ['comelo' = 'comer' + 'o']) 
        s.insert(r2, put_accent_gl(root2)); 
      }
      else if (root1.length()==root2.length() && present_s_accusative_allomorph_gl(root1) && 
               present_s_accusative_allomorph_gl(root2))  {
        // it removes -r candidate (infinitive or future subjunctive: 
        // '*cómelo' is not 'comer' + 'o' ['cómelo' = 'comes' + 'o'])          
        s.erase(r1);
      }

      r1--; r2--;
    }
  }
} // namespace
