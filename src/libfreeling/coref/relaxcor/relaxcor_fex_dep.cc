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
  /// Class for a feature extractor for coreference resolution
  //////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////
  /// Constructor. Sets defaults
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::relaxcor_fex_dep(const wstring &filename, const relaxcor_model &m) : relaxcor_fex_abs(m), _Morf(filename), reEMPTY(L"^$") {

    // read configuration file and store information       
    enum sections {DEPLABELS, POSTAGS, WORDFEAT, PRONWORDS};
    config_file cfg(true,L"%");

    // add compulsory sections
    cfg.add_section(L"DepLabels",DEPLABELS,true);
    cfg.add_section(L"PosTags",POSTAGS,true);
    cfg.add_section(L"WordFeatures",WORDFEAT,true);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

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
	_Labels.insert(make_pair(L"WRD_"+name,re));
	break;
      }

      default: break;
      }
    }
    // close config file
    cfg.close();

    // register implemented feature functionrs
    register_features();
    // check whether all model features are implemented
    for (auto f=model.begin_features(); f!=model.end_features(); ++f) {
      if (_FeatureFunction.find(f->first)==_FeatureFunction.end())
        WARNING(L"Feature '"<<f->first<<L" not implemented. It will be ignored");
    }

    TRACE(2,L"Module successfully loaded");
  }

  //////////////////////////////////////////////////////////////////
  /// Destructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::~relaxcor_fex_dep() { }


  //////////////////////////////////////////////////////////////////
  /// register exisiting feature functions
  /////////////////////////////////////////////////////////

  void relaxcor_fex_dep::register_features() {

    _FeatureFunction[L"RCF_DIST_SEN_0"] = make_pair(&relaxcor_fex_dep::dist_sentences_0, ff_YES);
    _FeatureFunction[L"RCF_DIST_SEN_1"] = make_pair(&relaxcor_fex_dep::dist_sentences_1, ff_YES);
    _FeatureFunction[L"RCF_DIST_SEN_LE3"] = make_pair(&relaxcor_fex_dep::dist_sentences_le3, ff_YES);

    _FeatureFunction[L"RCF_I_TYPE_P"] = make_pair(&relaxcor_fex_dep::mention_1_type_P, ff_YES);
    _FeatureFunction[L"RCF_I_TYPE_S"] = make_pair(&relaxcor_fex_dep::mention_1_type_S, ff_YES);
    _FeatureFunction[L"RCF_I_TYPE_C"] = make_pair(&relaxcor_fex_dep::mention_1_type_C, ff_YES);
    _FeatureFunction[L"RCF_I_TYPE_E"] = make_pair(&relaxcor_fex_dep::mention_1_type_E, ff_YES);
    _FeatureFunction[L"RCF_J_TYPE_P"] = make_pair(&relaxcor_fex_dep::mention_2_type_P, ff_YES);
    _FeatureFunction[L"RCF_J_TYPE_S"] = make_pair(&relaxcor_fex_dep::mention_2_type_S, ff_YES);
    _FeatureFunction[L"RCF_J_TYPE_C"] = make_pair(&relaxcor_fex_dep::mention_2_type_C, ff_YES);
    _FeatureFunction[L"RCF_J_TYPE_E"] = make_pair(&relaxcor_fex_dep::mention_2_type_E, ff_YES);

    _FeatureFunction[L"RCF_I_DEF_NP"] = make_pair(&relaxcor_fex_dep::mention_1_definite_NP, ff_YES);
    _FeatureFunction[L"RCF_J_DEF_NP"] = make_pair(&relaxcor_fex_dep::mention_2_definite_NP, ff_YES);
    _FeatureFunction[L"RCF_I_INDEF_NP"] = make_pair(&relaxcor_fex_dep::mention_1_indefinite_NP, ff_YES);
    _FeatureFunction[L"RCF_J_INDEF_NP"] = make_pair(&relaxcor_fex_dep::mention_2_indefinite_NP, ff_YES);

    _FeatureFunction[L"RCF_I_REL_PRON"] = make_pair(&relaxcor_fex_dep::mention_1_relative, ff_YES);
    _FeatureFunction[L"RCF_J_REL_PRON"] = make_pair(&relaxcor_fex_dep::mention_2_relative, ff_YES);
    _FeatureFunction[L"RCF_I_REFLX_PRON"] = make_pair(&relaxcor_fex_dep::mention_1_reflexive, ff_YES);
    _FeatureFunction[L"RCF_J_REFLX_PRON"] = make_pair(&relaxcor_fex_dep::mention_2_reflexive, ff_YES);

    _FeatureFunction[L"RCF_I_I"] = make_pair(&relaxcor_fex_dep::mention_1_I, ff_YES);
    _FeatureFunction[L"RCF_J_I"] = make_pair(&relaxcor_fex_dep::mention_2_I, ff_YES);
    _FeatureFunction[L"RCF_I_YOU"] = make_pair(&relaxcor_fex_dep::mention_1_you, ff_YES);
    _FeatureFunction[L"RCF_J_YOU"] = make_pair(&relaxcor_fex_dep::mention_2_you, ff_YES);
    _FeatureFunction[L"RCF_I_WE"] = make_pair(&relaxcor_fex_dep::mention_1_we, ff_YES);
    _FeatureFunction[L"RCF_J_WE"] = make_pair(&relaxcor_fex_dep::mention_2_we, ff_YES);

    _FeatureFunction[L"RCF_I_NUMBER_SG"] = make_pair(&relaxcor_fex_dep::mention_1_singular, ff_YES);
    _FeatureFunction[L"RCF_J_NUMBER_SG"] = make_pair(&relaxcor_fex_dep::mention_2_singular, ff_YES);
    _FeatureFunction[L"RCF_I_THIRD_PERSON"] = make_pair(&relaxcor_fex_dep::mention_1_3pers, ff_YES);
    _FeatureFunction[L"RCF_J_THIRD_PERSON"] = make_pair(&relaxcor_fex_dep::mention_2_3pers, ff_YES);

    _FeatureFunction[L"RCF_AGREEMENT_YES"] = make_pair(&relaxcor_fex_dep::agreement, ff_YES);
    _FeatureFunction[L"RCF_AGREEMENT_NO"] = make_pair(&relaxcor_fex_dep::agreement, ff_NO);
    _FeatureFunction[L"RCF_AGREEMENT_UNK"] = make_pair(&relaxcor_fex_dep::agreement, ff_UNK);
    _FeatureFunction[L"RCF_C_AGREEMENT_YES"] = make_pair(&relaxcor_fex_dep::closest_agreement, ff_YES);
    _FeatureFunction[L"RCF_C_AGREEMENT_NO"] = make_pair(&relaxcor_fex_dep::closest_agreement, ff_NO);
    _FeatureFunction[L"RCF_C_AGREEMENT_UNK"] = make_pair(&relaxcor_fex_dep::closest_agreement, ff_UNK);
    
    _FeatureFunction[L"RCF_I_IN_QUOTES"] = make_pair(&relaxcor_fex_dep::mention_1_quotes, ff_YES);
    _FeatureFunction[L"RCF_J_IN_QUOTES"] = make_pair(&relaxcor_fex_dep::mention_2_quotes, ff_YES);
    _FeatureFunction[L"RCF_SAME_QUOTE"] = make_pair(&relaxcor_fex_dep::same_quote, ff_YES);

    _FeatureFunction[L"RCF_NESTED_JI"] = make_pair(&relaxcor_fex_dep::mention_1_nested_in_m2, ff_YES);
    _FeatureFunction[L"RCF_NESTED_IJ"] = make_pair(&relaxcor_fex_dep::mention_2_nested_in_m1, ff_YES);
    _FeatureFunction[L"RCF_NESTED"] = make_pair(&relaxcor_fex_dep::nested_mentions, ff_YES);

    _FeatureFunction[L"RCF_APPOSITION_JI"] = make_pair(&relaxcor_fex_dep::apposition, ff_YES);
    _FeatureFunction[L"RCF_REL_ANTECEDENT_IJ"] = make_pair(&relaxcor_fex_dep::rel_antecedent, ff_YES);
    _FeatureFunction[L"RCF_PRED_NP_IJ"] = make_pair(&relaxcor_fex_dep::predicative_ij, ff_YES);
    _FeatureFunction[L"RCF_PRED_NP_JI"] = make_pair(&relaxcor_fex_dep::predicative_ji, ff_YES);

  }



  //////////////////////////////////////////////////////////////////
  /// Auxiliary to handle regex map
  /////////////////////////////////////////////////////////
  
  freeling::regexp relaxcor_fex_dep::get_label_RE(const wstring &key) const {
    auto re = _Labels.find(key);
    if (re==_Labels.end()) {
      WARNING(L"No regex defined for label "<<key<<L". Ignoring affected features");
      return reEMPTY;
    }
    return re->second;
  }

  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention is inside quotes
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::in_quotes(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":IN_QUOTES";
    bool inq;
    if (fcache.computed_feature(fid)) {
      inq = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"     " << fid << L" = " << wstring(inq?L"yes":L"no") << "  (cached)");
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
      TRACE(7,L"     " << fid << L" = "<< (inq?L"yes":L"no"));
    }

    return inq;
  }


  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention is a definite noun phrase
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::definite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":DEFINITE";
    bool def;
    if (fcache.computed_feature(fid)) {
      def = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"     " << fid << L" = " << (def?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the first mention word is in the list of definite determiners
      def = fex.get_label_RE(L"WRD_Definite").search(m.get_it_begin()->get_lemma());

      fcache.set_feature(fid,util::int2wstring(def));
      TRACE(7,L"     " << fid << L" = "<< (def?L"yes":L"no"));
    }

    return def;
  }
  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention is indefinite noun phrase
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::indefinite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":INDEFINITE";
    bool ind;
    if (fcache.computed_feature(fid)) {
      ind = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"     " << fid << L" = " << (ind?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the head or first are in the list of indefinite pronouns/adjectives
      ind = fex._Morf.get_type(m.get_head().get_lemma())==L'I' or
        fex._Morf.get_type(m.get_it_begin()->get_lemma())==L'I';
      
      fcache.set_feature(fid,util::int2wstring(ind));
      TRACE(7,L"     " << fid << L" = "<< (ind?L"yes":L"no"));
    }

    return ind;
  }

  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if the mention is a relative pronoun
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_dep::relative_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":RELATIVE";
    bool rp;
    if (fcache.computed_feature(fid)) {
      rp = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a relative pronoun
      rp = fex.get_label_RE(L"TAG_RelPron").search(m.get_head().get_tag());

      fcache.set_feature(fid,util::int2wstring(rp));
      TRACE(7,L"     " << fid << L" = "<< (rp?L"yes":L"no"));
    }

    return rp;
  }

  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if the mention is a reflexive pronoun
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_dep::reflexive_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":REFLEXIVE";
    bool rp;
    if (fcache.computed_feature(fid)) {
      rp = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a reflexive pronoun
      rp = (fex._Morf.get_type(m.get_head().get_lemma()) == L'R');
      fcache.set_feature(fid,util::int2wstring(rp));
      TRACE(7,L"     " << fid << L" = "<< (rp?L"yes":L"no"));
    }

    return rp;

  }

  /////////////////////////////////////////////////////////////////////////////
  ///    Returns a string-encoded list of <prednum, role> for each predicate 
  ///   in which the mention plays some role.
  /////////////////////////////////////////////////////////////////////////////

  wstring relaxcor_fex_dep::get_arguments(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m.get_id())+L":ARGUMENTS";
    wstring args;
    if (fcache.computed_feature(fid)) {
      args =  fcache.get_feature(fid);
      TRACE(7,L"     "<<fid<<L" = ["<<args<<"]   (cached)");
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
      TRACE(7,L"     "<<fid<<L" = ["<<args<<"]");
    }

    return args;
  }


  ////////////////////////////////////////////////////
  ///    Returns whether m1 matches given pronoun features
  ////////////////////////////////////////////////////
  
  bool relaxcor_fex_dep::match_pronoun_features(const mention &m, wchar_t type, wchar_t per, wchar_t num, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring code(3,L'.'); code[0]=type; code[1]=per; code[2]=num;
    wstring fid = util::int2wstring(m.get_id())+L":PRONFEAT:"+code;
    bool pf;
    if (fcache.computed_feature(fid)) {
      pf = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"     " << fid << L" = " << (pf?L"yes":L"no") << "  (cached)");
    }
    else {
      wstring w=m.get_head().get_lemma();
      pf = fex._Morf.get_type(w)==type and fex._Morf.get_person(w)==per
        and morph_features::compatible_number(fex._Morf.get_number(w), num)==L"yes";

      fcache.set_feature(fid, util::int2wstring(pf));
      TRACE(6,L"     " << fid << L" = " << (pf?L"yes":L"no"));
    }
    return pf;
  }

  ////////////////////////////////////////////////////
  ///   Computes number for given mention
  ////////////////////////////////////////////////////  

  wchar_t relaxcor_fex_dep::get_number(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m.get_id())+L":NUMBER";

    wchar_t num=L'u';
    if (fcache.computed_feature(fid)) {
      num = fcache.get_feature(fid)[0];
      TRACE(6,L"     " << fid << L" = " << num << "  (cached)");      
    }
    else {
      if (m.is_type(mention::PRONOUN)) {
        // check in pronWords list. 
        num = fex._Morf.get_number(m.get_head().get_lemma());
        // not found in list, set to unknown
        if (num==L'#') num=L'u';
      }
      else if (m.is_type(mention::COMPOSITE)) {
        // if it is a coordination, is plural
        num = L'p';
      }
      else if (m.is_type(mention::NOUN_PHRASE)) {
        // if it is a noun_phrase, check head PoS tag
        if (fex.get_label_RE(L"TAG_NounSg").search(m.get_head().get_tag())) num=L's';
        else if (fex.get_label_RE(L"TAG_NounPl").search(m.get_head().get_tag())) num=L'p';
        else num=L'u';
      }
      else if (m.is_type(mention::PROPER_NOUN)) {
        // if it is a proper noun, ORGs are '0', and the others 'u'
        if (fex.get_label_RE(L"TAG_OrgNE").search(m.get_head().get_tag())) num=L'0';
        else num=L'u';
      }

      fcache.set_feature(fid, wstring(1,num));
      TRACE(6,L"     " << fid << L" = " << num);
    }
    return num;
  }
   
  ////////////////////////////////////////////////////
  ///   Computes gender for given mention
  ////////////////////////////////////////////////////  

  wchar_t relaxcor_fex_dep::get_gender(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m.get_id())+L":GENDER";

    wchar_t gen=L'u';
    if (fcache.computed_feature(fid)) {
      gen = fcache.get_feature(fid)[0];
      TRACE(6,L"     " << fid << L" = " << gen << "  (cached)");      
    }
    else {
      if (m.is_type(mention::PRONOUN)) {
        // check in pronWords list. 
        gen = fex._Morf.get_gender(m.get_head().get_lemma());
        // not found in list, set to unknown
        if (gen==L'#') gen=L'u';
      }
      else gen=L'u';

      /*  TODO
      else if (m.is_type(mention::COMPOSITE)) {
        // if it is a coordination, is 
        gen = L'';
      }
      else if (m.is_type(mention::NOUN_PHRASE)) {
        // if it is a noun_phrase, check head PoS tag
        auto reNS = get_label_RE(L"TAG_NounSg"); 
        auto reNP = get_label_RE(L"TAG_NounPl"); 

        if (reNS!=_Labels.end() and reNS->second.search(m.get_head().get_tag())) gen=L's';
        else if (reNP!=_Labels.end() and reNP->second.search(m.get_head().get_tag())) gen=L'p';
        else gen=L'u';
      }
      else if (m.is_type(mention::PROPER_NOUN)) {
        // if it is a proper noun, ORGs are '0', and the others 'u'
        auto re = get_label_RE(L"TAG_OrgNE"); 
        if (re!=_Labels.end() and re->second.search(m.get_head().get_tag())) gen=L'0';
        else gen=L'u';
      }
      */

      fcache.set_feature(fid, wstring(1,gen));
      TRACE(6,L"     " << fid << L" = " << gen);
    }

    return gen;
  }
   
  ////////////////////////////////////////////////////
  ///   Computes person for given mention
  ////////////////////////////////////////////////////  

  wchar_t relaxcor_fex_dep::get_person(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":PERSON";

    wchar_t num=L'3';
    if (fcache.computed_feature(fid)) {
      num = fcache.get_feature(fid)[0];
      TRACE(6,L"     " << fid << L" = " << num << "  (cached)");      
    }
    else {
      if (m.is_type(mention::PRONOUN)) {
        // check in pronWords list. 
        num = fex._Morf.get_number(m.get_head().get_lemma());
        // not found in list, set to 3rd
        if (num==L'#') num=L'3';
      }
      else // all other mentions are 3rd.
        num = L'3';

      fcache.set_feature(fid, wstring(1,num));
      TRACE(6,L"     " << fid << L" = " << num);
    }

    return num;
  }
   
  //////////////////////////////////////////////////////////////////
  ///    Computes and caches the distance in sentences
  //////////////////////////////////////////////////////////////////

  int relaxcor_fex_dep::dist_sentences(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":DIST_SENTENCES";
    int res;
    if (fcache.computed_feature(fid)) {
      res = util::wstring2int(fcache.get_feature(fid));    
      TRACE(6,L"   " << fid << L" = " << res << L"  (cached)");
    }
    else {  
      res = abs(m1.get_n_sentence() - m2.get_n_sentence());
      fcache.set_feature(fid, util::int2wstring(res));
      TRACE(6,L"   " << fid << L" = " << res);
    }
    return res;
  }


  ////////////////////////////////////////////////////
  ///    Returns whether m1 is nested in m2
  ////////////////////////////////////////////////////

  bool relaxcor_fex_dep::nested(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

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

  unsigned int relaxcor_fex_dep::dist_in_phrases(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":DIST_PHRASES";
    unsigned int res;
    if (fcache.computed_feature(fid)) {
      res = util::wstring2int(fcache.get_feature(fid));    
      TRACE(6,L"   " << fid << L" = " << res << L"  (cached)");
    }
    else {
      if (m1.get_n_sentence() != m2.get_n_sentence()) return INFINITE;
      if (nested(m1,m2,fcache,fex) or nested(m2,m1,fcache,fex)) return 0;
      
      list<pair<int,wstring> > args1 = util::wstring2pairlist<int,wstring>(get_arguments(m1,fcache,fex),L":",L"/");
      list<pair<int,wstring> > args2 = util::wstring2pairlist<int,wstring>(get_arguments(m2,fcache,fex),L":",L"/");
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
  ///    Returns whether m1 is the SBJ and m2 is PRD of a copulative verb
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::predicative(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":PREDICATIVE";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      // m1 is SBJ and m2 is PRD of a copulative verb (or viceversa), and they are not possessive
      r = ( fex._Morf.get_type(m1.get_head().get_lemma())!=L'P' and  // not possessives 
            fex._Morf.get_type(m2.get_head().get_lemma())!=L'P' and
            fex.get_label_RE(L"FUN_Subject").search(m1.get_dtree()->get_label()) and
            fex.get_label_RE(L"FUN_Predicate").search(m2.get_dtree()->get_label())  and
            m1.get_dtree().get_parent() == m2.get_dtree().get_parent() and
            fex.get_label_RE(L"WRD_Copulative").search(m1.get_dtree().get_parent()->get_word().get_lemma())
            );

      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return r;
  } 


  //// ######################  REGISTERED FEATURE FUNCTIONS ###########################

  ///    Returns wether the distance in sentences is 1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_sentences_0(const mention &m1, const mention &m2, 
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_sentences(m1,m2,fcache,fex)==0 ? ff_YES : ff_NO);
  }

  ///    Returns wether the distance in sentences is 2
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_sentences_1(const mention &m1, const mention &m2, 
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_sentences(m1,m2,fcache,fex)==1 ? ff_YES : ff_NO);
  }

  ///    Returns wether the distance in sentences is <=3
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_sentences_le3(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_sentences(m1,m2,fcache,fex)<=3 ? ff_YES : ff_NO);
  }

  ///    Returns wether mention 1 is of type P
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_P(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::PRONOUN) ? ff_YES : ff_NO);
  }

  ///    Returns wether mention 1 is of type S
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_S(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::NOUN_PHRASE) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is of type E
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_E(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::PROPER_NOUN) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is of type C
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_C(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::COMPOSITE) ? ff_YES : ff_NO);
  }

  ///    Returns wether mention 2 is of type P
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_P(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::PRONOUN) ? ff_YES : ff_NO);
  }

  ///    Returns wether mention 2 is of type S
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_S(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::NOUN_PHRASE) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is of type E
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_E(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::PROPER_NOUN) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is of type C
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_C(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::COMPOSITE) ? ff_YES : ff_NO);
  }

  ///    Returns wether mention 1 is a definite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_definite_NP(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (definite(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is an indefinite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_indefinite_NP(const mention &m1, const mention &m2,
                                                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (indefinite(m1,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns wether mention 2 is a definite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_definite_NP(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (definite(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is an indefinite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_indefinite_NP(const mention &m1, const mention &m2,
                                                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (indefinite(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is a relative pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_relative(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (relative_pronoun(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is a relative pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_relative(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (relative_pronoun(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is a reflexive pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_reflexive(const mention &m1, const mention &m2,
                                                                        feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (reflexive_pronoun(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is a reflexive pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_reflexive(const mention &m1, const mention &m2,
                                                                        feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (reflexive_pronoun(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is "I"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_I(const mention &m1, const mention &m2,
                                                                feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L'S',L'1',L's',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is "I"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_I(const mention &m1, const mention &m2,
                                                                feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L'S',L'1',L's',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is "you"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_you(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L'S',L'2',L'0',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is "you"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_you(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L'S',L'2',L'0',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is "we"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_we(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L'S',L'1',L'p',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is "we"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_we(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L'S',L'1',L'p',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is singular
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_singular(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_number(m1,fcache,fex)==L's' ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is singular
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_singular(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_number(m2,fcache,fex)==L's' ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 1 is 3rd person
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_3pers(const mention &m1, const mention &m2,
                                                                    feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_person(m1,fcache,fex)==L'3' ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is 3rd person
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_3pers(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_person(m2,fcache,fex)==L'3' ? ff_YES : ff_NO);
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns "yes" if they agree in number and gender or they agree 
  ///              in one of the features and the agreement of the 
  ///              other feature is unknowm
  ///            "no" if they do not agree in number or in gender
  ///            "unk" otherwise
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::agreement(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":AGREEMENT";
    wstring ag;
    if (fcache.computed_feature(fid)) {
      ag = fcache.get_feature(fid);
      TRACE(6,L"   " << fid << L" = " << ag << "  (cached)");
    }
    else {
      wchar_t n1 = get_number(m1,fcache,fex);   
      wchar_t n2 = get_number(m2,fcache,fex); 
      wstring numagr = morph_features::compatible_number(n1,n2);

      wchar_t g1 = get_gender(m1,fcache,fex);
      wchar_t g2 = get_gender(m2,fcache,fex);
      wstring genagr = morph_features::compatible_gender(g1,g2);

      if (numagr==L"no" or genagr==L"no")
        ag = L"no";
      else if ((numagr==L"yes" and genagr==L"yes") or 
               (numagr==L"yes" and genagr==L"unk") or
               (numagr==L"unk" and genagr==L"yes") )
        ag = L"yes";
      else 
        ag = L"unk";

      fcache.set_feature(fid, util::int2wstring(ag));
      TRACE(6,L"   " << fid << L" = " << ag);
    }

    return (ag==L"yes"? ff_YES : (ag==L"no" ? ff_NO : ff_UNK));
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns "yes" if m2 is the closest agreeing referent for m1
  ///   WARNING:  it assumes that the pairs m1:mx (where mx is 
  ///            between m1 and m2) have been already processed
  ///            and uses the cache to retrieve the info.
  ///             It also assumes that the feature "agreement" has been computed
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::closest_agreement(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring pair = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id());
    wstring fid = pair+L":CLOSEST_AGREEMENT";
    wstring ag;
    if (fcache.computed_feature(fid)) {
      ag = fcache.get_feature(fid);
      TRACE(6,L"   " << fid << L" = " << ag << "  (cached)");
    }
    else {
      // complain if agreement was not activated
      if (not fcache.computed_feature(pair+L":AGREEMENT")) {
        WARNING(L"RCF_C_AGREEMENT feature requires RCF_AGREEMENT to be also activated.");
        return ff_NO;
      }

      // check whether m1 and m2 agree
      ag = fcache.get_feature(pair+L":AGREEMENT");
      if (ag == L"yes") {
        // if they agree, check closestness
        bool found=false;
        for (int mx=m1.get_id()+1; mx<m2.get_id() and not found; ++mx) {
          wstring fid2 = util::int2wstring(m1.get_id())+L":"+util::int2wstring(mx)+L":AGREEMENT";
          if (not fcache.computed_feature(fid2)) {
            WARNING(L"Feature "<<fid2<<" not found while computing "<<fid);
            return ff_NO;
          }
          wstring ag2 = fcache.get_feature(fid2);
          TRACE(6,L"     " << fid2 << L" = " << ag2);
          found = (ag2==L"yes");
        }          
        if (found) ag=L"no";
      }

      fcache.set_feature(fid, util::int2wstring(ag));
      TRACE(6,L"   " << fid << L" = " << ag);
    }

    return (ag==L"yes"? ff_YES : (ag==L"no" ? ff_NO : ff_UNK));
  }


  ///    Returns wether mention 1 is inside quotes
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_quotes(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (in_quotes(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns wether mention 2 is inside quotes
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_quotes(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (in_quotes(m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 1 is nested in m2
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_nested_in_m2(const mention &m1, const mention &m2,
                                                                           feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (nested(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is nested in m1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_nested_in_m1(const mention &m1, const mention &m2,
                                                                           feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (nested(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether one mention is nested in the other
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::nested_mentions(const mention &m1, const mention &m2,
                                                                    feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (nested(m1,m2,fcache,fex) or nested(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether m1 is SBJ and m2 is PRD of verb "to be"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::predicative_ij(const mention &m1, const mention &m2,
                                                                   feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (predicative(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether m2 is SBJ and m1 is PRD of verb "to be"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::predicative_ji(const mention &m1, const mention &m2,
                                                                   feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (predicative(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }


  ////////////////////////////////////////////////////
  ///    Returns whether m1 and m2 are inside the same quotation
  ////////////////////////////////////////////////////
  
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::same_quote(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":SAME_QUOTE";
    bool sq;
    if (fcache.computed_feature(fid)) {
      sq = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (sq?L"yes":L"no") << "  (cached)");
    }
    else {
      // check for quotes between m1 and m2. If none is found, they are in the same quotation (if any)
      sq = in_quotes(m1,fcache,fex) and in_quotes(m2,fcache,fex);      
      for (sentence::const_iterator k=m1.get_it_end(); k!=m2.get_it_begin() and sq; ++k ) {
        sq = (k->get_tag()==L"Fe" or k->get_tag()==L"Fra" or k->get_tag()==L"Frc");
      }
      
      fcache.set_feature(fid, util::int2wstring(sq));
      TRACE(6,L"   " << fid << L" = " << (sq?L"yes":L"no"));
    }

    return (sq ? ff_YES : ff_NO);
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether m2 is apposition of m1
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::apposition(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":APPOSITION";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      // m2 is child of m1 with label "APPO" or else, m1 and m2 are siblings and consecutive
      r = (m2.get_dtree().get_parent()==m1.get_dtree() and fex.get_label_RE(L"FUN_Apposition").search(m2.get_dtree()->get_label())) 
        or (m1.get_dtree().get_parent()==m2.get_dtree() and m1.get_pos_end()+1 == m2.get_pos_begin());
      
      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return (r ? ff_YES : ff_NO);
  } 
  
  

  //////////////////////////////////////////////////////////////////
  ///    Returns whether m1 is relative pronoun and m1 its antecedent
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::rel_antecedent(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":REL_ANTECEDENT";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      // m2 is relative pronoun, is and child (SBJ or OBJ) of a verb that is NMOD of m1
      r = fex.get_label_RE(L"TAG_RelPron").search(m2.get_head().get_tag()) and
        (fex.get_label_RE(L"FUN_Subject").search(m2.get_dtree()->get_label()) or
         fex.get_label_RE(L"FUN_Object").search(m2.get_dtree()->get_label())) and 
        not m2.get_dtree().get_parent().is_root() and  
        fex.get_label_RE(L"TAG_Verb").search(m2.get_dtree().get_parent()->get_word().get_tag()) and 
        fex.get_label_RE(L"FUN_NounModifier").search(m2.get_dtree().get_parent()->get_label()) and 
        m2.get_dtree().get_parent().get_parent() == m1.get_dtree();

      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return (r ? ff_YES : ff_NO);
  } 
  


  //////////////////////////////////////////////////////////////////
  /// Structural features
  //////////////////////////////////////////////////////////////////
  /*
  int relaxcor_fex_dep::get_structural(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    TRACE(6,L"get structural features");

    //counter of extracted features
    int count=0;

    // id of current feature
    unsigned int id;

    // distance in #sentences
   if (def(L"RCF_DIST_SEN_0") or def(L"RCF_DIST_SEN_1") or def(L"RCF_DIST_SEN_LE3") or def(L"RCF_DIST_SEN_G3")) {
      unsigned int dist = abs(m1.get_n_sentence() - m2.get_n_sentence());
      if (defid(L"RCF_DIST_SEN_0",id))  { ft[id] = (dist==0); count++; }
      if (defid(L"RCF_DIST_SEN_1",id))  { ft[id] = (dist==1); count++; }
      if (defid(L"RCF_DIST_SEN_LE3",id)) { ft[id] = (dist<=3); count++; }
      if (defid(L"RCF_DIST_SEN_G3",id)) { ft[id] = (dist>3); count++; }
    }
    // #mentions between both mentions
    if (def(L"RCF_DIST_MEN_0") or def(L"RCF_DIST_MEN_LE3") or def(L"RCF_DIST_MEN_LE6") or def(L"RCF_DIST_MEN_G6")) {
      unsigned int dist = abs(m1.get_id() - m2.get_id()) - 1;
      if (defid(L"RCF_DIST_MEN_0",id))   { ft[id] = (dist==0); count++; }
      if (defid(L"RCF_DIST_MEN_LE3",id))  { ft[id] = (dist<=3); count++; }
      if (defid(L"RCF_DIST_MEN_LE6",id)) { ft[id] = (dist<=6); count++; }
      if (defid(L"RCF_DIST_MEN_G6",id)) { ft[id] = (dist>6); count++; }
    }
    // distance in #phrases
    if (def(L"RCF_DIST_PHR_0") or def(L"RCF_DIST_PHR_1") or def(L"RCF_DIST_PHR_LE3")) {
      unsigned int dist = dist_in_phrases(m1,m2,fcache, *this);
      if (defid(L"RCF_DIST_PHR_0",id))  { ft[id] = (dist==0); count++; }
      if (defid(L"RCF_DIST_PHR_1",id))  { ft[id] = (dist==1); count++; }
      if (defid(L"RCF_DIST_PHR_LE3",id)) { ft[id] = (dist<=3); count++; }
    }
    // in quotes?
    if (defid(L"RCF_I_IN_QUOTES",id)) { ft[id] = in_quotes(m1,fcache,*this); count++; }
    if (defid(L"RCF_J_IN_QUOTES",id)) { ft[id] = in_quotes(m2,fcache,*this); count++; }



    //if (defid(L"RCF_SAME_QUOTE",id)) { ft[id] = same_quote(m1, m2, fcache); count++; }

    if (defid(L"RCF_SAME_QUOTE",id)) { 
      wcerr<<L"calling same quote registerd"<<endl;
      auto ff = _FeatureFunction.find(L"RCF_SAME_QUOTE")->second;
      ft[id] = ((*ff.first)(m1,m2,fcache,*this) == ff.second);
      count++;
    }

    // m2 is apposition of m1
    if (defid(L"RCF_APPOSITION_JI",id)) { ft[id] = apposition(m1,m2,fcache,*this); count++; }
    // one is nested to the other 
    if (def(L"RCF_NESTED") or def(L"RCF_NESTED_IJ") or def(L"RCF_NESTED_JI")) {
      bool ns1 = nested(m1,m2,fcache,*this);
      bool ns2 = nested(m2,m1,fcache,*this);
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

  int relaxcor_fex_dep::get_lexical(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    return count;
  }
  int relaxcor_fex_dep::get_morphological(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    // id of current feature
    unsigned int id;

    if (defid(L"RCF_I_I",id))   { ft[id] = match_pronoun_features(m1,L'S',L'1',L's',fcache,*this); count++; }
    if (defid(L"RCF_J_I",id))   { ft[id] = match_pronoun_features(m2,L'S',L'1',L's',fcache,*this); count++; }
    if (defid(L"RCF_I_YOU",id)) { ft[id] = match_pronoun_features(m1,L'S',L'2',L'0',fcache,*this); count++; }
    if (defid(L"RCF_J_YOU",id)) { ft[id] = match_pronoun_features(m2,L'S',L'2',L'0',fcache,*this); count++; }
    if (defid(L"RCF_I_WE",id))  { ft[id] = match_pronoun_features(m1,L'S',L'1',L'p',fcache,*this); count++; }
    if (defid(L"RCF_J_WE",id))  { ft[id] = match_pronoun_features(m2,L'S',L'1',L'p',fcache,*this); count++; }

    if (defid(L"RCF_I_NUMBER_SG",id))  { ft[id] = (get_number(m1,fcache,*this)==L's'); count++; }
    if (defid(L"RCF_J_NUMBER_SG",id))  { ft[id] = (get_number(m2,fcache,*this)==L's'); count++; }
    if (defid(L"RCF_I_THIRD_PERSON",id))  { ft[id] = (get_person(m1,fcache,*this)==L'3'); count++; }
    if (defid(L"RCF_J_THIRD_PERSON",id))  { ft[id] = (get_person(m2,fcache,*this)==L'3'); count++; }

    if (defid(L"RCF_SAME_PERSON",id))  { ft[id] = (get_person(m1,fcache,*this)==get_person(m2,fcache,*this)); count++; }

    if (defid(L"RCF_AGREEMENT_YES",id))  { ft[id] = (agreement(m1,m2,fcache,*this)==ff_YES); count++; }
    if (defid(L"RCF_C_AGREEMENT_YES",id))  { ft[id] = (closest_agreement(m1,m2,fcache,*this)==ff_YES); count++; }

    return count;
  }

  int relaxcor_fex_dep::get_syntactic(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    // id of current feature
    unsigned int id;

    // definite noun phrase
    if (defid(L"RCF_I_DEF_NP",id)) { ft[id] = definite(m1,fcache,*this); count++; }
    if (defid(L"RCF_J_DEF_NP",id)) { ft[id] = definite(m2,fcache,*this); count++; }
    // indefinite noun phrase
    if (defid(L"RCF_I_INDEF_NP",id)) { ft[id] = indefinite(m1,fcache,*this); count++; }
    if (defid(L"RCF_J_INDEF_NP",id)) { ft[id] = indefinite(m2,fcache,*this); count++; }
    // relative pronoun
    if (defid(L"RCF_I_REL_PRON",id)) { ft[id] = relative_pronoun(m1,fcache,*this); count++; }
    if (defid(L"RCF_J_REL_PRON",id)) { ft[id] = relative_pronoun(m2,fcache,*this); count++; }
    // reflexive pronoun
    if (defid(L"RCF_I_REFLX_PRON",id)) { ft[id] = reflexive_pronoun(m1,fcache,*this); count++; }
    if (defid(L"RCF_J_REFLX_PRON",id)) { ft[id] = reflexive_pronoun(m2,fcache,*this); count++; }

    // relative pronoun and its antecedent
    if (defid(L"RCF_REL_ANTECEDENT_IJ",id)) { ft[id] = rel_antecedent(m1,m2,fcache,*this); count++; }
    if (defid(L"RCF_REL_ANTECEDENT_JI",id)) { ft[id] = rel_antecedent(m2,m1,fcache,*this); count++; }
    // copulative sentence
    if (defid(L"RCF_PRED_NP_JI",id)) { ft[id] = predicative(m1,m2,fcache,*this); count++; }

    return count;
  }
  int relaxcor_fex_dep::get_semantic(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    return count;
  }
  int relaxcor_fex_dep::get_discourse(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {
    //counter of extracted features
    int count=0;
    return count;
  }
  
  
  //////////////////////////////////////////////////////////////////
  ///    Extract the configured features for a pair of mentions 
  //////////////////////////////////////////////////////////////////

  void relaxcor_fex_dep::extract_pair(const mention &m1, const mention &m2, 
                                      feature_cache &fcache,
                                      relaxcor_model::Tfeatures &ft) const {
    int nf=0;
    /// structural features
    nf += get_structural(m1, m2, fcache, ft);
    nf += get_lexical(m1, m2, fcache, ft);
    nf += get_morphological(m1, m2, fcache, ft);
    nf += get_syntactic(m1, m2, fcache, ft);
    nf += get_semantic(m1, m2, fcache, ft);
    nf += get_discourse(m1, m2, fcache, ft);
    
    TRACE(4,L"Extracted "<<nf<<" features.");
  }
*/

  ////////////////////////////////////////////////////////////////// 
  ///    Extract the configured features for all mentions  
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_abs::Mfeatures relaxcor_fex_dep::extract(const vector<mention> &mentions) const {
    relaxcor_fex_abs::Mfeatures M;

    feature_cache fcache;
    
    for (int m2=1; m2<mentions.size(); ++m2) {
      TRACE(4,L"Extracting all pairs for mention "<<mentions[m2].get_id()<<L" ("+mentions[m2].value()<<L") ["<<mentions[m2].get_head().get_form()<<L","<<mentions[m2].get_head().get_lemma()<<L","<<mentions[m2].get_head().get_tag()<<L"]");
      
      for (int m1=m2-1; m1>=0; --m1) {	
	TRACE(5,L"PAIR: "<<mentions[m1].get_id()<<L":"<<mentions[m2].get_id()<<L" "+mentions[m1].get_head().get_form()<<L":"<<mentions[m2].get_head().get_form()<<L" ["<<mentions[m1].value()<<"]:["<<mentions[m2].value()<<"]");	

        // feature tables are stored with key m2:m1 for compatibility with relaxcor and other extractors.
	wstring mention_pair = util::int2wstring(mentions[m2].get_id()) + L":" + util::int2wstring(mentions[m1].get_id());
        //        extract_pair(mentions[m1], mentions[m2], fcache, M[mention_pair]);

        int count = 0;
        for (auto f=model.begin_features(); f!=model.end_features(); ++f) {
          wstring fname = f->first;
          auto p = _FeatureFunction.find(fname);
          if (p != _FeatureFunction.end()) {
            int fid = model.feature_name_id(fname);
            TFeatureFunction fFunc = p->second.first; 
            TFeatureValue expected = p->second.second;
            M[mention_pair][fid] = ((*fFunc)(mentions[m1], mentions[m2], fcache, *this) == expected);
            ++count;
          }
        }
        TRACE(4,L"Extracted "<<count<<" features.");
      }
    }    
    return M;
  }


  ////////////////////////////////////////////////////////////////// 
  ///    Auxiliary class to store morphological information
  //////////////////////////////////////////////////////////////////

  /// --------------------------------------------------------
  /// Constructor
  
  relaxcor_fex_dep::morph_features::morph_features(const wstring &filename) {
    // read configuration file and store information       
    enum sections {PRONWORDS};
    config_file cfg(true,L"%");

    // add compulsory sections
    cfg.add_section(L"PronWords",PRONWORDS,true);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {
      case PRONWORDS: {
        // read a pronoun and its features
	wstring name, val;
        sin >> name >> val;
        _Words.insert(make_pair(name, val));
        break;
      }

      default: break;
      }
    }
    // close config file
    cfg.close();
  }

  /// --------------------------------------------------------
  /// Destructor

  relaxcor_fex_dep::morph_features::~morph_features() {}


  /// --------------------------------------------------------
  /// private to get morphological feature in position k

  wchar_t relaxcor_fex_dep::morph_features::get_feature(const wstring &w, int k) const {
    auto p = _Words.find(w);
    wchar_t res = L'#';
    if (p!=_Words.end()) res = p->second[k];
    return res;
  }


  /// --------------------------------------------------------
  /// get type, person, number, or gender

  wchar_t relaxcor_fex_dep::morph_features::get_type(const wstring &w) const { return get_feature(w,0); }
  wchar_t relaxcor_fex_dep::morph_features::get_person(const wstring &w) const { return get_feature(w,1); }
  wchar_t relaxcor_fex_dep::morph_features::get_gender(const wstring &w) const { return get_feature(w,2); }
  wchar_t relaxcor_fex_dep::morph_features::get_number(const wstring &w) const { return get_feature(w,3); }

  bool relaxcor_fex_dep::morph_features::same_type(const wstring &w1, const wstring &w2) const { return get_type(w1)==get_type(w2); }
  bool relaxcor_fex_dep::morph_features::same_person(const wstring &w1, const wstring &w2) const { return get_person(w1)==get_person(w2); }
  bool relaxcor_fex_dep::morph_features::same_number(const wstring &w1, const wstring &w2) const { return get_number(w1)==get_number(w2); }
  bool relaxcor_fex_dep::morph_features::same_gender(const wstring &w1, const wstring &w2) const { return get_gender(w1)==get_gender(w2); }
  
  wstring relaxcor_fex_dep::morph_features::compatible_number(wchar_t n1, wchar_t n2) {
    // number is compatible if they match or at least one of them is underspecified
    wstring cn;
    if (n1==L'0' or
        n2==L'0' or
        (n1==n2 and n1!=L'u'))
      cn = L"yes";
    else if (n1==L'u' or n2==L'u') 
      cn = L"unk";
    else 
      cn = L"no";
    TRACE(7,L"      - compatible number " << n1 << L":" << n2 << L"=" << cn);
    return cn;
  }

  wstring relaxcor_fex_dep::morph_features::compatible_gender(wchar_t g1, wchar_t g2) {
    // gender is compatible if they match, at least one of them is underspecified, or
    // one is M/F and the other is M or F
    wstring cg;
    if (g1==L'0' or 
        g2==L'0' or
        (g1==g2 and g1!=L'u') or
        (g1==L'b' and (g2==L'm' or g2==L'f')) or
        (g2==L'b' and (g1==L'm' or g1==L'f')))
      cg = L"yes";
    else if (g1==L'u' or g2==L'u') 
      cg = L"unk";
    else 
      cg = L"no";
    TRACE(7,L"      - compatible gender " << g1 << L":" << g2 << L"=" << cg);
    return cg;
  }
  
} // namespace
