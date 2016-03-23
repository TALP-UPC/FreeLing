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

#ifndef COREF_H
#define COREF_H

#include <set>

#include "freeling/windll.h"
#include "freeling/omlet/adaboost.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/mention_detector.h"
#include "freeling/morfo/coref_fex.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  ///  The class coref implements a ML-based coreference classificator
  ////////////////////////////////////////////////////////////////

  class WINDLL coref : processor<document> {
  private:
    /// mention detector
    mention_detector *detector;
    /// feature extractor
    coref_fex *extractor;
    /// adaboost classifier
    adaboost *classifier;
    /// Max distance to search for a coreference node
    int MaxDistance;
  
    bool check_coref(const mention_ab &s, const mention_ab &) const;
    void set_mention(parse_tree::iterator, int &, mention_ab &) const;
    void add_candidates(int, int &, int &, parse_tree::iterator, std::list<mention_ab> &) const;
  
  public:
    /// Constructor
    coref(const std::wstring &);
    /// Destructor
    ~coref();
  
    /// Classify SN's in given sentence in groups of coreference
    void analyze(document &) const;

    using processor<document>::analyze;
  };

} // namespace

#endif
