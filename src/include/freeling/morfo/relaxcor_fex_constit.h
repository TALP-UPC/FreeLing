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

#ifndef RELAXCOR_FEX_CONSTIT_H
#define RELAXCOR_FEX_CONSTIT_H

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
#include "freeling/morfo/relaxcor_model.h"
#include "freeling/morfo/relaxcor_fex_abs.h"

namespace freeling {

  // -- Feature group codes    
#define RCF_SET_STRUCTURAL      0x00000001
#define RCF_SET_LEXICAL         0x00000002
#define RCF_SET_MORPHO          0x00000004
#define RCF_SET_SYNTACTIC       0x00000008
#define RCF_SET_SEMANTIC        0x00000010
#define RCF_SET_DISCOURSE       0x00000020
  //                                                                          
#define RCF_SET_ALL             0xFFFFFFFF


  //////////////////////////////////////////////////////////////////
  ///    Class for the feature extractor.
  //////////////////////////////////////////////////////////////////

  class relaxcor_fex_constit : public relaxcor_fex_abs {
  public:

    relaxcor_fex_constit(const std::wstring&, const relaxcor_model &);
    ~relaxcor_fex_constit();

    relaxcor_fex_abs::Mfeatures extract(const std::vector<mention>&) const;

  private:

    static const freeling::regexp acronym_re1;
    static const freeling::regexp acronym_re2;
    static const freeling::regexp en_reflexive_re;
    static const freeling::regexp en_demostrative_re;
    static const freeling::regexp en_indefinite_re;
    static const freeling::regexp initial_letter_re1;
    static const freeling::regexp initial_letter_re2;
    static const freeling::regexp en_det_singular_re;
    static const freeling::regexp en_det_plural_re;
    static const freeling::regexp cat_verb_be_re1;
    static const freeling::regexp cat_verb_be_re2;
    static const freeling::regexp en_verb_be_re; 
    static const freeling::regexp es_verb_be_re; 
    static const freeling::regexp arg_re; 
    static const freeling::regexp role_re; 

    #define VERY_BIG 100000

    /// FROM PARAMETERS
    /// Language
    std::wstring _Language;
    /// semantic database to check for semantic properties
    semanticDB *_Semdb;
    /// active features
    unsigned int _Active_features;
    /// Tagset
    tagset *_POS_tagset;
    /// Regexp for syntactic labels useful to compute some features
    std::map<std::wstring, freeling::regexp> _Labels;
    /// determinants to be dropped out from mentions to compute string_matching
    std::set<std::wstring> _Det_words;
    /// words being pronouns and their features
    std::map<std::wstring, std::map<std::wstring, std::wstring> > _Prons_feat;
    /// semantic classes of common nouns
    std::map<std::wstring,std::pair<std::wstring, freeling::regexp> > _Sem_classes;
    /// capitals, countries and nationalitites (with GPE regexps also)
    std::map<std::wstring,std::wstring> _Capitals;
    std::map<std::wstring,std::wstring> _Nationalities;
    std::multimap<std::wstring,std::wstring> _Countries;
    std::vector<freeling::regexp> _GPE_regexps;
    /// forename aliases
    std::map<std::wstring, std::vector<unsigned int> > _Forenames;
    /// nick names
    std::map<std::wstring, std::vector<unsigned int> > _Nicks;
    /// person names with gender
    std::map<std::wstring, std::wstring> _Person_Names;
    /// titles with gender
    std::map<std::wstring, std::wstring> _Titles;
    /// acronym terms (infix and suffix)
    std::map<std::wstring, freeling::regexp> _AcroTerms;

    std::wstring subvector2wstring(const std::vector<std::wstring>&, unsigned int, unsigned int, const std::wstring&) const;

    /// group feature functions
    void get_structural(const mention&, const mention&, relaxcor_model::Tfeatures&, feature_cache &) const;
    void get_lexical(const mention&, const mention&, relaxcor_model::Tfeatures&, feature_cache &) const;
    void get_morphological(const mention &, const mention&, relaxcor_model::Tfeatures&, const std::vector<mention>&, feature_cache &) const;
    void get_syntactic(const mention &, const mention&, relaxcor_model::Tfeatures&, const std::vector<mention>&, feature_cache &) const;
    void get_semantic(const mention &, const mention&, relaxcor_model::Tfeatures&, const std::vector<mention>&, feature_cache &) const;
    void get_discourse(const mention &, const mention&, relaxcor_model::Tfeatures&, feature_cache &) const;

    void get_group_features(const std::vector<mention>&, relaxcor_model::Tfeatures&, feature_cache &) const;

