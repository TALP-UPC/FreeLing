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
  /// Constructor. Sets defaults
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::relaxcor_fex_dep(const wstring &filename, const relaxcor_model &m) : relaxcor_fex_abs(m) {

    TRACE(2,L"Module successfully loaded");
  }

  //////////////////////////////////////////////////////////////////
  /// Destructor
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_dep::~relaxcor_fex_dep() { }


  void relaxcor_fex_dep::get_structural(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                        feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {}
  void relaxcor_fex_dep::get_lexical(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                     feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {}
  void relaxcor_fex_dep::get_morphological(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                           feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {}
  void relaxcor_fex_dep::get_syntactic(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                       feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {}
  void relaxcor_fex_dep::get_semantic(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                      feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {}
  void relaxcor_fex_dep::get_discourse(const mention &m1, const mention &m2, const std::vector<mention> &mentions, 
                                       feature_cache &fcache, relaxcor_model::Tfeatures &ft) const {}
  
  
  //////////////////////////////////////////////////////////////////
  ///    Extract the configured features for a pair of mentions 
  //////////////////////////////////////////////////////////////////

  void relaxcor_fex_dep::extract_pair(const mention &m1, const mention &m2, 
                                      const vector<mention> &mentions, 
                                      feature_cache &fcache,
                                      relaxcor_model::Tfeatures &ft) const {
    /// structural features
    get_structural(m1, m2, mentions, fcache, ft);
    get_lexical(m1, m2, mentions, fcache, ft);
    get_morphological(m1, m2, mentions, fcache, ft);
    get_syntactic(m1, m2, mentions, fcache, ft);
    get_semantic(m1, m2, mentions, fcache, ft);
    get_discourse(m1, m2, mentions, fcache, ft);
  }


  ////////////////////////////////////////////////////////////////// 
  ///    Extract the configured features for all mentions  
  //////////////////////////////////////////////////////////////////

  relaxcor_fex_abs::Mfeatures relaxcor_fex_dep::extract(const vector<mention> &mentions) const {
    relaxcor_fex_abs::Mfeatures M;

    feature_cache fcache;
    
    for (vector<mention>::const_iterator m1=mentions.begin()+1; m1!=mentions.end(); ++m1) {
      TRACE(4,L"Extracting all pairs for mention "+util::int2wstring(m1->get_id())+L" ("+m1->value()+L") " + m1->get_head().get_form() + L" " + m1->get_head().get_lemma() + L" " + m1->get_head().get_tag());
      
      for (vector<mention>::const_iterator m2=mentions.begin(); m2!=m1; ++m2) {	
	wstring mention_pair = util::int2wstring(m1->get_id());
	mention_pair += L":";
	mention_pair += util::int2wstring(m2->get_id());
        
	TRACE(5,L"PAIR: "+mention_pair+L" "+m1->get_head().get_form()+L":"+m2->get_head().get_form());
	
        extract_pair(*m1, *m2, mentions, fcache, M[mention_pair]);
      }
    }

    return M;
  }
  
} // namespace
