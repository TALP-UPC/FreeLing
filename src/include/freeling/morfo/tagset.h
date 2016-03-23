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

#ifndef TAGSET_H
#define TAGSET_H

#include <string>
#include <map>
#include <set>
#include <list>

#include "freeling/windll.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// The class tagset handles PoS tags long to short conversion
  /// and morphosintactic feature decomposition 
  ////////////////////////////////////////////////////////////////

  class WINDLL tagset {

  private:
    // separators for MSD string
    const std::wstring PAIR_SEP;
    const std::wstring MSD_SEP;
    /// maps to store PoS tags translation rules and features.
    std::map<std::wstring,std::wstring> feat, val, name, name_inv, val_inv;
    /// short tag+mfs for direct entry rules
    std::map<std::wstring,std::pair<std::wstring,std::wstring> > direct;
    std::map<std::set<std::wstring>,std::wstring> direct_inv;
    /// size for short version of the tag
    std::map<std::wstring,std::list<int> > shtag_size;

    /// decompose a tag in morphological features
    std::list<std::pair<std::wstring,std::wstring> > compute_msd_features(const std::wstring &tag) const;
 
  public:
    /// constructor: load a map file
    tagset(const std::wstring &f);
    /// destructor
    ~tagset();

    /// get short version of given tag
    std::wstring get_short_tag(const std::wstring &tag) const;
    /// get map of <feature,value> pairs with morphological information
    std::map<std::wstring,std::wstring> get_msd_features_map(const std::wstring &tag) const;
    /// get list of <feature,value> pairs with morphological information
    std::list<std::pair<std::wstring,std::wstring> > get_msd_features(const std::wstring &tag) const;
    /// get list <feature,value> pairs with morphological information, in a string format
    std::wstring get_msd_string(const std::wstring &tag) const;
    /// get EAGLES tag from morphological info given as list <feature,value> pairs 
    std::wstring msd_to_tag(const std::wstring &cat,const std::list<std::pair<std::wstring,std::wstring> > &msd) const;
    /// get EAGLES tag from morphological info given as string 
    std::wstring msd_to_tag(const std::wstring &cat,const std::wstring &msd) const;
  };

} // namespace

#endif
