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


///////////////////////////////////////////////
//   Author: Lluis Padro
/////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//    Class for the feature extractor.
//////////////////////////////////////////////////////////////////

#include <string>
#include <stdlib.h> // abs
#include <ctime>

#include "freeling/morfo/traces.h"
#include "freeling/morfo/util.h"
#include "freeling/morfo/configfile.h"
#include "freeling/morfo/relaxcor_fex_dep.h"

using namespace std;

namespace freeling {

#define MOD_TRACENAME L"RELAXCOR_FEX"
#define MOD_TRACECODE COREF_TRACE


  //////////////////////////////////////////////////////////////////
  /// Constructor. Sets defaults
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::relaxcor_fex_dep(const wstring &filename, const relaxcor_model &m) : relaxcor_fex_abs(m) {

    // read configuration file and store information       
    enum sections {DEPLABELS, POSTAGS, WORDFEAT};
    config_file cfg(false,L"%");

    // add compulsory sections
    cfg.add_section(L"DepLabels",DEPLABELS,true);
    cfg.add_section(L"PosTags",POSTAGS,true);
    cfg.add_section(L"WordFeatures",WORDFEAT,true);
    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {
      case DEPLABELS: {
        // Read regexs to identify labels for dependency functions
	wstring name, val;
        sin >> name >> val;
	freeling::regexp re(val);
	_Labels.insert(make_pair(L"FUN_"+name,re));
        break;
      }
      case POSTAGS: {
        // Read regexs to identify labels for PoS tags
	wstring name, val;
        sin >> name >> val;
	freeling::regexp re(val);
	_Labels.insert(make_pair(L"TAG_"+name,re));
	break;
      }
      case WORDFEAT: {
        // Read regexs to identify labels for PoS tags
	wstring name, val;
        sin >> name >> val;
	freeling::regexp re(val);
	_Labels.insert(make_pair(L"WF_"+name,re));
	break;
      }
      default: break;
      }
    }
    // close config file
    cfg.close();