    /// feature functions
    unsigned int dist_in_phrases(const mention&, const mention&, feature_cache &) const; 
    unsigned int in_quotes(const mention&, feature_cache &) const;
    bool appositive(const mention&, const mention&, feature_cache &) const;
    bool nested(const mention&, const mention&) const;
    bool intersected(const mention&, const mention&) const;
    bool string_match(const mention&, const mention&, feature_cache &) const;
    bool pronoun_string_match(const mention&, const mention&, bool, feature_cache &) const;
    bool proper_noun_string_match(const mention&, const mention&, bool, feature_cache &) const;
    bool no_pronoun_string_match(const mention&, const mention&, bool, feature_cache &) const;
    unsigned int head_is_term(const mention&, feature_cache &) const;
    unsigned int alias(const mention&, const mention&, feature_cache &) const;
    unsigned int is_possessive(const mention&, feature_cache &) const;
    unsigned int same_number(const mention&, const mention&, feature_cache &) const;
    unsigned int same_gender(const mention&, const mention&, feature_cache &) const;
    unsigned int is_3rd_person(const mention&, feature_cache &) const;
    unsigned int agreement(const mention&, const mention&, feature_cache &) const;
    unsigned int closest_agreement(const mention&, const mention&, const std::vector<mention>&, feature_cache &) const;
    unsigned int is_reflexive(const mention&, feature_cache &) const;
    // syntactic
    unsigned int is_def_NP(const mention&, feature_cache &) const;
    unsigned int is_dem_NP(const mention&, feature_cache &) const;
    bool share_maximal_NP(const mention&, const mention&, const std::vector<mention>&, feature_cache &) const;
    unsigned int is_maximal_NP(const mention&, const std::vector<mention>&, feature_cache &) const;
    unsigned int is_indef_NP(const mention&, feature_cache &) const;
    unsigned int is_embedded_noun(const mention&, const std::vector<mention>&, feature_cache &) const;
    bool binding_pos(const mention&, const mention&, bool, feature_cache &) const;
    bool binding_neg(const mention&, const mention&, bool, feature_cache &) const;
    void get_arguments(const mention&, std::wstring&, std::wstring&, feature_cache &) const;
    bool same_preds(bool, const std::wstring&, const std::wstring&, feature_cache &) const;
    bool same_args(bool, const std::wstring&, const std::wstring&, relaxcor_model::Tfeatures&, feature_cache &) const;
    // semantic
    bool separated_by_verb_is(const mention&, const mention&, const std::vector<mention>&, feature_cache &) const;
    bool sem_class_match(const mention&, const mention&, feature_cache &) const;
    bool is_semantic_type(const mention&, const std::wstring&, feature_cache &) const;
    bool animacy(const mention&, const mention&, feature_cache &) const;
    bool incompatible(const mention&, const mention&, feature_cache &) const;
    void get_roles(const mention&, std::vector<std::wstring>& , feature_cache &) const;
    bool same_roles(const std::vector<std::wstring>&, const std::vector<std::wstring>&, feature_cache &) const;

    /// auxiliar functions
    void read_countries_capitals(const std::wstring&);
    void read_gpe_regexps(const std::wstring&);
    void read_pairs(const std::wstring&, std::map<std::wstring, std::wstring>&);
    void read_same_names(const std::wstring&, std::map<std::wstring, std::vector<unsigned int> >&);

    std::wstring drop_det(const mention&) const;
    std::wstring compute_term(const mention&) const;
    unsigned int geo_match(const mention&, const mention&) const;
    std::wstring string_merge(const mention&, bool) const;
    std::vector<std::wstring> split_words(const std::wstring&) const;
    bool is_acronym(const std::wstring&) const;
    unsigned int acronym_of(const std::vector<std::wstring>&, const std::vector<std::wstring>&) const;
    unsigned int initials_match(const std::vector<std::wstring>&, const std::vector<std::wstring>&) const;
    double lex_dist(const std::wstring&, const std::wstring&) const;
    unsigned int nick_name_match(const std::wstring&, const std::wstring&) const;
    unsigned int forenames_match(const std::vector<std::wstring>&, const std::vector<std::wstring>&) const;
    unsigned int first_name_match(const std::wstring&, const std::vector<std::wstring>&) const;
    double levenshtein(const std::wstring&, const std::wstring&) const;
    unsigned int get_number(const mention&) const;
    unsigned int get_gender(const mention&, feature_cache&) const;
    std::wstring extract_msd_feature(const std::wstring &tag, const std::wstring &feature) const;
      //std::wstring extract_number(const std::wstring&);
      //std::wstring extract_gender(const std::wstring&);
      //std::wstring extract_person(const std::wstring&);
    mention::SEMmentionType extract_semclass(const mention&, feature_cache &) const;
    void isa(const std::wstring&, std::vector<bool>&) const;
    int get_maximal_NP(const mention&, const std::vector<mention>&, feature_cache &) const;
    const std::wstring& get_argument(sentence::predicates::const_iterator, dep_tree::const_iterator, paragraph::const_iterator) const;
    bool verb_is_between(const mention&, const mention&) const;

    void extract_pair(const mention &, const mention &, relaxcor_model::Tfeatures &, const std::vector<mention>&, feature_cache &) const;
    
  };

} // namespace

#endif
