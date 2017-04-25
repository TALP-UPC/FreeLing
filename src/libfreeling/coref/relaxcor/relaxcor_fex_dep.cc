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

    _FeatureFunction[L"RCF_DIST_MEN_0"] = make_pair(&relaxcor_fex_dep::dist_mentions_0, ff_YES);
    _FeatureFunction[L"RCF_DIST_MEN_LE3"] = make_pair(&relaxcor_fex_dep::dist_mentions_le3, ff_YES);
    _FeatureFunction[L"RCF_DIST_MEN_LE9"] = make_pair(&relaxcor_fex_dep::dist_mentions_le9, ff_YES);

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
    _FeatureFunction[L"RCF_I_POSS_DET"] = make_pair(&relaxcor_fex_dep::mention_1_possessive, ff_YES);
    _FeatureFunction[L"RCF_J_POSS_DET"] = make_pair(&relaxcor_fex_dep::mention_2_possessive, ff_YES);

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
    _FeatureFunction[L"RCF_I_THIRD_PERSON"] = make_pair(&relaxcor_fex_dep::mention_1_3pers, ff_YES);
    _FeatureFunction[L"RCF_SAME_PERSON"] = make_pair(&relaxcor_fex_dep::same_person, ff_YES);

    _FeatureFunction[L"RCF_AGREEMENT_YES"] = make_pair(&relaxcor_fex_dep::agreement, ff_YES);
    _FeatureFunction[L"RCF_AGREEMENT_NO"] = make_pair(&relaxcor_fex_dep::agreement, ff_NO);
    _FeatureFunction[L"RCF_AGREEMENT_UNK"] = make_pair(&relaxcor_fex_dep::agreement, ff_UNK);
    _FeatureFunction[L"RCF_C_AGREEMENT_YES"] = make_pair(&relaxcor_fex_dep::closest_agreement, ff_YES);
    _FeatureFunction[L"RCF_C_AGREEMENT_NO"] = make_pair(&relaxcor_fex_dep::closest_agreement, ff_NO);
    _FeatureFunction[L"RCF_C_AGREEMENT_UNK"] = make_pair(&relaxcor_fex_dep::closest_agreement, ff_UNK);
    
    _FeatureFunction[L"RCF_I_IN_QUOTES"] = make_pair(&relaxcor_fex_dep::mention_1_quotes, ff_YES);
    _FeatureFunction[L"RCF_J_IN_QUOTES"] = make_pair(&relaxcor_fex_dep::mention_2_quotes, ff_YES);
    _FeatureFunction[L"RCF_SAME_QUOTE"] = make_pair(&relaxcor_fex_dep::same_quote, ff_YES);

    _FeatureFunction[L"RCF_NESTED_IJ"] = make_pair(&relaxcor_fex_dep::mention_1_nested_in_m2, ff_YES);
    _FeatureFunction[L"RCF_NESTED_JI"] = make_pair(&relaxcor_fex_dep::mention_2_nested_in_m1, ff_YES);
    _FeatureFunction[L"RCF_NESTED"] = make_pair(&relaxcor_fex_dep::nested_mentions, ff_YES);
    _FeatureFunction[L"RCF_I_EMBEDDED"] = make_pair(&relaxcor_fex_dep::mention_1_embedded, ff_YES);
    _FeatureFunction[L"RCF_J_EMBEDDED"] = make_pair(&relaxcor_fex_dep::mention_2_embedded, ff_YES);
    _FeatureFunction[L"RCF_I_NMOD"] = make_pair(&relaxcor_fex_dep::mention_1_nmod, ff_YES);
    _FeatureFunction[L"RCF_J_NMOD"] = make_pair(&relaxcor_fex_dep::mention_2_nmod, ff_YES);

    _FeatureFunction[L"RCF_APPOSITION_JI"] = make_pair(&relaxcor_fex_dep::apposition, ff_YES);
    _FeatureFunction[L"RCF_REL_ANTECEDENT_IJ"] = make_pair(&relaxcor_fex_dep::rel_antecedent, ff_YES);
    _FeatureFunction[L"RCF_PRED_NP_IJ"] = make_pair(&relaxcor_fex_dep::predicative_ij, ff_YES);
    _FeatureFunction[L"RCF_PRED_NP_JI"] = make_pair(&relaxcor_fex_dep::predicative_ji, ff_YES);

    _FeatureFunction[L"RCF_STR_MATCH_STRICT"] = make_pair(&relaxcor_fex_dep::str_match_strict, ff_YES);
    _FeatureFunction[L"RCF_STR_MATCH_RELAXED"] = make_pair(&relaxcor_fex_dep::str_match_relaxed, ff_YES);
    _FeatureFunction[L"RCF_HEAD_MATCH"] = make_pair(&relaxcor_fex_dep::str_head_match, ff_YES);
    _FeatureFunction[L"RCF_RELAXED_HEAD_MATCH_IJ"] = make_pair(&relaxcor_fex_dep::relaxed_head_match_ij, ff_YES);
    _FeatureFunction[L"RCF_RELAXED_HEAD_MATCH_JI"] = make_pair(&relaxcor_fex_dep::relaxed_head_match_ji, ff_YES);

    _FeatureFunction[L"RCF_NUM_MATCH_IJ"] = make_pair(&relaxcor_fex_dep::num_match_ij, ff_YES);
    _FeatureFunction[L"RCF_NUM_MATCH_JI"] = make_pair(&relaxcor_fex_dep::num_match_ji, ff_YES);
    _FeatureFunction[L"RCF_WORD_INCLUSION_IJ"] = make_pair(&relaxcor_fex_dep::word_inclusion_ij, ff_YES);
    _FeatureFunction[L"RCF_WORD_INCLUSION_JI"] = make_pair(&relaxcor_fex_dep::word_inclusion_ji, ff_YES);
    _FeatureFunction[L"RCF_COMPATIBLE_MODS_IJ"] = make_pair(&relaxcor_fex_dep::compatible_mods_ij, ff_YES);
    _FeatureFunction[L"RCF_COMPATIBLE_MODS_JI"] = make_pair(&relaxcor_fex_dep::compatible_mods_ji, ff_YES);

    _FeatureFunction[L"RCF_I_SUBJECT_REPORTING"] = make_pair(&relaxcor_fex_dep::mention_1_subj_reporting, ff_YES);
    _FeatureFunction[L"RCF_J_SUBJECT_REPORTING"] = make_pair(&relaxcor_fex_dep::mention_2_subj_reporting, ff_YES);
    _FeatureFunction[L"RCF_I_OBJECT_REPORTING"] = make_pair(&relaxcor_fex_dep::mention_1_obj_reporting, ff_YES);
    _FeatureFunction[L"RCF_J_OBJECT_REPORTING"] = make_pair(&relaxcor_fex_dep::mention_2_obj_reporting, ff_YES);
    _FeatureFunction[L"RCF_OBJ_SAME_REPORTING"] = make_pair(&relaxcor_fex_dep::obj_same_reporting, ff_YES);
    _FeatureFunction[L"RCF_SUBJ_OBJ_REPORTING_IJ"] = make_pair(&relaxcor_fex_dep::subj_obj_reporting_ij, ff_YES);
    _FeatureFunction[L"RCF_SUBJ_OBJ_REPORTING_JI"] = make_pair(&relaxcor_fex_dep::subj_obj_reporting_ji, ff_YES);
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
  ///    Returns true if the mention is a possessive determiner
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_dep::possessive_determiner(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":POSSESSIVE";
    bool rp;
    if (fcache.computed_feature(fid)) {
      rp = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a reflexive pronoun
      rp = (fex._Morf.get_type(m.get_head().get_lemma()) == L'P');
      fcache.set_feature(fid,util::int2wstring(rp));
      TRACE(7,L"     " << fid << L" = "<< (rp?L"yes":L"no"));
    }

    return rp;
  }

  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if the mention is a noun modifier in the dep tree
  ///////////////////////////////////////////////////////////////////////////// 

  bool relaxcor_fex_dep::is_noun_modifier(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m.get_id())+L":NMOD";
    bool rp;
    if (fcache.computed_feature(fid)) {
      rp = util::wstring2int(fcache.get_feature(fid));
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a reflexive pronoun
      rp = fex.get_label_RE(L"FUN_NounModifier").search(m.get_dtree()->get_label());
      fcache.set_feature(fid,util::int2wstring(rp));
      TRACE(7,L"     " << fid << L" = "<< (rp?L"yes":L"no"));
    }
    return rp;
  }


  /////////////////////////////////////////////////////////////////////////////
  ///    Returns a string-encoded list of <prednum, role> for each predicate 
  ///   in which the word in position mpos of sentence s plays some role.
  /////////////////////////////////////////////////////////////////////////////

  list<pair<int,wstring>> relaxcor_fex_dep::get_arguments(paragraph::const_iterator s, int mpos) {
      list<pair<int,wstring>> roles;
      for (sentence::predicates::const_iterator p=s->get_predicates().begin(); p!=s->get_predicates().end(); ++p) {
        if (p->has_argument(mpos)) {
          wstring role = p->get_argument_by_pos(mpos).get_role();
          roles.push_back(make_pair(p->get_position(),role));
        }
      }
      roles.sort();
      return roles;
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
      list<pair<int,wstring>> roles = get_arguments(m.get_sentence(), m.get_head().get_position());
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
        num = fex._Morf.get_person(m.get_head().get_lemma());
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
   
  ////////////////////////////////////////////////////
  ///  If given mention is the subject of a reporting verb, returns position of the verb
  ///  Otherwise, returns -1
  ////////////////////////////////////////////////////  

  int relaxcor_fex_dep::subj_reporting(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":SUBJ_REPORTING";

    int verb = -1;
    if (fcache.computed_feature(fid)) {
      verb = util::wstring2int(fcache.get_feature(fid));    
      TRACE(6,L"     " << fid << L" = " << verb << "  (cached)");      
    }
    else {
      list<pair<int,wstring> > args = util::wstring2pairlist<int,wstring>(get_arguments(m,fcache,fex),L":",L"/");

      bool found = false;
      paragraph::const_iterator s=m.get_sentence();
      for (list<pair<int,wstring> >::const_iterator p=args.begin(); p!=args.end() and not found; ++p) {        
        if (fex.get_label_RE(L"FUN_Arg0").search(p->second)) {
          int pred = p->first;
          wstring word = (*s)[pred].get_lemma();
          if (fex.get_label_RE(L"WRD_Reporting").search(word)) {
            found = true;
            verb = pred;
          }
        }
      }

      fcache.set_feature(fid, util::int2wstring(verb));
      TRACE(6,L"     " << fid << L" = " << verb);
    }

    return verb;
  }

  ////////////////////////////////////////////////////
  ///  If given mention is inside the object of a reporting verb, returns position of the verb
  ///  Otherwise, returns -1
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::obj_reporting(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = util::int2wstring(m.get_id())+L":OBJ_REPORTING";

    /////////////  ################   ATENCIO  !!!!!!!!!!!!!!
    /////////////  ################
    /////////////  ################  Canviar util.h per poder fer wstring_from i wstring_to de 
    /////////////  ################  sets<X> on X!=wstring
    /////////////  ################
    /////////////  ################

    set<int> verbs;
    if (fcache.computed_feature(fid)) {
      wstring fval = fcache.get_feature(fid);
      set<wstring> sverbs = util::wstring_to<set<wstring>>(fval,L","); 
      for (auto v : sverbs) verbs.insert(util::wstring2int(v));
      TRACE(6,L"     " << fid << L" = " << fval << "  (cached)");      
    }
    else {
      paragraph::const_iterator s = m.get_sentence();
      dep_tree::const_iterator n = s->get_dep_tree().get_node_by_pos(m.get_head().get_position());
      bool top = false;
      while (not top) {
        int mpos = n->get_word().get_position();
        list<pair<int,wstring>> roles = get_arguments(s, mpos); 
        for (list<pair<int,wstring> >::const_iterator p=roles.begin(); p!=roles.end(); ++p) {        
          if (fex.get_label_RE(L"FUN_Arg1").search(p->second)) {
            int pred = p->first;
            wstring word = (*s)[pred].get_lemma();
            if (fex.get_label_RE(L"WRD_Reporting").search(word)) 
              verbs.insert(pred);
          }
        }
          
        if (n.is_root()) top=true;
        else n = n.get_parent();
      }
     
      set<wstring> sverbs;
      for (auto v : verbs) sverbs.insert(util::int2wstring(v));

      wstring fval = util::wstring_from<set<wstring>>(sverbs,L",");
      fcache.set_feature(fid, fval);
      TRACE(6,L"     " << fid << L" = " << fval);
    }

    return verbs;
  }


  //////////////////////////////////////////////////////////////////
  ///    Computes and caches the distance in mentions
  //////////////////////////////////////////////////////////////////

  int relaxcor_fex_dep::dist_mentions(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":DIST_MEN";
    int res;
    if (fcache.computed_feature(fid)) {
      res = util::wstring2int(fcache.get_feature(fid));    
      TRACE(6,L"   " << fid << L" = " << res << L"  (cached)");
    }
    else {  
      res = abs(m1.get_id() - m2.get_id()) - 1;
      fcache.set_feature(fid, util::int2wstring(res));
      TRACE(6,L"   " << fid << L" = " << res);
    }
    return res;
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


  //////////////////////////////////////////////////////////////////
  ///    Returns whether m2 head is contained in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::relaxed_head_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":RELAXED_HEAD_MATCH";

    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      r = false;
      wstring head = m2.get_head().get_lc_form();
      for(sentence::const_iterator it=m1.get_it_begin(); it!=m1.get_it_end() and not r; it++)
        r = (it->get_lc_form()==head);
      
      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any modifier (with PoS 'label') of m2 is also in m1.
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::inclusion_match(const mention &m1, const mention &m2, const freeling::regexp &re) {
      dep_tree::const_iterator t;
      
      // get non-stopword modifiers of m1 head
      set<wstring> mod1;
      t = m1.get_dtree().begin();
      for (dep_tree::const_sibling_iterator c=t.sibling_begin(); c!=t.sibling_end(); ++c) {
        if (re.search(c->get_word().get_tag())) 
          mod1.insert(c->get_word().get_lemma());
      }
      
      // r is true as long as we don't find a m2 modifier that is not also in m1.
      bool r = true;
      t = m2.get_dtree().begin();
      for (dep_tree::const_sibling_iterator c=t.sibling_begin(); c!=t.sibling_end() and r; ++c) {
        if (re.search(c->get_word().get_tag())) 
          r = (mod1.find(c->get_word().get_lemma())!=mod1.end());
      }

      return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any numerical modifier of m2 is also in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::num_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":NUM_MATCH";

    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      r =  relaxcor_fex_dep::inclusion_match(m1,m2,fex.get_label_RE(L"TAG_Number"));
      
      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return r;
  }



  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any non-stop-word modifier of m2 is also in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::word_inclusion(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":WORD_INCLUSION";

    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      r =  relaxcor_fex_dep::inclusion_match(m1,m2,fex.get_label_RE(L"TAG_NonStopWord"));
      
      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return r;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any noun-or-adj modifier of m2 is also in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::compatible_mods(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":COMPATIBLE_MODS";

    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      r =  relaxcor_fex_dep::inclusion_match(m1,m2,fex.get_label_RE(L"TAG_NounAdj"));
      
      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return r;
  }


  //// ######################  REGISTERED FEATURE FUNCTIONS ###########################

  ///    Returns whether the distance in sentences is 1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_sentences_0(const mention &m1, const mention &m2, 
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_sentences(m1,m2,fcache,fex)==0 ? ff_YES : ff_NO);
  }
  ///    Returns whether the distance in sentences is 2
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_sentences_1(const mention &m1, const mention &m2, 
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_sentences(m1,m2,fcache,fex)==1 ? ff_YES : ff_NO);
  }
  ///    Returns whether the distance in sentences is <=3
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_sentences_le3(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_sentences(m1,m2,fcache,fex)<=3 ? ff_YES : ff_NO);
  }
  ///    Returns whether the number of mentions between m1 and m2 is 0
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_mentions_0(const mention &m1, const mention &m2, 
                                                                    feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_mentions(m1,m2,fcache,fex)==0 ? ff_YES : ff_NO);
  }
  ///    Returns whether the number of mentions between m1 and m2 is <=3
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_mentions_le3(const mention &m1, const mention &m2, 
                                                                      feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_mentions(m1,m2,fcache,fex)<=3 ? ff_YES : ff_NO);
  }
  ///    Returns whether the number of mentions between m1 and m2 is <=9
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::dist_mentions_le9(const mention &m1, const mention &m2, 
                                                                      feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (dist_mentions(m1,m2,fcache,fex)<=9 ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 1 is of type P
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_P(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::PRONOUN) ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 1 is of type S
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_S(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::NOUN_PHRASE) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is of type E
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_E(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::PROPER_NOUN) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is of type C
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_type_C(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.is_type(mention::COMPOSITE) ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 2 is of type P
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_P(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::PRONOUN) ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 2 is of type S
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_S(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::NOUN_PHRASE) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is of type E
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_E(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::PROPER_NOUN) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is of type C
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_type_C(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::COMPOSITE) ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 1 is a definite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_definite_NP(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (definite(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is an indefinite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_indefinite_NP(const mention &m1, const mention &m2,
                                                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (indefinite(m1,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 2 is a definite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_definite_NP(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (definite(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is an indefinite NP
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_indefinite_NP(const mention &m1, const mention &m2,
                                                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (indefinite(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is a relative pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_relative(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (relative_pronoun(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is a relative pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_relative(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (relative_pronoun(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is a reflexive pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_reflexive(const mention &m1, const mention &m2,
                                                                        feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (reflexive_pronoun(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is a reflexive pronoun
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_reflexive(const mention &m1, const mention &m2,
                                                                        feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (reflexive_pronoun(m2,fcache,fex) ? ff_YES : ff_NO);
  }
   ///    Returns whether mention 1 is a possessive
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_possessive(const mention &m1, const mention &m2,
                                                                         feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (possessive_determiner(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is a possessive
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_possessive(const mention &m1, const mention &m2,
                                                                        feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (possessive_determiner(m2,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is "I"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_I(const mention &m1, const mention &m2,
                                                                feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L'S',L'1',L's',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is "I"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_I(const mention &m1, const mention &m2,
                                                                feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L'S',L'1',L's',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is "you"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_you(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L'S',L'2',L'0',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is "you"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_you(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L'S',L'2',L'0',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is "we"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_we(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L'S',L'1',L'p',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is "we"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_we(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L'S',L'1',L'p',fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is singular
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_singular(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_number(m1,fcache,fex)==L's' ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is singular
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_singular(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_number(m2,fcache,fex)==L's' ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is 3rd person
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_3pers(const mention &m1, const mention &m2,
                                                                    feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_person(m1,fcache,fex)==L'3' ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is 3rd person
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_3pers(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_person(m2,fcache,fex)==L'3' ? ff_YES : ff_NO);
  }
  ///    Returns whether both mentions have the same person
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::same_person(const mention &m1, const mention &m2,
                                                                feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_person(m1,fcache,fex)==fex.get_person(m2,fcache,fex) ? ff_YES : ff_NO);
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
      // consider "unk" as "yes" for proper nouns
      if (numagr==L"unk" and (m1.is_type(mention::PROPER_NOUN) or m1.is_type(mention::PROPER_NOUN)))
        numagr = L"yes";

      wchar_t g1 = get_gender(m1,fcache,fex);
      wchar_t g2 = get_gender(m2,fcache,fex);
      wstring genagr = morph_features::compatible_gender(g1,g2);

      if (numagr==L"no" or genagr==L"no")
        ag = L"no";
      else if (numagr==L"unk" and genagr==L"unk") 
        ag = L"unk";
      else 
        ag = L"yes";


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


  ///    Returns whether mention 1 is inside quotes
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_quotes(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (in_quotes(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is inside quotes
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

  ///    Returns whether mention 1 is embedded in another
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_embedded(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (not m1.is_maximal() ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is embedded in another
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_embedded(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (not m2.is_maximal() ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is a noun modifier
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_nmod(const mention &m1, const mention &m2,
                                                                   feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (is_noun_modifier(m1,fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is a noun modifier
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_nmod(const mention &m1, const mention &m2,
                                                                   feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (is_noun_modifier(m2,fcache,fex) ? ff_YES : ff_NO);
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
  ///    Returns whether both mentions are identical
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_match_strict(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":STR_MATCH_STRICT";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      // get m1 value with lowercased first word (unless it is a proper noun)
      wstring dd1;
      if (fex.get_label_RE(L"TAG_ProperNoun").search(m1.get_it_begin()->get_tag())) dd1 = m1.value();
      else dd1 = m1.value(1);    
      // get m2 value with lowercased first word (unless it is a proper noun)
      wstring dd2;
      if (fex.get_label_RE(L"TAG_ProperNoun").search(m2.get_it_begin()->get_tag())) dd2 = m2.value();
      else dd2 = m2.value(1);
    
      r = not m1.is_type(mention::PRONOUN) and not m2.is_type(mention::PRONOUN) and dd1==dd2;

      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return (r ? ff_YES : ff_NO);
}

  //////////////////////////////////////////////////////////////////
  ///    Returns whether both mentions are identical up to the head
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_match_relaxed(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":STR_MATCH_RELAXED";
    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      sentence::const_iterator w;
      sentence::const_iterator wend;

      // get m1 first word lowercased (unless it is a proper noun)
      wstring dd1;        
      w = m1.get_it_begin();
      if (fex.get_label_RE(L"TAG_ProperNoun").search(w->get_tag())) dd1 = w->get_form();
      else dd1 = w->get_lc_form();
      // add other words up to head
      wend = m1.get_it_head(); ++wend;
      for (++w; w!=wend; ++w) dd1 += L" " + w->get_form();

      // get m2 first word lowercased (unless it is a proper noun)
      wstring dd2;        
      w = m2.get_it_begin();
      if (fex.get_label_RE(L"TAG_ProperNoun").search(w->get_tag())) dd2 = w->get_form();
      else dd2 = w->get_lc_form();
      // add other words up to head
      wend = m2.get_it_head(); ++wend;
      for (++w; w!=wend; ++w) dd2 += L" " + w->get_form();
    
      r = not m1.is_type(mention::PRONOUN) and not m2.is_type(mention::PRONOUN) and dd1==dd2;

      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return (r ? ff_YES : ff_NO);
}

  //////////////////////////////////////////////////////////////////
  ///    Returns whether both mentions heads are identical
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_head_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = util::int2wstring(m1.get_id())+L":"+util::int2wstring(m2.get_id())+L":STR_HEAD_MATCH";

    bool r;
    if (fcache.computed_feature(fid)) {
      r = util::wstring2int(fcache.get_feature(fid));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no") << "  (cached)");
    }
    else {
      r = not m1.is_type(mention::PRONOUN) and not m2.is_type(mention::PRONOUN) and  m1.get_head().get_lc_form()==m2.get_head().get_lc_form();

      fcache.set_feature(fid, util::int2wstring(r));
      TRACE(6,L"   " << fid << L" = " << (r?L"yes":L"no"));
    }

    return (r ? ff_YES : ff_NO);
  }


  ///    Returns whether m2 head is contained in m1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::relaxed_head_match_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (relaxed_head_match(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether m1 head is contained in m2
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::relaxed_head_match_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (relaxed_head_match(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }
  
  ///    Returns whether any numerical modifier of m2 is also in m1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::num_match_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (num_match(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether any numerical modifier of m1 is also in m2
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::num_match_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (num_match(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }
  
  ///    Returns whether all non-stop words modifying m2 are also in m1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::word_inclusion_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (word_inclusion(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether all non-stop words modifying m2 are also in m1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::word_inclusion_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (word_inclusion(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether all noun or adjs modifying m2 are also in m1
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::compatible_mods_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (compatible_mods(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether all noun or adjs modifying m1 are also in m2
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::compatible_mods_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (compatible_mods(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 1 is subject of a reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_subj_reporting(const mention &m1, const mention &m2,
                                                                             feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (subj_reporting(m1,fcache,fex)>=0 ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is subject of a reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_subj_reporting(const mention &m1, const mention &m2,
                                                                             feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (subj_reporting(m2,fcache,fex)>=0 ? ff_YES : ff_NO);
  }

  ///    Returns whether mention 1 is inside the object of a reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_obj_reporting(const mention &m1, const mention &m2,
                                                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (not obj_reporting(m1,fcache,fex).empty() ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is inside the object of a reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_obj_reporting(const mention &m1, const mention &m2,
                                                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (not obj_reporting(m2,fcache,fex).empty() ? ff_YES : ff_NO);
  }

  ///    Returns whether both mentions are inside the object of the same reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::obj_same_reporting(const mention &m1, const mention &m2,
                                                                       feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m1.get_n_sentence()==m2.get_n_sentence() and obj_reporting(m1,fcache,fex)==obj_reporting(m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether m1 is inside the subject and m2 inside the object of the same reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::subj_obj_reporting_ij(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    set<int> obj = obj_reporting(m2,fcache,fex);
    return (m1.get_n_sentence()==m2.get_n_sentence() and obj.find(subj_reporting(m1,fcache,fex))!=obj.end() ? ff_YES : ff_NO);
  }
  ///    Returns whether m2 is inside the subject and m1 inside the object of the same reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::subj_obj_reporting_ji(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    set<int> obj = obj_reporting(m1,fcache,fex);
    return (m1.get_n_sentence()==m2.get_n_sentence() and obj.find(subj_reporting(m2,fcache,fex))!=obj.end() ? ff_YES : ff_NO);
  }


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
