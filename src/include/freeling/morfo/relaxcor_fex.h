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

#ifndef RELAXCOR_FEX_H
#define RELAXCOR_FEX_H

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

  class relaxcor_fex {
  public:

    typedef std::map<std::wstring, relaxcor_model::Tfeatures > Mfeatures;

    relaxcor_fex(const std::wstring&, relaxcor_model *, const std::wstring &lang=L"");
    ~relaxcor_fex();

     /// Just for debugging!!!
    static void print(relaxcor_fex::Mfeatures&, unsigned int);

    void extract(std::vector<mention>&, Mfeatures &);

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

    typedef enum {IN_QUOTES, HEAD_TERM, IS_ACRONYM, POSSESSIVE, NUMBER, GENDER, SEM_CLASS, THIRD_PERSON, REFLEXIVE, DEF_NP, INDEF_NP, DEM_NP, MAXIMAL_NP, EMBEDDED_NOUN} mentionFeature;
    typedef enum {ARGUMENTS, ROLES} mentionWsFeature;

    #define ID(x) model->feature_name_id(x)
    #define VERY_BIG 100000

    /// FROM PARAMETERS
    /// Language
    std::wstring _Language;
    /// coreference model including feature model
    relaxcor_model *model;
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

    /// auxiliar maps of some feature values for individual mentions
    std::map<int, std::map<mentionFeature, unsigned int> > features;
    std::map<int, std::map<mentionWsFeature, std::vector<std::wstring> >> wsfeatures;

    /// auxiliary functions for feature extraction   
    void set_feature(int, mentionFeature, unsigned int);
    void set_feature(int, mentionWsFeature, const std::vector<std::wstring>&);
    void clean_features();
    unsigned int get_feature(int, mentionFeature) const;
    const std::vector<std::wstring>& get_feature(int, mentionWsFeature) const;
    bool computed_feature(int, mentionFeature) const;
    bool computed_feature(int, mentionWsFeature) const;
    std::wstring subvector2wstring(const std::vector<std::wstring>&, unsigned int, unsigned int, const std::wstring&);

    /// group feature functions
    void get_structural(const mention&, const mention&, relaxcor_model::Tfeatures&);
    void get_lexical(const mention&, const mention&, relaxcor_model::Tfeatures&);
    void get_morphological(const mention &, const mention&, relaxcor_model::Tfeatures&, std::vector<mention>&);
    void get_syntactic(const mention &, const mention&, relaxcor_model::Tfeatures&, std::vector<mention>&);
    void get_semantic(const mention &, const mention&, relaxcor_model::Tfeatures&, std::vector<mention>&);
    void get_discourse(const mention &, const mention&, relaxcor_model::Tfeatures&);

    void get_group_features(std::vector<mention>&, relaxcor_model::Tfeatures&);

    /// feature functions
    unsigned int dist_in_phrases(const mention&, const mention&); 
    unsigned int in_quotes(const mention&);
    bool appositive(const mention&, const mention&);
    bool nested(const mention&, const mention&);
    bool intersected(const mention&, const mention&);
    bool string_match(const mention&, const mention&);
    bool pronoun_string_match(const mention&, const mention&, bool);
    bool proper_noun_string_match(const mention&, const mention&, bool);
    bool no_pronoun_string_match(const mention&, const mention&, bool);
    unsigned int head_is_term(const mention&);
    unsigned int alias(const mention&, const mention&);
    unsigned int is_possessive(const mention&);
    unsigned int same_number(const mention&, const mention&);
    unsigned int same_gender(const mention&, const mention&);
    unsigned int is_3rd_person(const mention&);
    unsigned int agreement(const mention&, const mention&);
    unsigned int closest_agreement(const mention&, const mention&, std::vector<mention>&);
    unsigned int is_reflexive(const mention&);
    // syntactic
    unsigned int is_def_NP(const mention&);
    unsigned int is_dem_NP(const mention&);
    bool share_maximal_NP(const mention&, const mention&, std::vector<mention>&);
    unsigned int is_maximal_NP(const mention&, std::vector<mention>&);
    unsigned int is_indef_NP(const mention&);
    unsigned int is_embedded_noun(const mention&, std::vector<mention>&);
    bool binding_pos(const mention&, const mention&, bool);
    bool binding_neg(const mention&, const mention&, bool);
    void get_arguments(const mention&, std::wstring&, std::wstring&);
    bool same_preds(bool, const std::wstring&, const std::wstring&);
    bool same_args(bool, const std::wstring&, const std::wstring&, relaxcor_model::Tfeatures&);
    // semantic
    bool separated_by_verb_is(const mention&, const mention&, std::vector<mention>&);
    bool sem_class_match(const mention&, const mention&);
    bool is_semantic_type(const mention&, const std::wstring&);
    bool animacy(const mention&, const mention&);
    bool incompatible(const mention&, const mention&);
    void get_roles(const mention&, std::vector<std::wstring>& );
    bool same_roles(const std::vector<std::wstring>&, const std::vector<std::wstring>&);
    /// auxiliar functions
    void read_countries_capitals(const std::wstring&);
    void read_gpe_regexps(const std::wstring&);
    void read_pairs(const std::wstring&, std::map<std::wstring, std::wstring>&);
    void read_same_names(const std::wstring&, std::map<std::wstring, std::vector<unsigned int> >&);
    std::wstring drop_det(const mention&);
    std::wstring compute_term(const mention&);
    unsigned int geo_match(const mention&, const mention&);
    std::wstring string_merge(const mention&, bool);
    std::vector<std::wstring> split_words(const std::wstring&);
    bool is_acronym(const std::wstring&);
    unsigned int acronym_of(const std::vector<std::wstring>&, const std::vector<std::wstring>&);
    unsigned int initials_match(const std::vector<std::wstring>&, const std::vector<std::wstring>&);
    double lex_dist(const std::wstring&, const std::wstring&);
    unsigned int nick_name_match(const std::wstring&, const std::wstring&);
    unsigned int forenames_match(const std::vector<std::wstring>&, const std::vector<std::wstring>&);
    unsigned int first_name_match(const std::wstring&, const std::vector<std::wstring>&);
    double levenshtein(const std::wstring&, const std::wstring&);
    unsigned int get_number(const mention&);
    unsigned int get_gender(const mention&);
    std::wstring extract_msd_feature(const std::wstring &tag, const std::wstring &feature) const;
      //std::wstring extract_number(const std::wstring&);
      //std::wstring extract_gender(const std::wstring&);
      //std::wstring extract_person(const std::wstring&);
    mention::SEMmentionType extract_semclass(const mention&);
    void isa(const std::wstring&, std::vector<bool>&);
    int get_maximal_NP(const mention&, std::vector<mention>&);
    const std::wstring& get_argument(sentence::predicates::const_iterator, dep_tree::const_iterator, paragraph::const_iterator);
    bool verb_is_between(const mention&, const mention&);

    void extract_pair(mention &, mention &, relaxcor_model::Tfeatures &, std::vector<mention>&);
    
  };

} // namespace

#endif
