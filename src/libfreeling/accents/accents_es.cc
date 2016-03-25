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
  //           Spanish-specific accentuation                 //
  //---------------------------------------------------------//

  /// Init regexps
  const freeling::regexp accents_es::llana_acc(L"(.*)(([áéó][aeo])|([áéíóú]([iu])?[^aeiouáéíóú]+([aeiou]|[iu][aeiou]|[aeiou][iu])))([^aeiouáéíóú]*)$");
  const freeling::regexp accents_es::aguda_mal(L"(.*)([áéíóú])(([^nsaeiouáéíóú][^aeiouáéíóú]*)|([ns][^aeiouáéíóú]+))$");
  const freeling::regexp accents_es::monosil(L"([^aeiouáéíóú]+([aeiouáéíóú]|[iu][aeiou]|[aeiou][iu]))([^aeiouáéíóú]*)$");
  const freeling::regexp accents_es::last_vowel_put_acc(L"(.*)([aeiou])([ns]?)$");
  const freeling::regexp accents_es::last_vowel_not_acc(L"(.*)([aeiou])([^aeiou]*)$");
  const freeling::regexp accents_es::any_vowel_acc(L"(.*)([áéíóú])(.*)");
  const freeling::regexp accents_es::diacritic(L"(de|se)");

  /// maps to store equivalences between accentued and not accentued chars.
  const map<wstring,wstring> accents_es::with_acc = {{L"a",L"á"},{L"e",L"é"},{L"i",L"í"},{L"o",L"ó"},{L"u",L"ú"}};
  const map<wstring,wstring> accents_es::without_acc = {{L"á",L"a"},{L"é",L"e"},{L"í",L"i"},{L"ó",L"o"},{L"ú",L"u"}};


  //////  Spanish accent handling regexps ////////
  // check if a word has accents
#define is_accentued_esp(s) any_vowel_acc.search(s)
  // check if an accent in the last sylabe shouldn't be there
#define aguda_mal_acentuada(s)  aguda_mal.match(s)
  // check if there is an accent in the prior-to-last sylabe
#define llana_acentuada(s) llana_acc.match(s)
  // check if it is a single-sillabe word
#define is_monosylabic(s) monosil.match(s)
  // check if it is a possible diacritic
#define is_diacritic_esp(s) diacritic.match(s)

  ///////////////////////////////////////////////////////////////
  /// Create an accent handler for Spanish.
  ///////////////////////////////////////////////////////////////

  accents_es::accents_es(): accents_module() {
    TRACE(3,L"created Spanish accent handler ");
  }

  ///////////////////////////////////////////////////////////////
  /// specific Spanish behaviour: modify given roots according
  /// to Spanish accentuation requirements. Roots are obtanined
  /// after suffix removal and may require accent fixing, which
  /// is done here.
  ///////////////////////////////////////////////////////////////

  void accents_es::fix_accentuation(set<wstring> &candidates, const sufrule &suf) const
  {
    set<wstring> roots;
    set<wstring>::iterator r;

    TRACE(3,L"Number of candidate roots: "+util::int2wstring(candidates.size()));
    roots.clear();
    for (r=candidates.begin(); r!=candidates.end(); r++) {

      wstring s=(*r);

      TRACE(3,L"We store always the root in roots without any changes ");
      roots.insert(s);

      if (suf.enc) {
        TRACE(3,L"enclitic suffix. root="+s);
        if (llana_acentuada(s)) {
          TRACE(3,L"llana mal acentuada. Remove accents ");
          s = remove_accent_esp(s);
        }
        else if (aguda_mal_acentuada(s)) {
          TRACE(3,L"aguda mal acentuada. Remove accents ");
          s = remove_accent_esp(s);
        }
        else if (not is_accentued_esp(s) and (not is_monosylabic(s) or is_diacritic_esp(s)) ) {
          TRACE(3,L"No accents, not monosylabic (or diacritic). Add accent to last vowel.");
          s = put_accent_esp(s);
        }
        else if (is_monosylabic(s) and is_accentued_esp(s)) {
          TRACE(3,L"Monosylabic root, enclitic form accentued. Remove accents");
          s = remove_accent_esp(s);
        }
        roots.insert(s); // append to roots this element
      }
      else if (suf.acc) {
        TRACE(3,L"try with an accent in each sylabe ");
        // first remove all accents
        s = remove_accent_esp(s);

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
  /// given a non-accentued word, put an accent on the last vowel.
  ////////////////////////////////////////////////////////////

  wstring accents_es::put_accent_esp(const wstring &s) {

    wstring res;
    vector<wstring> rem;
    if (last_vowel_put_acc.match(s,rem)) {
      res = rem[1] + with_acc.find(rem[2])->second + rem[3];
    }
    else 
      res = s;

    return res;
  }


  ////////////////////////////////////////////////////////////
  /// given an accentued word, remove the accent(s).
  ////////////////////////////////////////////////////////////

  wstring accents_es::remove_accent_esp(const wstring &s)  {

    wstring res=s;
    vector<wstring> rem;
    while (any_vowel_acc.match(res,rem)) {
      res = rem[1] + without_acc.find(rem[2])->second + rem[3];
    }

    return res;
  }


} // namespace
