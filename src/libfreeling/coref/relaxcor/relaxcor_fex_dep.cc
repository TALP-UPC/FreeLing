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
#include <cstdlib> // abs
#include <ctime>
#include <climits> // INT_MAX

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

  const freeling::regexp relaxcor_fex_dep::re_Acronym(L"^[A-Z]+(\\&)?[A-Z]+$");
  const freeling::regexp relaxcor_fex_dep::re_EMPTY(L"^$");

  //////////////////////////////////////////////////////////////////
  /// Constructor. Sets defaults
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::relaxcor_fex_dep(const wstring &filename, const relaxcor_model &m) : relaxcor_fex_abs(m), _Morf(filename) {

    // read configuration file and store information       
    enum sections {SEM_DB, SENSESELECTION, DEPLABELS, POSTAGS, WORDFEAT, SEMFEAT, RESOURCES};
    config_file cfg(true,L"%");

    // add compulsory sections
    cfg.add_section(L"SemDB",SEM_DB,true);
    cfg.add_section(L"SenseSelection",SENSESELECTION,true);
    cfg.add_section(L"DepLabels",DEPLABELS,true);
    cfg.add_section(L"PosTags",POSTAGS,true);
    cfg.add_section(L"WordFeatures",WORDFEAT,true);
    cfg.add_section(L"SemanticFeatures",SEMFEAT,true);
    cfg.add_section(L"Resources",RESOURCES,false);

    if (not cfg.open(filename))
      ERROR_CRASH(L"Error opening file "+filename);

    wstring path=filename.substr(0,filename.find_last_of(L"/\\")+1);
    wstring line;
    while (cfg.get_content_line(line)) {

      wistringstream sin;
      sin.str(line);

      switch (cfg.get_section()) {
      case SEM_DB: {
        // load SEMDB section           
        wstring fname;
        sin>>fname;
        wstring sdbf= util::absolute(fname,path);
        _Semdb= new semanticDB(sdbf);
        break;
      }

      case SENSESELECTION: {
	wstring name;
        sin >> name;
        if (name == L"MinPageRank") sin >> _MinPageRank;
        else if (name == L"PRAccumWeight") sin >> _PRAccumWeight;
        else if (name == L"MinSenses") sin >> _MinSenses;
        else {
          WARNING("Ignoring unknown key '"<<name<<"' in <SenseSelection> section of file "<<filename);
        }
        break;
      }

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
	
      case SEMFEAT: {
        // Read regexs to identify labels for PoS tags
	wstring name, val;
        sin >> name >> val;
	freeling::regexp re(val);
	_Labels.insert(make_pair(L"SEM_"+name,re));
	break;
      }

      case RESOURCES: {
        // Read regexs to identify labels for PoS tags
	wstring name;
	wstring fname;
        sin >> name >> fname;
        wstring fabs = util::absolute(fname,path);
        if (name==L"PersonTitles") util::file2map<wstring,wchar_t>(fabs, _PersonTitles); 
        else if (name==L"PersonNames") util::file2map<wstring,wchar_t>(fabs, _PersonNames); 
        else {
          WARNING(L"Ignored unknown resource "<< name << " in file "<<filename);
        }
          
	break;
      }

      default: break;
      }
    }
    // close config file
    cfg.close();


    // create regexp matching no tag (used as default)
    freeling::regexp re(L"^$");
    _Labels.insert(make_pair(L"TAG_NONE",re));
    
    // register implemented feature functionrs
    register_features();

    // check whether all model features are implemented
    for (auto f=model.begin_features(); f!=model.end_features(); ++f) {
      if (_FeatureFunction.find(f->first)==_FeatureFunction.end())
        WARNING(L"Requested feature "<<f->first<<L" not implemented. It will be ignored.");
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
    _FeatureFunction[L"RCF_I_IT"] = make_pair(&relaxcor_fex_dep::mention_1_it, ff_YES);
    _FeatureFunction[L"RCF_J_IT"] = make_pair(&relaxcor_fex_dep::mention_2_it, ff_YES);

    _FeatureFunction[L"RCF_I_NUMBER_SG"] = make_pair(&relaxcor_fex_dep::mention_1_singular, ff_YES);
    _FeatureFunction[L"RCF_J_NUMBER_SG"] = make_pair(&relaxcor_fex_dep::mention_2_singular, ff_YES);
    _FeatureFunction[L"RCF_I_NUMBER_PL"] = make_pair(&relaxcor_fex_dep::mention_1_plural, ff_YES);
    _FeatureFunction[L"RCF_J_NUMBER_PL"] = make_pair(&relaxcor_fex_dep::mention_2_plural, ff_YES);
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
    _FeatureFunction[L"RCF_STR_MATCH_RELAXED_LEFT"] = make_pair(&relaxcor_fex_dep::str_match_relaxed_left, ff_YES);
    _FeatureFunction[L"RCF_STR_MATCH_RELAXED_RIGHT"] = make_pair(&relaxcor_fex_dep::str_match_relaxed_right, ff_YES);

    _FeatureFunction[L"RCF_HEAD_MATCH"] = make_pair(&relaxcor_fex_dep::str_head_match, ff_YES);
    _FeatureFunction[L"RCF_RELAXED_HEAD_MATCH_IJ"] = make_pair(&relaxcor_fex_dep::relaxed_head_match_ij, ff_YES);
    _FeatureFunction[L"RCF_RELAXED_HEAD_MATCH_JI"] = make_pair(&relaxcor_fex_dep::relaxed_head_match_ji, ff_YES);
    _FeatureFunction[L"RCF_NAME_MATCH_IJ"] = make_pair(&relaxcor_fex_dep::name_match_ij, ff_YES);
    _FeatureFunction[L"RCF_NAME_MATCH_JI"] = make_pair(&relaxcor_fex_dep::name_match_ji, ff_YES);
    _FeatureFunction[L"RCF_PRON_MATCH"] = make_pair(&relaxcor_fex_dep::str_pron_match, ff_YES);

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
    _FeatureFunction[L"RCF_SUBJ_OBJ_SAME_VERB_IJ"] = make_pair(&relaxcor_fex_dep::subj_obj_same_verb_ij, ff_YES);
    _FeatureFunction[L"RCF_SUBJ_OBJ_SAME_VERB_JI"] = make_pair(&relaxcor_fex_dep::subj_obj_same_verb_ji, ff_YES);

    _FeatureFunction[L"RCF_ACRONYM"] = make_pair(&relaxcor_fex_dep::acronym, ff_YES);
    _FeatureFunction[L"RCF_SAME_SEMCLASS_YES"] = make_pair(&relaxcor_fex_dep::same_semclass, ff_YES);
    _FeatureFunction[L"RCF_SAME_SEMCLASS_NO"] = make_pair(&relaxcor_fex_dep::same_semclass, ff_NO);
    _FeatureFunction[L"RCF_SAME_SEMCLASS_UNK"] = make_pair(&relaxcor_fex_dep::same_semclass, ff_UNK);

    #ifdef VERBOSE
    TRACE(3,L"Available features")
      for (auto f : _FeatureFunction)
        TRACE(3,L"    " << f.first);
    #endif
  }



  //////////////////////////////////////////////////////////////////
  /// Auxiliary to handle regex map
  /////////////////////////////////////////////////////////
  
  freeling::regexp relaxcor_fex_dep::get_label_RE(const wstring &key) const {
    auto re = _Labels.find(key);
    if (re==_Labels.end()) {
      WARNING(L"No regex defined for label "<<key<<L". Ignoring affected features");
      return re_EMPTY;
    }
    return re->second;
  }

  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention is inside quotes
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::in_quotes(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":IN_QUOTES";
    bool inq;
    if (fcache.get_bool_feature(fid,inq)) {
      TRACE(7,L"     " << fid << L" = " << wstring(inq?L"yes":L"no") << "  (cached)");
    }
    else {
      paragraph::const_iterator s = m.get_sentence();
      sentence::const_reverse_iterator b(m.get_it_begin());
      int best = m.get_sentence()->get_best_seq();
      int nq = 0;
      while (b!=s->rend()) {
        if (b->get_tag(best)==L"Fe" or b->get_tag(best)==L"Fra" or b->get_tag(best)==L"Frc") ++nq;
        ++b;
      }

      // if there is an (non zer) odd number of quotes to the left of the mention first word, 
      // we are inside quotes.
      inq = (nq%2!=0);
      fcache.set_feature(fid,inq);
      TRACE(7,L"     " << fid << L" = "<< (inq?L"yes":L"no"));
    }

    return inq;
  }


  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention is a definite noun phrase
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::definite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":DEFINITE";
    bool def;
    if (fcache.get_bool_feature(fid,def)) {
      TRACE(7,L"     " << fid << L" = " << (def?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the first mention word is in the list of definite determiners
      int best = m.get_sentence()->get_best_seq();
      def = fex.get_label_RE(L"WRD_Definite").search(m.get_it_begin()->get_lemma(best));

      fcache.set_feature(fid,def);
      TRACE(7,L"     " << fid << L" = "<< (def?L"yes":L"no"));
    }

    return def;
  }

  /////////////////////////////////////////////////////////////////////////////
  ///   check whether the mention is indefinite noun phrase
  /////////////////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::indefinite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":INDEFINITE";
    bool ind;
    if (fcache.get_bool_feature(fid,ind)) {
      TRACE(7,L"     " << fid << L" = " << (ind?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the head or first are in the list of indefinite pronouns/adjectives
      int best = m.get_sentence()->get_best_seq();
      ind = fex._Morf.has_type(m.get_head().get_lemma(best),L"I") or
            fex._Morf.has_type(m.get_it_begin()->get_lemma(best),L"I");
      
      fcache.set_feature(fid,ind);
      TRACE(7,L"     " << fid << L" = "<< (ind?L"yes":L"no"));
    }

    return ind;
  }

  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if the mention is a relative pronoun
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_dep::relative_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":RELATIVE";
    bool rp;
    if (fcache.get_bool_feature(fid,rp)) {
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a relative pronoun
      int best = m.get_sentence()->get_best_seq();
      rp = fex.get_label_RE(L"TAG_RelPron").search(m.get_head().get_tag(best));

      fcache.set_feature(fid,rp);
      TRACE(7,L"     " << fid << L" = "<< (rp?L"yes":L"no"));
    }

    return rp;
  }

  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if the mention is a reflexive pronoun
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_dep::reflexive_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":REFLEXIVE";
    bool rp;
    if (fcache.get_bool_feature(fid,rp)) {
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a reflexive pronoun
      int best = m.get_sentence()->get_best_seq();
      rp = fex._Morf.has_type(m.get_head().get_lemma(best),L"R");
      fcache.set_feature(fid,rp);
      TRACE(7,L"     " << fid << L" = "<< (rp?L"yes":L"no"));
    }
    return rp;
  }

  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if the mention is a possessive determiner
  /////////////////////////////////////////////////////////////////////////////  

  bool relaxcor_fex_dep::possessive_determiner(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":POSSESSIVE";
    bool rp;
    if (fcache.get_bool_feature(fid,rp)) {
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a possessive
      int best = m.get_sentence()->get_best_seq();
      rp = (fex.get_label_RE(L"TAG_Poss").search(m.get_head().get_tag(best)) and 
            fex._Morf.has_type(m.get_head().get_lemma(best),L"P"));
      fcache.set_feature(fid,rp);
      TRACE(7,L"     " << fid << L" = "<< (rp?L"yes":L"no"));
    }

    return rp;
  }

  ///////////////////////////////////////////////////////////////////////////// 
  ///    Returns true if the mention is a noun modifier in the dep tree
  ///////////////////////////////////////////////////////////////////////////// 

  bool relaxcor_fex_dep::is_noun_modifier(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = m.get_str_id()+L":NMOD";
    bool rp;
    if (fcache.get_bool_feature(fid,rp)) {
      TRACE(7,L"     " << fid << L" = " << (rp?L"yes":L"no") << "  (cached)");
    }
    else {
      // check whether the mention is a reflexive pronoun
      rp = fex.get_label_RE(L"FUN_NounModifier").search(m.get_dtree()->get_label());
      fcache.set_feature(fid,rp);
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
    wstring fid = m.get_str_id()+L":ARGUMENTS";
    wstring args;
    if (fcache.get_str_feature(fid,args)) {
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
  
  bool relaxcor_fex_dep::match_pronoun_features(const mention &m, const wstring &type, const wstring &pgn, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":PRONFEAT:"+type+L"/"+pgn;
    bool pf;
    if (fcache.get_bool_feature(fid,pf)) {
      TRACE(6,L"     " << fid << L" = " << (pf?L"yes":L"no") << "  (cached)");
    }
    else {
      wchar_t per = pgn[0];
      wchar_t gen = pgn[1];
      wchar_t num = pgn[2];
      wstring w=m.get_head().get_lc_form();
      pf = (type==L"-" or fex._Morf.has_type(w,type))
        and (per==L'-' or fex._Morf.get_person(w)==per) 
        and (gen==L'-' or morph_features::compatible_gender(fex._Morf.get_gender(w), gen)==ff_YES)
        and (num==L'-' or morph_features::compatible_number(fex._Morf.get_number(w), num)==ff_YES);

      fcache.set_feature(fid, pf);
      TRACE(6,L"     " << fid << L" = " << (pf?L"yes":L"no"));
    }
    return pf;
  }

  ////////////////////////////////////////////////////
  ///   Computes number for given mention
  ////////////////////////////////////////////////////  

  wchar_t relaxcor_fex_dep::get_number(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    wstring fid = m.get_str_id()+L":NUMBER";

    wstring morf;
    wchar_t num=L'u';
    if (fcache.get_str_feature(fid,morf)) {
      num = morf[0];
      TRACE(6,L"     " << fid << L" = " << num << "  (cached)");      
    }
    else {
      if (m.is_type(mention::PRONOUN)) {
        // check in pronWords list. 
        num = fex._Morf.get_number(m.get_head().get_lc_form());
        // not found in list, set to unknown
        if (num==L'#') num=L'u';
      }
      else if (m.is_type(mention::COMPOSITE)) {
        // if it is a coordination, is plural
        num = L'p';
      }
      else if (m.is_type(mention::NOUN_PHRASE)) {
        // if it is a noun_phrase, check head PoS tag
        int best = m.get_sentence()->get_best_seq();
        if (fex.get_label_RE(L"TAG_NounSg").search(m.get_head().get_tag(best))) num=L's';
        else if (fex.get_label_RE(L"TAG_NounPl").search(m.get_head().get_tag(best))) num=L'p';
        else num=L'u';
      }
      else if (m.is_type(mention::PROPER_NOUN)) {
        int best = m.get_sentence()->get_best_seq();
        // if it is a proper noun, ORGs are '0', and the others 'u'
        if (fex.get_label_RE(L"TAG_OrganizationNE").search(m.get_head().get_tag(best))) num=L'0';
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
    wstring fid = m.get_str_id()+L":GENDER";
    wchar_t gen=L'u';
    wstring morf;
    if (fcache.get_str_feature(fid,morf)) {
      gen = morf[0];
      TRACE(6,L"     " << fid << L" = " << gen << "  (cached)");      
    }
    else {
      if (m.is_type(mention::PRONOUN)) {
        // check in pronWords list. 
        gen = fex._Morf.get_gender(m.get_head().get_lc_form());
        // not found in list, leave as unknown
        if (gen==L'#') gen=L'u';
      }
 
      else if (m.is_type(mention::PROPER_NOUN)) {
        int best = m.get_sentence()->get_best_seq(); 

	vector<wstring> wds = util::wstring2vector(m.get_head().get_form(),L"_");
	int pos = 0;

	map<wstring,wchar_t>::const_iterator pT = fex._PersonTitles.find(wds[pos]);
	// first word is a title 
	if (pT != fex._PersonTitles.end()) {
	  // if the title has unambiguous gender (e.g. Mr., Duchess, Sir, Madam, etc), that's it.
	  if (pT->second != L'a') gen = pT->second;       
	  // title with ambiguous gender (e.g. President, Dr.), advance to second word (if it exists)
	  else if (wds.size() > 1) ++pos;
	}
	
	// gender still not solved (no title, or ambiguous title), check for 
	// person name ('pos' is either the first word or the word after an ambiguous title)
	if (gen == L'u') {
	  map<wstring,wchar_t>::const_iterator pN = fex._PersonNames.find(wds[pos]);
	  // name found in list as non-ambiguos
	  if (pN != fex._PersonNames.end() and pN->second != L'a') 
            gen = pN->second;          
	  // name found but ambiguous, or not found but NEC says it is a person -> gen = 'b'
	  else if (pN != fex._PersonNames.end() or
                   fex.get_label_RE(L"TAG_PersonNE").search(m.get_head().get_tag(best)))
            gen = L'b';
	}
      }

      else if (m.is_type(mention::COMPOSITE)) 
	gen = L'u';
            
      else if (m.is_type(mention::NOUN_PHRASE)) {
        // gender unknown, unless we find evidence otherwise
        gen=L'u';

        // if it is a noun_phrase, check head PoS tag,
        int best = m.get_sentence()->get_best_seq();
        if (fex.get_label_RE(L"TAG_NounMasc").search(m.get_head().get_tag(best))) gen=L'm';
        else if (fex.get_label_RE(L"TAG_NounFem").search(m.get_head().get_tag(best))) gen=L'f';

        // PoS tag had no gender information (e.g. English), check SUMO for semantic information
        else {
          wstring sense;
          int best = m.get_sentence()->get_best_seq();
          const list<pair<wstring,double>> &ls = m.get_head().get_senses(best);

	  gen = L'n';
          if (not ls.empty()) {

            TRACE(7, L"    - semantic gender for '"<< m.get_head().get_form() << L"'");
            // Use as class that provided by sense ranked highest by UKB that has a class

            // compute sum of page rank weigths for all senses
            double wsum = 0;
            for (auto s : ls) {
              TRACE(7, L"       "<<s.first<<" "<<s.second);
              wsum += s.second;
            } 
            TRACE(7, L"       wsum="<<wsum);
            // accumulated weight sum of senses seen so far
            double accsum = 0;
            
            // check senses, from most likely to less, stopping when no SenseSelection criteria holds
            int ns = 0;
            auto s = ls.begin();
            while  (s!=ls.end() and gen==L'n' and
                    (accsum/wsum < fex._PRAccumWeight or ns<fex._MinSenses or s->second>=fex._MinPageRank)) {
              accsum += s->second;
              sense_info si = fex._Semdb->get_sense_info(s->first);
              TRACE(7, L"      sense="<< s->first << L" (" << s->second << L") sumo="<< si.sumo << " tonto=" <<util::list2wstring(si.tonto,L":") <<" accsum/wsum="<<accsum/wsum);
              if (fex.get_label_RE(L"SEM_MaleSUMO").search(si.sumo)) gen = L'm';
              else if (fex.get_label_RE(L"SEM_FemaleSUMO").search(si.sumo)) gen = L'f';
              else if (fex.get_label_RE(L"SEM_PersonSUMO").search(si.sumo)) gen = L'b';
	      else {
		wstring topont = util::list2wstring(si.tonto,L":");
		TRACE(7,L"      TOPONT="<<topont);
		TRACE(7,L"        PersonTONTO (human)"<<fex.get_label_RE(L"SEM_PersonTONTO").search(topont));
		TRACE(7,L"        NotPersonTONTO (group)"<<fex.get_label_RE(L"SEM_NotPersonTONTO").search(topont));
		if (fex.get_label_RE(L"SEM_PersonTONTO").search(topont)
		    and not fex.get_label_RE(L"SEM_NotPersonTONTO").search(topont))
		  gen = L'b';
	      }
              ++ns;
              ++s;
            }

          }
        }
      }

      fcache.set_feature(fid, wstring(1,gen));
      TRACE(6,L"     " << fid << L" = " << gen);
    }

    return gen;
  }
   
  ////////////////////////////////////////////////////
  ///   Computes person for given mention
  ////////////////////////////////////////////////////  

  wchar_t relaxcor_fex_dep::get_person(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":PERSON";

    wchar_t per=L'3';
    wstring morf;
    if (fcache.get_str_feature(fid,morf)) {
      per = morf[0];
      TRACE(6,L"     " << fid << L" = " << per << "  (cached)");      
    }
    else {
      if (m.is_type(mention::PRONOUN)) {
        // check in pronWords list. 
        per = fex._Morf.get_person(m.get_head().get_lc_form());
        // not found in list, set to 3rd
        if (per==L'#') per=L'3';
      }
      else // all other mentions are 3rd.
        per = L'3';

      fcache.set_feature(fid, wstring(1,per));
      TRACE(6,L"     " << fid << L" = " << per);
    }

    return per;
  }

  ////////////////////////////////////////////////////
  ///  Return list of positions of verbs for which m is the argument defined by 're'
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::is_arg_of(const mention &m, const freeling::regexp &re) {
    set<int> verbs;
    paragraph::const_iterator s = m.get_sentence();
    int mpos = m.get_head().get_position();
    list<pair<int,wstring> > args = get_arguments(s, mpos);        
    for (list<pair<int,wstring> >::const_iterator p=args.begin(); p!=args.end(); ++p) {
      if (re.search(p->second)) 
        verbs.insert(p->first);
    }
    return verbs;
  }

  ////////////////////////////////////////////////////
  ///  Return list of positions of verbs for which m inside the argument defined by 're'
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::inside_arg_of(const mention &m, const freeling::regexp &re) {
    set<int> verbs;
    paragraph::const_iterator s = m.get_sentence();
    int best = s->get_best_seq();
    dep_tree::const_iterator n = s->get_dep_tree(best).get_node_by_pos(m.get_head().get_position());
    bool top = false;
    while (not top) {
      int mpos = n->get_word().get_position();
      list<pair<int,wstring>> args = get_arguments(s, mpos); 
      for (list<pair<int,wstring> >::const_iterator p=args.begin(); p!=args.end(); ++p) {
        if (re.search(p->second)) 
          verbs.insert(p->first);
      }      
      if (n.is_root()) top=true;
      else n = n.get_parent();
    }
    return verbs;
  }


  ////////////////////////////////////////////////////
  ///  Obtain verbs of which given mention is subject 
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::is_subj_of(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":SUBJECT_OF";

    set<int> verbs;
    wstring fval;
    if (fcache.get_str_feature(fid,fval)) {
      verbs = util::wstring_to<set<int>,int>(fval,L",");    
      TRACE(6,L"     " << fid << L" = [" << fval << "]  (cached)");      
    }
    else {
      verbs = is_arg_of(m, fex.get_label_RE(L"FUN_Arg0"));
      fval = util::wstring_from<set<int>>(verbs,L",");
      fcache.set_feature(fid, fval);
      TRACE(6,L"     " << fid << L" = [" << fval << L"]");
    }

    return verbs;
  }

  ////////////////////////////////////////////////////
  ///  Obtain verbs of which given mention is any non-A0 object
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::is_obj_of(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":OBJECT_OF";

    set<int> verbs;
    wstring fval;
    if (fcache.get_str_feature(fid,fval)) {
      verbs = util::wstring_to<set<int>,int>(fval,L",");    
      TRACE(6,L"     " << fid << L" = [" << fval << "]  (cached)");      
    }
    else {
      verbs = is_arg_of(m, fex.get_label_RE(L"FUN_Arg123"));
      fval = util::wstring_from<set<int>>(verbs,L",");
      fcache.set_feature(fid, fval);
      TRACE(6,L"     " << fid << L" = [" << fval << L"]");
    }

    return verbs;
  }

  ////////////////////////////////////////////////////
  ///  Obtain verbs of which given mention i is inside object
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::inside_obj_of(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":INSIDE_OBJECT_OF";

    set<int> verbs;
    wstring fval;
    if (fcache.get_str_feature(fid,fval)) {
      verbs = util::wstring_to<set<int>,int>(fval,L",");    
      TRACE(6,L"     " << fid << L" = [" << fval << "]  (cached)");      
    }
    else {
      verbs = inside_arg_of(m, fex.get_label_RE(L"FUN_Arg1"));
      fval = util::wstring_from<set<int>>(verbs,L",");
      fcache.set_feature(fid, fval);
      TRACE(6,L"     " << fid << L" = [" << fval << L"]");
    }

    return verbs;
  }


  ////////////////////////////////////////////////////
  ///  Given a sentence, a list of positions, and a regexp, return a list only with the positions whose lemma matches regex
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::select_by_lemma(paragraph::const_iterator s, const set<int> &pos, const freeling::regexp &re) {
    set<int> res;
    for (auto v=pos.begin(); v!=pos.end(); ++v) {
      int best = s->get_best_seq();
      if (re.search((*s)[*v].get_lemma(best))) 
        res.insert(*v);
    }
    return res;
  }  

   
  ////////////////////////////////////////////////////
  ///  Returns positions of reporting verbs the given mention is the subject of
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::subj_reporting(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":SUBJ_REPORTING";

    set<int> verbs;
    wstring fval;
    if (fcache.get_str_feature(fid,fval)) {
      verbs = util::wstring_to<set<int>,int>(fval,L",");    
      TRACE(6,L"     " << fid << L" = [" << fval << "]  (cached)");      
    }
    else {
      set<int> verbs = is_subj_of(m, fcache, fex);
      verbs = select_by_lemma(m.get_sentence(), verbs, fex.get_label_RE(L"WRD_Reporting"));

      fval = util::wstring_from<set<int>>(verbs,L",");
      fcache.set_feature(fid, fval);
      TRACE(6,L"     " << fid << L" = [" << fval << L"]");
    }

    return verbs;
  }

  ////////////////////////////////////////////////////
  ///  Returns positions of reporting verbs the given mention is inside the object of
  ////////////////////////////////////////////////////  

  set<int> relaxcor_fex_dep::obj_reporting(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":OBJ_REPORTING";

    set<int> verbs;
    wstring fval;
    if (fcache.get_str_feature(fid,fval)) {
      verbs = util::wstring_to<set<int>,int>(fval,L",");    
      TRACE(6,L"     " << fid << L" = [" << fval << "]  (cached)");      
    }
    else {
      set<int> verbs = inside_obj_of(m, fcache, fex);
      verbs = select_by_lemma(m.get_sentence(), verbs, fex.get_label_RE(L"WRD_Reporting"));

      fval = util::wstring_from<set<int>>(verbs,L",");
      fcache.set_feature(fid, fval);
      TRACE(6,L"     " << fid << L" = [" << fval << L"]");
    }

    return verbs;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether m1 is the subject and m2 inside the object of the same reporting verb
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::subj_obj_reporting(const mention &m1, const mention &m2,
                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {

    bool res = false;
    if (m1.get_n_sentence()==m2.get_n_sentence()) {
      set<int> sub1 = subj_reporting(m1,fcache,fex);
      set<int> obj2 = obj_reporting(m2,fcache,fex);
      for (auto v=sub1.begin(); v!=sub1.end() and not res; ++v) 
        res = (obj2.find(*v)!=obj2.end());
    }
    
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id()<<L":SUBJ_OBJ_REPORTING" << L" = " << (res ? L"yes" : L"no"));
    return res;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether m1 is the subject and m2 is the object of the same verb
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::subj_obj_same_verb(const mention &m1, const mention &m2,
                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {

    bool res = false;
    if (m1.get_n_sentence()==m2.get_n_sentence()) {
      set<int> sub1 = is_subj_of(m1,fcache,fex);
      set<int> obj2 = is_obj_of(m2,fcache,fex);
      for (auto v=sub1.begin(); v!=sub1.end() and not res; ++v) 
        res = (obj2.find(*v)!=obj2.end());
    }
    
    TRACE(6,L"   " <<  m1.get_id()<<L":"<<m2.get_id()<<L":SUBJ_OBJ_SAME_VERB" << L" = " << (res ? L"yes" : L"no"));
    return res;
  }

  //////////////////////////////////////////////////////////////////
  ///    Computes and caches the distance in mentions
  //////////////////////////////////////////////////////////////////

  int relaxcor_fex_dep::dist_mentions(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    int res = abs(m1.get_id() - m2.get_id()) - 1;
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id()<<L":DIST_MEN" << L" = " << res); 
    return res;
  }

  //////////////////////////////////////////////////////////////////
  ///    Computes and caches the distance in sentences
  //////////////////////////////////////////////////////////////////

  int relaxcor_fex_dep::dist_sentences(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    int res = abs(m1.get_n_sentence() - m2.get_n_sentence());
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id()<< L":DIST_SENTENCES" << L" = " << res);
    return res;
  }


  ////////////////////////////////////////////////////
  ///    Returns whether m1 is nested in m2
  ////////////////////////////////////////////////////

  bool relaxcor_fex_dep::nested(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    bool r = m1.get_n_sentence() == m2.get_n_sentence() and
            ((m2.get_pos_begin()<m1.get_pos_begin() and m2.get_pos_end()>=m1.get_pos_end()) or
            (m2.get_pos_begin()<=m1.get_pos_begin() and m2.get_pos_end()>m1.get_pos_end()));
    
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id()<< L":NESTED"<< L" = " << (r?L"yes":L"no"));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns the distance in number of phrases between m1 and m2
  //////////////////////////////////////////////////////////////////

  int relaxcor_fex_dep::dist_in_phrases(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    if (m1.get_n_sentence() != m2.get_n_sentence()) return INT_MAX;
    if (nested(m1,m2,fcache,fex) or nested(m2,m1,fcache,fex)) return 0;
      
    list<pair<int,wstring> > args1 = util::wstring2pairlist<int,wstring>(get_arguments(m1,fcache,fex),L":",L"/");
    list<pair<int,wstring> > args2 = util::wstring2pairlist<int,wstring>(get_arguments(m2,fcache,fex),L":",L"/");
    if (args1.empty() or args2.empty()) return INT_MAX;
      
    unsigned int mindif = INT_MAX;
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
     
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() <<L":DIST_PHRASES" << L" = " << mindif);
    return mindif;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether m1 is the SBJ and m2 is PRD of a copulative verb
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::predicative(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    // m1 is SBJ and m2 is PRD of a copulative verb (or viceversa), and they are not possessive
    int best1 = m1.get_sentence()->get_best_seq();
    int best2 = m2.get_sentence()->get_best_seq();

    bool r = ( not fex._Morf.has_type(m1.get_head().get_lemma(best1),L"P") and  // not possessives 
               not fex._Morf.has_type(m2.get_head().get_lemma(best2),L"P") and
               fex.get_label_RE(L"FUN_Subject").search(m1.get_dtree()->get_label()) and
               fex.get_label_RE(L"FUN_Predicate").search(m2.get_dtree()->get_label())  and
               m1.get_dtree().get_parent() == m2.get_dtree().get_parent() and
               fex.get_label_RE(L"WRD_Copulative").search(m1.get_dtree().get_parent()->get_word().get_lemma(best1))
               );

    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id()<<L":PREDICATIVE" << L" = " << (r?L"yes":L"no"));
    return r;
  } 


  //////////////////////////////////////////////////////////////////
  ///    Returns whether m2 head is contained in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::relaxed_head_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool r = false;
    wstring head = m2.get_head().get_lc_form();
    for(sentence::const_iterator it=m1.get_it_begin(); it!=m1.get_it_end() and not r; ++it)
      r = (it->get_lc_form()==head);
      
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id()<<L":RELAXED_HEAD_MATCH" << L" = " << (r?L"yes":L"no"));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///  Returns whether m1 and m2 are NE and the last word in m2 head is contained in m1 head, 
  ///  and there are no stop words before it (e.g. it matches "President_Putin" and "Putin", 
  ///  but not "President_Putin_of_Russia" and "Russia".
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::name_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool r = false;
    if (m1.is_type(mention::PROPER_NOUN) and m2.is_type(mention::PROPER_NOUN)) {
      vector<wstring> w1 = util::wstring2vector(m1.get_head().get_lc_form(),L"_");
      vector<wstring> w2 = util::wstring2vector(m2.get_head().get_lc_form(),L"_");
      wstring last = w2[w2.size()-1];

      for (auto w=w1.begin(); w!=w1.end() and not r and not fex.get_label_RE(L"WRD_AcronymInfix").search(*w); ++w)
        r = (*w)==last;
    }
      
    TRACE(6,L"   " <<  m1.get_id()<<L":"<<m2.get_id()<<L":NAME_MATCH"<< L" = " << (r?L"yes":L"no"));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any modifier (with PoS 'label') of m2 is also in m1, even if they are under a token with PoS 'under'
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::inclusion_match(const mention &m1, const mention &m2, const freeling::regexp &re, const freeling::regexp &under) {

    dep_tree::const_iterator t;
    int best1 = m1.get_sentence()->get_best_seq();
    int best2 = m2.get_sentence()->get_best_seq();
      
    // get non-stopword modifiers of m1 head
    set<wstring> mod1;
    t = m1.get_dtree().begin();
    for (dep_tree::const_sibling_iterator c=t.sibling_begin(); c!=t.sibling_end(); ++c) {
      if (re.search(c->get_word().get_tag(best1)))    // if the child has the "re" PoS, it is a modifier
        mod1.insert(c->get_word().get_lemma(best1));
      else if (under.search(c->get_word().get_tag(best1))) {   // otherwise, if it has the "under" PoS (e.g. preposition), check its first child
	if (re.search(c.sibling_begin()->get_word().get_tag(best1)))  // if the first child of the preposition has the "re" pos, get it as modifier
	  mod1.insert(c.sibling_begin()->get_word().get_tag(best1));
      }
    }
      
    // r is true as long as we don't find a m2 modifier that is not also in m1.
    bool r = true;
    t = m2.get_dtree().begin();
    for (dep_tree::const_sibling_iterator c=t.sibling_begin(); c!=t.sibling_end() and r; ++c) {
      if (re.search(c->get_word().get_tag(best2)))                    
        r = (mod1.find(c->get_word().get_lemma(best2))!=mod1.end());  // if the child has the "re" pos, check it directly.
	
      else if (under.search(c->get_word().get_tag(best2))) {   // if the child has the "under" PoS (e.g. preposition)
        if (c.sibling_begin() != c.sibling_end()) // and the preposition has a child (it should but who knows which sentence we'd get...)
          if (re.search(c.sibling_begin()->get_word().get_tag(best2)))  // and the the first child of the preposition has the "re" pos
            r = (mod1.find(c.sibling_begin()->get_word().get_tag(best2))!=mod1.end());  // check it was also in m1.
      }
    }

    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any numerical modifier of m2 is also in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::num_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool r =  relaxcor_fex_dep::inclusion_match(m1,m2,fex.get_label_RE(L"TAG_Number"),fex.get_label_RE(L"TAG_NONE"));
    TRACE(6,L"   "  <<  m1.get_id()<<L":"<<m2.get_id() <<L":NUM_MATCH" << L" = " << (r?L"yes":L"no"));
    return r;
  }



  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any non-stop-word modifier of m2 is also in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::word_inclusion(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool r =  relaxcor_fex_dep::inclusion_match(m1,m2,fex.get_label_RE(L"TAG_NonStopWord"),fex.get_label_RE(L"TAG_NONE"));
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() <<L":WORD_INCLUSION" << L" = " << (r?L"yes":L"no"));
    return r;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns true iff any noun-or-adj modifier of m2 is also in m1
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::compatible_mods(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool r =  relaxcor_fex_dep::inclusion_match(m1,m2, fex.get_label_RE(L"TAG_NounAdj"), fex.get_label_RE(L"TAG_Preposition"));      
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() <<L":COMPATIBLE_MODS" << L" = " << (r?L"yes":L"no"));
    return r;
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns true iff letters in m1 match initials of words in m2
  ///    assumes that m1 is an acronym (w.g. "IBM", "S&P", "C_&_A", "ABC_&_DE")
  //////////////////////////////////////////////////////////////////

  bool relaxcor_fex_dep::initial_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool acr = false;
    wstring f1 = m1.value();
    // get acronym without underscores or dots
    util::find_and_replace(f1,L"_",L"");
    util::find_and_replace(f1,L".",L"");

    vector<wstring> w2 = util::wstring2vector(m2.get_head().get_lc_form(),L"_");

    // is m1 is not an acronym, or it has more letters than words in m2, don't bother
    if (re_Acronym.search(f1) and f1.length() <= w2.size()) {

      f1 = util::lowercase(f1);
      int i=0;
      vector<wstring>::const_iterator w = w2.begin();
      acr = true;
      while (i<f1.length() and w!=w2.end() and acr) {
        // check if ith word in m2 starts with ith char in m1 (skipping stopwords)     
        if (not fex.get_label_RE(L"WRD_AcronymInfix").search(*w)) {
          acr = (*w)[0]==f1[i];
          ++i;
        }
        ++w;
      }

      // if all matches, but there are words left in m2, check they are valid stopwords followed by valid suffixes
      // (e.g. to accept when m2 is "Incredibly Blooming Margerites & Co", and m1 is just "IBM")
      if (acr and w!=w2.end()) {
        // consume stop words and suffix words, remembering whether the last seen was a suffix
        bool suf = true;
        while (w!=w2.end() and (fex.get_label_RE(L"WRD_AcronymInfix").search(*w) or suf)) {
          acr = fex.get_label_RE(L"WRD_AcronymSuffix").search(*w);
          ++w;
        }

        // if we consumed all the words and the last one is a suffix, we are good
        acr = (w==w2.end() and suf);
      }
    }

    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() << L":ACRONYM" << L" = " << (acr?L"yes":L"no"));
    return acr;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns the semantic class of the mention
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TSemanticClass relaxcor_fex_dep::get_semantic_class(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring fid = m.get_str_id()+L":SEMANTIC_CLASS";
    TSemanticClass sc;
    int fval;
    if (fcache.get_int_feature(fid,fval)) {
      sc = (TSemanticClass) fval;
      TRACE(6,L"   " << fid << L" = " << sc_string(sc) << "  (cached)");
    }
    else {
      sc = sc_UNK;
      if (m.is_type(mention::PROPER_NOUN)) {
        int best = m.get_sentence()->get_best_seq();
        if (fex.get_label_RE(L"TAG_OrganizationNE").search(m.get_head().get_tag(best))) sc = sc_ORG;
        else if (fex.get_label_RE(L"TAG_PersonNE").search(m.get_head().get_tag(best))) sc = sc_PER;
        else if (fex.get_label_RE(L"TAG_LocationNE").search(m.get_head().get_tag(best))) sc = sc_LOC;
      }

      else if (m.is_type(mention::PRONOUN)) {
        // check in pronWords list. 
        wchar_t h = fex._Morf.get_human(m.get_head().get_lc_form());
        if (h==L'h') sc = sc_PER;
        else if (h==L'n') sc = sc_NONPER;
      }

      else if (m.is_type(mention::NOUN_PHRASE)) {
        // get SUMO information for word sense, and use it to decide semantic class
        wstring sense;
        int best = m.get_sentence()->get_best_seq();
        const list<pair<wstring,double>> &ls = m.get_head().get_senses(best);
        if (not ls.empty()) {
          TRACE(7, L"    - semantic class for '"<< m.get_head().get_form() << L"'");
          // compute sum of page rank weigths for all senses
          double wsum = 0;
          for (auto s : ls) {
            TRACE(7, L"       "<<s.first<<" "<<s.second);
            wsum += s.second;
          } 
          // accumulated weight sum of senses seen so far
          double accsum = 0;

          // check senses, from most likely to less, stopping when no SenseSelection criteria holds
          // Use as class that provided by sense ranked highest by UKB that has a class
          int ns = 0 ;
          auto s = ls.begin();
          while  (s!=ls.end() and sc==sc_UNK and
                  (accsum/wsum < fex._PRAccumWeight or ns<fex._MinSenses or s->second>=fex._MinPageRank)) {
            accsum += s->second;
            sense_info si = fex._Semdb->get_sense_info(s->first);
            TRACE(7, L"      sense="<< s->first << L" (" << s->second << L") sumo="<< si.sumo << " accsum/wsum="<<accsum/wsum);
            if (fex.get_label_RE(L"SEM_OrganizationSUMO").search(si.sumo)) sc = sc_ORG;
            else if (fex.get_label_RE(L"SEM_PersonSUMO").search(si.sumo)) sc = sc_PER;
            else if (fex.get_label_RE(L"SEM_LocationSUMO").search(si.sumo)) sc = sc_LOC;

            ++ns;
            ++s;
          }
        }
        else {
          TRACE(7,"    - semantic class for '"<< m.get_head().get_form() << L"'. Not found in WN.");
        }
      }

      fcache.set_feature(fid, sc);
      TRACE(6,L"   " << fid << L" = " <<  sc_string(sc));
    }

    return sc;
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
    return (match_pronoun_features(m1,L"-",L"1-s",fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is "I"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_I(const mention &m1, const mention &m2,
                                                                feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L"-",L"1-s",fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is "you"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_you(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L"-",L"2-0",fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is "you"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_you(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L"-",L"2-0",fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is "we"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_we(const mention &m1, const mention &m2,
                                                                 feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L"-",L"1-p",fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is "we"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_we(const mention &m1, const mention &m2,
                                                                 feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L"-",L"1-p",fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 1 is "it"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_it(const mention &m1, const mention &m2,
                                                                 feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m1,L"SOPR",L"3ns",fcache,fex) ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is "it"
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_it(const mention &m1, const mention &m2,
                                                                 feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (match_pronoun_features(m2,L"SOPR",L"3ns",fcache,fex) ? ff_YES : ff_NO);
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
  ///    Returns whether mention 1 is plural
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_1_plural(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_number(m1,fcache,fex)==L'p' ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is singular
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_plural(const mention &m1, const mention &m2,
                                                                     feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (fex.get_number(m2,fcache,fex)==L'p' ? ff_YES : ff_NO);
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
    wstring fid = m1.get_str_id()+L":"+m2.get_str_id()+L":AGREEMENT";
    TFeatureValue ag;
    int fval;
    if (fcache.get_int_feature(fid,fval)) {
      ag = (TFeatureValue) fval;
      TRACE(6,L"   " << fid << L" = " << ff_string(ag) << "  (cached)");
    }
    else {
      wchar_t n1 = get_number(m1,fcache,fex);   
      wchar_t n2 = get_number(m2,fcache,fex); 
      TFeatureValue numagr = morph_features::compatible_number(n1,n2);
      // consider "unk" as "yes" for proper nouns
      if (numagr==ff_UNK and (m1.is_type(mention::PROPER_NOUN) or m1.is_type(mention::PROPER_NOUN)))
        numagr = ff_YES;

      wchar_t g1 = get_gender(m1,fcache,fex);
      wchar_t g2 = get_gender(m2,fcache,fex);
      TFeatureValue genagr = morph_features::compatible_gender(g1,g2);

      if (numagr==ff_NO or genagr==ff_NO)
        ag = ff_NO;
      else if (numagr==ff_UNK and genagr==ff_UNK) 
        ag = ff_UNK;
      else 
        ag = ff_YES;

      fcache.set_feature(fid, ag);
      TRACE(6,L"   " << fid << L" = " <<  ff_string(ag));
    }

    return ag;
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns "yes" if m2 is the closest agreeing referent for m1
  ///   WARNING:  it assumes that the pairs m1:mx (where mx is 
  ///            between m1 and m2) have been already processed
  ///            and uses the cache to retrieve the info.
  ///             It also assumes that the feature "agreement" has been computed
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::closest_agreement(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    wstring pair = m1.get_str_id()+L":"+m2.get_str_id();
    wstring fid = pair+L":CLOSEST_AGREEMENT";
    TFeatureValue ag;

    // complain if agreement was not activated
    int fval;
    if (not fcache.get_int_feature(pair+L":AGREEMENT",fval)) {
      WARNING(L"RCF_C_AGREEMENT feature requires RCF_AGREEMENT to be also activated.");
      return ff_NO;
    }

    ag = (TFeatureValue) fval;
    // check whether m1 and m2 agree
    if (ag == ff_YES) {
      // if they agree, check closestness
      bool found=false;
      for (int mx=m1.get_id()+1; mx<m2.get_id() and not found; ++mx) {
        wstring fid2 = m1.get_str_id()+L":"+std::to_wstring(mx)+L":AGREEMENT";
        if (not fcache.get_int_feature(fid2,fval)) {
          WARNING(L"Feature "<<fid2<<" should be computed before attempting to compute "<<fid);
          return ff_NO;
        }
        TFeatureValue ag2 = (TFeatureValue) fval;
        TRACE(6,L"     " << fid2 << L" = " <<  ff_string(ag2));
        found = (ag2==ff_YES);
      }          
      if (found) ag = ff_NO;
    }

    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() <<L":CLOSEST_AGREEMENT"<< L" = " <<  ff_string(ag));
    

    return ag;
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

    bool sq = false;
    if (m1.get_n_sentence()==m2.get_n_sentence()) {
      // check for quotes between m1 and m2. If none is found, they are in the same quotation (if any)
      int best = m1.get_sentence()->get_best_seq();
      sq = in_quotes(m1,fcache,fex) and in_quotes(m2,fcache,fex);

      //  if they are nested, they are in the same quote. No need to check anything else. Otherwise, check
      if (nested_mentions(m1,m2,fcache,fex)==ff_NO) {
        for (sentence::const_iterator k=m1.get_it_end(); k!=m2.get_it_begin() and sq; ++k ) {
          sq = (k->get_tag(best)==L"Fe" or k->get_tag(best)==L"Fra" or k->get_tag(best)==L"Frc");
        }
      }
    }
    
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() <<L":SAME_QUOTE" << L" = " << (sq?L"yes":L"no"));
    return (sq ? ff_YES : ff_NO);
  }


  //////////////////////////////////////////////////////////////////
  ///    Returns whether m2 is apposition of m1
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::apposition(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    // m2 is child of m1 with label "APPO" or else, m1 and m2 are siblings and consecutive
    bool r = (m2.get_dtree().get_parent()==m1.get_dtree() and fex.get_label_RE(L"FUN_Apposition").search(m2.get_dtree()->get_label())) 
      or (m1.get_dtree().get_parent()==m2.get_dtree() and m1.get_pos_end()+1 == m2.get_pos_begin());
    
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() <<L":APPOSITION" << L" = " << (r?L"yes":L"no"));
    return (r ? ff_YES : ff_NO);
  } 
  
  

  //////////////////////////////////////////////////////////////////
  ///    Returns whether m1 is relative pronoun and m1 its antecedent
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::rel_antecedent(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {

    bool r = false;
    int best2 = m2.get_sentence()->get_best_seq();
 
    // m2 must be relative pronoun
    if (fex.get_label_RE(L"TAG_RelPron").search(m2.get_head().get_tag(best2)) and not m2.get_dtree().is_root() ) {

      // locate verbal head and modifier 
      dep_tree::const_iterator vhead;
      dep_tree::const_iterator vmod;
      bool headok = false;
      
      // m2 is and child (SBJ or OBJ) of a verb that is NMOD of m1.
      if (fex.get_label_RE(L"TAG_Verb").search(m2.get_dtree().get_parent()->get_word().get_tag(best2))) {
        vmod = m2.get_dtree();
        vhead = vmod.get_parent();
        headok = true;
      }
      
      // or either, m2 is and child of a preposition that is child (SBJ or OBJ) of a verb that is NMOD of m1
      else if (fex.get_label_RE(L"TAG_Preposition").search(m2.get_dtree().get_parent()->get_word().get_tag(best2)) and
               not m2.get_dtree().get_parent().is_root() and
               fex.get_label_RE(L"TAG_Verb").search(m2.get_dtree().get_parent().get_parent()->get_word().get_tag(best2)) ) {
        vmod = m2.get_dtree().get_parent();
        vhead = vmod.get_parent();
        headok = true;
      }

      r = headok and
        (fex.get_label_RE(L"FUN_Subject").search(vmod->get_label()) or
         fex.get_label_RE(L"FUN_Object").search(vmod->get_label())) and 
        not vhead.is_root() and  
        fex.get_label_RE(L"FUN_NounModifier").search(vhead->get_label()) and 
        vhead.get_parent() == m1.get_dtree();
    }

    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() <<L":REL_ANTECEDENT" << L" = " << (r?L"yes":L"no"));
    return (r ? ff_YES : ff_NO);
  } 


  //////////////////////////////////////////////////////////////////
  ///    Returns whether both mentions are identical
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_match_strict(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    // get m1 value with lowercased first word (unless it is a proper noun)
    wstring dd1;
    int best1 = m1.get_sentence()->get_best_seq();
    if (fex.get_label_RE(L"TAG_ProperNoun").search(m1.get_it_begin()->get_tag(best1))) dd1 = m1.value();
    else dd1 = m1.value(1);    
    // get m2 value with lowercased first word (unless it is a proper noun)
    wstring dd2;
    int best2 = m2.get_sentence()->get_best_seq();
    if (fex.get_label_RE(L"TAG_ProperNoun").search(m2.get_it_begin()->get_tag(best2))) dd2 = m2.value();
    else dd2 = m2.value(1);
    
    bool r = not m1.is_type(mention::PRONOUN) and not m2.is_type(mention::PRONOUN) and dd1==dd2;
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() << L":STR_MATCH_STRICT"<< L" = " << (r?L"yes":L"no"));
    return (r ? ff_YES : ff_NO);
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether both mentions are identical from the start up to the head
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_match_relaxed_left(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    sentence::const_iterator w;
    sentence::const_iterator wend;

    // get m1 first word lowercased (unless it is a proper noun)
    wstring dd1;        
    int best1 = m1.get_sentence()->get_best_seq();
    w = m1.get_it_begin();
    if (fex.get_label_RE(L"TAG_ProperNoun").search(w->get_tag(best1))) dd1 = w->get_form();
    else dd1 = w->get_lc_form();
    // add other words up to head
    wend = m1.get_it_head(); ++wend;
    for (++w; w!=wend; ++w) dd1 += L" " + w->get_form();

    // get m2 first word lowercased (unless it is a proper noun)
    wstring dd2;        
    int best2 = m2.get_sentence()->get_best_seq();
    w = m2.get_it_begin();
    if (fex.get_label_RE(L"TAG_ProperNoun").search(w->get_tag(best2))) dd2 = w->get_form();
    else dd2 = w->get_lc_form();
    // add other words up to head
    wend = m2.get_it_head(); ++wend;
    for (++w; w!=wend; ++w) dd2 += L" " + w->get_form();
    
    bool r = not m1.is_type(mention::PRONOUN) and not m2.is_type(mention::PRONOUN) and dd1==dd2;
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() << L":STR_MATCH_RELAXED_LEFT" << L" = " << (r?L"yes":L"no"));
    return (r ? ff_YES : ff_NO);
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether both mentions are identical from the head to the end
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_match_relaxed_right(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    sentence::const_iterator w;
    sentence::const_iterator wend;

    // get m1 words from head to end
    wstring dd1;        
    for (w = m1.get_it_head(); w != m1.get_it_end(); ++w) dd1 += L" " + w->get_form();

    // get m2 words from head to end
    wstring dd2;        
    for (w = m2.get_it_head(); w != m2.get_it_end(); ++w) dd2 += L" " + w->get_form();

    // check if they are the same
    bool r = not m1.is_type(mention::PRONOUN) and not m2.is_type(mention::PRONOUN) and dd1==dd2;
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() << L":STR_MATCH_RELAXED_RIGHT" << L" = " << (r?L"yes":L"no"));
    return (r ? ff_YES : ff_NO);
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether both mentions heads are non-pronouns and identical
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_head_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool r = not m1.is_type(mention::PRONOUN) and not m2.is_type(mention::PRONOUN) and  m1.get_head().get_lc_form()==m2.get_head().get_lc_form();
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() << L":STR_HEAD_MATCH" << L" = " << (r?L"yes":L"no"));
    return (r ? ff_YES : ff_NO);
  }

  //////////////////////////////////////////////////////////////////
  ///    Returns whether both mentions heads are pronouns and identical
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::str_pron_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    bool r = m1.is_type(mention::PRONOUN) and m2.is_type(mention::PRONOUN) and  m1.get_head().get_lc_form()==m2.get_head().get_lc_form();
    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() << L":STR_PRON_MATCH" << L" = " << (r?L"yes":L"no"));
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
  
  ///    Returns whether m1 and m2 are NE and last word in m2 head is contained in m1 head
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::name_match_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (name_match(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether m1 and m2 are NE and last word in m1 head is contained in m2 head
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::name_match_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (name_match(m2,m1,fcache,fex) ? ff_YES : ff_NO);
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
    return (not subj_reporting(m1,fcache,fex).empty() ? ff_YES : ff_NO);
  }
  ///    Returns whether mention 2 is subject of a reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::mention_2_subj_reporting(const mention &m1, const mention &m2,
                                                                             feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (not subj_reporting(m2,fcache,fex).empty() ? ff_YES : ff_NO);
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
    bool res = false;
    if (m1.get_n_sentence()==m2.get_n_sentence()) {
      set<int> obj1 = obj_reporting(m1,fcache,fex);
      set<int> obj2 = obj_reporting(m2,fcache,fex);
      for (auto v=obj1.begin(); v!=obj1.end() and not res; ++v) 
        res = (obj2.find(*v)!=obj2.end());
    }

    TRACE(6,L"   " << m1.get_id()<<L":"<<m2.get_id() << L":OBJ_SAME_REPORTING"<< L" = " << (res ? L"yes" : L"no"));
    return (res ? ff_YES : ff_NO);
  }

  ///    Returns whether m1 is the subject and m2 inside the object of the same reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::subj_obj_reporting_ij(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (subj_obj_reporting(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether m2 is the subject and m1 inside the object of the same reporting verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::subj_obj_reporting_ji(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (subj_obj_reporting(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }


  ///    Returns whether m1 is the subject and m2 is the object of the same verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::subj_obj_same_verb_ij(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (subj_obj_same_verb(m1,m2,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether m2 is the subject and m1 is the object of the same verb
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::subj_obj_same_verb_ji(const mention &m1, const mention &m2,
                                                                          feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (subj_obj_same_verb(m2,m1,fcache,fex) ? ff_YES : ff_NO);
  }

  ///    Returns whether one mention is acronyim of the other
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::acronym(const mention &m1, const mention &m2,
                                                            feature_cache &fcache, const relaxcor_fex_dep &fex) {
    return (m2.is_type(mention::PROPER_NOUN) and m2.is_type(mention::PROPER_NOUN) and (initial_match(m1,m2,fcache,fex) or initial_match(m2,m1,fcache,fex))? ff_YES : ff_NO);
  }

  ///    Returns whether both mentions belong to the same semantic class (person, location, organization)
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::same_semclass(const mention &m1, const mention &m2,
                                                                  feature_cache &fcache, const relaxcor_fex_dep &fex) {
    TSemanticClass sc1 = get_semantic_class(m1, fcache, fex);
    TSemanticClass sc2 = get_semantic_class(m2, fcache, fex);

    TFeatureValue sclass;
    if (sc1==sc_UNK or sc2==sc_UNK) sclass = ff_UNK;
    else if (sc1==sc_NONPER and sc2!=sc_PER) sclass = ff_UNK;
    else if (sc2==sc_NONPER and sc1!=sc_PER) sclass = ff_UNK;
    else if (sc1==sc2) sclass = ff_YES;
    else sclass = ff_NO;

    TRACE(6,L"   " <<  m1.get_id()<<L":"<<m2.get_id() << L":SAME_SEMCLASS"<< L" = " <<  ff_string(sclass));
    return sclass;
  }


  ////////////////////////////////////////////////////////////////// 
  ///    Extract the configured features for all mentions  
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_abs::Mfeatures relaxcor_fex_dep::extract(const vector<mention> &mentions) const {
    relaxcor_fex_abs::Mfeatures M;

    feature_cache fcache;
    
    for (int m2=1; m2<mentions.size(); ++m2) {
      int best2 = mentions[m2].get_sentence()->get_best_seq();

      TRACE(4,L"Extracting all pairs for mention "<<mentions[m2].get_id()<<L" ("+mentions[m2].value()<<L") ["<<mentions[m2].get_head().get_form()<<L","<<mentions[m2].get_head().get_lemma(best2)<<L","<<mentions[m2].get_head().get_tag(best2)<<L"]");
      
      for (int m1=m2-1; m1>=0; --m1) {
	TRACE(5,L"PAIR: "<<mentions[m1].get_id()<<L":"<<mentions[m2].get_id()<<L" "+mentions[m1].get_head().get_form()<<L":"<<mentions[m2].get_head().get_form()<<L" ["<<mentions[m1].value()<<"]:["<<mentions[m2].value()<<"]");	

        // feature tables are stored with key m2:m1 for compatibility with relaxcor and other extractors.
	wstring mention_pair = mentions[m2].get_str_id() + L":" + mentions[m1].get_str_id();
        //        extract_pair(mentions[m1], mentions[m2], fcache, M[mention_pair]);

        int count = 0;
        for (auto f=model.begin_features(); f!=model.end_features(); ++f) {
          wstring fname = f->first;
          TRACE(6,L"   Extracting feature "<<fname);
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
	wstring name, type, val;
        sin >> name >> type >> val;
        _PronFeats.insert(make_pair(name, val));
        _PronTypes.insert(make_pair(name, type));
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
    auto p = _PronFeats.find(w);
    wchar_t res = L'#';
    if (p!=_PronFeats.end()) res = p->second[k];
    return res;
  }


  /// --------------------------------------------------------
  /// get type, and see if it is one of the given in t

  bool relaxcor_fex_dep::morph_features::has_type(const wstring &w, const wstring &t) const { 
    auto p = _PronTypes.find(w);
    // not in the list, no type to check. 
    if (p==_PronTypes.end()) 
      return false; 
    // check all types in t and see if the word has any of them
    for (int i=0; i<t.size(); ++i)
      if (p->second.find(t[i]) != wstring::npos) 
        return true;
    // no searched types found
    return false;
  }

  // get morhpological atributes
  wchar_t relaxcor_fex_dep::morph_features::get_human(const wstring &w) const { return get_feature(w,0); }
  wchar_t relaxcor_fex_dep::morph_features::get_person(const wstring &w) const { return get_feature(w,1); }
  wchar_t relaxcor_fex_dep::morph_features::get_gender(const wstring &w) const { return get_feature(w,2); }
  wchar_t relaxcor_fex_dep::morph_features::get_number(const wstring &w) const { return get_feature(w,3); }
  
  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::morph_features::compatible_number(wchar_t n1, wchar_t n2) {
    // number is compatible if they match or at least one of them is underspecified
    relaxcor_fex_dep::TFeatureValue cn;
    if (n1==L'0' or
        n2==L'0' or
        (n1==n2 and n1!=L'u'))
      cn = ff_YES;
    else if (n1==L'u' or n2==L'u') 
      cn = ff_UNK;
    else 
      cn = ff_NO;
    TRACE(7,L"      - compatible number " << n1 << L":" << n2 << L"=" <<  ff_string(cn));
    return cn;
  }

  relaxcor_fex_dep::TFeatureValue relaxcor_fex_dep::morph_features::compatible_gender(wchar_t g1, wchar_t g2) {
    // gender is compatible if they match, at least one of them is underspecified, or
    // one is M/F and the other is M or F
    relaxcor_fex_dep::TFeatureValue cg;
    if (g1==L'0' or 
        g2==L'0' or
        (g1==g2 and g1!=L'u') or
        (g1==L'b' and (g2==L'm' or g2==L'f')) or
        (g2==L'b' and (g1==L'm' or g1==L'f')))
      cg = ff_YES;
    else if (g1==L'u' or g2==L'u') 
      cg = ff_UNK;
    else 
      cg = ff_NO;
    TRACE(7,L"      - compatible gender " << g1 << L":" << g2 << L"=" <<  ff_string(cg));
    return cg;
  }
  
} // namespace
