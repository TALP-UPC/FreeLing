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
//
//    Author: Jordi Turmo
//    e-mail: turmo@lsi.upc.edu
//
//    This is an implementation based on the PhD Thesis of Emili Sapena
//
//////////////////////////////////////////////////////////////////

#ifndef RELAXCOR_FEX_DEP_H
#define RELAXCOR_FEX_DEP_H

#include <list>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "freeling/morfo/language.h"
#include "freeling/morfo/semdb.h"
#include "freeling/morfo/tagset.h"
#include "freeling/morfo/relaxcor_fex_abs.h"

namespace freeling {

  //////////////////////////////////////////////////////////////////
  ///    Class for the feature extractor.
  //////////////////////////////////////////////////////////////////

  class relaxcor_fex_dep : public relaxcor_fex_abs {
  public:
    relaxcor_fex_dep(const std::wstring&, const relaxcor_model &);
    ~relaxcor_fex_dep();
    relaxcor_fex_abs::Mfeatures extract(const std::vector<mention>&) const;    

  private:

    /// regexps from config file
    std::map<std::wstring, wchar_t> _PersonTitles;
    std::map<std::wstring, wchar_t> _PersonNames;
    std::map<std::wstring, freeling::regexp> _Labels;
    static const freeling::regexp re_EMPTY;
    static const freeling::regexp re_Acronym;

    freeling::regexp get_label_RE(const std::wstring &) const;

    // access to WN and SUMO information
    freeling::semanticDB *_Semdb;

    /// the following parametes are combined disjunctively: A sense is valid when any of these criteria accept it
    /// (They may be used separately setting a disabling value 0 for two of them)

    // minimum PgRank UKB value to accept a sense as valid. (1: no sense is valid, 0: all senses are valid)
    double _MinPageRank;
    // maximum Accumulated PR percenteage to accept a sense as valid. (-1: no sense is valid, 1: all senses are valid)
    double _PRAccumWeight;
    // minimum absolute number of senses to check (0: no sense is valid, 1000: all senses are valid)
    int _MinSenses;
    
    // mention semantic classes
    typedef enum {sc_PER, sc_ORG, sc_LOC, sc_NONPER, sc_UNK} TSemanticClass;

    // map of feature names to feature functions
    typedef enum {ff_YES,ff_NO,ff_UNK} TFeatureValue;
    typedef TFeatureValue (*TFeatureFunction)(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    std::map<std::wstring, std::pair<TFeatureFunction,TFeatureValue>> _FeatureFunction;
    void register_features();

    void extract_pair(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;

    /// mention pair features
    static TFeatureValue dist_sentences_0(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue dist_sentences_1(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue dist_sentences_le3(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue dist_mentions_0(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue dist_mentions_le3(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue dist_mentions_le9(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue agreement(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue closest_agreement(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue same_quote(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue apposition(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue rel_antecedent(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static TFeatureValue mention_1_type_P(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_type_S(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_type_C(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_type_E(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_type_P(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_type_S(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_type_C(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_type_E(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_definite_NP(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_definite_NP(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_indefinite_NP(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_indefinite_NP(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_relative(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_relative(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_reflexive(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_reflexive(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_possessive(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_possessive(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_I(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_I(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_you(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_you(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_we(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_we(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_it(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_it(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_singular(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_singular(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_plural(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_plural(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_3pers(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_3pers(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue same_person(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_quotes(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_quotes(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_nested_in_m2(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_nested_in_m1(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue nested_mentions(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_embedded(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_embedded(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_nmod(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_nmod(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static TFeatureValue predicative_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue predicative_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static TFeatureValue str_match_strict(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue str_match_relaxed(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue str_head_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue str_pron_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue relaxed_head_match_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue relaxed_head_match_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue name_match_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue name_match_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue num_match_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue num_match_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue word_inclusion_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue word_inclusion_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue compatible_mods_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue compatible_mods_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static TFeatureValue mention_1_subj_reporting(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_subj_reporting(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_obj_reporting(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_obj_reporting(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue obj_same_reporting(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue subj_obj_reporting_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue subj_obj_reporting_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue subj_obj_same_verb_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue subj_obj_same_verb_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static relaxcor_fex_dep::TFeatureValue acronym(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) ;
    static relaxcor_fex_dep::TFeatureValue same_semclass(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex) ;

    // auxiliar methods for feature computing
    static int dist_in_phrases(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool nested(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static int dist_sentences(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static int dist_mentions(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static bool predicative(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool relaxed_head_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool name_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static bool num_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool word_inclusion(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool compatible_mods(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool inclusion_match(const mention &m1, const mention &m2, const freeling::regexp &re);

    static bool subj_obj_reporting(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool subj_obj_same_verb(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool initial_match(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    /// single mention 
    static bool match_pronoun_features(const mention &m, const std::wstring &, const std::wstring &pgn,
                                       feature_cache &fcache, const relaxcor_fex_dep &fex);
    static wchar_t get_gender(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static wchar_t get_number(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static wchar_t get_person(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static std::wstring get_arguments(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TSemanticClass get_semantic_class(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static bool in_quotes(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool definite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool indefinite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool relative_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool reflexive_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool is_noun_modifier(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool possessive_determiner(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static std::set<int> subj_reporting(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static std::set<int> obj_reporting(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static std::set<int> is_subj_of(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static std::set<int> is_obj_of(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static std::set<int> inside_obj_of(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static std::set<int> is_arg_of(const mention &m, const freeling::regexp &re);
    static std::set<int> inside_arg_of(const mention &m, const freeling::regexp &re);

    // other
    static std::set<int> select_by_lemma(paragraph::const_iterator s, const std::set<int> &pos, const freeling::regexp &re);
    static std::list<std::pair<int,std::wstring>> get_arguments(paragraph::const_iterator s, int mpos);

    //////////////////////////////////////////////////////////////////
    ///    Auxiliary class for the feature extractor.
    ///    Used to store morphological information about words
    //////////////////////////////////////////////////////////////////
    
    class morph_features {
    public:
      morph_features(const std::wstring&);
      ~morph_features();
      
      bool has_type(const std::wstring&, const std::wstring&) const;
      wchar_t get_human(const std::wstring&) const;
      wchar_t get_person(const std::wstring&) const;
      wchar_t get_number(const std::wstring&) const;
      wchar_t get_gender(const std::wstring&) const;
      
      static relaxcor_fex_dep::TFeatureValue compatible_number(wchar_t, wchar_t);
      static relaxcor_fex_dep::TFeatureValue compatible_gender(wchar_t, wchar_t);
      
    private:
      std::map<std::wstring,std::wstring> _PronFeats;
      std::map<std::wstring,std::wstring> _PronTypes;
      
      wchar_t get_feature(const std::wstring &w, int k) const;      
    };
    
    // list of morphological features
    morph_features _Morf;

    // for easier tracing
    #define ff_string(x) (x==ff_YES ? L"yes" : \
                         (x==ff_NO ? L"no" : \
                          L"unk"))
    #define sc_string(x) (x==sc_PER ? L"person" : \
                         (x==sc_ORG ? L"organization" : \
                         (x==sc_LOC ? L"location" : \
                          L"unk")))
  };
  

} // namespace

#endif
