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
//   Author: Jordi Turmo (turmo@lsi.upc.edu)
//
///////////////////////////////////////////////


#ifndef MENTION_DETECTOR_CONSTIT_H
#define MENTION_DETECTOR_CONSTIT_H

#include <map>
#include <set>
#include <algorithm>

#include "freeling/windll.h"
#include "freeling/morfo/language.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class mention_detector implements a rule-based entity
  ///  mention detector
  ////////////////////////////////////////////////////////////////

  class WINDLL mention_detector_constit {

  private:
    typedef enum {NONE, REL_CLAUSE, ESSENTIAL, PRE_NON_ESSENTIAL, NON_ESSENTIAL} subordinateType;

    /// Configuration options
    std::map<std::wstring, freeling::regexp> _Labels;
    std::set<std::wstring> _No_heads;
    bool _Reduce_mentions;
    /// Language
    std::wstring _Language;

    /// Recursively finds all the mentions from a parse_tree
    /// NPs, coordinated NPs, pronouns, proper names and NEs
    void candidates(int, paragraph::const_iterator, int&, sentence::const_iterator &, int&, parse_tree::const_iterator, std::vector<mention>&, subordinateType&) const;

    /// Discard some mentions with the same head
    void discard_mentions(std::vector<mention>&) const;

    /// add a new mention in a vector
    void add_mention(const mention&, std::vector<mention>&, int&) const;
    /// Mark those mentions which are the initial ones in their sentence
    void mark_initial_mentions_from(int, std::vector<mention>&) const;

  public:
    /// Constructor
    mention_detector_constit(const std::wstring &);
    /// Destructor
    ~mention_detector_constit();
  
    /// get the labels of the configuration of the mention detector
    const std::map<std::wstring, freeling::regexp>& get_config_labels() const;

    /// Detects entity mentions from parse tree nodes of sentences, and fills given vector
    std::vector<mention> detect(const document &) const;
    
  };

} // namespace

#endif
