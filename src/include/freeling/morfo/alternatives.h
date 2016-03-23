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

#ifndef _ALTERNATIVES_H
#define _ALTERNATIVES_H

#include <map>

#include "freeling/windll.h"
#include "freeling/regexp.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/dictionary.h"
#include "freeling/morfo/processor.h"
#include "freeling/morfo/phonetics.h"
#include "freeling/morfo/foma_FSM.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Class alternatives suggests words that are 
  /// orthogrphically/phonetically similar to input word.
  /// Results may be used for spell checking.
  ////////////////////////////////////////////////////////////////

  class WINDLL alternatives : public processor {
  private:
    /// FSM for orthographic/phonetic edit distance
    foma_FSM *sed;
    /// FSM for orthographic/phonetic compound analysis
    foma_FSM *comp;

    /// remember from which word(s) every phonetic form came from
    /// (only for phonetic distances)
    std::multimap<std::wstring,std::wstring> orthography;

    /// The class that translates a word into phonetic sounds
    phonetics* ph;

    /// Maximum distance to consider an entry as an alternative 
    int DistanceThreshold;

    /// Maximum lentgh difference to consider a word as a possible correction
    int MaxSizeDiff;

    /// tags of known word to be be checked 
    freeling::regexp CheckKnownTags;
    ///  whether unknown words should be checked
    bool CheckUnknown;

    /// type of distance used 
    static const int ORTHOGRAPHIC = 1;
    static const int PHONETIC = 2;
    int DistanceType;

    /// filter given candidate and decide if it is a valid alternative.
    void filter_candidate(const std::wstring &, const std::wstring &, int distance, std::map<std::wstring,int> &) const;
    /// adds the new words that are posible correct spellings from original word to the word analysys data
    void filter_alternatives(const std::list<std::pair<std::wstring,int> >&, word &) const;

    /// retrieve all possible word sequence that match (one-to-one) given sound sequence
    std::list<std::wstring> recover_words(std::list<std::wstring> wds) const;
    
  public:
    /// Constructor
    alternatives(const std::wstring &);
    /// Destructor
    ~alternatives();

    /// direct access to results of underlying automata
    void get_similar_words(const std::wstring &, std::list<std::pair<std::wstring,int> > &) const;
    
    /// spell check each word in sentence
    void analyze(sentence &) const;

    /// inherit other 'analyze' methods
    using processor::analyze;
  };

} // namespace

#endif
