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
//    Feature extractor for coreference resolution
//////////////////////////////////////////////////////////////////

#ifndef CORE_FEX_H
#define CORE_FEX_H

#include <list>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "freeling/morfo/language.h"
#include "freeling/morfo/semdb.h"

namespace freeling {

  // -- Feature group codes
#define COREFEX_DIST            0x00000001
#define COREFEX_IPRON           0x00000002
#define COREFEX_JPRON           0x00000004
#define COREFEX_IPRONM          0x00000008
#define COREFEX_JPRONM          0x00000010
#define COREFEX_STRMATCH        0x00000020
#define COREFEX_DEFNP           0x00000040
#define COREFEX_DEMNP           0x00000080
#define COREFEX_NUMBER          0x00000100
#define COREFEX_GENDER          0x00000200
#define COREFEX_SEMCLASS        0x00000400
#define COREFEX_PROPNAME        0x00000800
#define COREFEX_ALIAS           0x00001000
#define COREFEX_APPOS           0x00002000
#define COREFEX_QUOTES          0x00004000
#define COREFEX_THIRDP          0x00010000
  //
#define COREFEX_ALL             0xFFFFFFFF

  class mention_ab {
  public:
    int sent;
    int numde;
    parse_tree::iterator ptree;
    int posbegin;
    int posend;
    std::vector<std::wstring> tokens;
    std::vector<std::wstring> tags;
    int chain;
  };

  //////////////////////////////////////////////////////////////////
  ///    Class for the feature extractor.
  //////////////////////////////////////////////////////////////////

  class coref_fex {

  private:
    /// semantic database to check for semantic properties
    semanticDB *semdb;
    /// active features
    unsigned int active_features;

    /// auxiliary functions for feature extraction   
    wchar_t extract_number(const std::wstring &);
    wchar_t extract_gender(const std::wstring &);
    std::wstring extract_semclass(const std::wstring &, const std::wstring &);
    const word& get_head_word(parse_tree::iterator);
    bool check_tag(const mention_ab &, int, const std::wstring &);
    bool check_word(const std::wstring &, const std::wstring &);
    bool check_acronim(const mention_ab &, const mention_ab &);
    bool check_prefix(const mention_ab &, const mention_ab &);
    bool check_sufix(const mention_ab &, const mention_ab &);
    bool check_order(const mention_ab &, const mention_ab &);

    /// feature functions
    int get_dist(const mention_ab &, const mention_ab &); 
    int get_numdedist(const mention_ab &, const mention_ab &);
    int get_dedist(const mention_ab &, const mention_ab &);
    int get_i_pronoun(const mention_ab &, const mention_ab &);
    int get_j_pronoun(const mention_ab &, const mention_ab &);
    int get_i_pronoun_p(const mention_ab &, const mention_ab &);
    int get_j_pronoun_p(const mention_ab &, const mention_ab &);
    int get_i_pronoun_d(const mention_ab &, const mention_ab &);
    int get_j_pronoun_d(const mention_ab &, const mention_ab &);
    int get_i_pronoun_x(const mention_ab &, const mention_ab &);
    int get_j_pronoun_x(const mention_ab &, const mention_ab &);
    int get_i_pronoun_i(const mention_ab &, const mention_ab &);
    int get_j_pronoun_i(const mention_ab &, const mention_ab &);
    int get_i_pronoun_t(const mention_ab &, const mention_ab &);
    int get_j_pronoun_t(const mention_ab &, const mention_ab &);
    int get_i_pronoun_r(const mention_ab &, const mention_ab &);
    int get_j_pronoun_r(const mention_ab &, const mention_ab &);
    int get_i_pronoun_e(const mention_ab &, const mention_ab &);
    int get_j_pronoun_e(const mention_ab &, const mention_ab &);
    int get_str_match(const mention_ab &, const mention_ab &);
    int get_def_np(const mention_ab &, const mention_ab &);
    int get_dem_np(const mention_ab &, const mention_ab &);
    int get_number(const mention_ab &, const mention_ab &);
    int get_gender(const mention_ab &, const mention_ab &);
    int get_semclass(const mention_ab &, const mention_ab &);
    int get_proper_noun_i(const mention_ab &, const mention_ab &);
    int get_proper_noun_j(const mention_ab &, const mention_ab &);
    int get_alias_acro(const mention_ab &, const mention_ab &);
    int get_alias_prefix(const mention_ab &, const mention_ab &);
    int get_alias_sufix(const mention_ab &, const mention_ab &);
    int get_alias_order(const mention_ab &, const mention_ab &);
    int get_appositive(const mention_ab &, const mention_ab &);
    int get_i_inquotes(const mention_ab &, const mention_ab &);
    int get_j_inquotes(const mention_ab &, const mention_ab &);
    int get_i_inparenthesis(const mention_ab &, const mention_ab &);
    int get_j_inparenthesis(const mention_ab &, const mention_ab &);
    int get_i_thirdperson(const mention_ab &, const mention_ab &);
    int get_j_thirdperson(const mention_ab &, const mention_ab &);
  
    inline void put_feature(int, std::vector<int> &);

  public:
    coref_fex(unsigned int, const std::wstring&);
    ~coref_fex();

    void extract(const mention_ab &, const mention_ab &, std::vector<int> &);
  };

} // namespace

#endif