    TRACE(2,L"Module successfully loaded");
  }

  //////////////////////////////////////////////////////////////////
  /// Destructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::~relaxcor_fex_dep() { }


  //////////////////////////////////////////////////////////////////
  /// Auxiliary to handle regex map
  //////////////////////////////////////////////////////////////////
  
  std::map<std::wstring,freeling::regexp>::const_iterator relaxcor_fex_dep::get_label(const wstring &key) const {
    auto re = _Labels.find(key);
    if (re==_Labels.end()) {
      WARNING(L"No regex defined for label "<<key<<L". Ignoring affected features");
    }
    return re;
  }

  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention is inside quotes
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::in_quotes(const mention &m, feature_cache &fcache) const {

    wstring fid = util::int2wstring(m.get_id())+L":IN_QUOTES";
    bool inq;
    if (fcache.computed_feature(fid)) {
      inq = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"      " << fid << L" = " << wstring(inq?L"yes":L"no") << "  (cached)");
    }
    else {
      paragraph::const_iterator s = m.get_sentence();
      sentence::const_reverse_iterator b(m.get_it_begin());
      ++b;
      int nq = 0;
      while (b!=s->rend()) {
        if (b->get_tag()==L"Fe" or b->get_tag()==L"Fra" or b->get_tag()==L"Frc") ++nq;
        ++b;
      }

      // if there is an odd number of quotes to the left of the mention first word, 
      // we are inside quotes.
      inq = (nq%2!=0);
      fcache.set_feature(fid,util::int2wstring(inq));
      TRACE(7,L"      " << fid << L" = "<< (inq?L"yes":L"no"));
    }

    return inq;
  }

  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention first word is indefinite
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::start_with_indef(const mention &m, feature_cache &fcache) const {

    wstring fid = util::int2wstring(m.get_id())+L":START_INDEF";
    bool ind;
    if (fcache.computed_feature(fid)) {
      ind = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"      " << fid << L" = " << wstring(ind?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the word is in the list of indefinite pronouns
      auto re = get_label(L"WD_Indefinite");  if (re==_Labels.end()) return false;
      ind = re->second.search(m.get_it_begin()->get_lemma());

      fcache.set_feature(fid,util::int2wstring(ind));
      TRACE(7,L"      " << fid << L" = "<< (ind?L"yes":L"no"));
    }

    return ind;
  }


  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention head is indefinite
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::indef_head(const mention &m, feature_cache &fcache) const {

    wstring fid = util::int2wstring(m.get_id())+L":INDEF";
    bool ind;
    if (fcache.computed_feature(fid)) {
      ind = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"      " << fid << L" = " << wstring(ind?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the word is in the list of indefinite pronouns
      auto re = get_label(L"WD_Indefinite");  if (re==_Labels.end()) return false;
      ind = re->second.search(m.get_head().get_lemma());

      fcache.set_feature(fid,util::int2wstring(ind));
      TRACE(7,L"      " << fid << L" = "<< (ind?L"yes":L"no"));
    }

    return ind;
  }

  /////////////////////////////////////////////////////////////////////////////
  ///    Returns a string-encoded list of <prednum, role> for each predicate 
  ///   in which the mention plays some role.
  /////////////////////////////////////////////////////////////////////////////

  wstring relaxcor_fex_dep::get_arguments(const mention &m, feature_cache &fcache) const {
    wstring fid = util::int2wstring(m.get_id())+L":ARGUMENTS";
    wstring args;
    if (fcache.computed_feature(fid)) {
      args =  fcache.get_feature(fid);
      TRACE(7,L"      "<<fid<<L" = ["<<args<<"]   (cached)");
    }
    else {
      list<pair<int,wstring>> roles;
      paragraph::const_iterator s=m.get_sentence();
      for (sentence::predicates::const_iterator p=s->get_predicates().begin(); p!=s->get_predicates().end(); ++p) {
        int mpos = m.get_head().get_position();
        if (p->has_argument(mpos)) {
          wstring role = p->get_argument_by_pos(mpos).get_role();
          roles.push_back(make_pair(p->get_position(),role));
        }
      }
      roles.sort();
      args = util::pairlist2wstring<int,wstring>(roles,L":",L"/");
      fcache.set_feature(fid, args);
      TRACE(7,L"      "<<fid<<L" = ["<<args<<"]");
    }

    return args;
  }

  ////////////////////////////////////////////////////
  ///    Returns whether m1 and m2 are inside the same quotation
  ////////////////////////////////////////////////////
  
  bool relaxcor_fex_dep::same_quote(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                    feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":SAME_QUOTE";
    bool sq;
    if (fcache.computed_feature(fid)) {
      sq = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (sq?L"yes":L"no") << "  (cached)");
    }
    else {
      // check for quotes between m1 and m2. If none is found, they are in the same quotation (if any)
      sq = in_quotes(m1,fcache) and in_quotes(m2,fcache);      
      sentence::const_iterator k = m1.get_it_end();
      ++k;
      while (k!=m2.get_it_begin() and sq) {
        sq = (k->get_tag()==L"Fe" or k->get_tag()==L"Fra" or k->get_tag()==L"Frc");
        ++k;
      }
      
      fcache.set_feature(fid, util::int2wstring(sq));
      TRACE(6,L"   " << fid << L" = " << (sq?L"yes":L"no"));
    }
    return sq;
  }


  ////////////////////////////////////////////////////
  ///    Returns whether m1 is nested in m2
  ////////////////////////////////////////////////////
  
  bool relaxcor_fex_dep::nested(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":NESTED";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      r = m1.get_n_sentence() == m2.get_n_sentence() and
        ((m2.get_pos_begin()<m1.get_pos_begin() and m2.get_pos_end()>=m1.get_pos_end()) or
         (m2.get_pos_begin()<=m1.get_pos_begin() and m2.get_pos_end()>m1.get_pos_end()));
      
      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }
    return r;
  }



  //////////////////////////////////////////////////////////////////
  ///    Returns the distance in number of phrases between m1 and m2
  //////////////////////////////////////////////////////////////////

  unsigned int relaxcor_fex_dep::dist_in_phrases(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                                 feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":DIST_PHRASES";
    unsigned int res;
    if (fcache.computed_feature(fid)) {
      res = util::wstring2int(fcache.get_feature(fid));    
      TRACE(6,L"   " << fid << L" = " << res << L"  (cached)");
    }
    else {
      if (m1.get_n_sentence() != m2.get_n_sentence()) return INFINITE;
      if (nested(m1,m2,mentions,fcache,ft) or nested(m2,m1,mentions,fcache,ft)) return 0;
      
      list<pair<int,wstring> > args1 = util::wstring2pairlist<int,wstring>(get_arguments(m1, fcache),L":",L"/");
      list<pair<int,wstring> > args2 = util::wstring2pairlist<int,wstring>(get_arguments(m2, fcache),L":",L"/");
      if (args1.empty() or args2.empty()) return INFINITE;
      
      unsigned int mindif=INFINITE;
      list<pair<int,wstring>>::const_iterator p1=args1.begin();
      list<pair<int,wstring>>::const_iterator p2=args2.begin();
      
      while (p1!=args1.end() and p2!=args2.end() and mindif>0) {
        
        TRACE(7,L"      m1 is argument " << p1->second << L" of pred " << p1->first);
        TRACE(7,L"      m2 is argument " << p2->second << L" of pred " << p2->first);
        
        unsigned int dif = abs (p1->first - p2->first);
        if (dif < mindif) mindif = dif;
        
        if (p1->first > p2->first) p2++;
        else if (p1->first < p2->first) p1++;
        else {p1++; p2++;}
      }
     
      res = mindif;
      fcache.set_feature(fid, util::int2wstring(res));
      TRACE(6,L"   " << fid << L" = " << res);
    }

    return res;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether m2 is apposition of m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::appositive(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                    feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":APPOSITION";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      auto re = get_label(L"FUN_Apposition");  if (re==_Labels.end()) return false;

      // m2 is child of m1 with label "APPO" or else, m1 and m2 are siblings and consecutive
      r = (m2.get_dtree().get_parent()==m1.get_dtree() and re->second.search(m2.get_dtree()->get_label())) 
        or (m1.get_dtree().get_parent()==m2.get_dtree() and m1.get_pos_end()+1 == m2.get_pos_begin());
      
      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }
    return r;
  } 
  

  //////////////////////////////////////////////////////////////////
  ///    Returns whether m1 and m2 are related by a copulative verb
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::predicate_nominative(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                              feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":PRED_NOMINATIVE";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      auto reSubj = get_label(L"FUN_Subject");  if (reSubj==_Labels.end()) return false;
      auto rePred = get_label(L"FUN_Predicate");  if (rePred==_Labels.end()) return false;
      auto reCop = get_label(L"WD_Copulative");  if (reCop==_Labels.end()) return false;
      auto rePos = get_label(L"WD_Possessive");  if (rePos==_Labels.end()) return false;

      // m1 is SBJ and m2 is PRD of a copulative verb (or viceversa), and they are not possessive
      r = (((reSubj->second.search(m1.get_dtree()->get_label()) and rePred->second.search(m2.get_dtree()->get_label())) or
            (reSubj->second.search(m2.get_dtree()->get_label()) and rePred->second.search(m1.get_dtree()->get_label()))) and
           m1.get_dtree().get_parent() == m2.get_dtree().get_parent() and
           reCop->second.search(m1.get_dtree().get_parent()->get_word().get_lemma()) and
           not rePos->second.search(m1.get_head().get_lemma()) and
           not rePos->second.search(m2.get_head().get_lemma())
           );

      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }
    return r;
  } 
  

  //////////////////////////////////////////////////////////////////
  ///    Returns whether m1 is relative pronoun and m1 its antecedent
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::rel_antecedent(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                        feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":REL_ANTECEDENT";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      auto reSubj = get_label(L"FUN_Subject");  if (reSubj==_Labels.end()) return false;
      auto reObj = get_label(L"FUN_Object");  if (reObj==_Labels.end()) return false;
      auto reNMod = get_label(L"FUN_NounModifier");  if (reNMod==_Labels.end()) return false;
      auto reVerb = get_label(L"TAG_Verb");  if (reVerb==_Labels.end()) return false;
      auto reRelP = get_label(L"TAG_RelPron");  if (reRelP==_Labels.end()) return false;

      // m2 is relative pronoun, is and child (SBJ or OBJ) of a verb that is NMOD of m1
      r = reRelP->second.search(m2.get_head().get_tag()) and
        (reSubj->second.search(m2.get_dtree()->get_label()) or
         reObj->second.search(m2.get_dtree()->get_label())) and 
        not m2.get_dtree().get_parent().is_root() and  
        reVerb->second.search(m2.get_dtree().get_parent()->get_label()) and 
        reNMod->second.search(m2.get_dtree().get_parent()->get_label()) and 
        m2.get_dtree().get_parent().get_parent() == m1.get_dtree();

      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }
    return r;
  } 
  

  //////////////////////////////////////////////////////////////////
  /// Structural features
  //////////////////////////////////////////////////////////////////

  int relaxcor_fex_dep::get_structural(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                       feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    TRACE(6,L"get structural features");

    //counter of extracted features
    int count=0;

    // id of current feature
    unsigned int id;

    // distance in #sentences
   if (def(L"RCF_DIST_SEN_0") or def(L"RCF_DIST_SEN_1") or def(L"RCF_DIST_SEN_L3") or def(L"RCF_DIST_SEN_G3")) {
      unsigned int dist = abs(m1.get_n_sentence() - m2.get_n_sentence());
      if (defid(L"RCF_DIST_SEN_0",id))  { ft[id] = (dist==0); count++; }
      if (defid(L"RCF_DIST_SEN_1",id))  { ft[id] = (dist==1); count++; }
      if (defid(L"RCF_DIST_SEN_L3",id)) { ft[id] = (dist<=3); count++; }
      if (defid(L"RCF_DIST_SEN_G3",id)) { ft[id] = (dist>3); count++; }
    }
    // #mentions between both mentions
    if (def(L"RCF_DIST_MEN_0") or def(L"RCF_DIST_MEN_L3") or def(L"RCF_DIST_MEN_L6") or def(L"RCF_DIST_MEN_G6")) {
      unsigned int dist = abs(m1.get_id() - m2.get_id()) - 1;
      if (defid(L"RCF_DIST_MEN_0",id))   { ft[id] = (dist==0); count++; }
      if (defid(L"RCF_DIST_MEN_L3",id))  { ft[id] = (dist<=3); count++; }
      if (defid(L"RCF_DIST_MEN_L6",id)) { ft[id] = (dist<=6); count++; }
      if (defid(L"RCF_DIST_MEN_G6",id)) { ft[id] = (dist>6); count++; }
    }
    // distance in #phrases
    if (def(L"RCF_DIST_PHR_0") or def(L"RCF_DIST_PHR_1") or def(L"RCF_DIST_PHR_L3")) {
      unsigned int dist = dist_in_phrases(m1,m2,mentions,fcache,ft);
      if (defid(L"RCF_DIST_PHR_0",id))  { ft[id] = (dist==0); count++; }
      if (defid(L"RCF_DIST_PHR_1",id))  { ft[id] = (dist==1); count++; }
      if (defid(L"RCF_DIST_PHR_L3",id)) { ft[id] = (dist<=3); count++; }
    }
    // in quotes?
    if (defid(L"RCF_I_IN_QUOTES",id)) { ft[id] = in_quotes(m1,fcache); count++; }
    if (defid(L"RCF_J_IN_QUOTES",id)) { ft[id] = in_quotes(m2,fcache); count++; }
    if (defid(L"RCF_SAME_QUOTE",id)) { ft[id] = same_quote(m1, m2, mentions, fcache, ft); count++; }

    // first mention in sentence?
    if (defid(L"RCF_I_FIRST",id)) { ft[id] = m1.is_initial(); count++; }
    if (defid(L"RCF_J_FIRST",id)) { ft[id] = m2.is_initial(); count++; }
    // one is appositive of the other
    if (defid(L"RCF_APPOSITIVE",id)) { ft[id] = appositive(m1,m2,mentions,fcache,ft) or appositive(m2,m1,mentions,fcache,ft); count++; }
    // one is nested to the other 
    if (def(L"RCF_NESTED") or def(L"RCF_NESTED_IJ") or def(L"RCF_NESTED_JI")) {
      bool ns1 = nested(m1,m2,mentions,fcache,ft);
      bool ns2 = nested(m2,m1,mentions,fcache,ft);
      if (defid(L"RCF_NESTED_IJ",id)) { ft[id] = ns1; count++; }
      if (defid(L"RCF_NESTED_JI",id)) { ft[id] = ns2; count++; }
      if (defid(L"RCF_NESTED",id)) { ft[id] = ns1 or ns2; count++; }
    }

    // type of mention
    if (defid(L"RCF_I_TYPE_P",id)) { ft[id] = m1.is_type(mention::PRONOUN); count++; }
    if (defid(L"RCF_I_TYPE_S",id)) { ft[id] = m1.is_type(mention::NOUN_PHRASE) or m1.is_type(mention::VERB_PHRASE); count++; }
    if (defid(L"RCF_I_TYPE_C",id)) { ft[id] = m1.is_type(mention::COMPOSITE); count++; }
    if (defid(L"RCF_I_TYPE_E",id)) { ft[id] = m1.is_type(mention::PROPER_NOUN); count++; }

    if (defid(L"RCF_J_TYPE_P",id)) { ft[id] = m2.is_type(mention::PRONOUN); count++; }
    if (defid(L"RCF_J_TYPE_S",id)) { ft[id] = m2.is_type(mention::NOUN_PHRASE) or m2.is_type(mention::VERB_PHRASE); count++; }
    if (defid(L"RCF_J_TYPE_C",id)) { ft[id] = m2.is_type(mention::COMPOSITE); count++; }
    if (defid(L"RCF_J_TYPE_E",id)) { ft[id] = m2.is_type(mention::PROPER_NOUN); count++; }
     
    return count;
  }

  int relaxcor_fex_dep::get_lexical(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                    feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    return count;
  }
  int relaxcor_fex_dep::get_morphological(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                          feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    return count;
  }
  int relaxcor_fex_dep::get_syntactic(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                      feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    // id of current feature
    unsigned int id;

    // head is indefinite
    if (defid(L"RCF_I_INDEF",id)) { ft[id] = indef_head(m1,fcache); count++; }
    if (defid(L"RCF_J_INDEF",id)) { ft[id] = indef_head(m2,fcache); count++; }
    // first mention word is indefinite
    if (defid(L"RCF_I_START_INDEF",id)) { ft[id] = start_with_indef(m1,fcache); count++; }
    if (defid(L"RCF_J_START_INDEF",id)) { ft[id] = start_with_indef(m2,fcache); count++; }

    // relative pronoun and its antecedent
    if (defid(L"RCF_REL_ANTECEDENT_IJ",id)) { ft[id] = rel_antecedent(m1,m2,mentions,fcache,ft); count++; }
    if (defid(L"RCF_REL_ANTECEDENT_JI",id)) { ft[id] = rel_antecedent(m2,m1,mentions,fcache,ft); count++; }
    // copulative sentence
    if (defid(L"RCF_PRED_NOMINATIVE",id)) { ft[id] = predicate_nominative(m1,m2,mentions,fcache,ft); count++; }

    return count;
  }
  int relaxcor_fex_dep::get_semantic(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                     feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    return count;
  }
  int relaxcor_fex_dep::get_discourse(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                      feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    return count;
  }
  
  
  //////////////////////////////////////////////////////////////////
  ///    Extract the configured features for a pair of mentions 
  //////////////////////////////////////////////////////////////////

  void relaxcor_fex_dep::extract_pair(const mention &m1, const mention &m2, 
                                      const vector<mention> &mentions, 
                                      feature_cache &fcache,
                                      relaxcor_model::Tfeatures &ft) const {
    int nf=0;
    /// structural features
    nf += get_structural(m1, m2, mentions, fcache, ft);
    nf += get_lexical(m1, m2, mentions, fcache, ft);
    nf += get_morphological(m1, m2, mentions, fcache, ft);
    nf += get_syntactic(m1, m2, mentions, fcache, ft);
    nf += get_semantic(m1, m2, mentions, fcache, ft);
    nf += get_discourse(m1, m2, mentions, fcache, ft);
    
    TRACE(4,L"Extracted "<<nf<<" features.");
  }


  ////////////////////////////////////////////////////////////////// 
  ///    Extract the configured features for all mentions  
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_abs::Mfeatures relaxcor_fex_dep::extract(const vector<mention> &mentions) const {
    relaxcor_fex_abs::Mfeatures M;

    feature_cache fcache;
    
    for (vector<mention>::const_iterator m1=mentions.begin()+1; m1!=mentions.end(); ++m1) {
      TRACE(4,L"Extracting all pairs for mention "+util::int2wstring(m1->get_id())+L" ("+m1->value()+L") " + m1->get_head().get_form() + L" " + m1->get_head().get_lemma() + L" " + m1->get_head().get_tag());
      
      for (vector<mention>::const_iterator m2=mentions.begin(); m2!=m1; ++m2) {	
	wstring mention_pair = util::int2wstring(m1->get_id());
	mention_pair += L":";
	mention_pair += util::int2wstring(m2->get_id());
        
	TRACE(5,L"PAIR: "+mention_pair+L" "+m1->get_head().get_form()+L":"+m2->get_head().get_form());
	
        extract_pair(*m1, *m2, mentions, fcache, M[mention_pair]);
      }
    }

    return M;
  }
  
} // namespace
