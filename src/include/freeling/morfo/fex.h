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

#ifndef _FEX
#define _FEX

#include <vector>
#include <set>
#include <list>

#include "freeling/morfo/fex_rule.h"
#include "freeling/morfo/fex_lexicon.h"
#include "freeling/morfo/tagset.h"

#define ENCODE_NAME 0x01
#define ENCODE_INT  0x02

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  Class fex implements a feature extractor.
  ////////////////////////////////////////////////////////////////

  class fex {
  private:
    /// tagset desription used to compute short versions of PoS tags
    const tagset *Tags;

    /// rules used by the feature extractor
    std::list<fex_rulepack> packs;
    /// lexicon used to filter features.
    fex_lexicon lex;

    /// loaded set files (loaded once, may be used by several conditions in different rules)
    std::map<std::wstring, std::set<std::wstring> > set_files;
    /// currently loaded specific feature extraction functions
    const std::map<std::wstring,const feature_function *> & feat_functs;

    /// extract features from a sentence    
    void get_features(sentence &, std::vector<std::set<std::wstring> > &, std::vector<std::set<int> > &, int) const;
    /// read rule conditions 
    void read_condition(std::wistringstream &, const std::wstring &, const std::wstring &, std::list<fex_condition> &, int &);
    /// Apply all rules in the pack once to each word in the sentence
    void precompute_once(const fex_rulepack &, sentence &) const;
    /// Apply all rules in the pack only to proper range around
    /// words matching pack condition.
    void precompute_range(const fex_rulepack &, sentence &) const;

  public:
    /// constructor, given rule file, lexicon file (may be empty), and custom functions
    fex(const std::wstring&, 
        const std::wstring&, 
        const std::map<std::wstring, const feature_function *> &custom=std::map<std::wstring,const feature_function *>());
    /// destructor
    ~fex();

    /// encode given sentence in features as feature names. 
    void encode_name(sentence &, std::vector<std::set<std::wstring> > &) const;
    /// encode given sentence in features as integer feature codes
    void encode_int(sentence &, std::vector<std::set<int> > &) const;
    /// encode given sentence in features as integer feature codes and as features names
    void encode_all(sentence &, std::vector<std::set<std::wstring> > &, std::vector<std::set<int> > &) const;

    /// encode given sentence in features as feature names. Return result suitable for Java/perl APIs
    std::vector<std::set<std::wstring> > encode_name(sentence &) const;
    /// encode given sentence in features as integer feature codes. Return result apt for Java/perl APIs
    std::vector<std::set<int> > encode_int(sentence &) const;

    /// clear lexicon
    void clear_lexicon(); 
    /// encode sentence and add features to current lexicon
    void encode_to_lexicon(sentence &);
    /// save lexicon to a file, filtering features with low occurrence rate
    void save_lexicon(const std::wstring &, double) const;
  };

} // namespace

#endif
