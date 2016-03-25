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

#ifndef _LOCUTIONS
#define _LOCUTIONS

#include <map> 
#include <set> 

#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/automat.h"
#include "freeling/morfo/tagset.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Class to store status information
  ////////////////////////////////////////////////////////////////

  class WINDLL locutions_status : public automat_status {
  public:
    /// partially build multiword.
    std::set<std::wstring> acc_mw,longest_mw;
    /// store mw components in case we need to recover them
    std::vector<sentence::const_iterator> components;
    /// count words scanned beyond last longest mw found.
    int over_longest;
    /// analysis assigned to the mw by the validation step
    std::list<analysis> mw_analysis;
    /// segmentantion ambiguity status of the multiword
    bool mw_ambiguous;
  };

  ////////////////////////////////////////////////////////////////
  /// Class locutions recognizes multiwords belonging to 
  /// a list obtained from a  configuration file.
  ////////////////////////////////////////////////////////////////

  class WINDLL locutions: public automat<locutions_status> {
  private:
    /// store multiword list
    std::map<std::wstring,std::wstring> locut;
    /// store multiword prefixes
    std::set<std::wstring> prefixes;
    /// Tagset handling modul
    tagset *Tags;
    /// Whether to check all analysis for lemma and PoS conditions, or just selected ones.
    bool OnlySelected;

    bool check(const std::wstring &, std::set<std::wstring> &, bool &, bool &, locutions_status *) const;
    int ComputeToken(int, sentence::iterator &, sentence &) const;
    void ResetActions(locutions_status *) const;
    void StateActions(int, int, int, sentence::const_iterator, locutions_status *) const;
    void SetMultiwordAnalysis(sentence::iterator, int, const locutions_status *) const;
    bool ValidMultiWord(const word &, locutions_status *pst=NULL) const;

  public:
    /// Constructor
    locutions(const std::wstring &);
    ~locutions();
    void add_locution(const std::wstring &);
    void set_OnlySelected(bool);
  };

} // namespace

#endif

