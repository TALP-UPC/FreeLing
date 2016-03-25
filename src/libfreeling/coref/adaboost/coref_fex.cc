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


//////////////////////////////////////////////////////////////////
//    Class for the feature extractor.
//////////////////////////////////////////////////////////////////


// ---  Feature codes
// group DIST
#define FEATURE_COREFEX_DIST_SENT_0     0
#define FEATURE_COREFEX_DIST_SENT_1     1
#define FEATURE_COREFEX_DIST_SENT_2     2
#define FEATURE_COREFEX_DIST_SENT_3     3
#define FEATURE_COREFEX_DIST_SENT_4     4
#define FEATURE_COREFEX_DIST_SENT5MORE  5

#define FEATURE_COREFEX_NUMDEDIST0      10
#define FEATURE_COREFEX_NUMDEDIST1      11
#define FEATURE_COREFEX_NUMDEDIST2      12
#define FEATURE_COREFEX_NUMDEDIST3      13
#define FEATURE_COREFEX_NUMDEDIST4      14
#define FEATURE_COREFEX_NUMDEDIST5      15
#define FEATURE_COREFEX_NUMDEDIST6MORE  16

#define FEATURE_COREFEX_DIST0           20
#define FEATURE_COREFEX_DIST1           21
#define FEATURE_COREFEX_DIST2           22
#define FEATURE_COREFEX_DIST3           23
#define FEATURE_COREFEX_DIST4           24
#define FEATURE_COREFEX_DIST5           25
#define FEATURE_COREFEX_DIST6MORE       26

// group IPRON
#define FEATURE_COREFEX_IPRON           30
#define FEATURE_COREFEX_IPRONP          31
#define FEATURE_COREFEX_IPROND          32
#define FEATURE_COREFEX_IPRONX          33
#define FEATURE_COREFEX_IPRONI          34
#define FEATURE_COREFEX_IPRONT          35
#define FEATURE_COREFEX_IPRONR          36
#define FEATURE_COREFEX_IPRONE          37
// group JPRON
#define FEATURE_COREFEX_JPRON           40
#define FEATURE_COREFEX_JPRONP          41
#define FEATURE_COREFEX_JPROND          42
#define FEATURE_COREFEX_JPRONX          43
#define FEATURE_COREFEX_JPRONI          44
#define FEATURE_COREFEX_JPRONT          45
#define FEATURE_COREFEX_JPRONR          46
#define FEATURE_COREFEX_JPRONE          47
// group STRMATCH
#define FEATURE_COREFEX_STRMATH         50
// group DEFNP
#define FEATURE_COREFEX_DEFNP           51
// group DEMNP
#define FEATURE_COREFEX_DEMNP           52
// group NUMBER
#define FEATURE_COREFEX_NUMBER          53
#define FEATURE_COREFEX_UNK_NUMBER      54
// group GENDER
#define FEATURE_COREFEX_GENDER          55
#define FEATURE_COREFEX_UNK_GENDER      56
// group SEMCLASS
#define FEATURE_COREFEX_SEMCLASS        57
#define FEATURE_COREFEX_UNK_SEMCLASS    58
// group PROPNAME
#define FEATURE_COREFEX_IPROPERNAME     59
#define FEATURE_COREFEX_JPROPERNAME     60
// group ALIAS
#define FEATURE_COREFEX_ACRONIM         61
#define FEATURE_COREFEX_PREFIX          62
#define FEATURE_COREFEX_SUFIX           63
#define FEATURE_COREFEX_ORDER           64
// group APPOS
#define FEATURE_COREFEX_APPOS           70
// group QUOTES
#define FEATURE_COREFEX_IQUOTE          80
#define FEATURE_COREFEX_JQUOTE          81
#define FEATURE_COREFEX_IPARENTHESIS    90
#define FEATURE_COREFEX_JPARENTHESIS    91
// group THIRDP
#define FEATURE_COREFEX_ITHIRD          100
#define FEATURE_COREFEX_JTHIRD          101

#include <string>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"

