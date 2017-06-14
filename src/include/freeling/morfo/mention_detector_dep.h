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
//
//   Author: Lluis Padro
//
///////////////////////////////////////////////


#ifndef MENTION_DETECTOR_DEP_H
#define MENTION_DETECTOR_DEP_H

#include <map>
#include <set>
#include <algorithm>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/tagset.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class mention_detector implements a rule-based entity
  ///  mention detector
  ////////////////////////////////////////////////////////////////

  class WINDLL mention_detector_dep {

  private:
    std::map<std::wstring, std::pair<std::wstring, mention::mentionType>> mention_tags;
    std::set<std::wstring> excluded;
    std::wstring CoordLabel;
    tagset *Tags;

    bool is_coordination(dep_tree::const_iterator h) const;
    void detect_mentions(freeling::dep_tree::const_iterator h, 
                         freeling::paragraph::const_iterator se, 
                         int sentn, bool maximal,
                         std::vector<mention> & mentions, int &mentn) const;
    bool check_mention_tags(freeling::dep_tree::const_iterator h, mention::mentionType &t) const;

  public:
    /// Constructor
    mention_detector_dep(const std::wstring &);
    /// Destructor
    ~mention_detector_dep();

    /// Detects entity mentions from parse tree nodes of sentences
    std::vector<mention> detect(const document &) const;
  };

} // namespace

#endif
