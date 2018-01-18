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


#ifndef RELAXCOR_FEX_ABS_H
#define RELAXCOR_FEX_ABS_H

#include <string>
#include <vector>

#include "freeling/morfo/language.h"
#include "freeling/morfo/relaxcor_model.h"

namespace freeling {

  //////////////////////////////////////////////////////////////////
  /// Auxiliary Class to store already computed features that may be needed again

  class feature_cache {
  public:
    typedef enum {IN_QUOTES, HEAD_TERM, IS_ACRONYM, POSSESSIVE, NUMBER, GENDER, SEM_CLASS, THIRD_PERSON, REFLEXIVE, DEF_NP, INDEF_NP, DEM_NP, MAXIMAL_NP, EMBEDDED_NOUN} mentionFeature;
    typedef enum {ARGUMENTS, ROLES} mentionWsFeature;

    feature_cache();
    ~feature_cache();

    /// auxiliary functions for feature extraction for relaxcor_fex_constit
    void set_feature(int, mentionFeature, unsigned int);
    void set_feature(int, mentionWsFeature, const std::vector<std::wstring>&);
    unsigned int get_feature(int, mentionFeature) const;
    const std::vector<std::wstring>& get_feature(int, mentionWsFeature) const;
    bool computed_feature(int, mentionFeature) const;
    bool computed_feature(int, mentionWsFeature) const;

    /// auxiliary functions for feature extraction for relaxcor_fex_dep
    void set_feature(const std::wstring&, const std::wstring&);
    void set_feature(const std::wstring&, int);
    void set_feature(const std::wstring&, bool);
    bool get_str_feature(const std::wstring&, std::wstring &val) const;
    bool get_int_feature(const std::wstring&, int &val) const;
    bool get_bool_feature(const std::wstring&, bool &val) const;

  private:
    /// auxiliar maps of some feature values for individual mentions
    std::map<int, std::map<mentionFeature, unsigned int>> i_features;
    std::map<int, std::map<mentionWsFeature, std::vector<std::wstring> >> vs_features;
    std::map<std::wstring, std::wstring> str_features;
    std::map<std::wstring, int> int_features;
    std::map<std::wstring, bool> bool_features;
  };


  //////////////////////////////////////////////////////////////////
  /// Class relaxcor_fex_abs is an abstract class for a relaxcor
  ///  feature extractor
  //////////////////////////////////////////////////////////////////

  class relaxcor_fex_abs {
  public:
    typedef std::map<std::wstring, relaxcor_model::Tfeatures > Mfeatures;
    relaxcor_fex_abs(const relaxcor_model &m);
    virtual ~relaxcor_fex_abs();

  protected:
    const relaxcor_model &model;
    unsigned int fid(const std::wstring &) const;
    bool def(const std::wstring &) const;
    bool defid(const std::wstring &, unsigned int &) const;

  private: 
    /// extract features for all mention pairs in given vector.
    virtual Mfeatures extract(const std::vector<mention>&) const = 0;
    /// dump extracted features for debugging
    virtual void print(Mfeatures&, unsigned int) const;
  };


}

#endif