#include "freeling/morfo/coref_fex.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"COREF_FEX"
#define MOD_TRACECODE COREF_TRACE



  //////////////////////////////////////////////////////////////////
  /// Constructor. Sets defaults
  //////////////////////////////////////////////////////////////////

  coref_fex::coref_fex(unsigned int v, const wstring &sdbf) {

    active_features = v;
    if (not sdbf.empty() ) {
      semdb= new semanticDB(sdbf);
      TRACE(3,L"Coreference solver loaded SemDB");
    }
  }

  //////////////////////////////////////////////////////////////////
  /// Destructor
  //////////////////////////////////////////////////////////////////

  coref_fex::~coref_fex() {
    delete semdb;
  }


  // ======== Auxliary functions for feature extraction ===========

  //////////////////////////////////////////////////////////////////
  /// get word heading a (sub)tree
  //////////////////////////////////////////////////////////////////

  const word& coref_fex::get_head_word(parse_tree::iterator p) {

    if (p->num_children()==0) {
      return p->info.get_word();
    }
    else {
      bool hd=false;
      parse_tree::sibling_iterator k;

      for (k=p->sibling_begin(); k!=p->sibling_end() and not hd; ++k) {
        hd=k->info.is_head();
        if (hd) return get_head_word(k);
      }
    }

    // if tree is ok, we will never reach here.
    ERROR_CRASH(L"Subtree without head.");
  }

  //////////////////////////////////////////////////////////////////
  ///  Extract number digit from a EAGLES (spanish) PoS tag
  //////////////////////////////////////////////////////////////////

  wchar_t coref_fex::extract_number(const wstring &tag) {
    if (tag[0]==L'N' and tag[1]==L'C') return tag[3];
    else if (tag[0]==L'A' or tag[0]==L'D' or tag[0]==L'P') return tag[4];
    else return L'0';
  }

  //////////////////////////////////////////////////////////////////
  ///  Extract gender digit from a EAGLES (spanish) PoS tag
  //////////////////////////////////////////////////////////////////

  wchar_t coref_fex::extract_gender(const wstring &tag) {
    if (tag[0]==L'N' and tag[1]==L'C') return tag[2];
    else if (tag[0]==L'A' or tag[0]==L'D' or tag[0]==L'P') return tag[3];
    else return L'0';
  }

  //////////////////////////////////////////////////////////////////
  ///  Extract semantic class digit from proper nouns, or 
  ///  search WN top-ontology for common nouns
  //////////////////////////////////////////////////////////////////

  wstring coref_fex::extract_semclass(const wstring &tag, const wstring &sense) {

    TRACE(3,L"  extract_semclass("+tag+L","+sense+L")");
    wstring sclass=L"V0";  

    if (tag.substr(0,2)==L"NP") 
      sclass = tag.substr(4,2);

    else if (tag.substr(0,2)==L"NC" and not sense.empty()) {
      sense_info si = semdb->get_sense_info(sense);
      bool human=false;
      bool group=false;
      bool place=false;
    
      for (list<wstring>::iterator it=si.tonto.begin(); it!=si.tonto.end(); it++) {
        if ((*it)==L"Human") human = true;
        else if ((*it)==L"Group") group = true;
        else if ((*it)==L"Place") place = true;
      }
    
      if (human and not group) sclass = L"SP";
      else if (group) sclass = L"O0";
      else if (place) sclass = L"G0";
    }

    return sclass;
  }

  //////////////////////////////////////////////////////////////////
  ///  returns whether the two words 'match' (i.e. whether *all*
  /// characters in w2 are found in w1 *in the same order*)
  //////////////////////////////////////////////////////////////////

  bool coref_fex::check_word(const wstring &w1, const wstring &w2) {

    wstring::const_iterator i1 = w1.begin();
    wstring::const_iterator i2 = w2.begin();
    while (i1!=w1.end() and i2!=w2.end()) {
      if ((*i1)==(*i2)) i2++;
      i1++;
    }
  
    return i2==w2.end();
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is an acronim of 'i'
  //////////////////////////////////////////////////////////////////

  bool coref_fex::check_acronim(const mention_ab &m1, const mention_ab &m2) {

    wstring tag1 = get_head_word(m1.ptree).get_tag();
    wstring tag2 = get_head_word(m2.ptree).get_tag();

    wstring text2;
    if (tag2.substr(0,2)!=L"NP") 
      return false;
    else 
      text2=get_head_word(m2.ptree).get_lemma();

    wstring text1;
    if (tag1.substr(0,2)==L"NP") 
      text1=get_head_word(m1.ptree).get_lemma();
    else 
      text1=util::vector2wstring(m1.tokens,L" ");

    // build acronym with all of txt1
    vector<wstring> s1=util::wstring_to<vector<wstring> >(text1,L" _",false);
    wstring acr;
    for (vector<wstring>::iterator k=s1.begin(); k!=s1.end(); k++)
      acr += k->substr(0,1);

    // check if text2 (acronym to be checked) is found inside the built acronym
    return (acr.find(text2)!=wstring::npos);
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a prefix of 'i'
  //////////////////////////////////////////////////////////////////

  bool coref_fex::check_prefix(const mention_ab &m1, const mention_ab &m2) {
    vector<wstring>::const_iterator itT1, itT2;
    int total1 = m1.tokens.size();
    int total2 = m2.tokens.size();
    int maxsz = max(total1,total2);
    int count;
    bool ret = false;
  
    if (total1>=1 and total2>=1 and maxsz>1) {
      itT1 = m1.tokens.begin();
      itT2 = m2.tokens.begin();
      count = 0;
      while (itT1!=m1.tokens.end() and itT2!=m2.tokens.end()) {
        if (check_word(*itT1, *itT2)) count++;
        else break;

        ++itT1;
        ++itT2;
      }
      if (count > maxsz/2) ret = true;
    }
  
    return ret;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a suffix of 'i'
  //////////////////////////////////////////////////////////////////

  bool coref_fex::check_sufix(const mention_ab &m1, const mention_ab &m2) {
    vector<wstring>::const_reverse_iterator ritT1, ritT2;
    int total1 = m1.tokens.size();
    int total2 = m2.tokens.size();
    int maxsz = max(total1,total2);
    int count;
    bool ret = false;
  
    if (total1 >= 1 and total2 >= 1 and maxsz > 1) {
      ritT1 = m1.tokens.rbegin();
      ritT2 = m2.tokens.rbegin();
      count = 0;
      while(ritT1!=m1.tokens.rend() and ritT2!=m2.tokens.rend()) {
        if (check_word(*ritT1, *ritT2)) count++;
        else break;

        ++ritT1;
        ++ritT2;
      }
      if (count > maxsz/2) ret = true;
    }
  
    return ret;
  }

  //////////////////////////////////////////////////////////////////
  /// Returns whether the given word in a mention has the given tag
  //////////////////////////////////////////////////////////////////

  bool coref_fex::check_tag(const mention_ab &m, int w, const wstring &tag) {
    return m.tags[w].substr(0,tag.size())==tag;  
  }

  //////////////////////////////////////////////////////////////////
  /// Returns whether all words in 'j' appear (in the same order) in 'i'
  //////////////////////////////////////////////////////////////////

  bool coref_fex::check_order(const mention_ab &m1, const mention_ab &m2) {
    vector<wstring>::const_iterator itT1, itT2;
  
    itT1 = m1.tokens.begin();
    itT2 = m2.tokens.begin();
    while (itT1!=m1.tokens.end() and itT2!=m2.tokens.end()) {
      if (check_word(*itT1, *itT2)) ++itT2;
      ++itT1;
    }
  
    return (itT2==m2.tokens.end() and m2.tokens.size() > 1);
  }


  // ================  Feature extraction functions ==============

  //////////////////////////////////////////////////////////////////
  ///    Returns the distance in sentences of the example.
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_dist(const mention_ab &m1, const mention_ab &m2) {

    TRACE(4,L"get_dist_sent");
    int dist = m2.sent - m1.sent;
    switch (dist) {
    case 0: return FEATURE_COREFEX_DIST_SENT_0; break;
    case 1: return FEATURE_COREFEX_DIST_SENT_1; break;
    case 2: return FEATURE_COREFEX_DIST_SENT_2; break;
    case 3: return FEATURE_COREFEX_DIST_SENT_3; break;
    case 4: return FEATURE_COREFEX_DIST_SENT_4; break;
    default: return FEATURE_COREFEX_DIST_SENT5MORE; break;
    }
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns the distance in mentions of the example.
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_numdedist(const mention_ab &m1, const mention_ab &m2) {

    TRACE(4,L"get_dist_ment");
    int dist = m2.numde - m1.numde;  
    switch (dist) {
    case 0: return FEATURE_COREFEX_NUMDEDIST0; break;
    case 1: return FEATURE_COREFEX_NUMDEDIST1; break;
    case 2: return FEATURE_COREFEX_NUMDEDIST2; break;
    case 3: return FEATURE_COREFEX_NUMDEDIST3; break;
    case 4: return FEATURE_COREFEX_NUMDEDIST4; break;
    case 5: return FEATURE_COREFEX_NUMDEDIST5; break;
    default: return FEATURE_COREFEX_NUMDEDIST6MORE; break;
    }
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns the distance in words of the example.
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_dedist(const mention_ab &m1, const mention_ab &m2) {

    TRACE(4,L"get_dedist: m1.end="+util::int2wstring(m1.posend)+L" m2.beg="+util::int2wstring(m2.posbegin));
    int res = m2.posbegin - m1.posend;
    switch(res) {
    case 0: return FEATURE_COREFEX_DIST0; break;
    case 1: return FEATURE_COREFEX_DIST1; break;
    case 2: return FEATURE_COREFEX_DIST2; break;
    case 3: return FEATURE_COREFEX_DIST3; break;
    case 4: return FEATURE_COREFEX_DIST4; break;
    case 5: return FEATURE_COREFEX_DIST5; break;
    default: return FEATURE_COREFEX_DIST6MORE; break;
    }
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is a pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_pronoun");
    if (m1.tags.size()==1 and check_tag(m1,0,L"P"))
      return FEATURE_COREFEX_IPRON;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun");
    if (m2.tags.size()==1 and check_tag(m2,0,L"P"))
      return FEATURE_COREFEX_JPRON;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is a personal pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun_p(const mention_ab &m1, const mention_ab &m2) { 
    TRACE(4,L"get_i_pronoun_p");
    if (m1.tags.size()==1 and check_tag(m1,0,L"PP"))
      return FEATURE_COREFEX_IPRONP;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a personal pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun_p(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun_p");
    if (m2.tags.size()==1 and check_tag(m2,0,L"PP"))
      return FEATURE_COREFEX_JPRONP;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is a demonstrative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun_d(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_pronoun_d");
    if (m1.tags.size()==1 and check_tag(m1,0,L"PD"))
      return FEATURE_COREFEX_IPROND;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a demostrative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun_d(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun_d");
    if (m2.tags.size()==1 and check_tag(m2,0,L"PD"))
      return FEATURE_COREFEX_JPROND;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is a possessive pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun_x(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_pronoun_x");
    if (m1.tags.size()==1 and check_tag(m1,0,L"PX"))
      return FEATURE_COREFEX_IPRONX;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a possessive pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun_x(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun_x");
    if (m2.tags.size()==1 and check_tag(m2,0,L"PX"))
      return FEATURE_COREFEX_JPRONX;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is an indefinite pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun_i(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_pronoun_i");
    if (m1.tags.size()==1 and check_tag(m1,0,L"PI"))
      return FEATURE_COREFEX_IPRONI;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is an indefinite pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun_i(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun_i");
    if (m2.tags.size()==1 and check_tag(m2,0,L"PI"))
      return FEATURE_COREFEX_JPRONI;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is an interrogative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun_t(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_pronoun_t");
    if (m1.tags.size()==1 and check_tag(m1,0,L"PT"))
      return FEATURE_COREFEX_IPRONT;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is an interrogative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun_t(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun_t");
    if (m2.tags.size()==1 and check_tag(m2,0,L"PT"))
      return FEATURE_COREFEX_JPRONT;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is a relative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun_r(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_pronoun_r");
    if (m1.tags.size()==1 and check_tag(m1,0,L"PR"))
      return FEATURE_COREFEX_IPRONR;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a relative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun_r(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun_r");
    if (m2.tags.size()==1 and check_tag(m2,0,L"PR"))
      return FEATURE_COREFEX_JPRONR;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is an exclamative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_pronoun_e(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_pronoun_e");
    if (m1.tags.size()==1 and check_tag(m1,0,L"PE"))
      return FEATURE_COREFEX_IPRONE;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is an exclamative pronoun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_pronoun_e(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_pronoun_e");
    if (m2.tags.size()==1 and check_tag(m2,0,L"PE"))
      return FEATURE_COREFEX_JPRONE;
    else
      return 0;
  }



  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' matches the string of 'j'
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_str_match(const mention_ab &m1, const mention_ab &m2) {
    wstring str1, str2;
  
    TRACE(4,L"get_str_match");

    for (int i=0; i<(int)m1.tags.size(); i++) {
      if (check_tag(m1,i,L"V") or check_tag(m1,i,L"N") 
          or check_tag(m1,i,L"A") or (check_tag(m1,i,L"P") and not check_tag(m1,i,L"PD")) )
        str1 += L" "+m1.tokens[i];
    }
  
    for (int i=0; i<(int)m2.tags.size(); i++) {
      if (check_tag(m2,i,L"V") or check_tag(m2,i,L"N") 
          or check_tag(m2,i,L"A") or (check_tag(m2,i,L"P") and not check_tag(m2,i,L"PD")) )
        str2 += L" "+m2.tokens[i];
    }
    
    if (str1==str2 and str1.size() > 1)
      return FEATURE_COREFEX_STRMATH;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a definite noun phrase
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_def_np(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_def_np");
    // check whether the NP is headed by a common noun (NC)
    // that has a definite determiner (DA) as a left modifier.
    wstring tag;
    bool hd=false, da=false, nc=false;
    parse_tree::sibling_iterator k;
    for (k=m2.ptree->sibling_begin(); k!=m2.ptree->sibling_end() and not hd; ++k) {
      if (k->info.is_head()) {
        hd=true;
        tag = get_head_word(k).get_tag();
        if (tag.substr(0,2)==L"NC") nc=true;
      }
      else {
        tag= get_head_word(k).get_tag();
        if (tag.substr(0,2)==L"DA") da=true;
      }
    }

    if (da and nc) 
      return FEATURE_COREFEX_DEFNP;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a demonstrative noun phrase
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_dem_np(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_dem_np");
    // check whether the NP is headed by a common noun (NC)
    // that has a demonstrative determiner (DD) as a left modifier.
    wstring tag;
    bool hd=false, dd=false, nc=false;
    parse_tree::sibling_iterator k;
    for (k=m2.ptree->sibling_begin(); k!=m2.ptree->sibling_end() and not hd; ++k) {
      if (k->info.is_head()) {
        hd=true;
        tag = get_head_word(k).get_tag();
        if (tag.substr(0,2)==L"NC") nc=true;
      }
      else {
        tag= get_head_word(k).get_tag();
        if (tag.substr(0,2)==L"DD") dd=true;
      }
    }

    if (dd and nc) 
      return FEATURE_COREFEX_DEMNP;
    else 
      return 0;

  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' and 'j' agree in number
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_number(const mention_ab &m1, const mention_ab &m2) {

    TRACE(4,L"get_number");

    wstring tag1 = get_head_word(m1.ptree).get_tag();
    wstring tag2 = get_head_word(m2.ptree).get_tag();

    wchar_t num1 = extract_number(tag1);
    wchar_t num2 = extract_number(tag2);

    if (num1==L'0' or num2==L'0')
      return FEATURE_COREFEX_UNK_NUMBER;
    else if (num1==num2 or num1==L'N' or num2==L'N') 
      return FEATURE_COREFEX_NUMBER;
    else
      return 0;

  }


  //////////////////////////////////////////////////////////////////
  ///    Returns if 'i' and 'j' agree in gender
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_gender(const mention_ab &m1, const mention_ab &m2) {

    TRACE(4,L"get_gender");

    wstring tag1 = get_head_word(m1.ptree).get_tag();
    wstring tag2 = get_head_word(m2.ptree).get_tag();

    wchar_t gen1 = extract_gender(tag1);
    wchar_t gen2 = extract_gender(tag2);
  
    if (gen1==L'0' or gen2==L'0')
      return FEATURE_COREFEX_UNK_GENDER;
    else if (gen1==gen2 or gen1==L'C' or gen2==L'C') 
      return FEATURE_COREFEX_GENDER;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns if 'i' and 'j' are of the same semantic class
  ///    Uses the NEC if the word is a proper noun or wordnet if the
  ///    is a common noun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_semclass(const mention_ab &m1, const mention_ab &m2) {

    TRACE(4,L"get_semclass");

    const word &w1 = get_head_word(m1.ptree);
    const word &w2 = get_head_word(m2.ptree);
    wstring tag1 = w1.get_tag();
    wstring tag2 = w2.get_tag();

    wstring sen1=L"",sen2=L"";
    const list<pair<wstring,double> > & ls1 = w1.get_senses();
    if (not ls1.empty()) sen1 = ls1.begin()->first;
    const list<pair<wstring,double> > & ls2 = w2.get_senses();
    if (not ls2.empty()) sen2 = ls2.begin()->first;

    //Get class given by NEC if the head is a proper noun, or use wordnet for common nouns
    wstring sclass1 = extract_semclass(tag1,sen1);
    TRACE(4,L"   sc1="+sclass1);
    wstring sclass2 = extract_semclass(tag2,sen2);
    TRACE(4,L"   sc2="+sclass2);

    if (sclass1==L"V0" or sclass2==L"V0")
      return FEATURE_COREFEX_UNK_SEMCLASS;
    else if (sclass1==sclass2 or (sclass1==L"SP" and tag2.substr(0,2)==L"PP"))
      return FEATURE_COREFEX_SEMCLASS;
    else 
      return 0;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is proper noun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_proper_noun_i(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_proper_noun_i");
    wstring tag = get_head_word(m1.ptree).get_tag();
    if (tag.substr(0,2)==L"NP")
      return FEATURE_COREFEX_IPROPERNAME;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is proper noun
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_proper_noun_j(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_proper_noun_j");
    wstring tag = get_head_word(m2.ptree).get_tag();
    if (tag.substr(0,2)==L"NP")
      return FEATURE_COREFEX_JPROPERNAME;
    else
      return 0;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is an acronim of 'i'
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_alias_acro(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_alias_acro");
    if (check_acronim(m1,m2))
      return FEATURE_COREFEX_ACRONIM;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a prefix of 'i'
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_alias_prefix(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_alias_prefix");
    if (check_prefix(m1,m2))
      return FEATURE_COREFEX_PREFIX;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is a suffix of 'i'
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_alias_sufix(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_alias_sufix");
    if (check_sufix(m1,m2))
      return FEATURE_COREFEX_SUFIX;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether the words of 'j' appear in the same order in 'i'
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_alias_order(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_alias_order");
    if (check_order(m1,m2))
      return FEATURE_COREFEX_ORDER;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is an apposition of 'i'
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_appositive(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_appositive");
    // check for two consecutive mentions (e.g. "Intelligent Bussines Machines (IBM) closed")
    // or for almost-consecutive (e.g. "the chinese presindent, Tsi Lau Fo, arrived").
    //  In the later case we should check for commas surrounding m2, but we have no 
    //  access to the sentence here!!!  FIX IT
    if ( m2.posbegin==m1.posend+2 or m2.posbegin==m1.posend+1 )
      return FEATURE_COREFEX_APPOS;
    else 
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is in quotes
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_inquotes(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_inquotes");
    int ret = 0;  
    if (m1.tags.size() > 2) {
      if (m1.tags[0]==L"Fe" and m1.tags[m1.tags.size()-1]==L"Fe")
        ret = FEATURE_COREFEX_IQUOTE;
      else if (m1.tags[0]==L"Fra" and m1.tags[m1.tags.size()-1]==L"Frc") 
        ret = FEATURE_COREFEX_IQUOTE;
    }
    return ret;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is in quotes
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_inquotes(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_inquotes");
    int ret = 0;  
    if (m2.tags.size() > 2) {
      if (m2.tags[0]==L"Fe" and m2.tags[m2.tags.size()-1]==L"Fe")
        ret = FEATURE_COREFEX_JQUOTE;
      else if (m2.tags[0]==L"Fra" and m2.tags[m2.tags.size()-1]==L"Frc")
        ret = FEATURE_COREFEX_JQUOTE;
    }
    return ret;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is in parenthesis
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_inparenthesis(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_inparenthesis");
    int ret = 0;  
    if (m1.tags.size() > 2) {
      if (m1.tags[0]==L"Fpa" and m1.tags[m1.tags.size()-1]==L"Fpt")
        ret = FEATURE_COREFEX_IPARENTHESIS;
    }
    return ret;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is in parenthesis
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_inparenthesis(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_inparenthesis");
    int ret = 0;  
    if (m2.tags.size() > 2) {
      if (m2.tags[0]==L"Fpa" and m2.tags[m2.tags.size()-1]==L"Fpt")
        ret = FEATURE_COREFEX_JPARENTHESIS;
    }
    return ret;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'i' is 3rd person
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_i_thirdperson(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_i_thirdperson");
    wstring tag = get_head_word(m1.ptree).get_tag();
    if (tag[0]==L'P' and tag[2]==L'3')
      return FEATURE_COREFEX_ITHIRD;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether 'j' is 3rd person
  //////////////////////////////////////////////////////////////////

  int coref_fex::get_j_thirdperson(const mention_ab &m1, const mention_ab &m2) {
    TRACE(4,L"get_j_thirdperson");
    wstring tag = get_head_word(m2.ptree).get_tag();
    if (tag[0]==L'P' and tag[2]==L'3')
      return FEATURE_COREFEX_JTHIRD;
    else
      return 0;
  }

  //////////////////////////////////////////////////////////////////
  ///  Add a relevant feature to the vector
  //////////////////////////////////////////////////////////////////

  void coref_fex::put_feature(int f, vector<int> &result) {
    if (f>0) result.push_back(f);
  }

  //////////////////////////////////////////////////////////////////
  ///    Extract the features configured to be extracted
  //////////////////////////////////////////////////////////////////

  void coref_fex::extract(const mention_ab &m1, const mention_ab &m2, vector<int> &result) {

    result.clear();
  
    if (active_features & COREFEX_DIST) {
      result.push_back(get_dist(m1,m2));
      result.push_back(get_numdedist(m1,m2));
      result.push_back(get_dedist(m1,m2));
    }
  
    if (active_features & COREFEX_IPRON)
      put_feature(get_i_pronoun(m1,m2), result);
    if (active_features & COREFEX_JPRON)
      put_feature(get_j_pronoun(m1,m2), result);
  
    if (active_features & COREFEX_IPRONM) {
      put_feature(get_i_pronoun_p(m1,m2), result);
      put_feature(get_i_pronoun_d(m1,m2), result);
      put_feature(get_i_pronoun_x(m1,m2), result);
      put_feature(get_i_pronoun_i(m1,m2), result);
      put_feature(get_i_pronoun_t(m1,m2), result);
      put_feature(get_i_pronoun_r(m1,m2), result);
      put_feature(get_i_pronoun_e(m1,m2), result);
    }
  
    if (active_features & COREFEX_JPRONM) {
      put_feature(get_j_pronoun_p(m1,m2), result);
      put_feature(get_j_pronoun_d(m1,m2), result);
      put_feature(get_j_pronoun_x(m1,m2), result);
      put_feature(get_j_pronoun_i(m1,m2), result);
      put_feature(get_j_pronoun_t(m1,m2), result);
      put_feature(get_j_pronoun_r(m1,m2), result);
      put_feature(get_j_pronoun_e(m1,m2), result);
    }
  
    if (active_features & COREFEX_STRMATCH)
      put_feature(get_str_match(m1,m2), result);
    if (active_features & COREFEX_DEFNP)
      put_feature(get_def_np(m1,m2), result);
    if (active_features & COREFEX_DEMNP)
      put_feature(get_dem_np(m1,m2), result);
    if (active_features & COREFEX_NUMBER)
      put_feature(get_number(m1,m2), result);
    if (active_features & COREFEX_GENDER)
      put_feature(get_gender(m1,m2), result);
    if (active_features & COREFEX_SEMCLASS)
      put_feature(get_semclass(m1,m2), result);
    if (active_features & COREFEX_PROPNAME) {
      put_feature(get_proper_noun_i(m1,m2), result);
      put_feature(get_proper_noun_j(m1,m2), result);
    }
  
    if (active_features & COREFEX_ALIAS) {
      put_feature(get_alias_acro(m1,m2), result);
      put_feature(get_alias_prefix(m1,m2), result);
      put_feature(get_alias_sufix(m1,m2), result);
      put_feature(get_alias_order(m1,m2), result);
    }
  
    if (active_features & COREFEX_APPOS)
      put_feature(get_appositive(m1,m2), result);
  
    if (active_features & COREFEX_QUOTES) {
      put_feature(get_i_inquotes(m1,m2), result);
      put_feature(get_j_inquotes(m1,m2), result);
      put_feature(get_i_inparenthesis(m1,m2), result);
      put_feature(get_j_inparenthesis(m1,m2), result);
    }

    if (active_features & COREFEX_THIRDP) {
      put_feature(get_i_thirdperson(m1,m2), result);
      put_feature(get_j_thirdperson(m1,m2), result);
    }
  }

} // namespace
