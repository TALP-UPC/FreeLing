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
    
    //////////////////////////////////////////////////////////////////
    ///    Auxiliary class for the feature extractor.
    ///    Used to store morphological information about words
    //////////////////////////////////////////////////////////////////
    
    class morph_features {
    public:
      morph_features(const std::wstring&);
      ~morph_features();
      
      wchar_t get_type(const std::wstring&) const;
      wchar_t get_person(const std::wstring&) const;
      wchar_t get_number(const std::wstring&) const;
      wchar_t get_gender(const std::wstring&) const;
      
      bool same_type(const std::wstring&, const std::wstring&) const;
      bool same_person(const std::wstring&, const std::wstring&) const;
      bool same_number(const std::wstring&, const std::wstring&) const;
      bool same_gender(const std::wstring&, const std::wstring&) const;
      
      static std::wstring compatible_number(wchar_t, wchar_t);
      static std::wstring compatible_gender(wchar_t, wchar_t);
      
    private:
      std::map<std::wstring,std::wstring> _Words;
      
      wchar_t get_feature(const std::wstring &w, int k) const;      
    };
    
    //////////// Members

    // list of morphological features
    morph_features _Morf;

    /// regexps from config file
    std::map<std::wstring, freeling::regexp> _Labels;
    freeling::regexp reEMPTY;
    freeling::regexp get_label_RE(const std::wstring &) const;

    // map of feature names to feature functions
    typedef enum {ff_YES,ff_NO,ff_UNK} TFeatureValue;
    typedef TFeatureValue (*TFeatureFunction)(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    std::map<std::wstring, std::pair<TFeatureFunction,TFeatureValue>> _FeatureFunction;
    void register_features();


    void extract_pair(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;
    /// feature groups
    int get_structural(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;
    int get_lexical(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;
    int get_morphological(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;
    int get_syntactic(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;
    int get_semantic(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;
    int get_discourse(const mention &m1, const mention &m2, feature_cache &fcache, relaxcor_model::Tfeatures &ft) const;

    /// mention pair features
    static TFeatureValue dist_sentences_0(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue dist_sentences_1(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue dist_sentences_le3(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
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
    static TFeatureValue mention_1_I(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_I(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_you(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_you(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_we(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_we(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_singular(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_singular(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_3pers(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_3pers(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_quotes(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_quotes(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_1_nested_in_m2(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue mention_2_nested_in_m1(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue nested_mentions(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

    static TFeatureValue predicative_ij(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static TFeatureValue predicative_ji(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);


    // auxiliar methods for feature computing
    static bool nested(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static int dist_sentences(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool match_pronoun_features(const mention &m, wchar_t type, wchar_t per, wchar_t num, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static std::wstring get_arguments(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static wchar_t get_gender(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static wchar_t get_number(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static wchar_t get_person(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static unsigned int dist_in_phrases(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);
    /// single mention , const relaxcor_fex_dep&);
    static bool in_quotes(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool definite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool indefinite(const mention &m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool relative_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool reflexive_pronoun(const mention& m, feature_cache &fcache, const relaxcor_fex_dep &fex);
    static bool predicative(const mention &m1, const mention &m2, feature_cache &fcache, const relaxcor_fex_dep &fex);

  };


} // namespace

#endif
