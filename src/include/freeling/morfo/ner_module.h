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

#ifndef _NER_MODULE
#define _NER_MODULE

#include <set>
#include "freeling/windll.h"
#include "freeling/morfo/language.h"
#include "freeling/morfo/automat.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Class to store status information
  ////////////////////////////////////////////////////////////////

  class ner_status : public automat_status {
  public:
    /// it is a noun at the beggining of the sentence (status for class np)
    bool initialNoun;
  };

  ////////////////////////////////////////////////////////////////
  ///  The class ner is an abstract class that implements a general NE Recognizer
  ////////////////////////////////////////////////////////////////

  class WINDLL ner_module : public automat<ner_status> {
  
  protected: 
    /// length beyond which a multiword made of all capitialized 
    /// words ("WRECKAGE: TITANIC DISAPPEARS IN NORTHERN SEA") will 
    /// be considered a title and not a proper noun.
    /// A value of zero deactivates this behaviour.
    unsigned int Title_length;
    // the same for title-formatted multiwords, e.g. "Strangers In Paradise")
    unsigned int AllCaps_Title_length;

    /// Tag to assign to detected NEs
    std::wstring NE_tag;
    
    /// if we want to split NEs, set this to true
    bool splitNPs;

  public:
    // constructor
    ner_module(const std::wstring &);
    // destructor
    virtual ~ner_module() {};

    /// Allow classes under ner to be incomplete automata
    virtual int ComputeToken(int, sentence::iterator &, sentence &) const;
    virtual void ResetActions(ner_status *) const;
    virtual void StateActions(int, int, int, sentence::const_iterator,ner_status *) const;

    virtual void SetMultiwordAnalysis(sentence::iterator, int, const ner_status *) const;

    ///  Perform last minute validation before effectively building multiword
    bool ValidMultiWord(const word &,ner_status *) const;
    /// Build a Multiword and sets its analysis
    sentence::iterator BuildMultiword(sentence &, sentence::iterator, sentence::iterator, int, bool&, ner_status *) const;

  };

} // namespace

#endif

