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

#ifndef _SEMDB
#define _SEMDB

#include <string>
#include <list>

#include "freeling/windll.h"
#include "freeling/morfo/database.h"
#include "freeling/morfo/language.h"

namespace freeling {

  ////////////////////////////////////////////////////////////////
  /// Class sense_info stores several semantic info about a sense
  ////////////////////////////////////////////////////////////////

  class WINDLL sense_info {
  public:
    /// sense code
    std::wstring sense;
    /// hyperonyms
    std::list<std::wstring> parents;
    /// WN semantic file code
    std::wstring semfile;
    /// list of synonyms (words in the synset)
    std::list<std::wstring> words;
    /// list of EWN top ontology properties
    std::list<std::wstring> tonto;
    /// Link to SUMO concept
    std::wstring sumo;
    /// Link to CYC concept
    std::wstring cyc;

    /// constructor
    sense_info(const std::wstring &,const std::wstring &);
    /// useful for java API
    std::wstring get_parents_string() const;
  };

  ////////////////////////////////////////////////////////////////
  /// Auxiliary class to map FL postags to WN codes
  ////////////////////////////////////////////////////////////////

  class posmaprule {
  public:
    /// original FreeLing tag
    std::wstring pos;
    /// corresponding WN tag
    std::wstring wnpos;
    /// lemma to check in WN
    std::wstring lemma;
  };


  ////////////////////////////////////////////////////////////////
  /// Class semanticDB implements a semantic DB interface
  ////////////////////////////////////////////////////////////////

  class WINDLL semanticDB {
  private:
    /// map of PoS tags to WN lemma+postag
    std::list<posmaprule> posmap;
    /// map of (lema,pos) -> form
    database *form_dict;

    /// actual storage
    database *sensesdb;
    database *wndb;

  public:
    /// Constructor
    semanticDB(const std::wstring &); 
    /// Destructor
    ~semanticDB();
 
    /// Compute list of lemma-pos to search in WN for given word, according to mapping rules.
    void get_WN_keys(const std::wstring &, const std::wstring &, const std::wstring &, std::list<std::pair<std::wstring,std::wstring> > &) const;
    /// get list of words for a sense
    std::list<std::wstring> get_sense_words(const std::wstring &) const;
    /// get list of senses for a lemma+pos
    std::list<std::wstring> get_word_senses(const std::wstring &, const std::wstring &, const std::wstring &) const;
    /// get sense info for a sense
    sense_info get_sense_info(const std::wstring &) const;
  };

} // namespace

#endif

